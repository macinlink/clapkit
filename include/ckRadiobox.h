/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKCheckbox
 * ----------------------------------------------------------------------
 * Defines a radiobox.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckCheckbox.h"

class CKRadiobox : public CKCheckbox {

	public:
		CKRadiobox(const CKControlInitParams& params);
		virtual ~CKRadiobox();
		virtual bool HandleEvent(const CKEvent& evt);

	public:
		uint32_t groupID = 0;
};