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
 * Copy a string (if it is a string) to a destination.
 * Free the destination if there is already something there.
 */
#define CK_SAFE_COPY_STRING(dest, src) ({        \
	if (dest != 0) {                             \
		CKFree(dest);                            \
		dest = 0;                                \
	}                                            \
	if (src != 0) {                              \
		dest = (char*)CKMalloc(strlen(src) + 1); \
		strcpy(dest, src);                       \
	}                                            \
})

/**
 * Use when something is not yet implemented.
 */
#define CK_UNIMPLEMENTED() ({ CKLog("Unimplemented function"); })