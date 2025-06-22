#include "ckMemory.h"
#include "ckUtils.h"
#include <algorithm>
#include <string.h>
#include <vector>

#if kCKDEBUGMEMORY

static unsigned long totalAllocated = 0;
static unsigned long totalFreed = 0;
struct CKAllocdMemory {
		void* ptr;
		size_t size;
		bool viaNew;
		char* funcName;
		char* fileName;
		int line;
		char* f_funcName;
		char* f_fileName;
		int f_line;
};

static std::vector<CKAllocdMemory> __allocationRecs;

CKAllocdMemory __CKReportAllocation(const char* func, int line, const char* _file, size_t size, bool viaNew) {

	CKAllocdMemory rec;
	rec.ptr = dlmalloc(size);
	rec.size = size;
	rec.viaNew = viaNew;
	rec.funcName = (char*)dlmalloc(strlen(func) + 1);
	strcpy(rec.funcName, func);
	const char* fileNameStart = strrchr(_file, '/') ? strrchr(_file, '/') + 1 : _file;
	rec.fileName = (char*)dlmalloc(strlen(fileNameStart) + 1);
	strcpy(rec.fileName, fileNameStart);
	rec.line = line;

	__allocationRecs.push_back(rec);
	totalAllocated += size;

	return rec;
}

void __CKReportDeallocation(void* ptr, const char* func, int line, const char* _file, bool viaDelete) {

	if (!ptr) {
		return;
	}

	char* lastSlash = strrchr(_file, '/');
	char* file = 0;
	if (lastSlash != nullptr) {
		file = lastSlash + 1;
	}

	auto it = std::find_if(__allocationRecs.begin(), __allocationRecs.end(), [ptr](auto& rec) { return rec.ptr == ptr; });
	if (it == __allocationRecs.end()) {
		CKDebugLog(1, "%d@%s: %x is NOT on allocation list. (%s)", line, file, ptr, viaDelete ? "Delete" : "Free");
	} else {
		totalFreed += it->size;
		__allocationRecs.erase(it);
	}
}

/**
 * Wrapper for `new` used to find potential leaks.
 */
void* __CKNew(const char* func, int line, const char* file, size_t size) {
	CKAllocdMemory p = __CKReportAllocation(func, line, file, size, true);
	return p.ptr;
}

/**
 * Wrapper for `delete` used to find potential leaks.
 */
void __CKDelete(void* ptr, const char* func, int line, const char* file) {
	__CKReportDeallocation(ptr, func, line, file, true);
	dlfree(ptr);
}

/**
 * Wrapper for `malloc` used to find potential leaks.
 */
void* __CKMalloc(const char* func, int line, const char* file, size_t size) {
	CKAllocdMemory p = __CKReportAllocation(func, line, file, size, false);
	return p.ptr;
}

/**
 * @internal
 * Wrapper for `free` used to find potential leaks.
 */
void __CKFree(const char* func, int line, const char* file, void* ptr) {
	__CKReportDeallocation(ptr, func, line, file, false);
	dlfree(ptr);
}

#endif

/**
 * Return the stats about memory usage.
 */
void CKMemoryUsage(size_t* _totalAllocd, size_t* _totalFreed, int* _numOfPtrs) {

#ifdef kCKDEBUGMEMORY
	*_totalAllocd = totalAllocated;
	*_totalFreed = totalFreed;
	*_numOfPtrs = __allocationRecs.size();
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

	for (auto& p : __allocationRecs) {
		__CKWriteToExitFile("-- <%s> %d bytes - %s on %s, Line %d", p.viaNew ? "new" : "mal", p.size, p.funcName, p.fileName, p.line);
	}

	__CKWriteToExitFile("-- END OF LEAK LIST --");

#else

	CKLog("Debugging of memory not enabled");

#endif
}

/**
 * @internal
 * @brief Copy a string (if it is a string) to a destination.
 * Free the destination if there is already something there.
 * @param dest
 * @param src
 * @return True on success
 */
bool __CKSafeCopyString(char** dest, const char* src, const char* func, int line, const char* file) {
	if (*dest) {
#ifdef kCKDEBUGMEMORY
		__CKFree(func, line, file, *dest);
#else
		CKFree(*dest);
#endif
		*dest = NULL;
	}
	if (src) {
		size_t len = strlen(src) + 1;
#ifdef kCKDEBUGMEMORY
		*dest = (char*)__CKMalloc(func, line, file, len);
#else
		*dest = (char*)CKMalloc(len);
#endif
		if (*dest) {
			strncpy(*dest, src, len);
			return true;
		} else {
			return false;
		}
	}
	return true;
}