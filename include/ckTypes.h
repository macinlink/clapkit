/*
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

#include "ckPlatform.h"
#include "ckMacros.h"
#include "ckObject.h"
#include "ckProperty.h"
#include "ckUtils.h"
#include <exception>

/**
 * @ingroup Types
 * @brief Return type for functions that might return an error. Also see: @ref CKPass and @ref CKErrorCode
 */
typedef int32_t CKError;

/**
 * @ingroup Types
 * @brief Functions that return CKError return CKPass on success.
 */
#define CKPass 1

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
 * @ingroup Types
 * @brief Defines a control type (i.e. PushButton, Checkbox, etc.)
 */
enum class CKControlType {
	Unknown = 0,
	PushButton,
	Label,
	Checkbox,
	RadioButton,
	Canvas,
	TextField,
	Dropdown,
};

/**
 * @ingroup Types
 * @brief Abstraction for mouse buttons.
 */
enum class CKMouseButton {
	None = 0,
	Left = 1,
	Middle = 2,
	Right = 3
};

/**
 * @ingroup Types
 * @brief Used for text labels, etc.
 */
enum class CKTextJustification {
	Left = 0,
	Center = 1,
	Right = 2
};

/**
 * @ingroup Types
 * @brief Defines an RGB color. A (Alpha) is usually not used in our case.
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
 * @ingroup Types
 * @brief Defines a point on the screen.
 * @note Use ToOS/FromOS to interact with native version(s) of this.
 */
struct CKPoint {

		CKProperty<int> x = 0;
		CKProperty<int> y = 0;

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

		inline bool operator==(const CKPoint& other) const {
			return this->x == other.x && this->y == other.y;
		}

		inline bool operator!=(const CKPoint& other) const {
			return !(*this == other);
		}

		/** For change tracking */
		void Subscribe(std::function<void()> cb) {
			x.onChange = cb;
			y.onChange = cb;
		}
};

/**
 * @ingroup Types
 * @brief Defines a rectangular area.
 * @note Use ToOS/FromOS to interact with native version(s) of this.
 */
struct CKSize {

		CKProperty<int> width = 0;
		CKProperty<int> height = 0;

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

		inline bool operator==(const CKSize& other) const {
			return this->width == other.width && this->height == other.height;
		}

		inline bool operator!=(const CKSize& other) const {
			return !(*this == other);
		}

		/** For change tracking */
		void Subscribe(std::function<void()> cb) {
			width.onChange = cb;
			height.onChange = cb;
		}
};

/**
 * @ingroup Types
 * @brief Defines a rectangular area at a specific location.
 * @note Use ToOS/FromOS to interact with native version(s) of this.
 */
struct CKRect {

		CKProperty<CKPoint> origin;
		CKProperty<CKSize> size;

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
			r.left = this->origin->x;
			r.right = this->origin->x + this->size->width;
			r.top = this->origin->y;
			r.bottom = this->origin->y + this->size->height;
			return r;
		}

		/**
		 * Same as ToOS but returns a Ptr.
		 */
		Rect* ToOSCopy() {
			Rect* toReturn = (Rect*)CKMalloc((sizeof(*toReturn)));
			toReturn->left = this->origin->x;
			toReturn->right = this->origin->x + this->size->width;
			toReturn->top = this->origin->y;
			toReturn->bottom = this->origin->y + this->size->height;
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
		bool IntersectsPoint(CKPoint p) {
			if (p.x >= this->origin->x && p.x <= this->origin->x + this->size->width) {
				if (p.y >= this->origin->y && p.y <= this->origin->y + this->size->height) {
					return true;
				}
			}
			return false;
		}

		bool IntersectsRect(CKRect r) {
			return (origin->x < (r.origin->x + r.size->width)) &&
				   ((origin->x + size->width) > r.origin->x) &&
				   (origin->y < (r.origin->y + r.size->height)) &&
				   ((origin->y + size->height) > r.origin->y);
		}

		inline bool operator==(const CKRect& other) const {
			return origin->x == other.origin->x &&
				   origin->y == other.origin->y &&
				   size->width == other.size->width &&
				   size->height == other.size->height;
		}

		inline bool operator!=(const CKRect& other) const {
			return !(*this == other);
		}

		/** For change tracking */
		void Subscribe(std::function<void()> cb) {
			origin.onChange = cb;
			size.onChange = cb;
			origin.get().Subscribe(cb);
			size.get().Subscribe(cb);
		}
};

/**
 * @ingroup Types
 * @brief Defines an event type for controls (and windows.)
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

	/**
	 * @brief TCP socket was unable to connect to the specified address.
	 */
	tcpConnectionFailed,

	/**
	 * @brief TCP socket has disconnected.
	 *
	 * Note: This does NOT get called if you close the socket yourself.
	 * This is only for "unexpected" disconnections.
	 */
	tcpDisconnected,

	/**
	 * @brief TCP socket has connected.
	 */
	tcpConnected,

	/**
	 * @brief TCP socket has data to read.
	 */
	tcpReceivedData,

	/**
	 * @brief TCP socket's incoming data buffer became full
	 * and you've missed incoming data - and will be missing data
	 * until you read the buffer.
	 */
	tcpBufferFull,

	/**
	 * @brief TCP socket has an error.
	 */
	tcpError,

	/**
	 * @brief User wants to perform an undo (Cmd + Z)
	 * If no such event handler is installed, Clapkit will try to
	 * perform the task itself if it's a known type.
	 * If you install an event handler, you'll have to do it yourself.
	 * You have to install an event handler for non-standard data types.
	 * Additionally, you can overwrite 'PerformUndo' of your object.
	 */
	dataUndo,

	/**
	 * @brief User wants to perform a cut (Cmd + X)
	 * If no such event handler is installed, Clapkit will try to
	 * perform the task itself if it's a known type.
	 * If you install an event handler, you'll have to do it yourself.
	 * You have to install an event handler for non-standard data types.
	 * Additionally, you can overwrite 'PerformCut' of your object.
	 */
	dataCut,

	/**
	 * @brief User wants to perform a copy (Cmd + C)
	 * If no such event handler is installed, Clapkit will try to
	 * perform the task itself if it's a known type.
	 * If you install an event handler, you'll have to do it yourself.
	 * You have to install an event handler for non-standard data types.
	 * Additionally, you can overwrite 'PerformCopy' of your object.
	 */
	dataCopy,

	/**
	 * @brief User wants to perform a paste (Cmd + V)
	 * If no such event handler is installed, Clapkit will try to
	 * perform the task itself if it's a known type.
	 * If you install an event handler, you'll have to do it yourself.
	 * You have to install an event handler for non-standard data types.
	 * Additionally, you can overwrite 'PerformPaste' of your object.
	 */
	dataPaste,

	/**
	 * @brief User wants to perform a 'clear' -
	 * i.e. delete all data on an user-editable control.
	 * If no such event handler is installed, Clapkit will try to
	 * perform the task itself if it's a known type.
	 * If you install an event handler, you'll have to do it yourself.
	 * You have to install an event handler for non-standard data types.
	 * Additionally, you can overwrite 'PerformClear' of your object.
	 */
	dataClear,

};

class CKWindow;
class CKControl;

/**
 * @ingroup Types
 * @brief Defines an event raised by the framework, mostly for user actions.
 */
struct CKEvent {

		CKEventType type;
		CKPoint point;
		CKMouseButton mouseButton;

		bool shiftDown = false;
		bool cmdDown = false;
		bool optDown = false;
		bool ctrlDown = false;

		char key;		// Keycode of the key being pressed/released
		char character; // Character Code (A, B, C..) of the key pressed/released

		const CKWindow* window = nullptr;	// always set
		const CKControl* control = nullptr; // can be nullptr for window-only events

		CKError errCode; // Set on tcpError.

		CKEvent(CKEventType type)
			: type(type) {}
		CKEvent(CKEventType type, CKPoint point)
			: type(type), point(point) {}

		void fillFromOS(EventRecord e) {
			shiftDown = (e.modifiers & shiftKey) == shiftKey;
			cmdDown = (e.modifiers & cmdKey) == cmdKey;
			optDown = (e.modifiers & optionKey) == optionKey;
			ctrlDown = (e.modifiers & controlKey) == controlKey;
		}
};

/**
 * @ingroup Types
 * @brief Defines a system icon type.
 */
enum class CKSystemIcon {
	noIcon = 0,
	message = 1,
	warning = 2,
	error = 3,
};

/**
 * @ingroup Types
 * @brief Defines a scroll type.
 */
enum class CKScrollType {
	none = 0,
	vertical = 1,
	horizontal = 2,
	both = 3
};
