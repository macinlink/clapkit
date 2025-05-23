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

#pragma once

#include "ckApp.h"
#include "ckControlToolbox.h"
#include "ck_pValueContainingControl.h"

class CKCheckbox : public CKControlToolbox, public CKValueContainingControl {

	public:
		CKCheckbox(const CKControlInitParams& params);
		virtual ~CKCheckbox();
		virtual bool HandleEvent(const CKEvent& evt);
		virtual void ReflectToOS();
};