/*
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

#include "ckMemory.h"
#include "dlmalloc.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>
#include <stdio.h>

#ifdef kCKAPPDEBUG
#define __FILENAME__ __FILE__
#else
#define __FILENAME__ ""
#endif

/**
 * @ingroup Utils
 * @brief Convert milliseconds to ticks
 */
#define CKMSToTicks(milliseconds) ((int)((milliseconds) / 16.66))

#ifdef kCKAPPDEBUG

/**
 * @brief Wrapper for logging.
 * @code
 * CKLog("Hello! My value is %d", myval);
 * @endcode
 * @note Only active if kCKAPPDEBUG defined.
 */
#define CKLog(s, ...)		  __CKDebugLog(0, s __VA_OPT__(, ) __VA_ARGS__)
#define CKDebugLog(l, s, ...) __CKDebugLog(l, s __VA_OPT__(, ) __VA_ARGS__)
#else

/**
 * @brief Wrapper for logging.
 * @note Only active if kCKAPPDEBUG defined.
 */
#define CKLog(s, ...) \
	do {              \
	} while (0)
#define CKDebugLog(l, s, ...) \
	do {                      \
	} while (0)
#endif

unsigned char* __CKC2P(const char* src, const char* func, int line, const char* file);
char* __CKP2C(const unsigned char* src, const char* func, int line, const char* file);

#ifdef kCKAPPDEBUG
/**
 * @ingroup Utils
 * @brief Convert a C-string to a Pascal-string
 * @returns unsigned char* or null on failure.
 */
#define CKC2P(s) __CKC2P(s, __func__, __LINE__, __FILENAME__)

/**
 * @ingroup Utils
 * @brief Convert a Pascal-string to a C-String
 * @returns char* or null on failure.
 */
#define CKP2C(s) __CKP2C(s, __func__, __LINE__, __FILENAME__)
#else
/**
 * @ingroup Utils
 * @brief Convert a C-string to a Pascal-string
 */
#define CKC2P(s) __CKC2P(s, "", 0, "")

/**
 * @ingroup Utils
 * @brief Convert a Pascal-string to a C-String
 * @returns char* or null on failure.
 */
#define CKP2C(s) __CKP2C(s, "", 0, "")
#endif

void __CKDebugLog(int level, const char* s, ...);
void CKConsolePrint(const char* toPrint);

/**
 * Profiling
 */
#ifdef kCKAPPDEBUG
#include <Timer.h>
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
void __CKWriteToExitFile(const char* s, ...);
#define CKPROFILE CKScopeProfiler __p(__PRETTY_FUNCTION__);
#else
#define CKPROFILE
#endif

bool CKHasAppearanceManager();
bool CKHasColorQuickDraw();
bool CKHasDebugger();
UInt32 CKMillis();
