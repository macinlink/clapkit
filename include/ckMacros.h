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