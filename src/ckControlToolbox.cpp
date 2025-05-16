/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKControlOS
 * ----------------------------------------------------------------------
 * The base of controls that are mostly handled by the OS.
 *
 */

#include "ckControlToolbox.h"
#include "ckWindow.h"
#include <Appearance.h>
#include <Controls.h>

#define kControlProcIDButton   0
#define kControlProcIDCheckbox 1
#define kControlProcIDRadio	   2

CKControlToolbox::CKControlToolbox(const CKControlInitParams& params, CKControlType type)
	: CKControl(params, type) {

	this->owner = nil;
	this->__ptr = 0;
	this->__type = type;

	this->SetText(params.title);
}

CKControlToolbox::~CKControlToolbox() {
}

void CKControlToolbox::AddedToWindow(CKWindow* window) {

	CKPROFILE

	CKControl::AddedToWindow(window);

	if (this->owner == nil || this->owner->__windowPtr == 0) {
		CKLog("CKControlToolbox added to window but owner or it's ptr is missing!");
		return;
	}

	Rect r = this->__rect->ToOS();
	unsigned char* title = CKC2P(this->__text);

	switch (this->__type) {
		case CKControlType::PushButton:
			this->__ptr = NewControl(this->owner->__windowPtr, &r, title, false, 0, 0, 0, kControlProcIDButton, 0);
			break;
		case CKControlType::Checkbox:
			this->__ptr = NewControl(this->owner->__windowPtr, &r, title, false, 0, 0, 1, kControlProcIDCheckbox, 0);
			break;
		case CKControlType::RadioButton:
			this->__ptr = NewControl(this->owner->__windowPtr, &r, title, false, 0, 0, 1, kControlProcIDRadio, 0);
			break;
		default:
			throw CKNew CKException("Unknown/unhandled type passed to Toolbox control initializer!");
			break;
	}

	// These might've been set before they were added to the window..
	// TODO: A better solution for this?

	this->SetEnabled(this->GetEnabled());
	this->SetToggleValue(this->GetToggleValue());

	CKFree(title);

	if (this->GetVisible()) {
		this->Show();
	}
}

void CKControlToolbox::Show() {

	if (this->__ptr == 0) {
		CKLog("Show called on control (%x) with no ptr!", this);
		return;
	}

	if ((**(this->__ptr)).contrlOwner == 0) {
		CKLog("Show called on control (%x) but contrlOwner is nil", this);
		return;
	}

	CKControl::Show();
	ShowControl(this->__ptr);
}

void CKControlToolbox::Hide() {

	CKControl::Hide();

	if (this->__ptr == 0) {
		CKLog("Hide called on control with no ptr!");
		return;
	}

	HideControl(this->__ptr);
}

void CKControlToolbox::Redraw() {

	if (this->__ptr == 0) {
		CKLog("Redraw called for control %x but it has no control pointer!", this);
		return;
	}

	if ((**(this->__ptr)).contrlOwner == 0) {
		CKLog("Redraw called for control %x but it has no owner.", this);
		return;
	}

	// if (CKHasAppearanceManager()) {
	if (false) { // TODO: Not working right now.

		// If we have Apperance Manager, controls should be disabled
		// when the window is not active.

		// TODO: This is a hack, we should definitely move to ThemeDraw[...]
		// API calls and stop using Draw1Control entirely in Mac OS 8+.

		short oldHilite = (**(this->__ptr)).contrlHilite;

		// Set temporary hilite based on window active state
		if (this->owner && this->owner->GetIsActive()) {
			(**(this->__ptr)).contrlHilite = this->__enabled ? 0 : 255;
		} else {
			(**(this->__ptr)).contrlHilite = 255; // draw as disabled
		}

		CKLog("oldHilite = %x, (**(this->__ptr)).contrlHilite = %x", oldHilite, (**(this->__ptr)).contrlHilite);

		// Draw with temporary hilite state
		Draw1Control(this->__ptr);

		// Restore previous state (without causing another redraw)
		(**(this->__ptr)).contrlHilite = oldHilite;

	} else {
		Draw1Control(this->__ptr);
	}

	CKControl::Redraw();
}

void CKControlToolbox::SetEnabled(bool enabled) {

	CKControl::SetEnabled(enabled);

	if (!this->__ptr) {
		return;
	}

	if (enabled) {
		HiliteControl(this->__ptr, 0);
	} else {
		HiliteControl(this->__ptr, 255);
	}

	this->MarkAsDirty();
}

bool CKControlToolbox::HandleEvent(const CKEvent& evt) {

	if (CKControl::HandleEvent(evt)) {
		// Already handled, stop here.
		return true;
	}

	if (evt.type == CKEventType::mouseDown) {
		bool didClick = false;
		if (TrackControl(this->__ptr, evt.point.ToOS(), 0)) {
			didClick = true;
		}
		if (didClick) {
			// TODO: The line below is kind of stupid and should be moved somewhere else.
			this->SetToggleValue(!this->GetToggleValue()); // For radio & checkboxes..
			this->MarkAsDirty();
			this->HandleEvent(CKEvent(CKEventType::click));
		}
		return true;
	}

	return false;
}

void CKControlToolbox::SetRect(CKRect* rect) {

	CKControl::SetRect(rect);

	if (!this->__ptr) {
		return;
	}

	SizeControl(this->__ptr, rect->size.width, rect->size.height);
	MoveControl(this->__ptr, rect->origin.x, rect->origin.y);
	this->MarkAsDirty();
}

void CKControlToolbox::SetText(const char* text) {

	CKTextableControl::SetText(text);

	if (this->__ptr) {
		unsigned char* title = CKC2P(this->__text);
		SetControlTitle(this->__ptr, title);
		CKFree(title);
	}

	this->MarkAsDirty();
}

void CKControlToolbox::SetToggleValue(bool value) {
	if (!__ptr) {
		return;
	}
	SetControlValue(__ptr, value ? 1 : 0); // update the OS control
	HandleEvent(CKEventType::changed);	   // notify your framework
}

bool CKControlToolbox::GetToggleValue() const {
	return __ptr && (GetControlValue(__ptr) != 0);
}