/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKControlOS
 * ----------------------------------------------------------------------
 * The base of controls that are mostly handled by the OS.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckControl.h"
#include "ck_pTextableControl.h"

class CKWindow;

class CKControlToolbox : public CKControl, public CKTextableControl {

	public:
		CKControlToolbox(const CKControlInitParams& params, CKControlType type = CKControlType::Unknown);
		virtual ~CKControlToolbox();
		virtual void AddedToWindow(CKWindow* window);
		virtual void Show();
		virtual void Hide();
		virtual void Redraw();
		virtual void SetEnabled(bool enabled);
		virtual bool HandleEvent(CKControlEvent evt);
		virtual void SetRect(CKRect* rect);
		void SetToggleValue(bool value);
		bool GetToggleValue();

		void SetText(const char* text);

	private:
	public:
	protected:
		CKControlPtr __ptr;
		CKControlType __type;
		bool toggleValue;
};