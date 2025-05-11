/**
 * 
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 * 
 * CKButton
 * ----------------------------------------------------------------------
 * Defines a push-button.
 * 
*/

#include "ckButton.h"
#include "ckWindow.h"

CKButton::CKButton(const CKControlInitParams& params): CKControlToolbox(params, CKControlType::PushButton) {


}

CKButton::~CKButton() {


}

void CKButton::SetDefault(bool isDefault) {
    this->__is_default = isDefault;
}

void CKButton::Redraw() {

    CKControlToolbox::Redraw();

    if (!this->__is_default) {
        return;
    }

    Rect* r = this->GetRect()->ToOSPtr();
    PenSize(3, 3);
    InsetRect(r, -4, -4);
    FrameRoundRect(r, 16, 16);
    PenSize(1, 1);
    CKFree(r);


}