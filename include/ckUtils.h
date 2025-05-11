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

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include "dlmalloc.h"

#ifdef CKAPPDEBUG
    #define CK_DEBUG_MEMORY     true
#else
    #ifndef CK_DEBUG_MEMORY
    #define CK_DEBUG_MEMORY     false
    #endif
    #ifndef NDEBUG
    #define NDEBUG
    #endif
#endif

#ifdef CKAPPDEBUG
// #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #define __FILENAME__ __FILE__
#else
    #define __FILENAME__ ""
#endif

#define CKMSToTicks(milliseconds) ((int)((milliseconds) / 16.66))

/**
 * Wrapper for simple logs.
*/
#ifdef CKAPPDEBUG
    #define CKLog(s, ...) __CKDebugLog(0, s __VA_OPT__(,) __VA_ARGS__)
    #define CKDebugLog(l, s, ...) __CKDebugLog(l, s __VA_OPT__(,) __VA_ARGS__)
#else
    #define CKLog(s, ...) do { } while(0)
    #define CKDebugLog(s, ...) do { } while(0)
#endif

/**
 * Wrappers for malloc/free so we can track leaks.
 * Set `CK_DEBUG_MEMORY` to true to enable.
*/
#if CK_DEBUG_MEMORY == true
    #define CKMalloc(s) __CKMalloc(__func__, __LINE__, __FILENAME__, s)
    #define CKFree(p) __CKFree(__func__, __LINE__, __FILENAME__, p)
#else
    #define CKMalloc(s) dlmalloc(s)
    #define CKFree(p) dlfree(p)
#endif

/**
 * These are hidden behind a macro as they might leak easily.
*/

unsigned char* __CKC2P(const char* src, const char* func, int line, const char* file);
char* __CKP2C(const unsigned char* src, const char* func, int line, const char* file);

/**
 * Use these macros instead.
*/

#ifdef CKAPPDEBUG
    #define CKC2P(s) __CKC2P(s, __func__, __LINE__, __FILENAME__)
    #define CKP2C(s) __CKP2C(s, __func__, __LINE__, __FILENAME__)
#else
    #define CKC2P(s) __CKC2P(s, "", 0, "")
    #define CKP2C(s) __CKP2C(s, "", 0, "")
#endif

void __CKDebugLog(int level, const char* s, ...);
void CKConsolePrint(const char* toPrint);

#if CK_DEBUG_MEMORY == true

    struct CKAllocdMemory;
    void* operator new(size_t size, const char* func, int line, const char* file);
    void* operator new[](size_t size, const char* func, int line, const char* file);
    void operator delete(void* ptr, const char* file, const char* func, int line) noexcept;
    void operator delete[](void* ptr, const char* file, const char* func, int line) noexcept;

    CKAllocdMemory* __CKReportAllocation(const char* func, int line, const char* file, size_t size, bool viaNew);
    void __CKReportDeallocation(void* ptr, const char* func, int line, const char* file, bool viaDelete = false);
    void* __CKMalloc(const char* func, int line, const char* file, size_t size);
    void __CKFree(const char* func, int line, const char* file, void* ptr);

    #define CKNew new(__func__, __LINE__, __FILENAME__)

    template<typename T>
    void __CKDestroy(T* ptr, const char* file, const char* func, int line) noexcept {
        if (ptr != nullptr) {
            ptr->~T(); // Call the destructor
            operator delete(ptr, file, func, line); // Call the custom delete operator
        }
    }

    template<typename T>
    void __CKDestroy_Array(T* ptr, const char* file, const char* func, int line) noexcept {
        if (ptr != nullptr) {
            for (std::size_t i = 0; i < sizeof(T); i++) {
                ptr[i].~T(); // Call the destructor for each element
            }
            operator delete[](ptr, file, func, line); // Call the custom delete[] operator
        }
    }

    #define CKDelete(ptr) __CKDestroy(ptr, __FILENAME__, __func__, __LINE__)
    #define CKDeleteArray(ptr) __CKDestroy_Array(ptr, __FILENAME__, __func__, __LINE__)

#else

    // Standard new/delete macros
    #define CKNew new
    #define CKDelete(ptr) delete ptr
    #define CKDeleteArray(ptr) delete[] ptr

#endif
void CKMemoryUsage(size_t* totalAllocd, size_t* totalFreed, int* numOfPtrs);
void CKMemoryDumpLeaks();

/**
 * Profiling
*/
#ifdef CKAPPDEBUG
    #include <vector>
    class CKProfilerData {
        public:
            char* name;
            long totalTime;
            int calls;
    };
    extern std::vector<CKProfilerData*> _profilerData;
    extern bool _profilerDataInit;
    class CKScopeProfiler {
        public:
            CKScopeProfiler(const char* fName) {
                this->name = (char*)dlmalloc(strlen(fName) + 1);
                strcpy(this->name, fName);
                // CKLog("PROFILE: %s started.", this->name);
                this->start = TickCount();
            }
            ~CKScopeProfiler() {
                long time = (TickCount() - this->start);
                // CKLog("PROFILE: %s ended. Took %lu ticks.", this->name, time);
                bool found = false;
                if (!_profilerDataInit) {
                    _profilerData = std::vector<CKProfilerData*>();
                    _profilerDataInit = true;
                }
                for (auto& d : _profilerData) {
                    if (strcmp(d->name, this->name) == 0) {
                        d->totalTime += time;
                        d->calls++;
                        found = true;
                    }
                }
                if (!found) {
                    CKProfilerData* nd = new CKProfilerData();
                    nd->name = (char*)dlmalloc(strlen(this->name) + 1);
                    strcpy(nd->name, this->name);
                    nd->totalTime = time;
                    nd->calls = 1;
                    _profilerData.push_back(nd);
                }
                dlfree(this->name);
            }
        private:
            char* name;
            long start;
    };
    void CKPrintProfileData();
    void CKPrintExitDebugData();
    void __CKWriteToExitFile( const char* s, ...);
    #define CKPROFILE    CKScopeProfiler __p(__PRETTY_FUNCTION__);
#else
    #define CKPROFILE
#endif