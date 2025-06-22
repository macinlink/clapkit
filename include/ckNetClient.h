/*
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

#include "ckNetBaseSocket.h"
#include "ckNetworking.h"

/**
 * @ingroup Networking
 * @brief Defines a client TCP socket.
 */
class CKNetClient : public CKNetBaseSocket {

	public:
		CKNetClient();
		~CKNetClient();

		virtual CKError Connect(const char* address, UInt16 port);
		virtual CKError Connect(CKIPAddress address, UInt16 port);
		virtual CKError Read(void* out, short len, short* actuallyRead);
		virtual CKError Write(const void* data, UInt32 len);
		bool IsConnected();
};