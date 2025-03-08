/**
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

CKControl::CKControl(const CKControlInitParams& params, CKControlType type): CKObject() {

    this->owner = nil;
    this->__enabled = true;
    this->__visible = true;
    this->__handlers = std::vector<CKHandlerContainer*>();
    this->__rect = 0;

    CKRect* r = CKNew CKRect(params.x, params.y, params.width, params.height);
    this->SetRect(r);
    CKDelete(r);

}

CKControl::~CKControl() {

    CKControlEvent evt = CKControlEvent(CKControlEventType::deleted);
    this->HandleEvent(evt);
    
    if (this->__rect) {
        CKDelete(this->__rect);
        this->__rect = nullptr;
    }

    while (this->__handlers.size() > 0) {
        this->RemoveHandler(this->__handlers.at(0)->type);
    }

}

/**
 * If not shown, show and draw the control.
*/
void CKControl::Show() {

    this->__visible = true;

}

/**
 * Make the control invisible.
*/
void CKControl::Hide() {

    this->__visible = false;
    
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
    
    CKControlEvent evt = CKControlEvent(CKControlEventType::removed);
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

    GrafPtr oldPort;
    GetPort(&oldPort);
    SetPort(this->owner->__windowPtr);
    Rect* r = this->GetRect()->ToOSPtr();
    InvalRect(r);
    SetPort(oldPort);
    CKFree(r);
    
}

/**
 * pwnd lol ya
*/
CKRect* CKControl::GetRect(bool getCopy) {
    
    if (getCopy) {
        CKRect* r = CKNew CKRect(this->__rect->x, this->__rect->y, this->__rect->width, this->__rect->height);
        return r;
    } else {
        return this->__rect;
    }

}

/**
 * Change the position/size of the control.
*/
void CKControl::SetRect(CKRect* rect) {

    CKPROFILE

    if (this->__rect) {
        this->MarkAsDirty();
        CKDelete(this->__rect);
        this->__rect = 0;
    }

    if (rect) {
        this->__rect = CKNew CKRect(rect->x, rect->y, rect->width, rect->height);
        this->MarkAsDirty();
    }

}

/**
 * Disable/enable control.
*/
void CKControl::SetEnabled(bool enabled) {

    this->__enabled = enabled;

}

/**
 * Returns true if control is enabled.
*/
bool CKControl::GetEnabled() {

    return this->__enabled;

}

/**
 * Called by CKApp when the user interacts with
 * our control. Override for custom controls.
 * 
 * Returns true if handled.
*/
bool CKControl::HandleEvent(CKControlEvent evt) {

    CKHandlerContainer* handler = this->HasHandler(evt.type);
    if (handler) {
        handler->callback(this, evt);
    }

    return false;

}

/**
 * @brief Is this control visible?
 * @return True if visible.
 */
bool CKControl::GetVisible() {

    return this->__visible;

}

/**
 * Add/replace a handler for an event type.
*/
void CKControl::AddHandler(CKControlEventType type, std::function<void(CKControl*, CKControlEvent)> callback) {

    CKPROFILE

    CKHandlerContainer* cbc = CKNew CKHandlerContainer();
    cbc->type = type;
    cbc->callback = callback;

    if (this->HasHandler(type)) {
        CKLog("Found a handler for type %d, replacing it.", type);
        this->RemoveHandler(type);
    }

    this->__handlers.push_back(cbc);

}

/**
 * Remove - if it's there - an event handler.
*/
void CKControl::RemoveHandler(CKControlEventType type) {

    CKPROFILE

    for (auto it = this->__handlers.begin(); it != this->__handlers.end(); ++it) {
        if ((*it)->type == type) {
            CKDelete(*it);
            this->__handlers.erase(it);
            break;
        }
    }

}

/**
 * Returns if we have a handler for this type.
*/
CKHandlerContainer* CKControl::HasHandler(CKControlEventType type) {

    CKPROFILE

    for (auto& h : this->__handlers) {
        if (h->type == type) {
            return h;
        }
    }

    return nil;

}

/**
 * @brief Called by the window if this control is the 'focused' one.
 * Used for making a textfield active, etc..
 * @param focused
 */
void CKControl::SetIsFocused(bool focused) {
    // Nothing here, override me.
}