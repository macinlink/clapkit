/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKCheckbox
 * ----------------------------------------------------------------------
 * Defines a checkbox.
 *
 */

#include "ckCheckbox.h"
#include "ckWindow.h"

CKCheckbox::CKCheckbox(const CKControlInitParams& params)
	: CKControlToolbox(params, CKControlType::Checkbox) {
}

CKCheckbox::~CKCheckbox() {
}

bool CKCheckbox::HandleEvent(const CKEvent& evt) {
	CKControlToolbox::HandleEvent(evt);
	if (evt.type == CKEventType::click) {
		this->SetValue(!this->GetBoolean());
		this->ReflectToOS();
		CKControl::HandleEvent(CKEventType::changed);
	}
	return false;
}

void CKCheckbox::ReflectToOS() {
	CKControlToolbox::ReflectToOS();
	if (this->__ptr) {
		SetControlValue(__ptr, this->GetBoolean() ? 1 : 0);
	}
}