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

#include "ckRadiobox.h"
#include "ckWindow.h"

CKRadiobox::CKRadiobox(const CKControlInitParams& params)
	: CKCheckbox(params) {
}

CKRadiobox::~CKRadiobox() {
}

void CKRadiobox::__ReflectToOS() {

	CKCheckbox::__ReflectToOS();

	if (this->owner) {
		auto buttons = this->owner->GetControlsOfType<CKRadiobox>();
		for (auto& b : buttons) {
			if (b->groupID == this->groupID && b != this) {
				b->SetValue(false);
			}
		}
	}
}