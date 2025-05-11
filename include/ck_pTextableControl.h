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

#pragma once

class CKTextableControl {

	public:
		CKTextableControl();
		~CKTextableControl();
		virtual void SetText(const char* text);
		virtual const char* GetText();

	protected:
		char* __text;
};