/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKPlatform
 * ----------------------------------------------------------------------
 * OS-specific aliases, macros and definitions.
 *
 */

#pragma once
#include <Controls.h>
#include <MacWindows.h>
#include <Quickdraw.h>

#define kCKAppleMenuID			128
#define kCKUserMenuStartID		200

/**
 * @ingroup Utils
 * @brief Suggested button height for platform
 */
#define kCKButtonHeight			20

/**
 * @ingroup Utils
 * @brief Suggested checkbox height for platform
 */
#define kCKCheckboxHeight		16

/**
 * @ingroup Utils
 * @brief Suggested radio button height for platform
 */
#define kCKRadioboxHeight		kCKCheckboxHeight

/**
 * @ingroup Utils
 * @brief Suggested textfield height for platform
 */
#define kCKTextFieldHeight		20

/**
 * @ingroup Utils
 * @brief Suggested dropdown control height for platform
 */
#define kCKDropdownHeight		20

#define kControlProcIDButton	0
#define kControlProcIDCheckbox	1
#define kControlProcIDRadio		2
#define kControlProcIDScrollbar 16
#define kControlProcPopup		1008

#define QD_BOLD					1
#define QD_ITALIC				2
#define QD_UNDERLINE			4

using CKWindowPtr = WindowPtr;
using CKControlPtr = ControlRef;

/**
 * @ingroup Utils
 * @brief Tries to find an unused menu ID.
 * @param
 * @return
 */
static inline int CKFindFreeMenuID(void) {
	int mid = 500;
	while (GetMenuHandle(mid) != nullptr) {
		++mid;
	}
	return mid;
}