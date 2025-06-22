/*
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
 * @ingroup UIControls
 * @brief Initialization parameters for a CKControl.
 */
struct CKControlInitParams {
	public:
		/**
		 * @brief Text to be used for window title or control text, if applicable.
		 */
		const char* title = 0;

		/**
		 * @brief Position (X) of the control. Should be > 0.
		 */
		int x = 0;

		/**
		 * @brief Position (Y) of the control. Should be > 0.
		 */
		int y = 0;

		/**
		 * @brief Width of the control. Must be > 0.
		 */
		int width = 100;

		/**
		 * @brief Height of the control. Must be > 0. See @ref kCKButtonHeight, @ref kCKCheckboxHeight, etc.
		 */
		int height = 50;
		/**
		 * @brief Create a control with a text and a size & position
		 * @param t The text of the control.
		 * @param r A CKRect defining the size and position of the control.
		 */
		CKControlInitParams(const char* t, CKRect r) {
			title = t;
			x = r.origin->x;
			y = r.origin->y;
			width = r.size->width;
			height = r.size->height;
		}
		/**
		 * @brief Create a control with a size & position
		 * @param r A CKRect defining the size and position of the control.
		 */
		CKControlInitParams(CKRect r) {
			x = r.origin->x;
			y = r.origin->y;
			width = r.size->width;
			height = r.size->height;
		}
		/**
		 * @brief Create a control with a size
		 * @param r A CKSize defining the size of the control.
		 */
		CKControlInitParams(CKSize s) {
			width = s.width;
			height = s.height;
		}
};

/**
 * @ingroup UIControls
 * @brief Defines the base of all UI Controls.
 */
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