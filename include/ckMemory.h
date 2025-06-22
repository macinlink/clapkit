/*
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

/** @internal */ void* __CKMalloc(const char* func, int line, const char* file, size_t size);

/** @internal */ void __CKFree(const char* func, int line, const char* file, void* ptr);
/** @internal */ void* __CKNew(const char* func, int line, const char* file, size_t size);
/** @internal */ void __CKDelete(void* ptr, const char* func, int line, const char* file);

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

/**
 * @ingroup Memory
 * @brief Use instead of malloc.
 */
#define CKMalloc(sz)  __CKMalloc(__func__, __LINE__, __FILENAME__, (sz))

/**
 * @ingroup Memory
 * @brief Use instead of free.
 */
#define CKFree(ptr)	  __CKFree(__func__, __LINE__, __FILENAME__, (ptr))

/**
 * @ingroup Memory
 * @brief Use instead of new.
 */
#define CKNew		  new (__func__, __LINE__, __FILENAME__)

/**
 * @ingroup Memory
 * @brief Use instead of delete.
 */
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

/**
 * @ingroup Memory
 * @brief Copy a string (if it is a string) to a destination.
 * Free the destination if there is already something there.
 * @param dest
 * @param src
 * @return True on success
 */
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