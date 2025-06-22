/*
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

#pragma once

#include "ckApp.h"
#include "ckControlToolbox.h"
#include "ck_pValueContainingControl.h"

/**
 * @ingroup UIControls
 * @brief Defines a checkbox control.
 */
class CKCheckbox : public CKControlToolbox, public CKValueContainingControl {

	public:
		CKCheckbox(const CKControlInitParams& params);
		virtual ~CKCheckbox();
		virtual bool HandleEvent(const CKEvent& evt);
		virtual void __ReflectToOS();

		// TODO: A better way of doing these?

		virtual void SetValue(const char* value) {
			CKValueContainingControl::SetValue(value);
			this->RaisePropertyChange("value");
		}

		virtual void SetValue(bool value) {
			CKValueContainingControl::SetValue(value);
			this->RaisePropertyChange("value");
		}

		virtual void SetValue(void* value) {
			CKValueContainingControl::SetValue(value);
			this->RaisePropertyChange("value");
		}

	protected:
		CKCheckbox(const CKControlInitParams& params, CKControlType forcedType);
};