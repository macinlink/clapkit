/**
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

#pragma once

#include "ckTypes.h"
#include <MacTCP.h>
#include <array>

using CKIPAddress = std::array<unsigned char, 4>;

class CKNetBaseSocket;

namespace CKNetworking {

bool IsAvailable();
bool IsInitialized();
CKError Initialize();
bool Deinitialize();
void Loop(std::vector<CKNetBaseSocket*> sockets);
CKIPAddress GetLocalIP();
CKError ResolveName(const char* hostname, CKIPAddress* result);

short GetDriverRefNum();

}; // namespace CKNetworking