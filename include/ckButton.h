/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKButton
 * ----------------------------------------------------------------------
 * Defines a push-button.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckControlToolbox.h"

/**
 * @ingroup UIControls
 * @brief Defines a push button
 */
class CKButton : public CKControlToolbox {

	public:
		CKButton(const CKControlInitParams& params);
		virtual ~CKButton();

	public:
		void SetDefault(bool isDefault);
		bool GetDefault() {
			return this->__is_default;
		}
		virtual void Redraw();

	private:
		bool __is_default = false;
};