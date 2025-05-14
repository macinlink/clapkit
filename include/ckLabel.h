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
		void AutoHeight(int maxHeight = 0);

		void SetFont(short fontId);
		short GetFont();

	protected:
		// Called once TE is created so you can override things in your class.
		virtual void TECreated();

	public:
		bool bold = false;
		bool italic = false;
		bool underline = false;
		bool multiline = false;
		CKColor color;
		int fontSize;
		CKTextJustification justification;
		TEHandle __teHandle;

	protected:
		short __fontNumber;
};