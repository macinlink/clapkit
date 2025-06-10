/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKLabel
 * ----------------------------------------------------------------------
 * Defines a static label.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckControl.h"
#include "ck_pTextableControl.h"

class CKLabel : public CKControl, public CKTextableControl {

	public:
		CKLabel(const CKControlInitParams& params);
		virtual ~CKLabel();

		virtual void AddedToWindow(CKWindow* window);
		virtual void RemovedFromWindow();

		virtual void PrepareForDraw();
		virtual void Redraw();
		virtual void SetText(const char* text);
		virtual const char* GetText();
		void AutoHeight(int maxHeight = 0);

		void SetFont(short fontId);
		short GetFont();

		void DoTEIdle() {
			TEIdle(this->__teHandle);
		}

		virtual void RaisePropertyChange(const char* propertyName);

	protected:
		// Called once TE is created so you can override things in your class.
		virtual void TECreated();
		virtual void ResizeTE();

	public:
		CKProperty<bool> bold = false;
		CKProperty<bool> italic = false;
		CKProperty<bool> underline = false;
		CKProperty<bool> multiline = false;
		CKProperty<CKColor> color;
		CKProperty<int> fontSize;
		CKProperty<CKTextJustification> justification;

	protected:
		short __fontNumber;
		TEHandle __teHandle = nullptr;
		char* __tempTextStorage = nullptr;
		bool __needsPreparing = true;
};