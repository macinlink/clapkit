/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKControl
 * ----------------------------------------------------------------------
 * The base of everything UI Control we have.
 *
 */

#include "ckControl.h"
#include "ckWindow.h"

CKControl::CKControl(const CKControlInitParams& params, CKControlType type)
	: CKObject() {

	this->owner = nil;
	this->enabled = true;
	this->enabled.onChange = CKOBSERVEVALUE("enabled");

	this->visible = true;
	this->visible.onChange = CKOBSERVEVALUE("visible");

	this->rect = CKRect(params.x, params.y, params.width, params.height);
	this->rect.onChange = CKOBSERVEVALUE("rect");
}

CKControl::~CKControl() {

	CKEvent evt = CKEvent(CKEventType::deleted);
	this->HandleEvent(evt);

	if (this->owner && this->owner->GetOwner()) {
		this->owner->GetOwner()->CKRemoveTimersOfOwner(this);
	}
}

/**
 * Called by CKApp when the control has been added
 * to a window.
 */
void CKControl::AddedToWindow(CKWindow* window) {

	this->owner = window;

	// So we actually draw, if needed.
	this->MarkAsDirty();
}

/**
 * Called by CKApp when the control has been removed
 * from a window.
 */
void CKControl::RemovedFromWindow() {

	CKEvent evt = CKEvent(CKEventType::removed);
	this->HandleEvent(evt);
}

/**
 * Called by CKApp when the control needs to (re-)draw
 * itself, usually due to an event like mouseDown or updateEvt.
 *
 * YOU MOST LIKELY DO NOT NEED TO CALL THIS. USE `MarkAsDirty`!
 */
void CKControl::Redraw() {
}

/**
 * Once changes are made to the control, we need to mark
 * it or its area as dirty so it can it can be redrawn later.
 * This function does this - call once you make changes.
 */
void CKControl::MarkAsDirty() {

	CKPROFILE

	if (this->owner == nil) {
		return;
	}

	this->owner.get()->DirtyArea(this->rect);
}

void CKControl::RaisePropertyChange(const char* propertyName) {
	if (!strcmp(propertyName, "rect")) {
		if (this->owner) {
			this->owner->DirtyArea(this->__lastRect);
		}
		this->__lastRect = this->rect;
	}
	this->MarkAsDirty();
	CKObject::RaisePropertyChange(propertyName);
}