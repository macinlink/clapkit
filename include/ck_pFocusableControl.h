/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CK_pFocusableControl
 * ----------------------------------------------------------------------
 * Describes a control that the user can 'focus' - textfield, etc..
 * This is is not a class you want to be the base of anything, use
 * in addition to a CKControl.
 *
 */

#pragma once
#include "ckProperty.h"

class CKFocusableControl {

	public:
		CKFocusableControl();
		virtual ~CKFocusableControl();
		virtual void Focused() {};
		virtual void Blurred() {};
		virtual void PerformUndo() {};
		virtual void PerformCut() {};
		virtual void PerformCopy() {};
		virtual void PerformPaste() {};
		virtual void PerformClear() {};

	public:
		CKProperty<bool> focused;
		const bool undoable = false;
		const bool copyable = true;
		const bool pastable = true;
		const bool cutable = true;
		const bool clearable = true;
};