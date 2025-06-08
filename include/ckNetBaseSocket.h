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

#include "ckObject.h"
#include "ckTypes.h"
#include <Devices.h>
#include <MacTCP.h>

pascal void CKNBSNotify(StreamPtr stream, unsigned short eventCode, Ptr userDataPtr, unsigned short terminReason, struct ICMPReport* icmpMsg);
pascal void CKNBSIOCompletion(TCPiopb* iopb);

extern TCPNotifyUPP ckgNotifyUPP;
extern TCPIOCompletionUPP ckgIOCompletionUPP;

struct CKNetBaseSocketEvtFromInterrupt {
	public:
		short csCode = 0;
		bool result = false;
		bool isWritten = false;
		bool isRead = false;
};

class CKNetBaseSocket : public CKObject {

	public:
		CKNetBaseSocket();
		~CKNetBaseSocket();

		virtual void Close();
		void SetBufferSize(UInt16 size) {
			if (this->__stream) {
				return;
			}
			this->__mactcpBufferSize = size;
		}
		UInt16 GetBufferSize() {
			return this->__mactcpBufferSize;
		}

		void Loop();

	protected:
		CKError __openStream();
		CKError __closeStream();

		friend pascal void CKNBSNotify(StreamPtr, unsigned short, Ptr, unsigned short, struct ICMPReport*);
		void __postNotifyEvent(unsigned short eventCode, unsigned short terminReason, struct ICMPReport* icmpMsg);

		friend pascal void CKNBSIOCompletion(TCPiopb* iopb);
		void __postIOCompletionEvent(unsigned short csCode, bool result);

	protected:
		UInt16 __mactcpBufferSize = 8192;
		Ptr __mactcpBuffer = 0;
		StreamPtr __stream = 0;
		bool __hasStream = false;
		bool __isConnected = false;

	private:
		bool __hasIntEvents = false;
		bool __hasIncomingData = false;
		bool __hasDisconnected = false;
		CKNetBaseSocketEvtFromInterrupt __intEvents[32];
		int __intEventsWriteIdx = 0;
		int __intEventsReadIdx = 0;
};