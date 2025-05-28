/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTextableControl
 * ----------------------------------------------------------------------
 * Describes a control where you can set a text value.
 * This is is not a class you want to be the base of anything, use
 * in addition to a CKControl.
 *
 */

#include "ck_pTextableControl.h"
#include "ckControl.h"
#include "ckMacros.h"

CKTextableControl::CKTextableControl() {

	this->__text = 0;
}

CKTextableControl::~CKTextableControl() {

	CKSafeCopyString(this->__text, 0);
}

void CKTextableControl::SetText(const char* text) {

	CKSafeCopyString(this->__text, text);
	if (auto c = dynamic_cast<CKControl*>(this)) {
		c->RaisePropertyChange("text");
	}
}

const char* CKTextableControl::GetText() {

	return this->__text;
}