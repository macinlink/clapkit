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
		CKControlInitParams(const char* t, CKRect r) {
			title = t;
			x = r.origin->x;
			y = r.origin->y;
			width = r.size->width;
			height = r.size->height;
		}
		CKControlInitParams(CKRect r) {
			x = r.origin->x;
			y = r.origin->y;
			width = r.size->width;
			height = r.size->height;
		}
		CKControlInitParams(CKSize s) {
			width = s.width;
			height = s.height;
		}
};

class CKControl : public CKObject {

	public:
		CKControl(const CKControlInitParams& params, CKControlType type = CKControlType::Unknown);
		virtual ~CKControl();
		virtual void AddedToWindow(CKWindow* window);
		virtual void RemovedFromWindow();
		virtual void MarkAsDirty();
		virtual void RaisePropertyChange(const char* propertyName);

	protected:
		friend class CKWindow;
		virtual void Redraw();

	public:
		CKProperty<CKWindow*> owner = nullptr;
		CKProperty<CKRect> rect;
		CKProperty<bool> enabled = true;
		CKProperty<bool> visible;

	private:
		CKRect __lastRect = CKRect(0, 0, 0, 0);
};