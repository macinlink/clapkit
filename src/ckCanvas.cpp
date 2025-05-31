/**
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
#include <Resources.h>

CKCanvas::CKCanvas(const CKControlInitParams& params)
	: CKControl(params, CKControlType::Canvas) {

	CKPROFILE

	Rect bounds = {0, 0, (short)params.height, (short)params.width};
	OSErr err = NewGWorld(&(this->__gworldptr), 16, &bounds, NULL, NULL, 0);
	if (err != noErr) {
		throw CKNew CKException("Can't create GWorld for canvas.");
	}

	this->Clear();
}

CKCanvas::~CKCanvas() {

	CKPROFILE

	CKLog("~CKCanvas called.");

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

	long t_start = TickCount();

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->owner->GetWindowPtr());

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		SetPort(oldPort);
		return;
	}

	Rect destRect = {this->rect->origin->x, this->rect->origin->y, (short)(this->rect->size->height + this->rect->origin->y), (short)(this->rect->size->width + this->rect->origin->x)};
	Rect srcRect = {0, 0, (short)this->rect->size->height, (short)this->rect->size->width};

	CopyBits((BitMap*)&(**offscreenPixMap),
			 &(this->owner->GetWindowPtr()->portBits),
			 &srcRect, &destRect, srcCopy, NULL);

	UnlockPixels(offscreenPixMap);

	CKLog("Redraw took %lu ticks.", (TickCount() - t_start));
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

	PixMapHandle offscreenPixMap = GetGWorldPixMap(this->__gworldptr);
	if (!LockPixels(offscreenPixMap)) {
		CKLog("Can't lock GWorldPixMap.. Purged, maybe?");
		return false;
	}

	CGrafPtr oldPort;
	GDHandle oldGD;
	GetGWorld(&oldPort, &oldGD);		// Save the old port
	SetGWorld(this->__gworldptr, NULL); // Set the GWorld as the current port

	Handle iconHandle = nullptr;

	// Try color first.
	iconHandle = (Handle)GetCIcon(resourceId);
	bool isColor = true;
	if (!iconHandle) {
		// OK, try monochrome.
		iconHandle = GetIcon(resourceId);
		isColor = false;
		if (!iconHandle) {
			CKLog("Resource %d not found.", resourceId);
			return false;
		}
	}
	Rect destRect = {0, 0, 32, 32};

	if (isColor) {
		PlotCIcon(&destRect, (CIconHandle)iconHandle);
	} else {
		PlotIcon(&destRect, iconHandle);
	}

	SetGWorld(oldPort, oldGD); // Restore the old port

	UnlockPixels(offscreenPixMap);
	return true;
}