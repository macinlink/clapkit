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
#include <Scrap.h>

CKTextField::CKTextField(const CKControlInitParams& params)
	: CKLabel(params) {
}

CKTextField::~CKTextField() {
}

void CKTextField::Redraw() {

	// Draw the outline.

	Rect r = this->rect->ToOS();
	Rect fillRect = r;
	InsetRect(&fillRect, 1, 1);

	RGBColor oldFore;
	RGBColor oldBack;
	GetForeColor(&oldFore);
	GetBackColor(&oldBack);

	if (CKHasAppearanceManager()) {

		ThemeDrawState s = kThemeStateActive;
		if (!this->enabled) {
			s = kThemeStateDisabled;
		}

		GDHandle deviceHdl = LMGetMainDevice();
		SInt16 gPixelDepth = (*(*deviceHdl)->gdPMap)->pixelSize;
		Boolean isColorDevice = gPixelDepth > 1;

		ThemeBrush backgroundBrush = kThemeBrushWhite;
		if (!this->enabled) {
			backgroundBrush = kThemeBrushDialogBackgroundInactive;
		}

		SetThemeBackground(backgroundBrush, gPixelDepth, isColorDevice);
		EraseRect(&fillRect);
		DrawThemeEditTextFrame(&r, s);
	} else {

		if (this->enabled) {
			if (CKHasColorQuickDraw()) {
				RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};
				RGBBackColor(&white);
			} else {
				BackColor(whiteColor);
			}
		} else {
			if (CKHasColorQuickDraw()) {
				RGBColor gray = {0xC000, 0xC000, 0xC000};
				RGBBackColor(&gray);
			} else {
				BackColor(blackColor);
			}
		}

		EraseRect(&fillRect);
		ForeColor(blackColor);
		FrameRect(&r);
	}

	CKLabel::Redraw();

	RGBForeColor(&oldFore);
	RGBBackColor(&oldBack);
}

void CKTextField::TECreated() {

	TEAutoView(true, this->__teHandle);

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

	if (this->enabled) {
		if (CKHasColorQuickDraw()) {
			RGBColor color = this->color.get().ToOS();
			RGBForeColor(&color);
		} else {
			ForeColor(blackColor);
		}
	} else {
		// 50% gray.
		if (CKHasColorQuickDraw()) {
			RGBColor gray = {0x8000, 0x8000, 0x8000};
			RGBForeColor(&gray);
		} else {
			ForeColor(blackColor);
		}
	}

	// TODO: This padding works perfectly fine for
	// standard system font + kCKTextFieldHeight but
	// what if the user wants a taller text field or
	// uses a different font?
	Rect r = this->rect->ToOS();
	r.top += 2;
	r.left += 2;
	r.right -= 2;
	r.bottom -= 2;
	trecord->viewRect = r;
	trecord->destRect = r;
	trecord->txMode = srcCopy;

	HUnlock((Handle)this->__teHandle);
}

bool CKTextField::HandleEvent(const CKEvent& evt) {

	if (evt.type == CKEventType::mouseDown) {
		TEClick(evt.point.ToOS(), evt.shiftDown, this->__teHandle);
	}

	if (evt.type == CKEventType::keyDown) {
		TEKey(evt.key, this->__teHandle);
		CKEvent e = CKEvent(CKEventType::changed);
		this->HandleEvent(e);
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
