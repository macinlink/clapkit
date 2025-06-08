/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKNetBaseSocket
 * ----------------------------------------------------------------------
 * Implements the base socket for CKNetClient and CKNetServer
 *
 */

#include "ckNetBaseSocket.h"
#include "ckApp.h"
#include "ckErrors.h"
#include "ckGlobals.h"
#include "ckNetworking.h"

TCPNotifyUPP ckgNotifyUPP = nullptr;
TCPIOCompletionUPP ckgIOCompletionUPP = nullptr;

CKNetBaseSocket::CKNetBaseSocket() {
	if (ckgNotifyUPP == nullptr) {
		CKLog("Creating UPPs...");
		ckgNotifyUPP = NewTCPNotifyUPP(CKNBSNotify);
		ckgIOCompletionUPP = NewTCPIOCompletionUPP(CKNBSIOCompletion);
	}
	if (__ckgCurrentCKApp) {
		__ckgCurrentCKApp->__net_sockets.push_back(this);
		CKLog("I've added myself (%x) to __net_sockets of current app (%x)", this, __ckgCurrentCKApp);
	}
}

CKNetBaseSocket::~CKNetBaseSocket() {
	this->__closeStream();
	if (__ckgCurrentCKApp) {
		for (auto it = __ckgCurrentCKApp->__net_sockets.begin(); it != __ckgCurrentCKApp->__net_sockets.end(); ++it) {
			if (*it == this) {
				it = __ckgCurrentCKApp->__net_sockets.erase(it);
				break;
			}
		}
	}
}

void CKNetBaseSocket::Close() {
	this->__closeStream();
}

CKError CKNetBaseSocket::__closeStream() {
	if (!this->__stream) {
		return CKPass;
	}

	// TODO: Do we need to `close` first or can we just release?

	TCPiopb pb;
	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.csCode = TCPClose;
	pb.tcpStream = this->__stream;

	OSErr err = PBControl((ParmBlkPtr)&pb, false);
	if (err != noErr) {
		CKLog("Closing of stream failed: %d", err);
		return CKError_DriverActionFailed;
	}

	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.csCode = TCPRelease;
	pb.tcpStream = this->__stream;

	err = PBControl((ParmBlkPtr)&pb, false);
	if (err != noErr) {
		CKLog("Releasing of stream failed: %d", err);
		return CKError_DriverActionFailed;
	}

	if (this->__mactcpBuffer) {
		CKFree(this->__mactcpBuffer);
	}

	CKLog("this->__stream closed via closeStream successfully");
	this->__stream = 0;
	return CKPass;
}

CKError CKNetBaseSocket::__openStream() {
	if (!CKNetworking::IsInitialized()) {
		CKError initResult = CKNetworking::Initialize();
		if (initResult != CKPass) {
			CKLog("CKNetworking initalization failed. (Code: %d)", initResult);
			return CKError_DriverNotAvailable;
		}
	}
	if (this->__hasStream) {
		CKError closeResult = this->__closeStream();
		if (closeResult != CKPass) {
			CKLog("Closing of previous stream failed. (Code: %d)", closeResult);
			return CKError_DriverActionFailed;
		}
	}

	this->__hasStream = false;

	if (this->__mactcpBufferSize < 1024) {
		CKLog("TCP buffer size must be larger than 1024, but it's %d", this->__mactcpBufferSize);
		return CKError_InvalidParameters;
	}

	this->__mactcpBuffer = (Ptr)CKMalloc(this->__mactcpBufferSize);
	if (!this->__mactcpBuffer) {
		CKLog("Can't create TCP buffer!");
		return CKError_OutOfMemory;
	}

	TCPiopb pb;
	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.csCode = TCPCreate;

	pb.csParam.create.rcvBuff = this->__mactcpBuffer;
	pb.csParam.create.rcvBuffLen = this->__mactcpBufferSize;
	pb.csParam.create.notifyProc = ckgNotifyUPP;
	pb.csParam.create.userDataPtr = (Ptr)this;

	OSErr err = PBControlAsync((ParmBlkPtr)&pb);
	if (err != noErr) {
		CKLog("Opening of stream failed: %d", err);
		return CKError_DriverActionFailed;
	}

	this->__stream = pb.tcpStream;
	CKLog("Stream created with ref number %d", this->__stream);
	this->__hasStream = true;
	return CKPass;
}

void CKNetBaseSocket::__postNotifyEvent(unsigned short eventCode, unsigned short terminReason, struct ICMPReport* icmpMsg) {

	switch (eventCode) {
		case TCPUrgent:
		case TCPDataArrival:
			this->__hasIncomingData = true;
			break;
		case TCPClosing:
		case TCPTerminate:
			this->__hasDisconnected = true;
			break;
		default:
			DebugStr("\pUnknown notification received from MacTCP!");
			break;
	}
}

void CKNetBaseSocket::__postIOCompletionEvent(unsigned short csCode, bool result) {

	this->__intEvents[this->__intEventsWriteIdx].csCode = csCode;
	this->__intEvents[this->__intEventsWriteIdx].result = result;
	this->__intEvents[this->__intEventsWriteIdx].isWritten = true;
	this->__intEvents[this->__intEventsWriteIdx].isRead = false;

	this->__intEventsWriteIdx++;
	if (this->__intEventsWriteIdx >= 32) {
		this->__intEventsWriteIdx = 0;
	}
	this->__hasIntEvents = true;
}

void CKNetBaseSocket::Loop() {

	if (this->__hasIncomingData) {
		this->HandleEvent(CKEvent(CKEventType::tcpReceivedData));
		this->__hasIncomingData = false;
	}

	if (this->__hasDisconnected) {
		this->HandleEvent(CKEvent(CKEventType::tcpDisconnected));
		this->__isConnected = false;
		this->__hasDisconnected = false;
	}

	if (!this->__hasIntEvents) {
		return;
	}

	bool foundOne = false;
	int loopCount = 0;
	CKNetBaseSocketEvtFromInterrupt* found;
	while (true) {
		found = &this->__intEvents[this->__intEventsReadIdx];
		if (found->isWritten && !found->isRead) {
			CKLog("Found unread int evt at index %d", this->__intEventsReadIdx);
			foundOne = true;
			break;
		}

		this->__intEventsReadIdx++;
		if (this->__intEventsReadIdx >= 32) {
			this->__intEventsReadIdx = 0;
			loopCount++;
		}
		if (loopCount >= 2) {
			break;
		}
	}

	if (!foundOne) {
		this->__hasIntEvents = false;
		return;
	}

	found->isRead = true;
	CKLog("Found event of csCode %d with result %s", found->csCode, found->result ? "true" : "false");

	switch (found->csCode) {
		case TCPCreate:
			// Not handling, this is sync.
			break;
		case TCPActiveOpen:
			CKLog("Got TCPActiveOpen result of %s", found->result ? "success" : "failure");
			if (found->result) {
				this->__isConnected = true;
				this->HandleEvent(CKEvent(CKEventType::tcpConnected));
			} else {
				this->__isConnected = false;
				this->HandleEvent(CKEvent(CKEventType::tcpConnectionFailed));
			}
			break;
		default:
			CKLog("Warning! Unimplemented csCode %d!", found->csCode);
			break;
	}

	// Call one more time to flush.
	this->Loop();
}

pascal void CKNBSNotify(StreamPtr stream, unsigned short eventCode, Ptr userDataPtr, unsigned short terminReason, struct ICMPReport* icmpMsg) {

	auto* sock = reinterpret_cast<CKNetBaseSocket*>(userDataPtr);
	if (sock) {
		sock->__postNotifyEvent(eventCode, terminReason, icmpMsg);
	} else {
		CKLog("CKNBSNotify got an object (%x) that's not a base socket!", userDataPtr);
	}
}

pascal void CKNBSIOCompletion(TCPiopb* iopb) {

	if (!iopb) {
		return;
	}

	CKNetBaseSocket* socket = nullptr;

	void* userPtr = nullptr;
	switch (iopb->csCode) {
		case TCPActiveOpen:
			userPtr = iopb->csParam.open.userDataPtr;
			break;
		case TCPRcv:
			userPtr = iopb->csParam.receive.userDataPtr;
			break;
		case TCPSend:
			userPtr = iopb->csParam.send.userDataPtr;
			break;
		case TCPClose:
			userPtr = iopb->csParam.close.userDataPtr;
			break;
		default:
			return;
	}

	socket = (CKNetBaseSocket*)userPtr;
	if (!socket) {
		return;
	}

	bool opSuccessful = iopb->ioResult == noErr;
	socket->__postIOCompletionEvent(iopb->csCode, opSuccessful);
}