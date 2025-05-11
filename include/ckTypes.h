/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTypes
 * ----------------------------------------------------------------------
 * App-wide type definitions.
 *
 */

#pragma once

#include "ckApp.h"
#include "ckMacros.h"
#include "ckUtils.h"

#define CKError		 int

#define CKWindowPtr	 WindowPtr
#define CKControlPtr ControlRef
#define CKRGBColor	 RGBColor

#define QD_BOLD		 1
#define QD_ITALIC	 2
#define QD_UNDERLINE 4
#define QD_OUTLINE	 8

/**
 * We throw this and subclasses of this when we fail.
 */
class CKException : public std::exception {
	public:
		const char* message;
		CKException(const char* msg)
			: message(msg) {}
		const char* what() {
			return this->message;
		}
};

/**
 * Defines a control type (i.e. PushButton, Checkbox, etc.)
 */
enum class CKControlType {
	Unknown = 0,
	PushButton = 1,
	Label = 2,
	Checkbox = 3,
	Canvas = 4,
	TextField = 5,
};

/**
 * Abstraction for mouse buttons.
 */
enum class CKMouseButton {
	None = 0,
	Left = 1,
	Middle = 2,
	Right = 3
};

/**
 * @brief Used for text labels, etc.
 */
enum class CKTextJustification {
	Left = 0,
	Center = 1,
	Right = 2
};

/**
 * @brief Defines an RGB color. A is usually not used in our calse.
 */
struct CKColor {
		u_int8_t r = 0;
		u_int8_t g = 0;
		u_int8_t b = 0;
		u_int8_t a = 255;

		RGBColor ToOS() {
			RGBColor c;
			c.blue = b;
			c.green = g;
			c.red = r;
			return c;
		}
};

/**
 * Defines a point on the screen.
 * Use ToOS/FromOS to interact with native version(s) of this.
 */
struct CKPoint {

		int x;
		int y;

		CKPoint(int x, int y)
			: x(x), y(y) {};
		CKPoint()
			: x(0), y(0) {};

		/**
		 * Convert our Rect to what the OS expects.
		 */
		Point ToOS() {
			Point p;
			p.h = x;
			p.v = y;
			return p;
		}

		/**
		 * Convert our Rect to what the OS expects.
		 */
		Point* ToOSPtr() {
			Point* p = (Point*)CKMalloc(sizeof(*p));
			p->h = x;
			p->v = y;
			return p;
		}

		/**
		 * Convert OS-rect to what we use.
		 */
		static CKPoint FromOS(Point p) {
			CKPoint toReturn = {p.h, p.v};
			return toReturn;
		}
};

/**
 * Defines a Rectangular area.
 * Use ToOS/FromOS to interact with native version(s) of this.
 */
struct CKRect {

		int x;
		int y;
		int width;
		int height;

		CKRect(int x, int y, int w, int h)
			: x(x), y(y), width(w), height(h) {};
		CKRect(int w, int h)
			: x(0), y(0), width(w), height(h) {};
		CKRect()
			: x(0), y(0), width(0), height(0) {};

		/**
		 * Convert our Rect to what the OS expects.
		 */
		Rect ToOS() {
			Rect r;
			r.left = x;
			r.right = x + width;
			r.top = y;
			r.bottom = y + height;
			return r;
		}

		/**
		 * Same as ToOS but returns a Ptr.
		 */
		Rect* ToOSPtr() {
			Rect* toReturn = (Rect*)CKMalloc((sizeof(*toReturn)));
			toReturn->left = x;
			toReturn->right = x + width;
			toReturn->top = y;
			toReturn->bottom = y + height;
			return toReturn;
		}

		/**
		 * Convert OS-rect to what we use.
		 */
		static CKRect FromOS(Rect r) {
			CKRect toReturn;
			toReturn.x = r.left;
			toReturn.y = r.top;
			toReturn.width = r.right - r.left;
			toReturn.height = r.bottom - r.top;
			return toReturn;
		}

		static CKRect copy(CKRect r) {
			CKRect toReturn;
			toReturn.x = r.x;
			toReturn.y = r.y;
			toReturn.width = r.width;
			toReturn.height = r.height;
			return toReturn;
		}

		/**
		 * Is the CKPoint inside us?
		 */
		bool intersectsPoint(CKPoint p) {
			if (p.x >= this->x && p.x <= this->x + this->width) {
				if (p.y >= this->y && p.y <= this->y + this->height) {
					return true;
				}
			}
			return false;
		}
};

/**
 * Defines an event type for controls (and windows.)
 */
enum class CKControlEventType {
	/**
	 * @brief Nothing.
	 */
	nullEvent = 0,

	/**
	 * @brief User clicked on window/control.
	 */
	click,

	/**
	 * @brief User double-clicked on window/control.
	 */
	doubleClick,

	/**
	 * @brief Mouse button pressed down on window or control.
	 */
	mouseDown,

	/**
	 * @brief Mouse is being dragged while clicking.
	 */
	mouseMove,

	/**
	 * @brief Mouse button released.
	 */
	mouseUp,

	/**
	 * @brief Keyboard key pressed.
	 */
	keyDown,

	/**
	 * @brief Keyboard key released.
	 */
	keyUp,

	/**
	 * @brief (Window-only) Window has moved.
	 */
	moved,

	/**
	 * @brief (Window-only) Window has been resized.
	 */
	resized,

	/**
	 * @brief Window or control has been deleted/released.
	 */
	deleted,

	/**
	 * @brief Window has been closed or control removed from window.
	 */
	removed,

	/**
	 * @brief For textfields, etc where value can be changed.
	 */
	changed,

};

/**
 * Defines an event, handled internally.
 */
struct CKControlEvent {

		CKControlEventType type;
		CKPoint point;
		CKMouseButton mouseButton;
		char key;
		char character;

		CKControlEvent(CKControlEventType type)
			: type(type) {}
		CKControlEvent(CKControlEventType type, CKPoint point)
			: type(type), point(point) {}

		/**
		 * @brief Reserved.
		 */
		void* primaryValue;
		/**
		 * @brief Reserved.
		 */
		void* secondaryValue;
};

/**
 * Event Callback container definition
 */
class CKControl;
class CKWindow;
class CKHandlerContainer {
	public:
		CKControlEventType type;
		std::function<void(CKControl*, CKControlEvent)> callback;
		std::function<void(CKWindow*, CKControlEvent)> callback_window;
};