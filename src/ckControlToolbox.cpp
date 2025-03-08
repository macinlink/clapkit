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

CKControlToolbox::CKControlToolbox(const CKControlInitParams& params, CKControlType type): CKControl(params, type) {

    this->owner = nil;
    this->__ptr = 0;
    this->__type = type;

    this->toggleValue = false;
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

    Rect* r = this->__rect->ToOSPtr();
    unsigned char* title = CKC2P(this->__text);

    switch (this->__type) {
        case CKControlType::PushButton:
            this->__ptr = NewControl(this->owner->__windowPtr, r, title, false, 0, 0, 0, 0 /* button */, 0);
            break;
        case CKControlType::Checkbox:
            this->__ptr = NewControl(this->owner->__windowPtr, r, title, false, 0, 0, 1, 1 /* checkbox */, 0);
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
    CKFree(r);

    if (this->GetVisible()) {
        this->Show();
    }

}

void CKControlToolbox::Show() {

    if (this->__ptr == 0) {
        throw CKNew CKException("Show called on control with no ptr!");
    }

    if ((**(this->__ptr)).contrlOwner == 0) {
        CKLog("Show called on control but contrlOwner is nil");
        return;
    }

    CKControl::Show();
    ShowControl(this->__ptr);

}

void CKControlToolbox::Hide() {
    
    CKControl::Hide();

    if (this->__ptr == 0) {
        throw CKNew CKException("Hide called on control with no ptr!");
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

    Draw1Control(this->__ptr);
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

bool CKControlToolbox::HandleEvent(CKControlEvent evt) {

    if (CKControl::HandleEvent(evt)) {
        // Already handled, stop here.
        return true;
    }

    if (evt.type == CKControlEventType::mouseDown) {
        bool didClick = false;
        if (TrackControl(this->__ptr, evt.point.ToOS(), 0)) {
            didClick = true;
        }
        if (didClick) {
            // TODO: The line below is kind of stupid and should be moved somewhere else.
            this->SetToggleValue(!this->GetToggleValue()); // For radio & checkboxes..
            this->MarkAsDirty();
            this->HandleEvent(CKControlEvent(CKControlEventType::click));
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

    SizeControl(this->__ptr, rect->width, rect->height);
    MoveControl(this->__ptr, rect->x, rect->y);
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

    this->toggleValue = value;

    if (!this->__ptr) {
        return;
    }

    if (this->__type == CKControlType::Checkbox) {
        SetControlValue(this->__ptr, value);
    }

    CKControlEvent e = CKControlEvent(CKControlEventType::changed);
    this->HandleEvent(e);

}

bool CKControlToolbox::GetToggleValue() {

    return this->toggleValue;

}