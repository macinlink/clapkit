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
#include <cstring>
#include <vector>

struct CKWindowInitParams {
		/** Initial size of the window */
		CKSize size = CKSize(0, 0);
		/** Provide a title for your window (optional) */
		const char* title = 0;
		/** Provide an initial position for your window (optional - if null, will be centered) */
		CKPoint* point = nullptr;
		/** Allows window to be closed by the user. */
		bool closable = true;
		/** True if modal dialog */
		bool modal = false;

		CKWindowInitParams(int w, int h, const char* title = nullptr, bool closable = true, bool modal = false) {
			this->size = CKSize(w, h);
			CKSafeCopyString((char**)&this->title, title);
		}
};

class CKWindow : public CKObject {

	public:
		CKWindow(CKWindowInitParams params);
		~CKWindow();

		void SetTitle(const char* title);
		char* GetTitle();

		void Show();
		void Hide();
		bool IsVisible();
		void Focus();

		CKRect* GetRect(bool getCopy = false);
		void Move(int x, int y);
		void Resize(int width, int height);
		void Center();
		void Close();

		bool AddControl(CKControl* control);
		void RemoveControl(CKControl* control, bool free);
		void Redraw(CKRect rectToRedraw);

		void SetOwner(CKApp* owner);
		CKControl* FindControl(CKPoint point);
		bool ContainsControl(CKControl* control);
		void SetActiveControl(CKControl* control);

		void SetIsActive(bool active);

		virtual bool HandleEvent(const CKEvent& evt);

	public:
		CKWindowPtr __windowPtr;

		/**
		 * @brief Contains the last control the user has been pushing down on.
		 */
		CKControl* latestDownControl;

		/**
		 * @brief Contains the active text input (textarea, textbox, etc..) control.
		 */
		CKControl* activeTextInputControl;

		/**
		 * @brief True if we should receive mouseMove events.
		 * We are storing this as a hack to speed things up as HasHandler is pretty slow.
		 */
		bool shouldReceiveMouseMoveEvents;

	private:
		CKRect* __rect;
		std::vector<CKControl*> __controls;
		CKApp* __owner;
		bool __visible = false;
		bool __dead = false;
};