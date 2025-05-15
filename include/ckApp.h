/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKApp
 * ----------------------------------------------------------------------
 * Defines an application.
 *
 */

#pragma once

// Need to move these somewhere else soon.
#ifndef TARGET_OS_MAC
#define TARGET_OS_MAC true
// #define TARGET_API_MAC_OS8      true
// #define TARGET_API_MAC_CARBON   false
#endif

#include "ckMacros.h"
#include "ckPlatform.h"
#include "ckTypes.h"
#include "ckUtils.h"
#include <Dialogs.h>
#include <Events.h>
#include <Fonts.h>
#include <MacTypes.h>
#include <MacWindows.h>
#include <Memory.h>
#include <Menus.h>
#include <Quickdraw.h>
#include <functional>
#include <stdlib.h>
#include <vector>

class CKWindow;
class CKTimer;
struct CKWindowInitParams;

class CKApp {

	public:
		CKApp();
		~CKApp();

		int Loop(int waitTime = 60);
		void Quit();

		CKWindow* CKNewWindow(const CKWindowInitParams& params);
		CKWindow* CKFindWindow(CKWindowPtr ptr);
		void CKRemoveWindow(CKWindow* window);

		void IncreaseWork();
		void DecreaseWork();
		void RestoreCursor();

		CKWindow* CKNewAlert(const char* title, const char* message, const char* btnOk = "OK", const char* btnCancel = 0, std::function<void(int button)> callback = 0);

		CKWindow* TopMostWindow();

		short FontToId(const char* font);

		void AddTimer(CKTimer* timer);

	private:
		inline void HandleEvtKey(EventRecord event, bool isKeyUp, bool isAutoKey);
		inline void HandleEvtMouseDown(EventRecord event);
		inline void HandleEvtMouseUp(EventRecord event);
		inline void HandleEvtMouseMove(EventRecord event);
		inline void HandleEvtUpdate(EventRecord event);
		inline void HandleEvtActivate(EventRecord event);
		inline void HandleEvtOS(EventRecord event);

	public:
	protected:
		std::vector<CKWindow*> __windows;
		std::vector<CKTimer*> __timers;

	private:
		int workCount;
		CKWindow* lastMouseDownWindow;
		std::vector<CKWindow*> __gc_windows;
};