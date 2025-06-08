/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKNetClient
 * ----------------------------------------------------------------------
 * Implements a basic TCP socket using MacTCP.
 *
 */

#include "ckNetClient.h"
#include "ckErrors.h"
#include <Devices.h>

CKNetClient::CKNetClient()
	: CKNetBaseSocket() {
}

CKNetClient::~CKNetClient() {
}

CKError CKNetClient::Connect(const char* address, UInt16 port) {

	CKIPAddress r;
	CKError err = CKNetworking::ResolveName(address, &r);

	if (err == CKPass) {
		return this->Connect(r, port);
	} else {
		return err;
	}
}

CKError CKNetClient::Connect(CKIPAddress address, UInt16 port) {

	if (this->__isConnected) {
		this->Close();
	}

	this->__isConnected = false;

	if (!this->__hasStream) {
		CKError streamInitResult = this->__openStream();
		if (streamInitResult != CKPass) {
			return streamInitResult;
		}
	}

	CKLog("Calling TCPActiveOpen with stream %d and address %d.%d.%d.%d and port %d", this->__stream, address[0], address[1], address[2], address[3], port);

	TCPiopb pb;
	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.tcpStream = this->__stream;
	pb.ioCompletion = ckgIOCompletionUPP;
	pb.csCode = TCPActiveOpen;
	pb.csParam.open.userDataPtr = (Ptr)this;
	pb.csParam.open.localPort = 0;
	pb.csParam.open.remoteHost =
		(address[0] << 24) | (address[1] << 16) | (address[2] << 8) | address[3];
	pb.csParam.open.remotePort = port;
	pb.csParam.open.tosFlags = 0;
	pb.csParam.open.precedence = 0;
	pb.csParam.open.dontFrag = 0;
	pb.csParam.open.timeToLive = 0;
	pb.csParam.open.security = 0;
	pb.csParam.open.optionCnt = 0;

	OSErr err = PBControlAsync((ParmBlkPtr)&pb);
	CKLog("PB result is %d", err);
	if (err != noErr && err != inProgress) {
		CKLog("TCPActiveOpen failed with error code %d", err);
		this->Close();
		this->HandleEvent(CKEvent(CKEventType::tcpConnectionFailed));
		if (err == connectionExists) {
			// A stray connection?
			// TODO: In this case, should we retry or not?
			return CKError_AlreadyOpen;
		}
		return CKError_TCPConnectionFailed;
	}

	return CKPass;
}

CKError CKNetClient::Read(void* out, short len, short* actuallyRead) {
	// If not connected, return CKError_TCPNotConnected;
	if (!this->__isConnected) {
		return CKError_TCPNotConnected;
	}

	if (actuallyRead)
		*actuallyRead = 0;

	TCPiopb pb;
	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.tcpStream = this->__stream;
	pb.ioCompletion = ckgIOCompletionUPP;
	pb.csCode = TCPRcv;
	pb.csParam.receive.rcvBuff = (Ptr)out;
	pb.csParam.receive.rcvBuffLen = len;
	pb.csParam.receive.userDataPtr = (Ptr)this;

	OSErr err = PBControl((ParmBlkPtr)&pb, false);

	if (err != noErr) {
		CKLog("Read failed: %d", err);
		return CKError_DriverActionFailed;
	}

	if (actuallyRead) {
		*actuallyRead = pb.csParam.receive.rcvBuffLen;
	}
	CKLog("ActuallRead: %d", *actuallyRead);
	return CKPass;
}

CKError CKNetClient::Write(const void* data, UInt32 len) {
	// If not connected, return CKError_TCPNotConnected;
	if (!this->__isConnected) {
		return CKError_TCPNotConnected;
	}

	TCPiopb pb;
	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.tcpStream = this->__stream;
	pb.ioCompletion = ckgIOCompletionUPP;
	pb.csCode = TCPSend;
	pb.csParam.send.ulpTimeoutValue = 30; // 30 second timeout
	pb.csParam.send.ulpTimeoutAction = 1; // Abort on timeout
	pb.csParam.send.validityFlags = timeoutValue | timeoutAction;
	pb.csParam.send.pushFlag = 1; // Push data immediately
	pb.csParam.send.urgentFlag = 0;
	pb.csParam.send.wdsPtr = (Ptr)NewPtr(sizeof(wdsEntry));

	if (!pb.csParam.send.wdsPtr) {
		return CKError_OutOfMemory;
	}

	// Set up write data structure
	wdsEntry* wds = (wdsEntry*)pb.csParam.send.wdsPtr;
	wds[0].length = len;
	wds[0].ptr = (Ptr)data;
	wds[1].length = 0; // End marker
	wds[1].ptr = nullptr;

	OSErr err = PBControlAsync((ParmBlkPtr)&pb);
	DisposePtr(pb.csParam.send.wdsPtr);
	if (err != noErr) {
		return CKError_DriverActionFailed;
	}

	return CKPass;
}

bool CKNetClient::IsConnected() {
	return __isConnected;
}