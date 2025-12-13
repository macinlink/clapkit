/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKLabel
 * ----------------------------------------------------------------------
 * Defines a static label.
 *
 */

#include "ckCanvas.h"
#include "ckWindow.h"
#include <Quickdraw.h>
#include <Icons.h>
#include <Resources.h>
#include <Memory.h>

static bool __CKPlotBWIcon(short resourceId, const Rect& destRect, const WindowPtr window) {

	Handle iconHandle = GetResource('ICN#', resourceId);
	bool hasMask = true;
	if (!iconHandle) {
		iconHandle = GetResource('ICON', resourceId);
		hasMask = false;
	}

	if (!iconHandle) {
		CKLog("Resource %d not found.", resourceId);
		return false;
	}

	HLock(iconHandle);
	const UInt8* data = (const UInt8*)*iconHandle;

	const UInt8* maskData = hasMask ? data : nullptr;
	const UInt8* iconData = hasMask ? data + 128 : data;

	BitMap srcBM;
	srcBM.baseAddr = (Ptr)iconData;
	srcBM.rowBytes = 4;
	SetRect(&srcBM.bounds, 0, 0, 32, 32);

	const BitMap* dstBits = &(window->portBits);

	Rect srcRect = srcBM.bounds;
	Rect dstRectCopy = destRect;

	if (hasMask) {
		BitMap maskBM;
		maskBM.baseAddr = (Ptr)maskData;
		maskBM.rowBytes = 4;
		SetRect(&maskBM.bounds, 0, 0, 32, 32);
		Rect maskRect = maskBM.bounds;
		CopyMask(&srcBM, &maskBM, dstBits, &srcRect, &maskRect, &dstRectCopy);
	} else {
		CopyBits(&srcBM, dstBits, &srcRect, &dstRectCopy, srcCopy, NULL);
	}

	HUnlock(iconHandle);
	ReleaseResource(iconHandle);
	return true;
}

CKCanvas::CKCanvas(const CKControlInitParams& params)
	: CKControl(params, CKControlType::Canvas) {

	CKPROFILE

	this->__gworldptr = NULL;

	if (!CKHasColorQuickDraw()) {
		CKLog("Color QuickDraw not available; offscreen drawing disabled (drawing directly to window).");
		return;
	}

	Rect bounds = {0, 0, (short)params.height, (short)params.width};
	OSErr err = NewGWorld(&(this->__gworldptr), 16, &bounds, NULL, NULL, 0);
	if (err != noErr) {
		throw CKNew CKException("Can't create GWorld for canvas.");
	}

	this->Clear();
}

CKCanvas::~CKCanvas() {

	CKPROFILE

	if (this->__gworldptr) {
		DisposeGWorld(this->__gworldptr);
		this->__gworldptr = NULL;
	}
}

void CKCanvas::Redraw() {

	CKPROFILE

	if (this->owner == nil) {
		return;
	}

	// No offscreen buffer – draw directly into the window (mono fallback).
	if (!this->__gworldptr) {
		if (!this->__hasQueuedIcon) {
			return;
		}

		GrafPtr oldPort;
		GetPort(&oldPort);
		SetPort(this->owner->GetWindowPtr());

		short top = this->rect->origin->y + this->__queuedIconWhere.y;
		short left = this->rect->origin->x + this->__queuedIconWhere.x;
		Rect destRect = {(short)top, (short)left, (short)(top + 32), (short)(left + 32)};

		const bool hasIconUtils = CKHasIconUtilities();
		const bool hasColorQD = CKHasColorQuickDraw();

		if (!hasColorQD || !hasIconUtils) {
			if (!__CKPlotBWIcon(this->__queuedIconResourceId, destRect, this->owner->GetWindowPtr())) {
				this->__hasQueuedIcon = false;
			}
		} else {

			CIconHandle cIconHandle = GetCIcon(this->__queuedIconResourceId);
			if (!cIconHandle) {
				Handle iconHandle = GetIcon(this->__queuedIconResourceId);
				if (!iconHandle) {
					CKLog("Resource %d not found.", this->__queuedIconResourceId);
					this->__hasQueuedIcon = false;
					SetPort(oldPort);
					return;
				}
				PlotIcon(&destRect, iconHandle);
				ReleaseResource(iconHandle);
			} else {
				PlotCIcon(&destRect, cIconHandle);
				DisposeCIcon(cIconHandle);
			}
		}

		SetPort(oldPort);
		return;
	}

#ifdef CKDEBUGTIMEDRAWS
	long t_start = TickCount();
#endif

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->owner->GetWindowPtr());

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		SetPort(oldPort);
		return;
	}

	Rect destRect = {(short)this->rect->origin->x, (short)this->rect->origin->y, (short)(this->rect->size->height + this->rect->origin->y), (short)(this->rect->size->width + this->rect->origin->x)};
	Rect srcRect = {0, 0, (short)this->rect->size->height, (short)this->rect->size->width};

	CopyBits((BitMap*)&(**offscreenPixMap),
			 &(this->owner->GetWindowPtr()->portBits),
			 &srcRect, &destRect, srcCopy, NULL);

	UnlockPixels(offscreenPixMap);

#ifdef CKDEBUGTIMEDRAWS
	CKLog("Redraw took %lu ticks.", (TickCount() - t_start));
#endif

	SetPort(oldPort);
}

const GWorldPtr CKCanvas::GetOSPointer() {
	return this->__gworldptr;
}

void CKCanvas::Clear() {
	this->Fill(CKColor(255, 255, 255));
}

void CKCanvas::FillRect(CKRect rect, CKColor c) {

	CKPROFILE

	if (!this->__gworldptr) {
		return;
	}

	RGBColor rgb = c.ToOS();
	PixPatHandle pixPat = NewPixPat();
	if (pixPat != NULL) {
		MakeRGBPat(pixPat, &rgb);
	} else {
		CKLog("NewPixPat failed!");
		return;
	}

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		return;
	}

	CGrafPtr oldPort;
	GDHandle oldGD;
	GetGWorld(&oldPort, &oldGD);		// Save the old port
	SetGWorld(this->__gworldptr, NULL); // Set the GWorld as the current port

	Rect osRect = rect.ToOS();
	FillCRect(&osRect, pixPat);

	DisposePixPat(pixPat);

	SetGWorld(oldPort, oldGD); // Restore the old port

	UnlockPixels(offscreenPixMap);
}

void CKCanvas::Fill(CKColor c) {

	CKPROFILE

	CKRect rect = {0, 0, this->rect->size->width, this->rect->size->height};
	this->FillRect(rect, c);
}

void CKCanvas::SetPixel(CKPoint p, CKColor c) {

	CKPROFILE

	if (!this->__gworldptr) {
		return;
	}

	if (p.x < 0 || p.x > this->rect->size->width) {
		return;
	}

	if (p.y < 0 || p.y > this->rect->size->height) {
		return;
	}

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		return;
	}

	int trueRowBytes = (*offscreenPixMap)->rowBytes & 0x3FFF;

	u_int16_t* gWorldData = (u_int16_t*)GetPixBaseAddr(offscreenPixMap);
	int index = p.y * (trueRowBytes / 2) + p.x;							   // trueRowBytes / 2 gives us the number of pixels per row
	u_int16_t color = ((c.r >> 3) << 11) | ((c.g >> 2) << 5) | (c.b >> 3); // convert 8-bit RGB to 5-6-5 RGB

	gWorldData[index] = color;

	UnlockPixels(offscreenPixMap);
}

void CKCanvas::DrawLine(CKPoint start, CKPoint end, CKColor c) {

	if (!this->__gworldptr) {
		return;
	}

	RGBColor rgb = c.ToOS();
	CGrafPtr oldPort;
	GDHandle oldGD;
	GetGWorld(&oldPort, &oldGD);		// Save the old port
	SetGWorld(this->__gworldptr, NULL); // Set the GWorld as the current port

	RGBForeColor(&rgb);		  // Set the color to draw with.
	MoveTo(start.x, start.y); // Move to the start of the line.
	LineTo(end.x, end.y);	  // Draw a line to the end point.

	SetGWorld(oldPort, oldGD); // Restore the old port
}

bool CKCanvas::DrawResourceIcon(short resourceId, CKPoint where) {

	this->__hasQueuedIcon = true;
	this->__queuedIconResourceId = resourceId;
	this->__queuedIconWhere = where;

	if (!this->__gworldptr) {
		if (this->owner) {
			this->MarkAsDirty();
		}
		return true;
	}

	if (!CKHasIconUtilities()) {
		CKLog("Icon Utilities unavailable; cannot draw icons.");
		this->__hasQueuedIcon = false;
		return false;
	}

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		return false;
	}

	CGrafPtr oldPort;
	GDHandle oldGD;
	GetGWorld(&oldPort, &oldGD);		// Save the old port
	SetGWorld(this->__gworldptr, NULL); // Set the GWorld as the current port

	const bool hasIconUtils = CKHasIconUtilities();
	const bool hasColorQD = CKHasColorQuickDraw();

	CIconHandle cIconHandle = nullptr;
	Handle iconHandle = nullptr;

	// Try color first.
	if (hasIconUtils && hasColorQD) {
		cIconHandle = GetCIcon(resourceId);
	}
	if (!cIconHandle) {
		// OK, try monochrome.
		iconHandle = GetIcon(resourceId);
		if (!iconHandle) {
			CKLog("Resource %d not found.", resourceId);
			this->__hasQueuedIcon = false;
			SetGWorld(oldPort, oldGD);
			UnlockPixels(offscreenPixMap);
			return false;
		}
	}
	Rect destRect = {(short)where.y, (short)where.x, (short)(where.y + 32), (short)(where.x + 32)};

	if (cIconHandle) {
		PlotCIcon(&destRect, cIconHandle);
	} else {
		if (hasIconUtils) {
			PlotIcon(&destRect, iconHandle);
		}
	}

	SetGWorld(oldPort, oldGD); // Restore the old port

	if (cIconHandle) {
		DisposeCIcon(cIconHandle);
	} else if (iconHandle) {
		ReleaseResource(iconHandle);
	}

	UnlockPixels(offscreenPixMap);
	if (this->owner) {
		this->MarkAsDirty();
	}
	return true;
}
