/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTextField
 * ----------------------------------------------------------------------
 * Defines an editable, one-line text field.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckLabel.h"
#include "ck_pFocusableControl.h"

class CKTextField : public CKLabel, public CKFocusableControl {

	public:
		CKTextField(const CKControlInitParams& params);
		virtual ~CKTextField();
		virtual void Redraw();
		virtual void Blurred();
		virtual void Focused();
		virtual void PrepareForDraw();
		virtual bool HandleEvent(const CKEvent& evt);

	protected:
		virtual void TECreated();

	protected:
		bool focused = false;
};