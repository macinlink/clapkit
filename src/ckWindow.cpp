/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKWindow
 * ----------------------------------------------------------------------
 * Defines a window.
 *
 */

#include "ckWindow.h"
#include "ckButton.h"
#include "ck_pFocusableControl.h"

/**
 * Create a new window with the parameters passed.
 * Check the CKWindowInitParams object for required/optional parameters.
 */
CKWindow::CKWindow(CKWindowInitParams params)
	: CKObject() {

	CKPROFILE

	this->__controls = std::vector<CKControl*>();
	this->__visible = false;

	// this->__rect = (CKRect*)CKMalloc(sizeof(*this->__rect));
	this->__rect = CKNew CKRect(params.x, params.y, params.width, params.height);

	Rect r = this->__rect->ToOS();
	this->__windowPtr = NewCWindow(nil, &r, "\p", false, 0, 0, params.closable, 0);
	this->SetTitle(params.title);

	this->latestDownControl = 0;
	this->activeTextInputControl = 0;
	this->shouldReceiveMouseMoveEvents = false;

	CKLog("Created window %x", this);
}

CKWindow::~CKWindow() {

	CKPROFILE

	CKLog("Destroying window %x", this);

	CKEvent evt = CKEvent(CKEventType::deleted);
	this->HandleEvent(evt);

	if (this->__rect) {
		CKDelete(this->__rect);
	}

	while (this->__controls.size() > 0) {
		this->RemoveControl(this->__controls.at(0), true);
	}
}

/**
 * Change the window's title.
 */
void CKWindow::SetTitle(const char* title) {

	CKPROFILE

	unsigned char* tbResult = CKC2P(title);
	SetWTitle(this->__windowPtr, tbResult);
	CKFree(tbResult);
}

/**
 * Get the window's title.
 */
char* CKWindow::GetTitle() {

	CKPROFILE

	unsigned char* tbResult = (unsigned char*)CKMalloc(256);
	GetWTitle(this->__windowPtr, tbResult);

	char* result = CKP2C(tbResult);
	CKFree(tbResult);
	return result;
}

/**
 * Show the window.
 */
void CKWindow::Show() {

	CKPROFILE

	ShowWindow(this->__windowPtr);
	this->__visible = true;
}

/**
 * Hide the window, but keep in memory.
 */
void CKWindow::Hide() {

	CKPROFILE

	HideWindow(this->__windowPtr);
	this->__visible = false;
}

/**
 * @brief Is this window visible currently?
 * @return True if visible.
 */
bool CKWindow::IsVisible() {

	return this->__visible;
}

/**
 * @brief Make the window foremost window. Make visible if invisible.
 */
void CKWindow::Focus() {

	if (!this->IsVisible()) {
		this->Show();
	}

	SelectWindow(this->__windowPtr);
}

/**
 * pwnd lol ya
 */
CKRect* CKWindow::GetRect(bool getCopy) {

	CKPROFILE

	if (getCopy) {
		CKRect* r = CKNew CKRect(this->__rect->x, this->__rect->y, this->__rect->width, this->__rect->height);
		return r;
	} else {
		return this->__rect;
	}
}

/**
 * Move window to a specific location.
 */
void CKWindow::Move(int x, int y) {

	CKPROFILE

	this->__rect->x = x;
	this->__rect->y = y;

	CKLog("Moving window %x to %d,%d (size: %dx%d)", this->__windowPtr, x, y, this->GetRect()->width, this->GetRect()->height);
	MoveWindow(this->__windowPtr, x, y, false);

	CKPoint p = CKPoint(x, y);
	CKEvent evt = CKEvent(CKEventType::moved, p);
	this->HandleEvent(evt);
}

/**
 * Change the size of the window..
 */
void CKWindow::Resize(int width, int height) {

	CKPROFILE

	this->__rect->width = width;
	this->__rect->height = height;

	SizeWindow(this->__windowPtr, this->__rect->width, this->__rect->height, true);
}

/**
 * Center the window on screen.
 */
void CKWindow::Center() {

	CKPROFILE

	GrafPtr globalPort;
	GetPort(&globalPort);

	int x = ((globalPort->portRect.right - globalPort->portRect.left) - this->__rect->width) / 2;
	int y = ((globalPort->portRect.bottom - globalPort->portRect.top) - this->__rect->height) / 2;
	this->Move(x, y);
}

/**
 * Override to stop closage.
 */
void CKWindow::Close() {

	CKPROFILE

	this->__dead = true;

	this->Hide();

	CKEvent evt = CKEvent(CKEventType::removed);
	this->HandleEvent(evt);

	this->__owner->CKRemoveWindow(this);
}

/**
 * Add control to the window.
 * Control must NOT be already in another window, or this fails.
 * Returns false if fails (i.e. control already exists)
 */
bool CKWindow::AddControl(CKControl* control) {

	CKPROFILE

	if (control == nil) {
		CKLog("Nil control passed to AddControl!");
		return false;
	}

	if (control->owner != nil) {
		if (control->owner == this) {
			// Let's check if its actually here.
			// We have to do this because even though Toolbox controls
			// do require and utilize the window passed to them, we
			// still need to track them ourselves, so AddControl needs to
			// be called.
			if (this->ContainsControl(control)) {
				CKLog("AddControl called but control already inside me.");
				return true;
			}
		} else {
			CKLog("AddControl called but control already has an owner");
			return false;
		}
	}

	control->owner = this;
	this->__controls.push_back(control);
	control->AddedToWindow(this);

	return true;
}

/**
 * Remove control from the window.
 * Does nothing if the control is already not there.
 */
void CKWindow::RemoveControl(CKControl* control, bool free) {

	CKPROFILE

	// Mark as dirty so it is then deleted once we are done.
	control->MarkAsDirty();

	// If we don't do this, we'll surely explode in a crash
	// later on when we get a mouseDown event.
	if (this->latestDownControl == control) {
		this->latestDownControl = 0;
	}

	control->owner = nil;

	bool found = false;
	for (auto it = this->__controls.begin(); it != this->__controls.end(); ++it) {
		if (*it == control) {
			it = this->__controls.erase(it);
			found = true;
			break;
		}
	}

	if (!found) {
		CKLog("RemoveControl called for %x but can't find it!");
	} else {
		CKLog("RemoveControl successfully removed control %x.", control);
		if (free) {
			CKDelete(control);
		}
	}
}

/**
 * Redraw part(s) of the window.
 */
void CKWindow::Redraw(CKRect rectToRedraw) {
	CKPROFILE

	// TODO: This is stupid and draws the entire control list.
	// Maybe don't do that.

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->__windowPtr);

	// Use stack-allocated Rect (No malloc needed)
	Rect r;
	r.top = 0;
	r.left = 0;
	r.right = this->__rect->width;
	r.bottom = this->__rect->height;

	EraseRect(&r); // Clear the window

	// Redraw all controls
	for (auto& c : this->__controls) {
		c->Redraw();
	}

	SetPort(oldPort); // Restore port AFTER drawing everything
}

/**
 * Set the owner – if this is set to nil,
 * we won't be able to do much.
 */
void CKWindow::SetOwner(CKApp* owner) {

	this->__owner = owner;
}

/**
 * Try to find a control at the point.
 * Returns nil if nothing is at that point.
 * Due to the way it works, undefined behavior when
 * two controls overlap (so don't do that, perhaps.)
 *
 * CKPoint passed here MUST be local to the window.
 */
CKControl* CKWindow::FindControl(CKPoint point) {

	CKPROFILE

	for (auto& c : this->__controls) {
		if (!c->GetEnabled()) {
			continue;
		}
		if (!c->GetVisible()) {
			continue;
		}
		if (c->GetRect()->intersectsPoint(point)) {
			return c;
		}
	}

	return nil;
}

/**
 * True if we have the control here.
 */
bool CKWindow::ContainsControl(CKControl* control) {

	CKPROFILE

	for (auto& c : this->__controls) {
		if (c == control) {
			return true;
		}
	}

	return false;
}

/**
 * @brief Called by ckApp on a click event - to set the
 * active control (like a textfield.)
 * @param control Can be nil.
 */
void CKWindow::SetActiveControl(CKControl* control) {

	if (this->__dead) {
		return;
	}

	if (this->activeTextInputControl) {
		if (control && this->activeTextInputControl == control) {
			return;
		}
		if (auto c = dynamic_cast<CKFocusableControl*>(this->activeTextInputControl)) {
			c->Blurred();
		}
	}

	this->activeTextInputControl = nil;

	if (control) {
		if (auto c = dynamic_cast<CKFocusableControl*>(control)) {
			c->Focused();
			this->activeTextInputControl = control;
		}
	}
}

/**
 * Called when the window becomes active/inactive.
 */
void CKWindow::SetIsActive(bool active) {

	// TODO: Handle scrollbars, etc.
}

/**
 * Called by CKApp when the user interacts with
 * our control. Override for custom controls.
 *
 * Returns true if handled.
 */
bool CKWindow::HandleEvent(const CKEvent& evt) {

	CKPROFILE

	if (this->__dead) {
		return true;
	}

	if (evt.type == CKEventType::keyDown && evt.key == 13) {
		for (auto it = this->__controls.begin(); it != this->__controls.end(); ++it) {
			CKButton* button = dynamic_cast<CKButton*>(*it);
			if (button && button->GetDefault()) {
				CKEvent clickEvt = CKEvent(CKEventType::click);
				button->HandleEvent(clickEvt);
			}
		}
	}

	if (this->activeTextInputControl) {
		// TODO: Text-editable controls need to handle this, of course.
		this->activeTextInputControl->HandleEvent(evt);
	}

	return CKObject::HandleEvent(evt);
}