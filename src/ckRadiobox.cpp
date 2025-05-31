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
	: CKCheckbox(params, CKControlType::RadioButton) {}

CKRadiobox::~CKRadiobox() {
}

bool CKRadiobox::HandleEvent(const CKEvent& evt) {

	CKControlToolbox::HandleEvent(evt);

	if (evt.type == CKEventType::click) {
		if (!this->GetBoolean()) {
			this->SetValue(true);
		}
		if (this->groupID != 0 && this->owner) {
			auto buttons = this->owner->GetControlsOfType<CKRadiobox>();
			for (auto& b : buttons) {
				if (b->groupID == this->groupID && b != this) {
					b->SetValue(false);
				}
			}
		}
		this->__ReflectToOS();
		CKControl::HandleEvent(CKEventType::changed);
	}

	return false;
}