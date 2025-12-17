/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKNetworking
 * ----------------------------------------------------------------------
 * Network related (MacTCP) functionality
 *
 */

#include "ckNetworking.h"
#include "MacTCP/AddressXlation.h"
#include "ckErrors.h"
#include "ckNetBaseSocket.h"
#include "ckUtils.h"
#include <Devices.h>
#include <Errors.h>
#include <Events.h>
#include <MacTCP.h>

short MacTCPDriverRefNum = -1;
bool MacTCPDriverOpened = false;

/**
 * @brief Check if MacTCP is installed and available.
 * @return
 */
bool CKNetworking::IsAvailable() {
	short refNum;
	OSErr err = MacOpenDriver("\p.IPP", &refNum);
	if (err == noErr) {
		MacCloseDriver(refNum);
		return true;
	}
	return false;
}

/**
 * @brief Check if networking has been initialized by calling `Initialize`
 * @return
 */
bool CKNetworking::IsInitialized() {
	return MacTCPDriverOpened;
}

/**
 * @brief Try to open the MacTCP driver.
 * @return CKPass on success
 */
CKError CKNetworking::Initialize() {
	short refNum;
	if (MacTCPDriverOpened) {
		CKNetworking::Deinitialize();
	}
	MacTCPDriverOpened = false;
	OSErr err = MacOpenDriver("\p.IPP", &refNum);
	if (err != noErr) {
		CKLog("TCP driver open failed: %d", err);
		// TODO: Provide more detailed error codes.
		return CKError_DriverActionFailed;
	}
	MacTCPDriverOpened = true;
	MacTCPDriverRefNum = refNum;
	CKLog("Networking initialized, driver ref number is %d", refNum);
	return CKPass;
}

/**
 * @brief
 * @return
 */
bool CKNetworking::Deinitialize() {
	if (!MacTCPDriverOpened) {
		return false;
	}
	CKLog("CKNetworking::Deinitialize closing driver (%d)", MacTCPDriverRefNum);
	MacCloseDriver(MacTCPDriverRefNum);
	MacTCPDriverRefNum = -1;
	MacTCPDriverOpened = false;
	return true;
}

/** * @brief Return the IP Address of the current device
 * @return Returns a blank (0.0.0.0) IP address on failure.
 */
CKIPAddress CKNetworking::GetLocalIP() {
	if (!MacTCPDriverOpened) {
		return {0, 0, 0, 0};
	}

	GetAddrParamBlock pb = {};
	pb.ioCRefNum = MacTCPDriverRefNum;
	pb.csCode = ipctlGetAddr;

	OSErr err = PBControlSync((ParmBlkPtr)&pb);
	if (err != noErr) {
		CKLog("GetLocalIP PBControlSync failed: %d (ioResult=%d)", err, pb.ioResult);
		return {0, 0, 0, 0};
	}

	ip_addrbytes* ipAddrBytes = (ip_addrbytes*)&pb.ourAddress;
	CKLog("CKNetworking::GetLocalIP ioResult=%d localHost=%lu", pb.ioResult, (unsigned long)pb.ourAddress);
	return {ipAddrBytes->a.byte[0], ipAddrBytes->a.byte[1], ipAddrBytes->a.byte[2], ipAddrBytes->a.byte[3]};
}

short CKNetworking::GetDriverRefNum() {
	if (!MacTCPDriverOpened) {
		return 0;
	}
	return MacTCPDriverRefNum;
}

pascal void ResolveNameResultUPP(struct hostInfo* hInfoPtr, char* userDataPtr) {
	volatile Boolean* donePtr = (volatile Boolean*)userDataPtr;
	*donePtr = true;
}

CKError CKNetworking::ResolveName(const char* hostname, CKIPAddress* result) {

	*result = {0, 0, 0, 0};

	if (strlen(hostname) > 255) {
		return CKError_InvalidParameters;
	}

	OSErr err;
	CKError finalErr = CKPass;
	bool resolverOpened = false;
	ResultUPP completionUPP = nullptr;

	CKLog("CKNetworking::ResolveName start host=%s", hostname);
	err = OpenResolver(nil);
	if (err != noErr) {
		CKLog("Failed to open DNR: %d", err);
		return CKError_DriverActionFailed;
	}
	resolverOpened = true;

	struct hostInfo hResult;

	volatile Boolean done = false;

	// Define completion routine
	completionUPP = NewResultProc(ResolveNameResultUPP);

	err = StrToAddr((char*)hostname, &hResult, completionUPP, (char*)&done);
	if (err != noErr && err != cacheFault) {
		CKLog("Calling StrToAddress failed: %d", err);
		finalErr = CKError_DriverActionFailed;
		goto cleanup;
	}

	if (err == cacheFault) {
		UInt32 timeout = TickCount() + 60 * 5;
		while (!done && TickCount() < timeout) {
			SystemTask();
			EventRecord evt;
			if (EventAvail(keyDownMask, &evt)) {
				if ((evt.modifiers & cmdKey) && (evt.message & charCodeMask) == '.') {
					finalErr = CKError_UserCancelled;
					goto cleanup;
				}
			}
		}
		if (!done) {
			CKLog("DNR request timed out waiting for StrToAddr completion");
			finalErr = CKError_DriverActionFailed;
			goto cleanup;
		}
	} else {
		CKLog("Got cached DNR result.");
	}
	if (hResult.rtnCode == noErr && hResult.addr[0] != 0) {
		// Convert the address (it's in network byte order)
		unsigned long addr = hResult.addr[0];

		(*result)[0] = (unsigned char)((addr >> 24) & 0xFF);
		(*result)[1] = (unsigned char)((addr >> 16) & 0xFF);
		(*result)[2] = (unsigned char)((addr >> 8) & 0xFF);
		(*result)[3] = (unsigned char)(addr & 0xFF);
		finalErr = CKPass;
		CKLog("CKNetworking::ResolveName success %u.%u.%u.%u", (*result)[0], (*result)[1], (*result)[2], (*result)[3]);
		goto cleanup;
	}

	// Map DNR error codes
	switch (hResult.rtnCode) {
		case nameSyntaxErr:
			CKLog("DNR result is nameSyntaxErr.");
			finalErr = CKError_InvalidParameters;
			break;
		case noNameServer:
			CKLog("DNR result is noNameServer.");
			finalErr = CKError_TCPUnreachable;
			break;
		case noAnsErr:
			CKLog("DNR result is noAnsErr.");
			finalErr = CKError_TCPUnreachable;
			break;
		case authNameErr:
			CKLog("DNR result is authNameErr.");
			finalErr = CKError_NotFound;
			break;
		case outOfMemory:
			CKLog("DNR result is outOfMemory.");
			finalErr = CKError_OutOfMemory;
			break;
		default:
			CKLog("DNR failed with code: %d", hResult.rtnCode);
			finalErr = CKError_DriverActionFailed;
			break;
	}

cleanup:
#if GENERATINGCFM
	if (completionUPP) {
		DisposeRoutineDescriptor((UniversalProcPtr)completionUPP);
	}
#endif
	if (resolverOpened) {
		OSErr closeErr = CloseResolver();
		if (closeErr != noErr) {
			CKLog("CloseResolver failed: %d", closeErr);
		}
	}
	return finalErr;
}

/**
 * @brief
 */
void CKNetworking::Loop(std::vector<CKNetBaseSocket*> sockets) {
	if (!MacTCPDriverOpened) {
		// Nothing to do!
		return;
	}
	for (auto& s : sockets) {
		if (s) {
			CKLog("CKNetworking::Loop driving socket %p", s);
			s->Loop();
		}
	}
}
