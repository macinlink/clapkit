/**
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

CKTextField::CKTextField(const CKControlInitParams& params)
	: CKLabel(params) {
}

CKTextField::~CKTextField() {
}

void CKTextField::Redraw() {

	// Draw the outline.

	Rect r = this->rect->ToOS();

	if (CKHasAppearanceManager()) {

		ThemeDrawState s = kThemeStateActive;
		if (!this->enabled) {
			s = kThemeStateDisabled;
		} else {
			if (this->focused) {
				s = kThemeStatePressed;
			}
		}
		DrawThemeEditTextFrame(&r, s);
	} else {

		if (this->enabled) {
			RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};
			RGBForeColor(&white);
		} else {
			RGBColor gray = {0xC000, 0xC000, 0xC000};
			RGBForeColor(&gray);
		}
		PaintRect(&r);
		ForeColor(blackColor);
		FrameRect(&r);
	}

	CKLabel::Redraw();
}

void CKTextField::TECreated() {

	TEAutoView(true, this->__teHandle);

	// TODO: Setup the `clickLoop` using TESetClickLoop?
}

void CKTextField::Blurred() {

	CKLog("CKTextField %x blurred.", this);

	this->focused = false;

	if (!this->__teHandle) {
		return;
	}

	TEDeactivate(this->__teHandle);
}

void CKTextField::Focused() {

	CKLog("CKTextField %x Focused.", this);

	this->focused = true;

	if (!this->__teHandle) {
		return;
	}

	TEActivate(this->__teHandle);
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
		RGBColor color = this->color.get().ToOS();
		RGBForeColor(&color);
	} else {
		// 50% gray.
		RGBColor gray = {0x8000, 0x8000, 0x8000};
		RGBForeColor(&gray);
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
		CKLog("TEClick on %dx%d", evt.point.x.get(), evt.point.y.get());
		TEClick(evt.point.ToOS(), evt.shiftDown, this->__teHandle);
	}

	if (evt.type == CKEventType::keyDown) {
		TEKey(evt.key, this->__teHandle);
		CKEvent e = CKEvent(CKEventType::changed);
		this->HandleEvent(e);
	}

	return CKLabel::HandleEvent(evt);
}