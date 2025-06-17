/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKDropdown
 * ----------------------------------------------------------------------
 * Defines a dropdown list control.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckControlToolbox.h"
#include "ck_pValueContainingControl.h"

class CKDropdown : public CKControlToolbox, public CKValueContainingControl {

	public:
		CKDropdown(const CKControlInitParams& params);
		virtual ~CKDropdown();
		virtual void AddedToWindow(CKWindow* window);
		virtual bool HandleEvent(const CKEvent& evt);
		virtual void RaisePropertyChange(const char* propertyName);

	protected:
		CKDropdown(const CKControlInitParams& params, CKControlType forcedType);

	private:
		virtual void __ReflectToOS();

	public:
		CKProperty<int> labelWidth = -1;
		CKProperty<std::vector<const char*>> items;
		CKProperty<int> selectedIndex = -1;

	protected:
		MenuID __menuId;
		MenuHandle __menu = nullptr;
		bool __rebuildMenu = false;
		int __lastRaisedSelectedIndex = -1;
};