/*
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

/**
 * @ingroup Types
 * @brief Defines the type of the window.
 */
enum class CKWindowType {
	Standard = 0,
	StandardResizable = 1,
	Modal = 2,
	Floating = 3,
};

class CKFocusableControl;

/**
 * @ingroup Types
 * @brief Initialization parameters for a CKWindow.
 * @code
 * CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 100)));
 * window->SetTitle("My Window!");
 * @endcode
 */
struct CKWindowInitParams {
	public:
		CKSize size = CKSize(0, 0);
		std::optional<CKPoint> origin;
		CKWindowType type = CKWindowType::Standard;
		char* title = nullptr;

	public:
		/**
		 * @brief Initialization parameters for a CKWindow. You must set a size larger than 0x0. Title/type can be set later on.
		 * @code
		 * CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 100)));
		 * window->SetTitle("My Window!");
		 * @endcode
		 */
		CKWindowInitParams(CKSize size) {
			this->size = size;
		}

		CKWindowInitParams& SetTitle(const char* title) {
			CKSafeCopyString(this->title, title);
			return *this;
		}

		CKWindowInitParams& SetType(CKWindowType type) {
			this->type = type;
			return *this;
		}

		CKWindowInitParams& SetOrigin(CKPoint point) {
			this->origin = point;
			return *this;
		}

		CKWindowInitParams& UnsetOrigin() {
			this->origin.reset();
			return *this;
		}
};

/**
 * @ingroup UIControls
 * @brief Defines a window.
 * Window type (modal, document) is determined by CKWindowInitParams and CKWindowType
 */
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
		const std::vector<CKControl*>& GetControls() const;

		/**
		 * @brief Get the list of controls of type T in this window.
		 * @code
		 * auto buttons = myWindow->GetControlsOfType<CKButton>();
		 * @endcode
		 * @return
		 */
		template <typename T>
		std::vector<T*> GetControlsOfType() const {

			std::vector<T*> out;
			out.reserve(__controls.size());
			for (auto* c : __controls) {
				if (auto* t = dynamic_cast<T*>(c)) {
					out.push_back(t);
				}
			}
			return out;
		}

		bool ContainsControl(CKControl* control);
		CKFocusableControl* GetActiveControl();
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
		CKProperty<bool> closable = true;

		int minimumWidth = 0;
		int minimumHeight = 0;
		int maximumWidth = 1000;
		int maximumHeight = 1000;

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
		CKWindowType __type;
		std::vector<CKRect> __controlDirtifiedAreas;
};