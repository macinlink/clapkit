/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTextArea
 * ----------------------------------------------------------------------
 * Defines a large text area, with or without scrollbars and editing enabled.
 *
 */

#include "ckTextArea.h"
#include "ckWindow.h"
#include <Appearance.h>

const short kScrollBarWidth = 16;

// TODO: These must be defined somewhere but I'm too dumb to find them.
#define kControlUpButtonPart   20
#define kControlDownButtonPart 21
#define kControlPageUpPart	   22
#define kControlPageDownPart   23

pascal void __ScrollActionUPP(ControlHandle control, ControlPartCode part) {

	// Any part code less than 1 means “no part was actually clicked”:
	if (part < 1)
		return;

	// Fetch the CKTextArea pointer from the control’s CRefCon:
	CKTextArea* ta = (CKTextArea*)GetControlReference(control);
	if (!ta)
		return;

	// If it’s the thumb‐track, scroll by thumb position:
	if (part == kControlIndicatorPart) {
		// ta->__UpdateTextScroll();
		return;
	}

	// Otherwise it is an arrow or page click:
	int delta = 0;
	switch (part) {
		case kControlUpButtonPart:
			delta = ta->__GetLineHeight() * -1;
			break;
		case kControlDownButtonPart:
			delta = ta->__GetLineHeight();
			break;
		case kControlPageUpPart:
			delta = ta->__GetPageHeight() * -1;
			break;
		case kControlPageDownPart:
			delta = ta->__GetPageHeight();
			break;
		default:
			return;
	}

	// Adjust the control’s value, clamped to [0..max]:
	int oldVal = GetControlValue(control);
	int maxVal = GetControlMaximum(control);
	int newVal = oldVal + delta;
	if (newVal < 0)
		newVal = 0;
	if (newVal > maxVal)
		newVal = maxVal;
	SetControlValue(control, newVal);

	// Finally, tell the text‐area to refresh itself at that scroll offset:
	ta->__UpdateTextScroll(delta, 0);
}

CKTextArea::CKTextArea(const CKControlInitParams& params)
	: CKTextField(params) {

	this->scrollType = CKScrollType::none;
	this->scrollType.onChange = CKOBSERVEVALUE("scrollType");
}

CKTextArea::~CKTextArea() {

	this->__SetupScrollbars(true);
}

void CKTextArea::ResizeTE() {

	if (this->__teHandle == nullptr) {
		CKLog("CKTextArea's resize called but handle is null.");
		return;
	}

	// A ResizeTE call means our rect and/or our scrollbars have
	// changed, meaning we need to clear the background or we'll have
	// artifacts stuck on the screen.
	this->__needsFullRedraw = true;

	Rect cr = this->rect->ToOS();
	HLock((Handle)this->__teHandle);

	cr.top += 4;
	cr.left += 4;
	cr.bottom -= 2;
	cr.right -= 4;

	if (this->__vScrollBar) {
		cr.right -= kScrollBarWidth;
	}
	if (this->__hScrollBar) {
		cr.bottom -= kScrollBarWidth;
	}

	(*this->__teHandle)->viewRect = cr;
	(*this->__teHandle)->destRect = cr;

	TECalText(this->__teHandle);

	HUnlock((Handle)this->__teHandle);
}

void CKTextArea::TECreated() {

	TEAutoView(true, this->__teHandle);
	TEFeatureFlag(teFOutlineHilite, teBitSet, this->__teHandle);
	TEFeatureFlag(teFUseWhiteBackground, teBitSet, this->__teHandle);
	this->ResizeTE();
}

void CKTextArea::PrepareForDraw() {

	CKLabel::PrepareForDraw();

	if (!this->__teHandle) {
		return;
	}

	HLock((Handle)this->__teHandle);
	TEPtr te = *(this->__teHandle);

	if (this->__vScrollBar) {
		int viewHeight = te->viewRect.bottom - te->viewRect.top;
		int textHeight = TEGetHeight(te->nLines, 0, this->__teHandle);
		int maxScroll = textHeight - viewHeight;

		if (maxScroll > 0) {
			SetControlMaximum(this->__vScrollBar, maxScroll);
			SetControlValue(this->__vScrollBar, te->viewRect.top - te->destRect.top);
			HiliteControl(this->__vScrollBar, 0); // Enable
		} else {
			SetControlMaximum(this->__vScrollBar, 0);
			SetControlValue(this->__vScrollBar, 0);
			HiliteControl(this->__vScrollBar, 255); // Disable
		}
	}

	HUnlock((Handle)this->__teHandle);
}

void CKTextArea::Redraw() {

	if (!this->__teHandle) {
		return;
	}

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->owner->GetWindowPtr());

	RgnHandle clipHandle = NewRgn();
	GetClip(clipHandle);
	Rect cr = this->rect->ToOS();
	cr.top += 1;
	cr.left += 1;

	Rect r = this->rect->ToOS();

	if (this->__needsFullRedraw) {
		if (this->enabled) {
			RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};
			RGBForeColor(&white);
		} else {
			RGBColor gray = {0xC000, 0xC000, 0xC000};
			RGBForeColor(&gray);
		}
		EraseRect(&cr);
		this->__needsFullRedraw = false;
	}

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
		ForeColor(blackColor);
		FrameRect(&r);
	}

	if (this->__hScrollBar) {
		Draw1Control(this->__hScrollBar);
		cr.bottom -= kScrollBarWidth;
	}

	if (this->__vScrollBar) {
		Draw1Control(this->__vScrollBar);
		cr.right -= kScrollBarWidth;
	}

	if (this->__needsPreparing) {
		this->PrepareForDraw();
		this->__needsPreparing = false;
	}

	ClipRect(&cr);

	HLock((Handle)this->__teHandle);
	TEPtr trecord = *(this->__teHandle);

	cr.bottom -= 1;
	cr.right -= 1;
	EraseRect(&(trecord->viewRect));
	TEUpdate(&(trecord->viewRect), this->__teHandle);

	SetClip(clipHandle);
	DisposeRgn(clipHandle);

	SetPort(oldPort);
	HUnlock((Handle)this->__teHandle);
}

void CKTextArea::__SetupScrollbars(bool removeOnly) {

	if (this->__hScrollBar != nullptr) {
		DisposeControl(this->__hScrollBar);
		this->__hScrollBar = nullptr;
	}

	if (this->__vScrollBar != nullptr) {
		DisposeControl(this->__vScrollBar);
		this->__vScrollBar = nullptr;
	}

	if (removeOnly) {
		return;
	}

	if (!this->owner) {
		CKLog("SetupScrollbars called but owner of CKTextArea is not set yet!");
		return;
	}

	if (this->scrollType == CKScrollType::vertical || this->scrollType == CKScrollType::both) {

		Rect bounds = this->rect->ToOS();
		Rect vScrollRect;
		vScrollRect.left = bounds.right - kScrollBarWidth;
		vScrollRect.top = bounds.top;
		vScrollRect.right = bounds.right;
		vScrollRect.bottom = bounds.bottom;

		if (this->scrollType == CKScrollType::both) {
			vScrollRect.bottom -= kScrollBarWidth - 1;
		}

		this->__vScrollBar = NewControl(this->owner->GetWindowPtr(), &vScrollRect, "\p", true, 0, 0, 0, kControlProcIDScrollbar, (long)this);
	}

	if (this->scrollType == CKScrollType::horizontal || this->scrollType == CKScrollType::both) {

		Rect bounds = this->rect->ToOS();
		Rect hScrollRect;
		hScrollRect.left = bounds.left;
		hScrollRect.top = bounds.bottom - kScrollBarWidth;
		hScrollRect.right = bounds.right;
		hScrollRect.bottom = bounds.bottom;

		this->__hScrollBar = NewControl(this->owner->GetWindowPtr(), &hScrollRect, "\p", true, 0, 0, 0, kControlProcIDScrollbar, (long)this);
	}
}

void CKTextArea::RaisePropertyChange(const char* propertyName) {

	if (!strcmp(propertyName, "scrollType") || !strcmp(propertyName, "rect")) {
		this->__SetupScrollbars();
		this->ResizeTE();
	}
	if (!strcmp(propertyName, "enabled")) {
		this->__needsFullRedraw = true;
	}
	this->__needsPreparing = true;
	CKTextField::RaisePropertyChange(propertyName);
}

bool CKTextArea::HandleEvent(const CKEvent& evt) {

	if (evt.type == CKEventType::mouseDown) {

		if (this->__vScrollBar) {
			if (evt.point.x >= this->rect->size->width - kScrollBarWidth) {
				this->__HandleScrollBarClick(this->__vScrollBar, evt.point);
				return true;
			}
		}

		if (this->__hScrollBar) {
			if (evt.point.y >= this->rect->size->height - kScrollBarWidth) {
				this->__HandleScrollBarClick(this->__hScrollBar, evt.point);
				return true;
			}
		}
	}

	return CKTextField::HandleEvent(evt);
}

void CKTextArea::__HandleScrollBarClick(ControlHandle control, CKPoint point) {

	ControlPartCode part = TestControl(control, point.ToOS());
	if (part == 0) {
		return;
	}

	if (part == kControlIndicatorPart) {
		// Installing an UPP to IndicatorPart crashes.
		// This is apparently the right way of doing it.
		int vScroll = this->__vScrollBar ? GetControlValue(this->__vScrollBar) : 0;
		int hScroll = this->__hScrollBar ? GetControlValue(this->__hScrollBar) : 0;
		if (TrackControl(control, point.ToOS(), nullptr) != 0) {
			int vScrollNew = this->__vScrollBar ? GetControlValue(this->__vScrollBar) : 0;
			int hScrollNew = this->__hScrollBar ? GetControlValue(this->__hScrollBar) : 0;
			int vDelta = vScrollNew - vScroll;
			int hDelta = hScrollNew - hScroll;
			this->__UpdateTextScroll(vDelta, hDelta);
		}
		return;
	}

	HLock((Handle)control);
	ControlActionUPP hook = NewControlActionUPP(__ScrollActionUPP);
	TrackControl(control, point.ToOS(), hook);
	DisposeControlActionUPP(hook);
	HUnlock((Handle)control);
}

void CKTextArea::__UpdateTextScroll(int vDelta, int hDelta) {
	if (!this->__teHandle || (vDelta == 0 && hDelta == 0)) {
		return;
	}

	TEPinScroll(-hDelta, -vDelta, this->__teHandle);
	this->__needsPreparing = true;
	this->MarkAsDirty();
}