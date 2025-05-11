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
#include "ckMacros.h"

CKTextableControl::CKTextableControl() {

	this->__text = 0;
}

CKTextableControl::~CKTextableControl() {

	CK_SAFE_COPY_STRING(this->__text, 0);
}

void CKTextableControl::SetText(const char* text) {

	CK_SAFE_COPY_STRING(this->__text, text);
}

const char* CKTextableControl::GetText() {

	return this->__text;
}