/*
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
#include <Events.h>
#include <Memory.h>

TCPNotifyUPP ckgNotifyUPP = nullptr;
TCPIOCompletionUPP ckgIOCompletionUPP = nullptr;

CKNetBaseSocket::CKNetBaseSocket() {
	if (ckgNotifyUPP == nullptr) {
		ckgNotifyUPP = NewTCPNotifyUPP(CKNBSNotify);
		ckgIOCompletionUPP = NewTCPIOCompletionUPP(CKNBSIOCompletion);
	}
	if (__ckgCurrentCKApp) {
		__ckgCurrentCKApp->__net_sockets.push_back(this);
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

	CKLog("CKNetBaseSocket::__closeStream stream=%lx pendingOps=%u", this->__stream, this->__pendingAsyncOps);

	TCPiopb* pb = (TCPiopb*)NewPtrSysClear(sizeof(*pb));
	if (!pb) {
		pb = (TCPiopb*)NewPtrClear(sizeof(*pb));
	}
	if (!pb) {
		return CKError_OutOfMemory;
	}
	pb->ioCRefNum = CKNetworking::GetDriverRefNum();
	pb->csCode = TCPClose;
	pb->tcpStream = this->__stream;
	pb->ioCompletion = ckgIOCompletionUPP;
	pb->csParam.close.userDataPtr = (Ptr)this;

	OSErr err = PBControlAsync((ParmBlkPtr)pb);
	if (err != noErr && err != inProgress) {
		CKLog("Closing of stream failed: %d", err);
		DisposePtr((Ptr)pb);
		return CKError_DriverActionFailed;
	}

	this->__pendingAsyncOps++;
	this->__waitingForClose = true;

	UInt32 timeout = TickCount() + 60 * 5; // wait up to ~5 seconds for close completion
	while (this->__waitingForClose && TickCount() < timeout) {
		SystemTask();
		this->Loop();
	}

	if (this->__waitingForClose) {
		CKLog("TCPClose did not complete before timeout; forcing release");
		this->__waitingForClose = false;
		// Fall through to release attempts to avoid leaks.
		this->__releaseStream();
	}

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

	CKLog("CKNetBaseSocket::__openStream bufferSize=%u", this->__mactcpBufferSize);
	// Allocate receive buffer in the system heap first to stay 24-bit clean and compatible with MacTCP driver expectations.
	this->__mactcpBuffer = NewPtrSysClear(this->__mactcpBufferSize);
	if (!this->__mactcpBuffer) {
		this->__mactcpBuffer = NewPtrClear(this->__mactcpBufferSize);
	}
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

	OSErr err = PBControlSync((ParmBlkPtr)&pb);
	if (err != noErr) {
		CKLog("Opening of stream failed: %d", err);
		if (this->__mactcpBuffer) {
			DisposePtr(this->__mactcpBuffer);
			this->__mactcpBuffer = nullptr;
		}
		return CKError_DriverActionFailed;
	}

	this->__stream = pb.tcpStream;
	CKLog("Stream created with ref number %d", this->__stream);
	this->__hasStream = true;
	return CKPass;
}

CKError CKNetBaseSocket::__releaseStream() {

	CKLog("CKNetBaseSocket::__releaseStream stream=%lx", this->__stream);

	if (!this->__stream) {
		return CKPass;
	}

	TCPiopb pb;
	memset(&pb, 0, sizeof(pb));
	pb.ioCRefNum = CKNetworking::GetDriverRefNum();
	pb.csCode = TCPRelease;
	pb.tcpStream = this->__stream;

	OSErr err = PBControlSync((ParmBlkPtr)&pb);
	if (err != noErr) {
		CKLog("Releasing of stream failed: %d", err);
		return CKError_DriverActionFailed;
	}

	if (this->__mactcpBuffer) {
		DisposePtr(this->__mactcpBuffer);
		this->__mactcpBuffer = nullptr;
	}

	this->__stream = 0;
	this->__hasStream = false;
	CKLog("CKNetBaseSocket::__releaseStream done");
	return CKPass;
}

void CKNetBaseSocket::__postNotifyEvent(unsigned short eventCode, unsigned short terminReason, struct ICMPReport* icmpMsg) {
	// Record for later logging in the main loop to avoid doing any heavy work at interrupt time.
	CKNetBaseSocketNotifyEvt* slot = &this->__notifyEvents[this->__notifyEventsWriteIdx];
	slot->eventCode = eventCode;
	slot->terminReason = terminReason;
	slot->icmpMsg = icmpMsg;
	slot->isWritten = true;
	slot->isRead = false;
	this->__notifyEventsWriteIdx++;
	if (this->__notifyEventsWriteIdx >= 32) {
		this->__notifyEventsWriteIdx = 0;
	}
	this->__hasNotifyEvents = true;

	switch (eventCode) {
		case TCPUrgent:
		case TCPDataArrival:
			this->__hasIncomingData = true;
			break;
		case TCPClosing:
		case TCPULPTimeout:
		case TCPTerminate:
		case TCPICMPReceived:
			this->__hasDisconnected = true;
			break;
		default:
			// Logging deferred to Loop to remain interrupt-safe.
			break;
	}
}

void CKNetBaseSocket::__postIOCompletionEvent(unsigned short csCode, bool result, TCPiopb* pb, bool countsPending, bool pbFromNewPtr) {

	this->__intEvents[this->__intEventsWriteIdx].csCode = csCode;
	this->__intEvents[this->__intEventsWriteIdx].result = result;
	this->__intEvents[this->__intEventsWriteIdx].pb = pb;
	this->__intEvents[this->__intEventsWriteIdx].countsPending = countsPending;
	this->__intEvents[this->__intEventsWriteIdx].pbFromNewPtr = pbFromNewPtr;
	this->__intEvents[this->__intEventsWriteIdx].isWritten = true;
	this->__intEvents[this->__intEventsWriteIdx].isRead = false;

	this->__intEventsWriteIdx++;
	if (this->__intEventsWriteIdx >= 32) {
		this->__intEventsWriteIdx = 0;
	}
	this->__hasIntEvents = true;
}

void CKNetBaseSocket::Loop() {

	if (this->__hasNotifyEvents) {
		bool foundNotify = false;
		int loopCount = 0;
		while (true) {
			CKNetBaseSocketNotifyEvt* n = &this->__notifyEvents[this->__notifyEventsReadIdx];
			if (n->isWritten && !n->isRead) {
				CKLog("MacTCP notify: stream %lx event %u reason %u", this->__stream, n->eventCode, n->terminReason);
				n->isRead = true;
				foundNotify = true;
				this->__notifyEventsReadIdx++;
				if (this->__notifyEventsReadIdx >= 32) {
					this->__notifyEventsReadIdx = 0;
				}
				break;
			}

			this->__notifyEventsReadIdx++;
			if (this->__notifyEventsReadIdx >= 32) {
				this->__notifyEventsReadIdx = 0;
				loopCount++;
			}
			if (loopCount >= 2) {
				break;
			}
		}
		if (!foundNotify) {
			this->__hasNotifyEvents = false;
		}
	}

	if (this->__hasIncomingData) {
		CKEvent evt(CKEventType::tcpReceivedData);
		this->HandleEvent(evt);
		this->__hasIncomingData = false;
	}

	if (this->__hasDisconnected) {
		CKEvent evt(CKEventType::tcpDisconnected);
		this->HandleEvent(evt);
		this->__isConnected = false;
		this->__hasDisconnected = false;
	}

	while (this->__hasIntEvents) {

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
			break;
		}

		found->isRead = true;

		// CKLog("MacTCP completion: csCode %d result %d stream %lx pb %p", found->csCode, found->result, this->__stream, found->pb);
		// if (found->pb) {
		// 	CKLog("MacTCP completion detail: ioResult %d userData %p", found->pb->ioResult, (void*)(found->pb->csCode == TCPActiveOpen ? found->pb->csParam.open.userDataPtr : found->pb->csCode == TCPRcv ? found->pb->csParam.receive.userDataPtr
		// 																																								   : found->pb->csCode == TCPSend  ? found->pb->csParam.send.userDataPtr
		// 																																								   : found->pb->csCode == TCPClose ? found->pb->csParam.close.userDataPtr
		// 																																																   : nullptr));
		// }

		switch (found->csCode) {
			case TCPCreate:
				// Not handling, this is sync.
				break;
			case TCPActiveOpen:
				CKLog("Got TCPActiveOpen result of %s", found->result ? "success" : "failure");
				if (found->pb) {
					CKLog("TCPActiveOpen detail: remote %lu.%lu.%lu.%lu port %u localPort %u",
						  (unsigned long)((found->pb->csParam.open.remoteHost >> 24) & 0xFF),
						  (unsigned long)((found->pb->csParam.open.remoteHost >> 16) & 0xFF),
						  (unsigned long)((found->pb->csParam.open.remoteHost >> 8) & 0xFF),
						  (unsigned long)(found->pb->csParam.open.remoteHost & 0xFF),
						  found->pb->csParam.open.remotePort,
						  found->pb->csParam.open.localPort);
				}
				if (found->result) {
					this->__isConnected = true;
					this->HandleEvent(CKEvent(CKEventType::tcpConnected));
				} else {
					this->__isConnected = false;
					this->HandleEvent(CKEvent(CKEventType::tcpConnectionFailed));
				}
				break;
			case TCPClose:
				CKLog("TCPClose completed with result %s", found->result ? "success" : "failure");
				this->__waitingForClose = false;
				if (found->result) {
					this->__releaseStream();
				} else {
					CKLog("TCPClose reported failure; attempting release anyway");
					this->__releaseStream();
				}
				break;
			default:
				CKLog("Warning! Unimplemented csCode %d!", found->csCode);
				break;
		}

		if (found->pb) {
			if (found->pbFromNewPtr) {
				DisposePtr((Ptr)found->pb);
			} else {
				CKFree(found->pb);
			}
			found->pb = nullptr;
		}
		if (found->countsPending && this->__pendingAsyncOps > 0) {
			this->__pendingAsyncOps--;
		}
	}
}

pascal void CKNBSNotify(StreamPtr stream, unsigned short eventCode, Ptr userDataPtr, unsigned short terminReason, struct ICMPReport* icmpMsg) {

	auto* sock = reinterpret_cast<CKNetBaseSocket*>(userDataPtr);
	if (sock) {
		sock->__postNotifyEvent(eventCode, terminReason, icmpMsg);
	}
}

// MacTCP IO completion callbacks use the C calling convention (stack cleaned by caller on 68k).
void CKNBSIOCompletion(TCPiopb* iopb) {

	if (!iopb) {
		return;
	}

	CKNetBaseSocket* socket = nullptr;

	void* userPtr = nullptr;
	bool countsPending = false;
	bool pbFromNewPtr = true; // async PBs allocated via NewPtr*
	switch (iopb->csCode) {
		case TCPActiveOpen:
			userPtr = iopb->csParam.open.userDataPtr;
			countsPending = true;
			break;
		case TCPRcv:
			userPtr = iopb->csParam.receive.userDataPtr;
			break;
		case TCPSend:
			userPtr = iopb->csParam.send.userDataPtr;
			break;
		case TCPClose:
			userPtr = iopb->csParam.close.userDataPtr;
			countsPending = true;
			break;
		default:
			return;
	}

	socket = (CKNetBaseSocket*)userPtr;
	if (!socket) {
		return;
	}

	bool opSuccessful = iopb->ioResult == noErr;
	socket->__postIOCompletionEvent(iopb->csCode, opSuccessful, iopb, countsPending, pbFromNewPtr);
}
