/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKUtils
 * ----------------------------------------------------------------------
 * Utilies for commonly done actions.
 *
 */

#include "ckUtils.h"
#include <Appearance.h>
#include <Gestalt.h>
#include <Timer.h>

#ifdef kCKAPPDEBUG
std::vector<CKProfilerData*> _profilerData;
bool _profilerDataInit = false;
#endif

/**
 * Convert a C-style string to Pascal string
 * that most Macintosh Toolbox calls require.
 */
unsigned char* __CKC2P(const char* src, const char* func, int line, const char* file) {

	CKPROFILE

	int length = strlen(src);
	if (length > 254) {
		length = 254;
	}

#ifdef kCKDEBUGMEMORY
	unsigned char* toPStrd = (unsigned char*)__CKMalloc(func, line, file, length + 1);
#else
	unsigned char* toPStrd = (unsigned char*)CKMalloc(length + 1);
#endif
	if (!toPStrd) {
		return 0;
	}

	toPStrd[0] = length;
	strncpy((char*)(toPStrd + 1), src, length);
	return toPStrd;
}

/**
 * Converts Pascal string to C string.
 */
char* __CKP2C(const unsigned char* src, const char* func, int line, const char* file) {

	CKPROFILE

	int length = src[0];
#ifdef kCKDEBUGMEMORY
	char* toReturn = (char*)__CKMalloc(func, line, file, length + 1);
#else
	char* toReturn = (char*)CKMalloc(length + 1);
#endif
	memcpy(toReturn, src + 1, length);
	toReturn[length] = 0;
	return toReturn;
}

/**
 * Print to console a warning, log or an error.
 *
 * Level 0 => Log
 * Level 1 => Warning
 * Level 2 => Error
 * Level 3 => Fatal Error
 */
void __CKDebugLog(int level, const char* s, ...) {
	// Allocate a larger buffer to avoid overflows
	char* buffer = (char*)CKMalloc(250);
	if (!buffer) {
		DebugStr("\pCant Allocate Mem to Print Debug Str! (1)");
		ExitToShell();
		return;
	}

	va_list va;
	va_start(va, s);
	const int ret = vsprintf(buffer, s, va); // Retro68 supports `vsprintf`
	va_end(va);

	if (ret < 0 || ret >= 250) { // Check if vsprintf overflowed
		DebugStr("\pCant do vsprintf to Print Debug Str!");
		CKFree(buffer);
		return;
	}

	// Prepend the log level manually without snprintf
	const char* logPrefix;
	switch (level) {
		case 1:
			logPrefix = "< WARN > ";
			break;
		case 2:
			logPrefix = "<!ERR!!> ";
			break;
		default:
			logPrefix = "[ LOG  ] ";
			break;
	}

	// Create a new buffer for the prefixed log
	char* finalBuffer = (char*)CKMalloc(300);
	if (!finalBuffer) {
		DebugStr("\pCant Allocate Mem to Print Debug Str! (2)");
		CKFree(buffer);
		ExitToShell();
		return;
	}

	// Concatenate manually (safer than strcat)
	int prefixLen = strlen(logPrefix);
	int textLen = strlen(buffer);
	int totalLen = prefixLen + textLen;

	if (totalLen >= 299) {
		textLen = 299 - prefixLen; // Truncate text to fit
	}

	memcpy(finalBuffer, logPrefix, prefixLen);
	memcpy(finalBuffer + prefixLen, buffer, textLen);
	finalBuffer[prefixLen + textLen] = '\0'; // Ensure null termination

	CKConsolePrint(finalBuffer);

	CKFree(buffer);
	CKFree(finalBuffer);
}

/**
 * Print to wherever the OS expects us to print.
 */
void CKConsolePrint(const char* toPrint) {

#ifdef kCKAPPDEBUG
	if (!toPrint)
		return; // Avoid passing NULL to DebugStr

	unsigned char* toPStrd = CKC2P(toPrint);
	if (!toPStrd) {
		DebugStr("\pCant Allocate Mem to Print Debug Str! (2)");
		ExitToShell();
	} else {
		DebugStr(toPStrd);
		CKFree(toPStrd);
	}
#endif
}

#ifdef kCKAPPDEBUG

short __eddf = -1;

void __CKWriteToExitFile(const char* s, ...) {

	char* buffer = (char*)CKMalloc(250);
	if (!buffer) {
		// We are in deep shit.
		DebugStr("\pCant Allocate Mem to Print Debug Str! (1)");
		ExitToShell();
		return;
	}

	va_list va;
	va_start(va, s);
	const int ret = vsprintf(buffer, s, va);
	va_end(va);

	if (ret < 0) {
		DebugStr("\pCant do vsprintf to Print Debug Str!");
		return;
	}

	CKConsolePrint(buffer);

	if (__eddf != -1) {
		long l = strlen(buffer);
		FSWrite(__eddf, &l, buffer);
		l = 1;
		char nl[1];
		nl[0] = '\r';
		FSWrite(__eddf, &l, nl);
	}

	CKFree(buffer);
}

void CKPrintExitDebugData() {
	// Delete the file, just in case.
	OSErr err;
	err = FSDelete("\pCKDebugResults", 0);
	if (err != 0 && err != fnfErr) {
		CKLog("Can't delete to write exit debug data. Error = %d", err);
		goto start;
	}
	err = Create("\pCKDebugResults", 0, 'ttxt', 'TEXT');
	if (err != 0) {
		CKLog("Can't create to write exit debug data. Error = %d", err);
		goto start;
	}
	err = FSOpen("\pCKDebugResults", 0, &__eddf);
	if (err != 0) {
		CKLog("Can't open to write exit debug data. Error = %d", err);
		goto start;
	}

start:
#ifdef kCKDEBUGMEMORY
	CKMemoryDumpLeaks();
#endif
	__CKWriteToExitFile("\r");
	CKPrintProfileData();

	if (__eddf != -1) {
		FSClose(__eddf);
	}
}

void CKPrintProfileData() {
	__CKWriteToExitFile("Profiling data:\r");

	__CKWriteToExitFile("Highest:\r");

	for (auto& d : _profilerData) {
		if (d->totalTime > 1 && (d->totalTime / d->calls) > 1) {
			__CKWriteToExitFile("%s: %d total ticks, %d calls. (Avg: %d ticks)", d->name, d->totalTime, d->calls, (d->totalTime / d->calls));
		}
	}

	__CKWriteToExitFile("\rAll:\r");
	for (auto& d : _profilerData) {
		if (d->calls > 1) {
			__CKWriteToExitFile("%s: %d total ticks, %d calls. (Avg: %d ticks)", d->name, d->totalTime, d->calls, (d->totalTime / d->calls));
		} else {
			__CKWriteToExitFile("%s: %d total ticks.", d->name, d->totalTime);
		}
	}
	__CKWriteToExitFile("-- END OF PROFILE LIST --");
}
#endif

/**
 * @brief Checks if Appearance Manager is present (8.x+)
 * @return True if Appearance Manager is available
 */
bool CKHasAppearanceManager() {
	long result;
	return (Gestalt(gestaltAppearanceAttr, &result) == noErr) && (result & (1 << gestaltAppearanceExists));
}

/**
 * @brief Return the number of milliseconds since computer booted up.
 * @return
 */
UInt32 CKMillis() {
	UnsignedWide usec;
	Microseconds(&usec);
	return usec.lo / 1000;
}