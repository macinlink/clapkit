/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKControl
 * ----------------------------------------------------------------------
 * The base of everything UI Control we have.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckObject.h"
#include "ckProperty.h"
#include <functional>

class CKWindow;

/**
 * Initialization parameters for a CKControl.
 */
struct CKControlInitParams {
	public:
		const char* title = 0;
		int x = 0;
		int y = 0;
		int width = 100;
		int height = 50;
		CKControlInitParams(const char* t, int xx, int yy, int w, int h)
			: title(t), x(xx), y(yy), width(w), height(h) {}
		CKControlInitParams(int w, int h)
			: width(w), height(h) {}
};

class CKControl : public CKObject {

	public:
		CKControl(const CKControlInitParams& params, CKControlType type = CKControlType::Unknown);
		virtual ~CKControl();
		virtual void AddedToWindow(CKWindow* window);
		virtual void RemovedFromWindow();
		virtual void Redraw();
		virtual void MarkAsDirty();
		virtual void RaisePropertyChange(const char* propertyName);

	public:
		CKProperty<CKWindow*> owner = nullptr;
		CKProperty<CKRect> rect;
		CKProperty<bool> enabled = true;
		CKProperty<bool> visible;
};