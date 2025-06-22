/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKErrors
 * ----------------------------------------------------------------------
 * A list of CK-defined Error Codes.
 * Note that all CK error codes are > 1.
 * If we HAVE to return a platform-specific code, you have to make sure it's
 * negative.
 *
 */

#pragma once
#include "ckTypes.h"

/**
 * @ingroup Types
 * @brief CK-Framework Specific Error Codes
 * @note Note that all CK error codes are > 1. If an error code is negative,
 * it's coming directly from the OS. (We negate if the number is positive, even if it might create conflicts.)
 */
enum CKErrorCode : CKError {

	// ------------------------------------------------------------
	// General Error Codes
	// ------------------------------------------------------------

	/**
	 * @brief Ran out of memory before completion.
	 */
	CKError_OutOfMemory = 1000,

	/**
	 * @brief One or more parameters supplied are not correct. Check logs.
	 */
	CKError_InvalidParameters,

	/**
	 * @brief User pressed CMD + [.] to stop the operation.
	 */
	CKError_UserCancelled,

	/**
	 * @brief File/URL/Control/Command specified could not be found.
	 */
	CKError_NotFound,

	/**
	 * @brief File/URL/Control/Command specified is already open and can't be re-opened.
	 */
	CKError_AlreadyOpen,

	/**
	 * @brief Requested driver is not installed/available.
	 */
	CKError_DriverNotAvailable,

	/**
	 * @brief Requested driver failed to complete task.
	 */
	CKError_DriverActionFailed,

	// ------------------------------------------------------------
	// Networking Error Codes
	// ------------------------------------------------------------
	CKError_TCPConnectionFailed,
	CKError_TCPNotConnected,
	CKError_TCPUnreachable,
};