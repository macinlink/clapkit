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
#include <Appearance.h>

CKTextField::CKTextField(const CKControlInitParams& params)
	: CKLabel(params) {
}

CKTextField::~CKTextField() {
}

void CKTextField::Redraw() {

	// Draw the outline.
	// TODO: Use AppearanceManager in MacOS 8+ to make this pretty.

	if (CKHasAppearanceManager()) {
		// TODO: Use DrawThemeEditTextFrame() to draw it.
	}

	Rect r = this->GetRect()->ToOS();
	ForeColor(blackColor);
	FillRect(&r, &qd.white);
	BackColor(whiteColor);
	ForeColor(blackColor);
	FrameRect(&r);

	CKLabel::Redraw();
}

void CKTextField::TECreated() {

	TEAutoView(true, this->__teHandle);

	// TODO: Setup the `clickLoop` using TESetClickLoop?
}

void CKTextField::Blurred() {

	CKLog("CKTextField %x blurred.", this);

	if (!this->__teHandle) {
		return;
	}

	TEDeactivate(this->__teHandle);
}

void CKTextField::Focused() {

	CKLog("CKTextField %x Focused.", this);

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

	RGBColor color = this->color.ToOS();
	RGBForeColor(&color);

	Rect r = this->GetRect()->ToOS();
	r.top += 2;
	r.left += 2;
	r.right -= 4;
	r.bottom -= 4;
	trecord->viewRect = r;
	trecord->destRect = r;
	trecord->txMode = srcCopy;

	HUnlock((Handle)this->__teHandle);
}

bool CKTextField::HandleEvent(CKControlEvent evt) {

	if (evt.type == CKControlEventType::keyDown) {
		TEKey(evt.key, this->__teHandle);
		CKControlEvent e = CKControlEvent(CKControlEventType::changed);
		this->HandleEvent(e);
	}

	return CKLabel::HandleEvent(evt);
}

const char* CKTextField::GetText() {

	CKLog("GetText for CKTextField called.");
	return "yarrak";
}