/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKWindow
 * ----------------------------------------------------------------------
 * Defines a window.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckControl.h"
#include "ckObject.h"
#include <MacWindows.h>
#include <cstring>
#include <vector>

struct CKWindowInitParams {
		/** Initial size of the window */
		CKSize size = CKSize(0, 0);
		/** Provide a title for your window (optional) */
		char* title = 0;
		/** Provide an initial position for your window (optional - if null, will be centered) */
		CKPoint* point = nullptr;
		/** Allows window to be closed by the user. */
		bool closable = true;
		/** True if modal dialog */
		bool modal = false;

		CKWindowInitParams(int w, int h, const char* _title = nullptr, bool _closable = true, bool _modal = false) {
			this->size = CKSize(w, h);
			this->closable = _closable;
			this->modal = _modal;
			CKSafeCopyString(this->title, _title);
		}
};

class CKWindow : public CKObject {

	public:
		CKWindow(CKWindowInitParams params);
		virtual ~CKWindow();
		void Loop();

		void SetTitle(const char* title);
		char* GetTitle();

		void Show();
		void Hide();
		void Focus();
		void Center();
		void Close();

		bool AddControl(CKControl* control);
		void RemoveControl(CKControl* control, bool free);
		void Redraw(CKRect rectToRedraw);

		void SetOwner(CKApp* owner);
		CKApp* GetOwner();

		CKControl* FindControl(CKPoint point);
		bool ContainsControl(CKControl* control);
		void SetActiveControl(CKControl* control);

		virtual bool HandleEvent(const CKEvent& evt);

		CKControl* GetLastControl() const;
		void SetLastControl(CKControl* control);

		const CKWindowPtr GetWindowPtr() const;

		void DirtyArea(const CKRect r);

		bool GetIsActive();

	private:
		void __InvalidateEntireWindow();

	protected:
		friend class CKApp;
		void SetIsActive(bool active);
		void __ReflectToOS();
		virtual void RaisePropertyChange(const char* propertyName);

	public:
		CKProperty<CKRect> rect;
		CKProperty<bool> visible;
		CKProperty<bool> hasCustomBackgroundColor;
		CKProperty<CKColor> backgroundColor = CKColor(255, 255, 255);

		/**
		 * @brief True if we should receive mouseMove events.
		 * We are storing this as a hack to speed things up as HasHandler is pretty slow.
		 */
		bool shouldReceiveMouseMoveEvents;

	private:
		CKApp* __owner;
		std::vector<CKControl*> __controls;
		CKWindowPtr __windowPtr = nullptr;
		CKControl* __activeTextInputControl = nullptr;
		CKControl* __lastDownControl = nullptr;
		bool __dead = false;
		bool __isCurrentlyActive = false;
};