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

bool __CKSafeCopyString(char** dest, const char* src, const char* func = 0, int line = 0, const char* file = 0);
void CKMemoryUsage(size_t* totalAllocd, size_t* totalFreed, int* numOfPtrs);
void CKMemoryDumpLeaks();

#ifdef kCKDEBUGMEMORY

void* __CKMalloc(const char* func, int line, const char* file, size_t size);
void __CKFree(const char* func, int line, const char* file, void* ptr);
void* __CKNew(const char* func, int line, const char* file, size_t size);
void __CKDelete(void* ptr, const char* func, int line, const char* file);

template <typename T>
inline void __CKDestroy(T* ptr, const char* file, const char* func, int line) noexcept {
	if (ptr) {
		ptr->~T();
		__CKFree(func, line, file, ptr);
	}
}

template <typename T>
inline void __CKDestroyArray(T* ptr, int count, const char* file, const char* func, int line) noexcept {
	if (ptr) {
		for (int i = 0; i < count; ++i)
			ptr[i].~T();
		__CKFree(func, line, file, ptr);
	}
}

#define CKMalloc(sz)  __CKMalloc(__func__, __LINE__, __FILENAME__, (sz))
#define CKFree(ptr)	  __CKFree(__func__, __LINE__, __FILENAME__, (ptr))

#define CKNew		  new (__func__, __LINE__, __FILENAME__)
#define CKDelete(ptr) __CKDestroy(ptr, __FILENAME__, __func__, __LINE__)

#define CKNewArray	  new (__func__, __LINE__, __FILENAME__)
#define CKDeleteArray(ptr, count) \
	__CKDestroyArray(ptr, (count), __FILENAME__, __func__, __LINE__)

inline void* operator new(size_t size, const char* func, int line, const char* file) {
	return __CKNew(func, line, file, size);
}
inline void* operator new[](size_t size, const char* func, int line, const char* file) {
	return __CKNew(func, line, file, size);
}

#define CKSafeCopyString(dest, src) __CKSafeCopyString(&dest, src, __func__, __LINE__, __FILENAME__)

#else

#define CKMalloc(size)				dlmalloc(size)
#define CKFree(ptr)					dlfree(ptr)
#define CKNew						new
#define CKDelete(ptr)				delete ptr
#define CKNewArray					new[]
#define CKDeleteArray(ptr, count)	delete[] ptr

#define CKSafeCopyString(dest, src) __CKSafeCopyString(&dest, src, 0, 0, 0)

#endif