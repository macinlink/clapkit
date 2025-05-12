/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKMemory
 * ----------------------------------------------------------------------
 * Memory-related functionality
 *
 */

#pragma once

#include "ckUtils.h"
#include "dlmalloc.h"
#include <Memory.h>
#include <cstddef>

/**
 * Wrappers for malloc/free so we can track leaks.
 * Set `kCKDEBUGMEMORY` to true to enable.
 */
#ifdef kCKDEBUGMEMORY
#define CKMalloc(s) __CKMalloc(__func__, __LINE__, __FILENAME__, s)
#define CKFree(p)	__CKFree(__func__, __LINE__, __FILENAME__, p)
#else
#define CKMalloc(s) dlmalloc(s)
#define CKFree(p)	dlfree(p)
#endif

// Standard new/delete macros

#ifdef kCKDEBUGMEMORY

struct CKAllocdMemory;
void* operator new(size_t size, const char* func, int line, const char* file);
void* operator new[](size_t size, const char* func, int line, const char* file);
void operator delete(void* ptr, const char* file, const char* func, int line) noexcept;
void operator delete[](void* ptr, const char* file, const char* func, int line) noexcept;

CKAllocdMemory* __CKReportAllocation(const char* func, int line, const char* file, size_t size, bool viaNew);
void __CKReportDeallocation(void* ptr, const char* func, int line, const char* file, bool viaDelete = false);
void* __CKMalloc(const char* func, int line, const char* file, size_t size);
void __CKFree(const char* func, int line, const char* file, void* ptr);

#define CKNew new (__func__, __LINE__, __FILENAME__)

template <typename T>
void __CKDestroy(T* ptr, const char* file, const char* func, int line) noexcept {
	if (ptr != nullptr) {
		ptr->~T();								// Call the destructor
		operator delete(ptr, file, func, line); // Call the custom delete operator
	}
}

template <typename T>
void __CKDestroy_Array(T* ptr, const char* file, const char* func, int line) noexcept {
	if (ptr != nullptr) {
		for (std::size_t i = 0; i < sizeof(T); i++) {
			ptr[i].~T(); // Call the destructor for each element
		}
		operator delete[](ptr, file, func, line); // Call the custom delete[] operator
	}
}

#define CKDelete(ptr)	   __CKDestroy(ptr, __FILENAME__, __func__, __LINE__)
#define CKDeleteArray(ptr) __CKDestroy_Array(ptr, __FILENAME__, __func__, __LINE__)

#else

#define CKNew			   new
#define CKDelete(ptr)	   delete ptr
#define CKDeleteArray(ptr) delete[] ptr

#endif

void CKMemoryUsage(size_t* totalAllocd, size_t* totalFreed, int* numOfPtrs);
void CKMemoryDumpLeaks();

bool CKSafeCopyString(char** dest, const char* src);