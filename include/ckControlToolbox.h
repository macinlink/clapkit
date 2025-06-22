/*
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
#include "ckPlatform.h"
#include "ck_pTextableControl.h"

class CKWindow;

/**
 * @ingroup UIControls
 * @brief Defines the base of all UIControls that are managed by Macintosh Toolbox.
 */
class CKControlToolbox : public CKControl, public CKTextableControl {

	public:
		CKControlToolbox(const CKControlInitParams& params, CKControlType type = CKControlType::Unknown);
		virtual ~CKControlToolbox();
		virtual void AddedToWindow(CKWindow* window);
		virtual void Redraw();
		virtual bool HandleEvent(const CKEvent& evt);
		virtual void RaisePropertyChange(const char* propertyName);

	protected:
		virtual void __ReflectToOS();

	protected:
		CKControlPtr __ptr;
		CKControlType __type;
};