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

class CKObject;
class CKWindow;
class CKTimer;
class CKNetBaseSocket;
struct CKWindowInitParams;
struct CKMenuBar;
struct CKMenuBarItem;
struct CKMenuItem;

class CKApp {

	public:
		CKApp();
		~CKApp();

		int CKLoop(int waitTime = 60);
		void CKQuit();

		CKWindow* CKNewWindow(const CKWindowInitParams& params);
		CKWindow* CKFindWindow(CKWindowPtr ptr);
		void CKRemoveWindow(CKWindow* window);

		void CKIncreaseWork();
		void CKDecreaseWork();
		void CKRestoreCursor();

		CKWindow* CKNewMsgBoxPlain(const char* message, const char* title = nullptr, const char* btnOk = "OK", const char* btnCancel = nullptr, std::function<void(int button)> callback = 0);
		CKWindow* CKNewMsgBoxNote(const char* message, const char* title = nullptr, const char* btnOk = "OK", const char* btnCancel = nullptr, std::function<void(int button)> callback = 0);
		CKWindow* CKNewMsgBoxWarning(const char* message, const char* title = nullptr, const char* btnOk = "OK", const char* btnCancel = "Cancel", std::function<void(int button)> callback = 0);
		CKWindow* CKNewMsgBoxError(const char* message, const char* title = nullptr, const char* btnOk = "OK", const char* btnCancel = "Cancel", std::function<void(int button)> callback = 0);

		CKWindow* CKTopMostWindow();

		short CKFontToId(const char* font);

		void CKAddTimer(CKTimer* timer, CKObject* owner = nullptr);
		void CKRemoveTimer(CKTimer* timer);
		void CKRemoveTimersOfOwner(CKObject* owner);

		CKError CKSetMenu(CKMenuBar* menu);
		void CKShowMenuBar();
		void CKHideMenuBar();

	private:
		void __DoHousekeepingTasks();
		void __DispatchEvent(EventRecord event);
		inline void __HandleEvtKey(EventRecord event, bool isKeyUp, bool isAutoKey);
		inline void __HandleEvtMouseDown(EventRecord event);
		inline void __HandleEvtMouseUp(EventRecord event);
		inline void __HandleEvtMouseMove(EventRecord event);
		inline void __HandleEvtUpdate(EventRecord event);
		inline void __HandleEvtActivate(EventRecord event);
		inline void __HandleEvtOS(EventRecord event);
		void __HandleMenuPropertyChange(const CKObject* obj, const char* propName);
		CKWindow* __CreateAlertDialog(const char* title, const char* message, const CKSystemIcon icon, const char* btnOk = "OK", const char* btnCancel = 0, std::function<void(int button)> callback = 0);

	private:
		int __workCount;
		CKWindow* __lastMouseDownWindow;
		std::vector<CKWindow*> __windows;
		std::vector<CKWindow*> __gc_windows;
		std::vector<CKTimer*> __timers;
		CKMenuBar* __menubar = nullptr;

		friend CKNetBaseSocket;
		std::vector<CKNetBaseSocket*> __net_sockets;
};