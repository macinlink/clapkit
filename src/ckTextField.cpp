/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTextField
 * ----------------------------------------------------------------------
 * Defines an editable, one-line text field.
 *
 */

#include "ckTextField.h"
#include "ckWindow.h"
#include <Appearance.h>
#include <Gestalt.h>
#include <Quickdraw.h>
#include <Scrap.h>

// Check if we need bkPixPat manipulation for TextEdit
// System 7.x and early Mac OS 8.x need it, Mac OS 8.5+ doesn't
static bool __CKNeedsBkPixPatForTextEdit() {
	long version;
	if (Gestalt(gestaltSystemVersion, &version) != noErr) {
		return true; // Assume old system, be safe
	}
	return version < 0x0850; // True for < Mac OS 8.5
}

static PixPatHandle __CKWhiteBkPixPat() {
	static PixPatHandle pat = nullptr;
	if (!pat) {
		pat = NewPixPat();
		if (pat) {
			RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};
			MakeRGBPat(pat, &white);
		}
	}
	return pat;
}

static PixPatHandle __CKDisabledGrayBkPixPat() {
	static PixPatHandle pat = nullptr;
	if (!pat) {
		pat = NewPixPat();
		if (pat) {
			RGBColor gray = {0xC000, 0xC000, 0xC000};
			MakeRGBPat(pat, &gray);
		}
	}
	return pat;
}

struct CKTextFieldColors {
		RGBColor background;
		RGBColor text;
};

static CKTextFieldColors __CKGetFieldColors(bool enabled) {
	CKTextFieldColors colors;
	bool hasColorQD = CKHasColorQuickDraw();

	if (!hasColorQD) {
		colors.background = {0xFFFF, 0xFFFF, 0xFFFF};
		colors.text = enabled ? RGBColor{0x0000, 0x0000, 0x0000} : // Black
						  RGBColor{0x8000, 0x8000, 0x8000};		   // 50% gray
	} else {
		colors.background = {0xFFFF, 0xFFFF, 0xFFFF};			   // White for both enabled/disabled
		colors.text = enabled ? RGBColor{0x0000, 0x0000, 0x0000} : // Black
						  RGBColor{0x8000, 0x8000, 0x8000};		   // 50% gray (not dark gray)
	}

	return colors;
}

CKTextField::CKTextField(const CKControlInitParams& params)
	: CKLabel(params) {
}

CKTextField::~CKTextField() {
}

void CKTextField::Redraw() {
	Rect outerRect = this->rect->ToOS();
	Rect fillRect = outerRect;
	InsetRect(&fillRect, 1, 1);

	// Save graphics state
	RGBColor oldFore, oldBack;
	PenState oldPen;
	GetForeColor(&oldFore);
	GetBackColor(&oldBack);
	GetPenState(&oldPen);

	// Get platform colors
	CKTextFieldColors colors = __CKGetFieldColors(this->enabled);

	// Paint background
	PenNormal();
	if (CKHasColorQuickDraw()) {
		RGBForeColor(&colors.background);
	} else {
		ForeColor(whiteColor);
	}
	PaintRect(&fillRect);

	// Set background color for TextEdit
	if (CKHasColorQuickDraw()) {
		RGBBackColor(&colors.background);
	} else {
		BackColor(whiteColor);
	}

	// Draw frame - simple black frame on all systems
	ForeColor(blackColor);
	FrameRect(&outerRect);

	// Draw text with conditional bkPixPat swap
	// System 7.x/8.0-8.1 need bkPixPat set for TextEdit word selection
	// Mac OS 8.5+ has improved TextEdit that doesn't need it
	bool swappedBkPixPat = false;
	PixPatHandle savedBkPixPat = nullptr;
	if (__CKNeedsBkPixPatForTextEdit() && CKHasColorQuickDraw() && this->owner && this->__teHandle) {
		CGrafPtr port = (CGrafPtr)this->owner->GetWindowPtr();
		if (port) {
			savedBkPixPat = port->bkPixPat;
			port->bkPixPat = this->enabled ? __CKWhiteBkPixPat() : __CKDisabledGrayBkPixPat();
			swappedBkPixPat = true;
		}
	}

	CKLabel::Redraw(); // Calls TEUpdate()

	if (swappedBkPixPat) {
		CGrafPtr port = (CGrafPtr)this->owner->GetWindowPtr();
		if (port) {
			port->bkPixPat = savedBkPixPat;
		}
	}

	// Restore state
	RGBForeColor(&oldFore);
	RGBBackColor(&oldBack);
	SetPenState(&oldPen);
}

void CKTextField::TECreated() {

	TEAutoView(true, this->__teHandle);

	// NOTE: TEFeatureFlag was introduced in Mac OS 8.5, so we can't use it
	// for backwards compatibility with System 7.x and early Mac OS 8.x.
	// The feature flags (teFOutlineHilite, teFUseWhiteBackground) are just
	// optimizations for selection rendering and aren't critical for functionality.

	// TODO: Setup the `clickLoop` using TESetClickLoop?
}

void CKTextField::Blurred() {

	this->focused = false;

	if (!this->__teHandle) {
		return;
	}

	TEDeactivate(this->__teHandle);
	this->MarkAsDirty();
}

void CKTextField::Focused() {

	this->focused = true;

	if (!this->__teHandle) {
		return;
	}

	TEActivate(this->__teHandle);
	this->MarkAsDirty();
}

void CKTextField::PrepareForDraw() {
	HLock((Handle)this->__teHandle);
	TEPtr trecord = *(this->__teHandle);

	trecord->txSize = this->fontSize;
	trecord->txFont = 0;
	trecord->txFace = 0;
	trecord->lineHeight = this->fontSize + 3;
	trecord->fontAscent = this->fontSize;

	// Set text color and mode
	if (this->enabled) {
		if (CKHasColorQuickDraw()) {
			RGBColor color = this->color.get().ToOS();
			RGBForeColor(&color);
		} else {
			ForeColor(blackColor);
		}
		trecord->txMode = srcCopy;
	} else {
		// Use platform-specific disabled text color
		CKTextFieldColors colors = __CKGetFieldColors(false);
		if (CKHasColorQuickDraw()) {
			RGBForeColor(&colors.text);
			trecord->txMode = srcCopy;
		} else {
			ForeColor(blackColor);
			trecord->txMode = grayishTextOr; // Mode 49 - dithered gray text
		}
	}

	// Setup rects
	Rect r = this->rect->ToOS();
	r.top += 2;
	r.left += 2;
	r.right -= 2;
	r.bottom -= 2;
	trecord->viewRect = r;
	trecord->destRect = r;

	HUnlock((Handle)this->__teHandle);
}

bool CKTextField::HandleEvent(const CKEvent& evt) {

	if ((evt.type == CKEventType::mouseDown || evt.type == CKEventType::keyDown) && this->__teHandle && this->owner) {

		GrafPtr oldPort;
		GetPort(&oldPort);

		RGBColor oldFore;
		RGBColor oldBack;
		PenState oldPen;
		GetForeColor(&oldFore);
		GetBackColor(&oldBack);
		GetPenState(&oldPen);

		SetPort(this->owner->GetWindowPtr());
		PenNormal();
		PenMode(patCopy);
		TextMode(srcCopy);

		// Set background color for TextEdit
		CKTextFieldColors colors = __CKGetFieldColors(this->enabled);
		if (CKHasColorQuickDraw()) {
			RGBBackColor(&colors.background);
		} else {
			BackColor(whiteColor);
		}

		// Swap bkPixPat if needed for old TextEdit (System 7.x - Mac OS 8.1)
		bool swappedBkPixPat = false;
		PixPatHandle savedBkPixPat = nullptr;
		if (__CKNeedsBkPixPatForTextEdit() && CKHasColorQuickDraw()) {
			CGrafPtr port = (CGrafPtr)this->owner->GetWindowPtr();
			if (port) {
				savedBkPixPat = port->bkPixPat;
				port->bkPixPat = this->enabled ? __CKWhiteBkPixPat() : __CKDisabledGrayBkPixPat();
				swappedBkPixPat = true;
			}
		}

		if (evt.type == CKEventType::mouseDown) {
			TEClick(evt.point.ToOS(), evt.shiftDown, this->__teHandle);
		} else {
			TEKey(evt.key, this->__teHandle);
			CKEvent e = CKEvent(CKEventType::changed);
			this->HandleEvent(e);
		}

		if (swappedBkPixPat) {
			CGrafPtr port = (CGrafPtr)this->owner->GetWindowPtr();
			if (port) {
				port->bkPixPat = savedBkPixPat;
			}
		}

		SetPenState(&oldPen);
		RGBForeColor(&oldFore);
		RGBBackColor(&oldBack);
		SetPort(oldPort);
	} else {
		if (evt.type == CKEventType::mouseDown) {
			TEClick(evt.point.ToOS(), evt.shiftDown, this->__teHandle);
		}

		if (evt.type == CKEventType::keyDown) {
			TEKey(evt.key, this->__teHandle);
			CKEvent e = CKEvent(CKEventType::changed);
			this->HandleEvent(e);
		}
	}

	return CKLabel::HandleEvent(evt);
}

void CKTextField::PerformClear() {
	this->SetText(nullptr);
}

void CKTextField::PerformCut() {
	if (this->__teHandle) {
		TECut(this->__teHandle);
	}
}

void CKTextField::PerformCopy() {
	if (this->__teHandle) {
		TECopy(this->__teHandle);
	}
}

void CKTextField::PerformPaste() {
	if (this->__teHandle) {
		TEPaste(this->__teHandle);
	}
}

bool CKTextField::CanPerformPaste() {
	long offset; // TODO: needed?
	long r = GetScrap(nil, 'TEXT', &offset);
	return r != noTypeErr;
}
