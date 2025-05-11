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
#include <functional>

class CKWindow;

/**
 * Initialization parameters for a CKControl.
 */
struct CKControlInitParams {
		const char* title = 0;
		int x = 0;
		int y = 0;
		int width = 100;
		int height = 50;
};

class CKControl : public CKObject {

	public:
		CKControl(const CKControlInitParams& params, CKControlType type = CKControlType::Unknown);
		virtual ~CKControl();
		virtual void Show();
		virtual void Hide();
		virtual void AddedToWindow(CKWindow* window);
		virtual void RemovedFromWindow();
		virtual void Redraw();
		virtual void MarkAsDirty();
		virtual CKRect* GetRect(bool getCopy = false);
		virtual void SetRect(CKRect* rect);
		virtual void SetEnabled(bool enabled);
		virtual bool GetEnabled();
		virtual bool HandleEvent(CKControlEvent evt);
		virtual bool GetVisible();

		virtual void AddHandler(CKControlEventType type, std::function<void(CKControl*, CKControlEvent)> callback);
		virtual void RemoveHandler(CKControlEventType type);
		virtual CKHandlerContainer* HasHandler(CKControlEventType type);

		virtual void SetIsFocused(bool focused);

	private:
	public:
		CKWindow* owner;

	protected:
		CKRect* __rect;
		bool __enabled;
		bool __visible;
		bool __focused;
		std::vector<CKHandlerContainer*> __handlers;
};