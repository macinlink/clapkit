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

#include "ckMacros.h"
#include "ckUtils.h"
#include <exception>

#define CKError			uint32_t
#define CKPass			1

#define QD_BOLD			1
#define QD_ITALIC		2
#define QD_UNDERLINE	4

#define CKTextBold		QD_BOLD
#define CKTextItalic	QD_ITALIC
#define CKTextUnderline QD_UNDERLINE

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
	PushButton,
	Label,
	Checkbox,
	RadioButton,
	Canvas,
	TextField,
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

		CKColor(u_int8_t red, u_int8_t green, u_int8_t blue)
			: r(red), g(green), b(blue) {};

		CKColor()
			: r(0), g(0), b() {};

		RGBColor ToOS() {
			RGBColor toReturn;
			toReturn.red = r * 255;
			toReturn.green = g * 255;
			toReturn.blue = b * 255;
			return toReturn;
		}

		bool operator==(const CKColor& other) const {
			return r == other.r && g == other.g && b == other.b;
		}

		bool operator!=(const CKColor& other) const {
			return !(*this == other);
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
		Point ToOS() const {
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
 * @brief Defines a rectangular area.
 */
struct CKSize {

		int width;
		int height;

		CKSize(int w, int h)
			: width(w), height(h) {};

		CKSize()
			: width(0), height(0) {};

		Point ToOS() const {
			Point p;
			p.h = width;
			p.v = height;
			return p;
		}

		/**
		 * Convert OS-rect to what we use.
		 */
		static CKSize FromOS(Point p) {
			CKSize toReturn;
			toReturn.width = p.h;
			toReturn.height = p.v;
			return toReturn;
		}
};

/**
 * @brief Defines a rectangular area at a specific location.
 * Use ToOS/FromOS to interact with native version(s) of this.
 */
struct CKRect {

		CKPoint origin;
		CKSize size;

		CKRect(const CKRect& f) {
			this->origin = f.origin;
			this->size = f.size;
		}

		CKRect(CKPoint o, CKSize s) {
			this->origin = o;
			this->size = s;
		}

		CKRect(int x, int y, int w, int h) {
			this->origin = CKPoint(x, y);
			this->size = CKSize(w, h);
		}

		CKRect(int w, int h) {
			this->origin = CKPoint();
			this->size = CKSize(w, h);
		}

		CKRect() {
			this->origin = CKPoint();
			this->size = CKSize();
		}

		/**
		 * Convert our Rect to what the OS expects.
		 */
		Rect ToOS() const {
			Rect r;
			r.left = this->origin.x;
			r.right = this->origin.x + this->size.width;
			r.top = this->origin.y;
			r.bottom = this->origin.y + this->size.height;
			return r;
		}

		/**
		 * Same as ToOS but returns a Ptr.
		 */
		Rect* ToOSCopy() {
			Rect* toReturn = (Rect*)CKMalloc((sizeof(*toReturn)));
			toReturn->left = this->origin.x;
			toReturn->right = this->origin.x + this->size.width;
			toReturn->top = this->origin.y;
			toReturn->bottom = this->origin.y + this->size.height;
			return toReturn;
		}

		/**
		 * Convert OS-rect to what we use.
		 */
		static CKRect FromOS(Rect r) {
			CKRect toReturn;
			toReturn.origin = CKPoint(r.left, r.top);
			toReturn.size = CKSize(r.right - r.left, r.bottom - r.top);
			return toReturn;
		}

		/**
		 * Is the CKPoint inside us?
		 */
		bool intersectsPoint(CKPoint p) {
			if (p.x >= this->origin.x && p.x <= this->origin.x + this->size.width) {
				if (p.y >= this->origin.y && p.y <= this->origin.y + this->size.height) {
					return true;
				}
			}
			return false;
		}
};

/**
 * Defines an event type for controls (and windows.)
 */
enum class CKEventType {
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
class CKWindow;
class CKControl;
struct CKEvent {

		CKEventType type;
		CKPoint point;
		CKMouseButton mouseButton;
		char key;
		char character;

		CKWindow* window = nullptr;	  // always set
		CKControl* control = nullptr; // can be nullptr for window-only events

		CKEvent(CKEventType type)
			: type(type) {}
		CKEvent(CKEventType type, CKPoint point)
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