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