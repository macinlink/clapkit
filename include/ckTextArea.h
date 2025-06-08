/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTextArea
 * ----------------------------------------------------------------------
 * Defines a large text area, with or without scrollbars and editing enabled.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckTextField.h"
#include <TextEdit.h>

class CKTextArea : public CKTextField {

	public:
		CKTextArea(const CKControlInitParams& params);
		~CKTextArea();

		virtual void PrepareForDraw();
		virtual void Redraw();

		virtual void RaisePropertyChange(const char* propertyName);
		virtual bool HandleEvent(const CKEvent& evt);

		/**
		 * @brief This is only to be called from the UPPs for the scrollbar.
		 */
		virtual void __UpdateTextScroll();

		/**
		 * @brief This is only to be called from the UPPs for the scrollbar.
		 */
		virtual int __GetLineHeight() {
			if (!this->__teHandle) {
				return 0;
			}
			HLock((Handle)this->__teHandle);
			// TODO: Instead of hard-coding first line,
			// maybe we should determine what line is visible on top
			// and use that.
			int oneLinePixels = TEGetHeight(1, 0, this->__teHandle);
			HUnlock((Handle)this->__teHandle);
			return oneLinePixels;
		};

		/**
		 * @brief This is only to be called from the UPPs for the scrollbar.
		 */
		virtual int __GetPageHeight() {
			if (!this->__teHandle) {
				return 0;
			}
			HLock((Handle)this->__teHandle);
			TEPtr te = *(this->__teHandle);
			int onePagePixels = te->viewRect.bottom - te->viewRect.top;
			HUnlock((Handle)this->__teHandle);
			return onePagePixels;
		};

	protected:
		virtual void ResizeTE();
		virtual void TECreated();

	private:
		virtual void __SetupScrollbars(bool removeOnly = false);
		virtual void __HandleScrollBarClick(ControlHandle ctl, CKPoint point);

	protected:
		bool focused = false;
		ControlHandle __vScrollBar = nullptr;
		ControlHandle __hScrollBar = nullptr;
		bool __drewChrome = false;

	public:
		CKProperty<CKScrollType> scrollType;
};