/*
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
#include "ckPlatform.h"
#include "ckWindow.h"
#include "ck_pValueContainingControl.h"
#include <Appearance.h>
#include <Controls.h>

CKControlToolbox::CKControlToolbox(const CKControlInitParams& params, CKControlType type)
	: CKControl(params, type) {

	this->owner = nil;
	this->__ptr = 0;
	this->__type = type;

	this->SetText(params.title);
}

CKControlToolbox::~CKControlToolbox() {
	if (this->__ptr) {
		DisposeControl(this->__ptr);
	}
}

void CKControlToolbox::AddedToWindow(CKWindow* window) {

	CKPROFILE

	CKControl::AddedToWindow(window);

	if (this->owner == nil || this->owner->GetWindowPtr() == 0) {
		CKLog("CKControlToolbox added to window but owner or it's ptr is missing!");
		return;
	}

	Rect r = this->rect->ToOS();
	unsigned char* title = CKC2P(this->__text);

	const CKWindowPtr windowPtr = this->owner->GetWindowPtr();

	switch (this->__type) {
		case CKControlType::PushButton:
			this->__ptr = NewControl(windowPtr, &r, title, false, 0, 0, 0, kControlProcIDButton, 0);
			break;
		case CKControlType::Checkbox:
			this->__ptr = NewControl(windowPtr, &r, title, false, 0, 0, 1, kControlProcIDCheckbox, 0);
			break;
		case CKControlType::RadioButton:
			this->__ptr = NewControl(windowPtr, &r, title, false, 0, 0, 1, kControlProcIDRadio, 0);
			break;
		case CKControlType::Dropdown:
			// Handled over at CKDropdown::AddedToWindow override.
			break;
		default:
			throw CKNew CKException("Unknown/unhandled type passed to Toolbox control initializer!");
			break;
	}

	if (!this->__ptr) {
		CKLog("Unable to create new control!");
	}

	CKFree(title);

	this->__ReflectToOS();
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

	Draw1Control(this->__ptr);
	CKControl::Redraw();
}

bool CKControlToolbox::HandleEvent(const CKEvent& evt) {

	CKControl::HandleEvent(evt);

	if (evt.type == CKEventType::mouseDown) {
		bool didClick = false;
		if (TrackControl(this->__ptr, evt.point.ToOS(), (ControlActionUPP)(-1))) {
			didClick = true;
		}
		if (didClick) {
			this->HandleEvent(CKEvent(CKEventType::click));
			this->MarkAsDirty();
		}
		return true;
	}

	return false;
}

void CKControlToolbox::__ReflectToOS() {

	if (!this->__ptr) {
		return;
	}

	if (!this->visible) {
		HideControl(this->__ptr);
		return;
	} else {
		ShowControl(this->__ptr);
	}

	unsigned char* title = CKC2P(this->__text);
	SetControlTitle(this->__ptr, title);
	CKFree(title);

	SizeControl(this->__ptr, rect->size->width, rect->size->height);
	MoveControl(this->__ptr, rect->origin->x, rect->origin->y);

	HiliteControl(this->__ptr, this->enabled ? 0 : 255);
}

void CKControlToolbox::RaisePropertyChange(const char* propertyName) {

	this->__ReflectToOS();
	CKControl::RaisePropertyChange(propertyName);
}