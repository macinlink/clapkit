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
#include "ckApp.h"

#ifdef CKAPPDEBUG
std::vector<CKProfilerData*> _profilerData;
bool _profilerDataInit = false;
#endif

#if CK_DEBUG_MEMORY == true
static unsigned long totalAllocated = 0;
static unsigned long totalFreed = 0;
static int totalPointers = 0;
struct CKAllocdMemory {
		void* ptr;
		size_t size;
		bool viaNew;
		CKAllocdMemory* prev;
		char* funcName;
		char* fileName;
		int line;
		char* f_funcName;
		char* f_fileName;
		int f_line;
};

CKAllocdMemory* lastAlloc = 0;
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

#if CK_DEBUG_MEMORY == true
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
#if CK_DEBUG_MEMORY == true
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
	char* logPrefix;
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

#ifdef CKAPPDEBUG
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

#if CK_DEBUG_MEMORY == true

void* operator new(size_t size, const char* func, int line, const char* file) {

	if (size == 0) {
		CKLog("new called with size of zero.");
		return nil;
	}

	CKAllocdMemory* p = __CKReportAllocation(func, line, file, size, true);
	p->ptr = dlmalloc(size);
	return p->ptr;
}

void* operator new[](size_t size, const char* func, int line, const char* file) {

	if (size == 0) {
		CKLog("new called with size of zero.");
		return nil;
	}

	CKAllocdMemory* p = __CKReportAllocation(func, line, file, size, true);
	p->ptr = dlmalloc(size);
	return p->ptr;
}

void operator delete(void* ptr, const char* file, const char* func, int line) noexcept {
	if (ptr != nullptr) {
		__CKReportDeallocation(ptr, func, line, file, true);
		dlfree(ptr);
	}
}

void operator delete[](void* ptr, const char* file, const char* func, int line) noexcept {
	if (ptr != nullptr) {
		__CKReportDeallocation(ptr, func, line, file, true);
		dlfree(ptr);
	}
}

CKAllocdMemory* __CKReportAllocation(const char* func, int line, const char* _file, size_t size, bool viaNew) {

	char* lastSlash = strrchr(_file, '/');
	char* file = 0;
	if (lastSlash != nullptr) {
		file = lastSlash + 1;
	}

	CKAllocdMemory* p = (CKAllocdMemory*)dlmalloc(sizeof(*p));
	p->size = size;
	p->viaNew = viaNew;
	p->prev = lastAlloc;

	p->funcName = (char*)dlmalloc(strlen(func) + 1);
	strcpy(p->funcName, func);
	p->fileName = (char*)dlmalloc(strlen(file) + 1);
	strcpy(p->fileName, file);
	p->line = line;

	lastAlloc = p;
	totalAllocated += size;

	return p;
}

void __CKReportDeallocation(void* ptr, const char* func, int line, const char* _file, bool viaDelete) {

	char* lastSlash = strrchr(_file, '/');
	char* file = 0;
	if (lastSlash != nullptr) {
		file = lastSlash + 1;
	}

	// Try to find the thingy.
	CKAllocdMemory* p = lastAlloc;
	CKAllocdMemory* p_prev = 0;
	bool found = false;
	while (p) {
		if (p->ptr == ptr) {
			// We got it! Re-link the list..
			if (p_prev && p) {
				p_prev->prev = p->prev;
			} else {
				lastAlloc = p->prev;
			}
			totalFreed += p->size;
			found = true;
			break;
		}
		p_prev = p;
		p = p->prev;
	}

	if (!found) {
		CKDebugLog(0, "On Function: %s, Line: %d, File: %s:", func, line, file);
		CKDebugLog(1, " - %s called on pointer (%x) not on memory alloc list!", viaDelete ? "Delete" : "Free", ptr);
	}
}

/**
 * Wrapper for malloc used to find potential leaks.
 */
void* __CKMalloc(const char* func, int line, const char* file, size_t size) {

	CKAllocdMemory* p = __CKReportAllocation(func, line, file, size, false);
#ifdef CKAPPDEBUG
	p->ptr = NewPtr(size);
	if (MemError()) {
		DebugStr("\pCan't allocate using NewPtr!");
		p->ptr = 0;
		return 0;
	}
	return p->ptr;
#else
	p->ptr = dlmalloc(size);
	return p->ptr;
#endif
}

/**
 * Wrapper for free used to find potential leaks.
 */
void __CKFree(const char* func, int line, const char* file, void* ptr) {

	__CKReportDeallocation(ptr, func, line, file);

#ifdef CKAPPDEBUG
	DisposePtr((Ptr)ptr);
	int r = MemError();
	if (r) {
		char t[255];
		sprintf(t, "Can't free using DisposePtr! Error code: %d", r);
		CKConsolePrint(t);
		sprintf(t, " -- From: %s, line %d on %s", func, line, file);
		CKConsolePrint(t);
	}
#else
	dlfree(ptr);
#endif
}

#endif

/**
 * Return the stats about memory usage.
 */
void CKMemoryUsage(size_t* _totalAllocd, size_t* _totalFreed, int* _numOfPtrs) {

#if CK_DEBUG_MEMORY == true
	CKAllocdMemory* p = lastAlloc;
	*_totalAllocd = totalAllocated;
	*_totalFreed = totalFreed;
	*_numOfPtrs = totalPointers;
#else
	*_totalAllocd = 0;
	*_totalFreed = 0;
	*_numOfPtrs = 0;
#endif
}

/**
 * (Try to) print all the leaks to the console.
 */
void CKMemoryDumpLeaks() {

#if CK_DEBUG_MEMORY == true

	size_t leak = totalAllocated - totalFreed;
	__CKWriteToExitFile("Memory Dump: %d bytes alloc'd, %d bytes freed.", totalAllocated, totalFreed);

	if (leak == 0) {
		__CKWriteToExitFile("Congrats, you have zero leaks!");
		return;
	} else {
		__CKWriteToExitFile("- %d bytes leaked:", leak);
	}

	CKAllocdMemory* p = lastAlloc;

	while (p) {
		__CKWriteToExitFile("-- <%s> %d bytes - %s on %s, Line %d", p->viaNew ? "new" : "mal", p->size, p->funcName, p->fileName, p->line);
		p = p->prev;
	}

	__CKWriteToExitFile("-- END OF LEAK LIST --");

#else

	CKLog("Debugging of memory not enabled");

#endif
}

#ifdef CKAPPDEBUG

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
#if CK_DEBUG_MEMORY == true
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