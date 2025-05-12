#include "ckMemory.h"
#include "ckUtils.h"
#include <string.h>

#ifdef kCKDEBUGMEMORY
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

#ifdef kCKDEBUGMEMORY

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
#ifdef kCKAPPDEBUG
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

#ifdef kCKAPPDEBUG
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

#ifdef kCKDEBUGMEMORY
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

#ifdef kCKDEBUGMEMORY

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

/**
 * @brief Copy a string (if it is a string) to a destination.
 * Free the destination if there is already something there.
 * @param dest
 * @param src
 * @return True on success
 */
bool CKSafeCopyString(char** dest, const char* src) {
	if (*dest) {
		CKFree(*dest);
		*dest = NULL;
	}
	if (src) {
		size_t len = strlen(src) + 1;
		*dest = (char*)CKMalloc(len);
		if (*dest) {
			strncpy(*dest, src, len);
			return true;
		} else {
			return false;
		}
	}
	return true;
}