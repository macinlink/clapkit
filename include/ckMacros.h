/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * Macros
 * ----------------------------------------------------------------------
 * Things that should (hopefully) make your life easier.
 * At least when while writing code.
 *
 */

#pragma once

#include "ckApp.h"

#include <stdio.h>
#include <string.h>

/**
 * Use when something is not yet implemented.
 */
#define CK_UNIMPLEMENTED() ({ CKLog("Unimplemented function"); })

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef LOWORD
#define LOWORD(l) ((((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) (((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif