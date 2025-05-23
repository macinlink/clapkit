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
		virtual void Redraw();
		virtual bool HandleEvent(const CKEvent& evt);
		virtual void ReflectToOS();
		virtual void RaisePropertyChange(const char* propertyName);

	protected:
		CKControlPtr __ptr;
		CKControlType __type;
};