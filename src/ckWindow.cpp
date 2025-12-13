/*
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
#include "ckLabel.h"
#include "ck_pFocusableControl.h"
#include <Appearance.h>
#include <MacWindows.h>

/**
 * Create a new window with the parameters passed.
 * Check the CKWindowInitParams object for required/optional parameters.
 */
CKWindow::CKWindow(CKWindowInitParams params)
	: CKObject() {

	CKPROFILE

	this->__controls = std::vector<CKControl*>();
	this->visible = false;
	this->hasCustomBackgroundColor = false;
	this->backgroundColor = CKColor(255, 255, 255);

	if (params.origin) {
		this->rect = CKRect(params.origin->x, params.origin->y, params.size.width, params.size.height);
	} else {
		this->rect = CKRect(params.size.width, params.size.height);
		Rect screen = qd.screenBits.bounds;
		this->rect->origin->x = (screen.right + screen.left - this->rect->size->width) / 2;
		this->rect->origin->y = (screen.bottom + screen.top - this->rect->size->height) / 2;
	}

	Rect r = this->rect->ToOS();

	short windowProcId = -1;

	switch (params.type) {
		case CKWindowType::Standard:
			windowProcId = 4;
			break;
		case CKWindowType::Floating:
			windowProcId = 16;
			break;
		case CKWindowType::Modal:
			windowProcId = 1;
			break;
		case CKWindowType::StandardResizable:
			windowProcId = 0;
			break;
		default:
			CKLog("Warning! Unknown window type '%d' passed to CKWindow init.", params.type);
			windowProcId = 0;
	}

	this->__type = params.type;
	if (CKHasColorQuickDraw()) {
		this->__windowPtr = NewCWindow(nil, &r, "\p", false, windowProcId, 0, this->closable, 0);
	} else {
		this->__windowPtr = NewWindow(nil, &r, "\p", false, windowProcId, 0, this->closable, 0);
	}

	if (!this->__windowPtr) {
		throw CKNew CKException("Can't create window.");
	}

	if (CKHasAppearanceManager()) {
		SetThemeWindowBackground(this->__windowPtr, kThemeBrushDialogBackgroundActive, true);
	} else {
		// System 7 or Appearance not available
		// Fill with white
		GrafPtr oldPort;
		GetPort(&oldPort);
		SetPort(this->__windowPtr);
		BackColor(whiteColor);
		SetPort(oldPort);
	}

	this->SetTitle(params.title);

	this->__lastDownControl = 0;
	this->__activeTextInputControl = 0;
	this->shouldReceiveMouseMoveEvents = false;

	this->visible.onChange = CKOBSERVEVALUE("visible");
	this->backgroundColor.onChange = CKOBSERVEVALUE("backgroundColor");
	this->hasCustomBackgroundColor.onChange = CKOBSERVEVALUE("hasCustomBackgroundColor");
	this->rect.onChange = CKOBSERVEVALUE("rect");
}

CKWindow::~CKWindow() {

	CKPROFILE

	CKEvent evt = CKEvent(CKEventType::deleted);
	this->HandleEvent(evt);

	if (this->__owner) {
		this->__owner->CKRemoveTimersOfOwner(this);
	}

	while (this->__controls.size() > 0) {
		this->RemoveControl(this->__controls.at(0), true);
	}

	// Release Mac OS window resource to prevent leak
	if (this->__windowPtr) {
		DisposeWindow(this->__windowPtr);
		this->__windowPtr = nullptr;
	}
}

/**
 * @brief Called from CKApp to do stuff like blinking the caret, etc.
 *
 */
void CKWindow::Loop() {

	if (this->__activeTextInputControl) {
		if (auto c = dynamic_cast<CKLabel*>(this->__activeTextInputControl)) {
			c->DoTEIdle();
		}
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

// /**
//  * Show the window.
//  */
void CKWindow::Show() {

	CKPROFILE
	this->visible = true;
	this->__ReflectToOS();

	SelectWindow(this->__windowPtr);
}

/**
 * Hide the window, but keep in memory.
 */
void CKWindow::Hide() {

	CKPROFILE
	this->visible = false;
	this->__ReflectToOS();
}

/**
 * @brief Make the window foremost window. Make visible if invisible.
 */
void CKWindow::Focus() {

	CKPROFILE
	this->visible = true;
	this->__ReflectToOS();
	SelectWindow(this->__windowPtr);
}

/**
 * Center the window on screen.
 */
void CKWindow::Center() {

	CKPROFILE

	Rect screen = qd.screenBits.bounds;
	this->rect->origin->x = (screen.right + screen.left - this->rect->size->width) / 2;
	this->rect->origin->y = (screen.bottom + screen.top - this->rect->size->height) / 2;
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

	if (this->__owner) {
		this->__owner->CKRemoveWindow(this);
	} else {
		CKLog("Warning: Window %x has no owner, cannot remove from app!", this);
	}
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
	if (this->__lastDownControl == control) {
		this->__lastDownControl = 0;
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

	Rect r = rectToRedraw.ToOS();

	if (this->__controlDirtifiedAreas.size() == 0) {

		// If we are NOT coming from a control wanting an update,
		// then we here because of the OS wanting us to update, which means
		// we should be drawing the background.

		if (this->hasCustomBackgroundColor) {
			// Use the user's picked color.
			if (CKHasColorQuickDraw()) {
				RGBColor c = this->backgroundColor->ToOS();
				RGBBackColor(&c);
			} else {
				// No color QuickDraw – fall back to black/white palette.
				BackColor(whiteColor);
			}
			EraseRect(&r);
		} else if (CKHasAppearanceManager()) {
			// Mac OS 8 and above: use theme color.
			SetThemeWindowBackground(this->__windowPtr, this->__isCurrentlyActive ? kThemeBrushDialogBackgroundActive : kThemeBrushDialogBackgroundInactive, false);
			EraseRect(&r);
		} else {
			// System 7 or below: default white
			BackColor(whiteColor);
			EraseRect(&r);
		}
	}

	// If we are resizable, we need to clip the draw area
	// before we draw the controls!
	// The scrollbars are 15px wide / tall.
	RgnHandle clipHandle = NewRgn();
	if (this->__type == CKWindowType::StandardResizable) {
		GetClip(clipHandle);
		Rect cr = r;
		cr.right -= 15;
		cr.bottom -= 15;
		ClipRect(&cr);
	}

	// Redraw all controls
	WindowPeek wp = (WindowPeek)(this->__windowPtr);
	Rect dr = (**wp->updateRgn).rgnBBox;
	for (auto& c : this->__controls) {
		if (c->rect->IntersectsRect(rectToRedraw)) {
			c->Redraw();
		}
	}

	if (this->__type == CKWindowType::StandardResizable) {
		SetClip(clipHandle);
		DrawGrowIcon(this->__windowPtr);
		DisposeRgn(clipHandle);
	}

	SetPort(oldPort);
	this->__controlDirtifiedAreas.clear();
}

/**
 * Set the owner – if this is set to nil,
 * we won't be able to do much.
 */
void CKWindow::SetOwner(CKApp* owner) {

	this->__owner = owner;
}

/**
 * @brief Return the app this window is a part of.
 */
CKApp* CKWindow::GetOwner() {

	return this->__owner;
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
		if (!c->enabled) {
			continue;
		}
		if (!c->visible) {
			continue;
		}
		if (c->rect->IntersectsPoint(point)) {
			return c;
		}
	}

	return nil;
}

/**
 * @brief Get the list of controls in this window.
 * @return
 */
const std::vector<CKControl*>& CKWindow::GetControls() const {
	return this->__controls;
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
 * @brief Called by ckApp to determine UI Changes needed,
 * like enabling/disabling menu items.
 * @return
 */
CKFocusableControl* CKWindow::GetActiveControl() {

	if (auto c = dynamic_cast<CKFocusableControl*>(this->__activeTextInputControl)) {
		return c;
	}
	return nullptr;
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

	if (this->__activeTextInputControl) {
		if (control && this->__activeTextInputControl == control) {
			return;
		}
		if (auto c = dynamic_cast<CKFocusableControl*>(this->__activeTextInputControl)) {
			c->Blurred();
		}
	}

	this->__activeTextInputControl = nil;

	if (control) {
		if (auto c = dynamic_cast<CKFocusableControl*>(control)) {
			c->Focused();
			this->__activeTextInputControl = control;
		}
	}
}

/**
 * Called when the window becomes active/inactive.
 */
void CKWindow::SetIsActive(bool active) {
	this->__isCurrentlyActive = active;

	if (this->__activeTextInputControl) {
		if (auto c = dynamic_cast<CKFocusableControl*>(this->__activeTextInputControl)) {
			if (active) {
				c->Focused();
			} else {
				c->Blurred();
			}
		}
	}

	if (active) {
		this->__InvalidateEntireWindow();
	} else {
		// We should not call this->__InvalidateEntireWindow() here
		// as if we've arrived here from an OSEvt, we won't get any update requests.
		this->__controlDirtifiedAreas.clear(); // Make sure we do a background paint.
		this->Redraw(CKRect(this->rect->size->width, this->rect->size->height));
	}
}

CKControl* CKWindow::GetLastControl() const {

	return this->__lastDownControl;
}

void CKWindow::SetLastControl(CKControl* control) {

	this->__lastDownControl = control;
}

const CKWindowPtr CKWindow::GetWindowPtr() const {

	return this->__windowPtr;
}

bool CKWindow::GetIsActive() {

	return this->__isCurrentlyActive;
}

void CKWindow::DirtyArea(const CKRect rect) {

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->__windowPtr);
	Rect r = rect.ToOS();
	InvalRect(&r);
	SetPort(oldPort);

	__controlDirtifiedAreas.push_back(rect);
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

	if (this->__activeTextInputControl) {
		// TODO: Text-editable controls need to handle this, of course.
		this->__activeTextInputControl->HandleEvent(evt);
	}

	return CKObject::HandleEvent(evt);
}

void CKWindow::__InvalidateEntireWindow() {

	Rect r;
	r.top = 0;
	r.left = 0;
	r.right = this->rect->size->width;
	r.bottom = this->rect->size->height;
	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->__windowPtr);
	InvalRect(&r);
	SetPort(oldPort);
	this->__controlDirtifiedAreas.clear(); // Make sure we do a background paint.
}

void CKWindow::__ReflectToOS() {

	if (this->visible) {
		ShowWindow(this->__windowPtr);
	} else {
		HideWindow(this->__windowPtr);
		return;
	}

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(this->__windowPtr);

	WindowPeek windowPeek = (WindowPeek)this->__windowPtr;
	windowPeek->goAwayFlag = this->closable;

	MoveWindow(this->__windowPtr, this->rect->origin->x, this->rect->origin->y, false);
	SizeWindow(this->__windowPtr, this->rect->size->width, this->rect->size->height, true);

	SetPort(oldPort);
}

void CKWindow::RaisePropertyChange(const char* propertyName) {

	this->__ReflectToOS();
	if (!strcmp(propertyName, "hasCustomBackgroundColor") || !strcmp(propertyName, "backgroundColor")) {
		this->__InvalidateEntireWindow();
	}
	if (!strcmp(propertyName, "rect")) {
		this->__InvalidateEntireWindow();
	}
	CKObject::RaisePropertyChange(propertyName);
}
