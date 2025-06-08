/**
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
 */

#include "ckTypes.h"

enum CKErrorCode : CKError {

	/**
	 * ------------------------------------------------------------
	 * General Error Codes
	 * ------------------------------------------------------------
	 */
	CKError_OutOfMemory = 1000,
	CKError_InvalidParameters,
	CKError_UserCancelled,
	CKError_NotFound,
	CKError_AlreadyOpen,
	CKError_DriverNotAvailable,
	CKError_DriverActionFailed,

	/**
	 * ------------------------------------------------------------
	 * Networking Error Codes
	 * ------------------------------------------------------------
	 */
	CKError_TCPConnectionFailed,
	CKError_TCPNotConnected,
	CKError_TCPUnreachable,
};