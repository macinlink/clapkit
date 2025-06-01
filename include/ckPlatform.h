/**
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

#define kCKAppleMenuID	   128
#define kCKUserMenuStartID 200

#define kCKButtonHeight	   20
#define kCKCheckboxHeight  16
#define kCKRadioboxHeight  kCKCheckboxHeight
#define kCKTextFieldHeight 20

using CKWindowPtr = WindowPtr;
using CKControlPtr = ControlRef;