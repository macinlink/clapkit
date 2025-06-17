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

#include "ckApp.h"
#include "ckButton.h"
#include "ckCanvas.h"
#include "ckLabel.h"
#include "ckMenu.h"
#include "ckNetBaseSocket.h"
#include "ckNetworking.h"
#include "ckTimer.h"
#include "ckUtils.h"
#include "ckWindow.h"
#include <Appearance.h>
#include <Devices.h>
#include <Quickdraw.h>

CKApp* __ckgCurrentCKApp = nullptr;

/**
 * Initialize the app, set up menus, etc.
 */
CKApp::CKApp() {

#if !TARGET_API_MAC_CARBON
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
#endif
	InitCursor();

	// Is/was this necessary?
	MaxApplZone();
	for (int i = 0; i < 10; i++) {
		MoreMasters();
	}

	if (CKHasAppearanceManager()) {
		RegisterAppearanceClient();
	}

	this->__workCount = 0;
	this->__windows = std::vector<CKWindow*>();
	this->__gc_windows = std::vector<CKWindow*>();
	this->__lastMouseDownWindow = nil;

	__ckgCurrentCKApp = this;
}

/**
 * Nothing much ere.
 */
CKApp::~CKApp() {
	__ckgCurrentCKApp = nullptr;
}

/**
 * Main event loop, handling events.
 * Needs to be called as much as possible.
 *
 * Returns:
 *  0   All OK, keep going.
 *  1   App is going to quit, clean up.
 */
int CKApp::CKLoop(int waitTime) {

	static EventRecord e;

	CKPROFILE

	if (this->__gc_windows.size() > 0) {
		CKLog("Doing Window garbage collectionism..");
		while (this->__gc_windows.size() > 0) {
			CKDelete(this->__gc_windows.at(0));
			this->__gc_windows.erase(this->__gc_windows.begin());
		}
	}

	UInt32 now = CKMillis();
	UInt32 soonestDue = UINT32_MAX;

	for (CKTimer* timer : this->__timers) {
		if (timer->enabled) {
			if (timer->nextRun <= now) {
				soonestDue = 0;
				break;
			}
			UInt32 wait = timer->nextRun - now;
			if (wait < soonestDue)
				soonestDue = wait;
		}
	}

	// Convert to ticks (~16.67ms per tick)
	UInt16 sleepTicks = (soonestDue == UINT32_MAX) ? waitTime : soonestDue / 17;
	if (sleepTicks < 1) {
		sleepTicks = 1;
	} else {
		if (sleepTicks > waitTime) {
			sleepTicks = waitTime;
		}
	}

	if (!WaitNextEvent(everyEvent, &e, sleepTicks, nil)) {

		static int lastMousePosX = 0;
		static int lastMousePosY = 0;

		if (e.where.h != lastMousePosX || e.where.v != lastMousePosY) {
			// TODO: We might be overcalling this call.
			// Maybe check if there has been a few ticks between movements
			// so we don't hammer this?
			this->__HandleEvtMouseMove(e);
			lastMousePosX = e.where.h;
			lastMousePosY = e.where.v;
		}

		goto doTimers;
	}

	this->__DispatchEvent(e);

doTimers:
	this->__DoHousekeepingTasks();
	return 0;
}

/**
 * Flush events, clean things up, quit.
 */
void CKApp::CKQuit() {

	CKPROFILE

	this->CKIncreaseWork();

	while (this->__windows.size() > 0) {
		this->CKRemoveWindow(this->__windows.at(0));
	}

	for (auto& s : this->__net_sockets) {
		s->Close();
	}
	CKNetworking::Deinitialize();

	this->CKSetMenu(nullptr);

#ifdef kCKAPPDEBUG
	// We don't really need Garbage Collection here at this point
	// but we should or we'll report a lot of leaks.
	if (this->__gc_windows.size() > 0) {
		CKLog("Doing Window garbage collectionism..");
		while (this->__gc_windows.size() > 0) {
			CKDelete(this->__gc_windows.at(0));
			this->__gc_windows.erase(this->__gc_windows.begin());
		}
	}
	CKPrintExitDebugData();
#endif

	FlushEvents(everyEvent, -1);
	ExitToShell();
}

/**
 * @brief Create a new window and if successful, add it to the
 * list of the windows the app has. You MUST use this function
 * instead of creating and initializing a CKWindow yourself.
 * @param params
 * @return
 * @throws std::exception
 */
CKWindow* CKApp::CKNewWindow(const CKWindowInitParams& params) {

	CKPROFILE

	try {

		CKWindow* toReturn = CKNew CKWindow(params);
		toReturn->SetOwner(this);
		this->__windows.push_back(toReturn);
		return toReturn;

	} catch (const std::exception& e) {

		throw e;
	}
}

/**
 * @brief Try to find a window by the WindowPtr.
 * Probably only applies to Toolbox Mac.
 * @param ptr
 * @return
 */
CKWindow* CKApp::CKFindWindow(CKWindowPtr ptr) {

	CKPROFILE

	for (auto& w : this->__windows) {
		if ((CKWindowPtr)(w->__windowPtr) == (CKWindowPtr)ptr) {
			return w;
		}
	}

	return nil;
}

/**
 * @brief Remove and destroy window.
 * You MUST use this function instead of deleting a window yourself.
 *
 * We don't immediately delete a window but rather collect it for future
 * deletion - this is because mouseUp, etc. events might still pop
 * during execution of an event handler.
 *
 * In fact, when possible, use [window]->Close() function.
 * @param window
 */
void CKApp::CKRemoveWindow(CKWindow* window) {

	CKPROFILE

	bool found = false;
	for (auto it = this->__windows.begin(); it != this->__windows.end(); ++it) {
		if (*it == window) {
			it = this->__windows.erase(it);
			found = true;
			break;
		}
	}

	if (!found) {
		CKLog("CKRemoveWindow called for %x but can't find it!");
	} else {
		this->__gc_windows.push_back(window);
	}
}

/**
 * @brief Create and show an alert.
 * Alerts are non-blocking.
 * btnCancel is optional and only shown if set to a non-null value.
 * `button` is set to "1" if the user clicks on OK and "0" if
 * the user clicks on the Cancel button (if one is provided, that is.)
 * While this function returns a CKWindow, you don't have to call `Show`
 * on the window as the window will be displayed immediately.
 * @param message Message to be displayed (required)
 * @param title Title text (optional) - setting this will make the window non-modal.
 * @param btnOk 'OK' button text. If set to null, "OK" is used.
 * @param btnCancel Optional 'Cancel' button. If set to null, not displayed.
 * @param callback Returns 1 if OK is clicked, 0 if Cancel is clicked and -1 if the window was closed.
 * @return
 */
CKWindow* CKApp::CKNewMsgBoxNote(const char* message, const char* title, const char* btnOk, const char* btnCancel, std::function<void(int button)> callback) {
	return this->__CreateAlertDialog(message, title, CKSystemIcon::message, btnOk, btnCancel, callback);
}

/**
 * @brief Create and show an alert.
 * Alerts are non-blocking.
 * btnCancel is optional and only shown if set to a non-null value.
 * `button` is set to "1" if the user clicks on OK and "0" if
 * the user clicks on the Cancel button (if one is provided, that is.)
 * While this function returns a CKWindow, you don't have to call `Show`
 * on the window as the window will be displayed immediately.
 * @param message Message to be displayed (required)
 * @param title Title text (optional) - setting this will make the window non-modal.
 * @param btnOk 'OK' button text. If set to null, "OK" is used.
 * @param btnCancel Optional 'Cancel' button. If set to null, not displayed.
 * @param callback Returns 1 if OK is clicked, 0 if Cancel is clicked and -1 if the window was closed.
 * @return
 */
CKWindow* CKApp::CKNewMsgBoxPlain(const char* message, const char* title, const char* btnOk, const char* btnCancel, std::function<void(int button)> callback) {
	return this->__CreateAlertDialog(message, title, CKSystemIcon::noIcon, btnOk, btnCancel, callback);
}

/**
 * @brief Create and show an alert.
 * Alerts are non-blocking.
 * btnCancel is optional and only shown if set to a non-null value.
 * `button` is set to "1" if the user clicks on OK and "0" if
 * the user clicks on the Cancel button (if one is provided, that is.)
 * While this function returns a CKWindow, you don't have to call `Show`
 * on the window as the window will be displayed immediately.
 * @param message Message to be displayed (required)
 * @param title Title text (optional) - setting this will make the window non-modal.
 * @param btnOk 'OK' button text. If set to null, "OK" is used.
 * @param btnCancel Optional 'Cancel' button. If set to null, not displayed.
 * @param callback Returns 1 if OK is clicked, 0 if Cancel is clicked and -1 if the window was closed.
 * @return
 */
CKWindow* CKApp::CKNewMsgBoxWarning(const char* message, const char* title, const char* btnOk, const char* btnCancel, std::function<void(int button)> callback) {
	return this->__CreateAlertDialog(message, title, CKSystemIcon::warning, btnOk, btnCancel, callback);
}

/**
 * @brief Create and show an alert.
 * Alerts are non-blocking.
 * btnCancel is optional and only shown if set to a non-null value.
 * `button` is set to "1" if the user clicks on OK and "0" if
 * the user clicks on the Cancel button (if one is provided, that is.)
 * While this function returns a CKWindow, you don't have to call `Show`
 * on the window as the window will be displayed immediately.
 * @param message Message to be displayed (required)
 * @param title Title text (optional) - setting this will make the window non-modal.
 * @param btnOk 'OK' button text. If set to null, "OK" is used.
 * @param btnCancel Optional 'Cancel' button. If set to null, not displayed.
 * @param callback Returns 1 if OK is clicked, 0 if Cancel is clicked and -1 if the window was closed.
 * @return
 */
CKWindow* CKApp::CKNewMsgBoxError(const char* message, const char* title, const char* btnOk, const char* btnCancel, std::function<void(int button)> callback) {
	return this->__CreateAlertDialog(message, title, CKSystemIcon::error, btnOk, btnCancel, callback);
}

CKWindow* CKApp::__CreateAlertDialog(const char* message, const char* title, const CKSystemIcon icon, const char* btnOk, const char* btnCancel, std::function<void(int button)> callback) {

	CKPROFILE

	int padding = 13;

	CKWindowInitParams params = CKWindowInitParams(CKSize(300, 0));
	params.SetTitle(title ? title : "Alert").SetType(title ? CKWindowType::Standard : CKWindowType::Modal);
	CKWindow* toReturn = this->CKNewWindow(params);

	int labelX = padding;
	if (icon != CKSystemIcon::noIcon) {
		labelX += 32 + padding;
	}
	CKLabel* label = CKNew CKLabel({message, CKRect(labelX, padding, params.size.width - (padding + labelX), 0)});
	label->AutoHeight(400);
	if (label->rect->size->height <= 28) {
		label->rect->size->height = 28;
	}
	toReturn->AddControl(label);

	CKCanvas* iconCanvas;
	if (icon != CKSystemIcon::noIcon) {
		iconCanvas = CKNew CKCanvas(CKSize(32, 32));
		iconCanvas->rect->origin->x = padding;
		iconCanvas->rect->origin->y = padding;
		switch (icon) {
			case CKSystemIcon::message:
				iconCanvas->DrawResourceIcon(1, {0, 0});
				break;
			case CKSystemIcon::error:
				iconCanvas->DrawResourceIcon(0, {0, 0});
				break;
			case CKSystemIcon::warning:
				iconCanvas->DrawResourceIcon(2, {0, 0});
				break;
			default:
				CKLog("Warning - unknown icon: %d", icon);
				iconCanvas->DrawResourceIcon(0, {0, 0});
		}
		toReturn->AddControl(iconCanvas);
	}

	int windowHeight = label->rect->size->height + (padding * 3) + 20;

	int okButtonWidth = 80;
	int okButtonLeft = params.size.width - padding - okButtonWidth;

	int buttonTop = windowHeight - (padding * 2) - 5;

	// TODO: On Classic Mac OS, OK is on the left, Cancel is on the right.
	// We have to flip these but at the same time, add maybe a check on the platform
	// we are compiling for so if it's macOS >= 10, we flip it back.

	if (btnCancel) {
		int cancelButtonLeft = okButtonLeft;
		CKButton* cancelButton = CKNew CKButton({btnCancel, CKRect(cancelButtonLeft, buttonTop, okButtonWidth, 20)});
		toReturn->AddControl(cancelButton);
		cancelButton->AddHandler(CKEventType::click, [callback, toReturn](CKEvent e) {
			toReturn->Close();
			if (callback) {
				callback(0);
			}
		});
		okButtonLeft -= okButtonWidth + padding;
	}

	CKButton* okButton = CKNew CKButton({btnOk ? btnOk : "OK", CKRect(okButtonLeft, buttonTop, okButtonWidth, 20)});
	toReturn->AddControl(okButton);
	okButton->AddHandler(CKEventType::click, [callback, toReturn](CKEvent e) {
		toReturn->Close();
		if (callback) {
			callback(1);
		}
	});
	okButton->SetDefault(true);

	// This is here for when/if the `closable` is changed afterwards.
	toReturn->AddHandler(CKEventType::removed, [callback](CKEvent e) {
		if (callback) {
			callback(-1);
		}
	});

	toReturn->closable = false;
	toReturn->rect->size->height = windowHeight;
	toReturn->Center();
	toReturn->Show();
	return toReturn;
}

/**
 * @brief Show the 'Working' cursor.
 */
void CKApp::CKIncreaseWork() {
	this->__workCount++;
	CKLog("increase work.. count is now %d", this->__workCount);
	this->CKRestoreCursor();
}

/**
 * @brief Decrease work count by one, if zero, hide the 'Working' cursor.
 */
void CKApp::CKDecreaseWork() {

	this->__workCount--;
	CKLog("decrease work.. count is now %d", this->__workCount);
	if (this->__workCount < 0) {
		CKLog("Work count is below zero for some reason, fix.");
		this->__workCount = 0;
	}
	this->CKRestoreCursor();
}

/**
 * @brief Changed the cursor yourself? Call this to get the default/waiting back.
 */
void CKApp::CKRestoreCursor() {

	if (this->__workCount == 0) {
		SetCursor(&qd.arrow);
	} else {
		static Cursor c;
		static CursHandle ch = 0;

		if (!ch) {
			ch = GetCursor(watchCursor);
			HLock((Handle)ch);
			c = **ch;
		}

		if (this->__workCount == 1) {
			SetCursor(&c);
		}
	}
}

/**
 * @brief Return the top-most window we have.
 * @return Nil if there are no windows open.
 */
CKWindow* CKApp::CKTopMostWindow() {

	// TODO: Check if this returns other people's windows as well.
	// Probably not but the THINK Reference was a bit vague.
	CKWindowPtr fwPtr = FrontWindow();

	if (!fwPtr) {
		return nil;
	}

	return this->CKFindWindow(fwPtr);
}

/**
 * ----------------------------------------------------------------------
 * Private Event Handlers
 * ----------------------------------------------------------------------
 */

void CKApp::__DoHousekeepingTasks() {

	SystemTask();

	CKWindow* tmw = this->CKTopMostWindow();
	if (tmw) {
		tmw->Loop();
	}

	for (auto it = this->__timers.begin(); it != this->__timers.end(); /* no ++ */) {
		if (!(*it)->Update()) {
			it = this->__timers.erase(it); // remove one-shot timer
		} else {
			++it;
		}
	}

	CKNetworking::Loop(this->__net_sockets);
}

void CKApp::__DispatchEvent(EventRecord e) {

	switch (e.what) {
		case nullEvent:
			/* Nada. */
			break;
		case mouseDown:
			this->__HandleEvtMouseDown(e);
			break;
		case mouseUp:
			this->__HandleEvtMouseUp(e);
			break;
		case keyDown:
		case keyUp:
		case autoKey:
			this->__HandleEvtKey(e, e.what == keyUp, e.what == autoKey);
			break;
		case updateEvt:
			this->__HandleEvtUpdate(e);
			break;
		case diskEvt:
			break;
		case activateEvt:
			this->__HandleEvtActivate(e);
			break;
		case osEvt:
			this->__HandleEvtOS(e);
			break;
		case kHighLevelEvent:
			break;
		default:
			// Probably would not happen under normal
			// conditions but let's cover our asses.
			CKDebugLog(3, "Unknown event type (%d) received.", e.what);
			break;
	}
}

void CKApp::__HandleEvtKey(EventRecord event, bool isKeyUp, bool isAutoKey) {

	char theChar = event.message & charCodeMask;
	short theKey = event.message & keyCodeMask >> 8;

	bool isCmd = event.modifiers & cmdKey;

	// Check if this is a command we need to handle.

	if (isCmd) {

		if (theChar == 'q' || theChar == 'Q') {
			// Close the app!
			this->CKQuit();
		}

		if (theChar == 'w' || theChar == 'W') {
			// Close the window.
			CKWindow* topmostWindow = this->CKTopMostWindow();
			if (topmostWindow && topmostWindow->closable) {
				topmostWindow->Close();
			}
		}

		// Input boxes do not get anything if isCmd, of course.
		return;
	}

	CKWindow* activeWindow = this->CKTopMostWindow();
	if (!activeWindow) {
		return;
	}

	CKEvent evt = CKEvent(isKeyUp ? CKEventType::keyUp : CKEventType::keyDown);
	evt.character = theChar;
	evt.key = theKey;
	evt.fillFromOS(event);
	activeWindow->HandleEvent(evt);
}

void CKApp::__HandleEvtMouseDown(EventRecord event) {

	WindowRef foundWindow;
	WindowPartCode where = 0;

	where = FindWindow(event.where, &foundWindow);

	if (where == inSysWindow) {
		SystemClick(&event, foundWindow);
		return;
	}

	if (where == inDrag) {
		DragWindow(foundWindow, event.where, &qd.screenBits.bounds);
		CKWindow* ckFoundWindow = this->CKFindWindow(foundWindow);
		if (ckFoundWindow) {
			// Gotta update the window rect..
			WindowPeek ckFoundWindowPeek = (WindowPeek)(ckFoundWindow->__windowPtr);
			RgnHandle strucRgn = ckFoundWindowPeek->contRgn;
			HLock((Handle)strucRgn);
			Rect r = (**strucRgn).rgnBBox;
			ckFoundWindow->rect->origin = CKPoint(r.left, r.top);
			HUnlock((Handle)strucRgn);
		}
		return;
	}

	if (where == inGoAway) {
		if (TrackGoAway(foundWindow, event.where)) {
			CKWindow* ckFoundWindow = this->CKFindWindow(foundWindow);
			if (ckFoundWindow != nil) {
				ckFoundWindow->Close();
			}
		}
		return;
	}

	if (where == inGrow) {
		CKWindow* ckFoundWindow = this->CKFindWindow(foundWindow);
		if (!ckFoundWindow) {
			return;
		}
		Rect growRect;
		growRect.top = ckFoundWindow->minimumHeight;
		growRect.bottom = ckFoundWindow->maximumHeight;
		growRect.left = ckFoundWindow->minimumWidth;
		growRect.right = ckFoundWindow->maximumWidth;
		long result = GrowWindow(foundWindow, event.where, &growRect);
		if (result == 0) {
			// No change.
			return;
		}
		short newWidth = LOWORD(result);
		short newHeight = HIWORD(result);
		ckFoundWindow->rect->size->width = newWidth;
		ckFoundWindow->rect->size->height = newHeight;
		return;
	}

	if (where == inMenuBar) {
		long choice = MenuSelect(event.where);
		if (choice) {
			// Handle choice.
			short menuId = HIWORD(choice);
			short menuItem = LOWORD(choice);

			if (menuId == kCKAppleMenuID) {
				bool launchDeskAcc = false;
				if (!this->__menubar) {
					launchDeskAcc = true;
				} else {
					if (menuItem > (short)this->__menubar->appleMenuItems.size()) {
						launchDeskAcc = true;
					}
				}
				if (launchDeskAcc) {
					Str255 str;
					GetMenuItemText(GetMenuHandle(kCKAppleMenuID), menuItem, str);
					OpenDeskAcc(str);
					goto cleanMenuActionUp;
				}
			}

			if (!this->__menubar) {
				// This really should not happen but just in case..
				goto cleanMenuActionUp;
			}

			if (menuId == kCKAppleMenuID) {

				int actualItemId = menuItem - 1;

				if (actualItemId >= (short)this->__menubar->appleMenuItems.size()) {
					// Should not happen but, just in case.
					CKLog("actualItemId = %d but size = %d", actualItemId, this->__menubar->appleMenuItems.size());
					goto cleanMenuActionUp;
				}

				auto& mi = this->__menubar->appleMenuItems[actualItemId];
				if (mi->callback) {
					mi->callback(CKEvent(CKEventType::click));
				}

			} else {

				int actualMenuId = menuId - kCKUserMenuStartID;
				int actualItemId = menuItem - 1;

				if (actualMenuId > (short)this->__menubar->items.size()) {
					// Should not happen but, just in case.
					CKLog("actualItemId = %d but size = %d", actualMenuId, this->__menubar->items.size());
					goto cleanMenuActionUp;
				}

				auto& mbi = this->__menubar->items[actualMenuId];

				auto& vec = mbi->items.get();
				if (actualItemId > (short)vec.size()) {
					// Again, hould not happen but, just in case.
					CKLog("actualItemId = %d, but size = %d.. (text = %s)", actualItemId, vec.size(), mbi->text);
					goto cleanMenuActionUp;
				}

				auto& mi = vec[actualItemId];
				if (mi->callback) {
					mi->callback(CKEvent(CKEventType::click));
				}
			}
		}

	cleanMenuActionUp:
		HiliteMenu(0);
		return;
	}

	if (where == inContent) {
		if (foundWindow != FrontWindow()) {
			SelectWindow(foundWindow);
		} else {
			// Handle in-window clicks.
			if (foundWindow) {
				CKWindow* ckFoundWindow = this->CKFindWindow(foundWindow);
				if (ckFoundWindow != nil) {
					GrafPtr oldPort;
					GetPort(&oldPort);
					SetPort(foundWindow);
					GlobalToLocal(&(event.where));
					SetPort(oldPort);
					CKControl* control = ckFoundWindow->FindControl(CKPoint::FromOS(event.where));
					CKControl* lastControl = ckFoundWindow->GetLastControl();
					if (lastControl) {
						if (lastControl != control) {
							CKPoint p = CKPoint().FromOS(event.where);
							CKEvent evt = CKEvent(CKEventType::mouseUp, p);
							evt.fillFromOS(event);
							lastControl->HandleEvent(evt);
							ckFoundWindow->SetLastControl(nullptr);
							ckFoundWindow->SetActiveControl(nullptr);
						}
					}
					ckFoundWindow->SetLastControl(control);

					CKPoint p = CKPoint().FromOS(event.where);
					CKEvent evt = CKEvent(CKEventType::mouseDown, p);
					evt.fillFromOS(event);

					if (control) {
						ckFoundWindow->SetActiveControl(control);
						control->HandleEvent(evt);
					}
					if (this->__lastMouseDownWindow == ckFoundWindow) {
						// Send mouseMove instead.
						evt.type = CKEventType::mouseMove;
						evt.mouseButton = CKMouseButton::Left;
					}
					this->__lastMouseDownWindow = ckFoundWindow;
					ckFoundWindow->HandleEvent(evt);

				} else {
					CKLog("ckFoundWindow is null.");
				}
			} else {
				CKLog("foundWindow is null.");
			}
		}

		return;
	}

	CKDebugLog(3, "Unhandled part code (%d) in mouseDown event.", where);
}

void CKApp::__HandleEvtMouseUp(EventRecord event) {

	WindowRef foundWindow;
	WindowPartCode where = 0;

	where = FindWindow(event.where, &foundWindow);

	this->__lastMouseDownWindow = nil;

	if (where == inContent) {
		if (foundWindow == FrontWindow()) {
			// Handle in-window clicks.
			if (foundWindow) {
				CKWindow* ckFoundWindow = this->CKFindWindow(foundWindow);
				if (ckFoundWindow != nil) {
					GrafPtr oldPort;
					GetPort(&oldPort);
					SetPort(foundWindow);
					GlobalToLocal(&(event.where));
					SetPort(oldPort);
					CKControl* control = ckFoundWindow->FindControl(CKPoint::FromOS(event.where));
					CKControl* lastControl = ckFoundWindow->GetLastControl();
					if (lastControl) {
						CKPoint p = CKPoint().FromOS(event.where);
						CKEvent evt = CKEvent(CKEventType::mouseUp, p);
						evt.fillFromOS(event);
						lastControl->HandleEvent(evt);
						evt.type = CKEventType::click;
						lastControl->HandleEvent(evt);
						ckFoundWindow->SetLastControl(nullptr);
					}
					CKPoint p = CKPoint().FromOS(event.where);
					CKEvent evt = CKEvent(CKEventType::mouseUp, p);
					evt.fillFromOS(event);
					if (control) {
						control->HandleEvent(evt);
					}
					ckFoundWindow->HandleEvent(evt);
				} else {
					CKLog("ckFoundWindow is null.");
				}
			} else {
				CKLog("foundWindow is null.");
			}
		}

		return;
	}
}

void CKApp::__HandleEvtMouseMove(EventRecord event) {

	WindowRef foundWindow;
	WindowPartCode where = 0;

	where = FindWindow(event.where, &foundWindow);
	if (where != inContent) {
		return;
	}

	if (foundWindow == FrontWindow()) {
		if (foundWindow) {
			CKWindow* ckFoundWindow = this->CKFindWindow(foundWindow);
			if (!ckFoundWindow) {
				CKLog("mm -> can't find window");
				return;
			} else {
				if (!ckFoundWindow->shouldReceiveMouseMoveEvents) {
					return;
				}
			}
			GrafPtr oldPort;
			GetPort(&oldPort);
			SetPort(foundWindow);
			GlobalToLocal(&(event.where));
			SetPort(oldPort);
			CKPoint p = CKPoint().FromOS(event.where);
			CKEvent evt = CKEvent(CKEventType::mouseMove, p);
			evt.fillFromOS(event);
			// TODO: We shouldn't need the second check here. Find out why lastMouseDownWindow gets 'stuck'
			if (this->__lastMouseDownWindow && this->__lastMouseDownWindow == ckFoundWindow) {
				evt.mouseButton = CKMouseButton::Left;
			} else {
				evt.mouseButton = CKMouseButton::None;
			}
			ckFoundWindow->HandleEvent(evt);
		}
	}
}

void CKApp::__HandleEvtUpdate(EventRecord event) {

	WindowPtr window = (WindowPtr)event.message;
	if (window == nil || ((WindowPeek)window)->windowKind != userKind) {
		// No window (or it's not ours), so no update.
		CKLog("__HandleEvtUpdate called, but window = %x and windowKind = %d", window, ((WindowPeek)window)->windowKind);
		return;
	}

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(window);

	BeginUpdate((WindowRef)event.message);

	WindowPeek wp = (WindowPeek)(window);
	Rect dr = (**wp->port.visRgn).rgnBBox;
	CKRect drk = CKRect().FromOS(dr);

	CKWindow* ckFoundWindow = this->CKFindWindow(window);
	if (ckFoundWindow) {
		ckFoundWindow->Redraw(drk);
	} else {
		CKLog("__HandleEvtUpdate called, but ckFoundWindow is nil");
	}

	EndUpdate((WindowRef)event.message);
	SetPort(oldPort);
}

void CKApp::__HandleEvtActivate(EventRecord event) {

	WindowPtr window = (WindowPtr)event.message;
	if (window == nil || ((WindowPeek)window)->windowKind != userKind) {
		// No window (or it's not ours), so no update.
		CKLog("HandleEvtActivate called, but window = %x and windowKind = %d", window, ((WindowPeek)window)->windowKind);
		return;
	}

	bool isActivating = (event.modifiers & activeFlag) == activeFlag;
	CKWindow* ckFoundWindow = this->CKFindWindow(window);
	if (ckFoundWindow) {
		ckFoundWindow->SetIsActive(isActivating);
	} else {
		CKLog("HandleEvtActivate called, but ckFoundWindow is nil");
	}
}

void CKApp::__HandleEvtOS(EventRecord event) {

	bool resuming = (event.message & suspendResumeMessage) == resumeFlag;
	CKWindow* top = this->CKTopMostWindow();

	if (top) {
		top->SetIsActive(resuming);
	}
}

/**
 * @brief Convert a font name to a font Id.
 * @param font
 * @return Returns 0 if not found or if font name == "System"
 */
short CKApp::CKFontToId(const char* font) {

	short toReturn;

	if (strlen(font) == 0 || !strcmp(font, "system") || !strcmp(font, "System")) {
		return 0;
	}

	unsigned char* fn = CKC2P(font);
	GetFNum(fn, &toReturn);
	CKFree(fn);

	return toReturn;
}

/**
 * @brief Add a new timer to the app, enabling (via `Start`) as you add it.
 * @param timer Timer to add and start
 * @param owner Adding this to a window or anything else that might go away? Make sure to set `owner`!
 */
void CKApp::CKAddTimer(CKTimer* timer, CKObject* owner) {

	this->__timers.push_back(timer);
	timer->owner = owner;
	timer->Start();
}

/**
 * @brief Stop and remove timer.
 * @param timer Timer to stop and remove
 */
void CKApp::CKRemoveTimer(CKTimer* timer) {

	for (auto it = __timers.begin(); it != __timers.end(); ++it) {
		if (*it == timer) {
			CKLog("Deleting timer %x...", *it);
			CKDelete(*it);
			__timers.erase(it);
			return;
		}
	}

	CKLog("Warning! Can't find and remove timer %x!");
}

/**
 * @brief Remove all timers of a specific owner.
 * @param owner
 */
void CKApp::CKRemoveTimersOfOwner(CKObject* owner) {

	std::vector<CKTimer*> toRemove;
	for (auto* timer : __timers) {
		if (timer->owner == owner) {
			toRemove.push_back(timer);
		}
	}

	CKLog("CKRemoveTimersOfOwner is looking for timers with owner %x, found %d", owner, toRemove.size());

	for (auto* timer : toRemove) {
		// We need this to avoid a loop as Timer's destructor also calls CKRemoveTimersOfOwner.
		timer->app = nullptr;
		timer->owner = nullptr;
		this->CKRemoveTimer(timer);
	}
}

/**
 * @brief Set the application's menu bar.
 * @param
 * @return CKPass on success.
 */
CKError CKApp::CKSetMenu(CKMenuBar* menu) {

	if (this->__menubar) {
		// TODO: Check that we remove all previously added/created menus
		// so we can start all over here.
		DeleteMenu(kCKAppleMenuID);
		for (auto& m : this->__menubar->items) {
			// TODO: Do we need this?
			for (auto& sm : m->items.get()) {
				DeleteMenuItem(m->__osMenuHandle, sm->__osMenuItemID);
			}
			DeleteMenu(m->__osMenuID);
			DisposeMenu(m->__osMenuHandle);
		}
		CKDelete(this->__menubar);
		this->__menubar = nullptr;
		ClearMenuBar();
	}

	// Do the Apple Menu, always.

	MenuHandle appleMh = NewMenu(kCKAppleMenuID, "\p\024");

	if (menu && menu->appleMenuItems.size() > 0) {
		short submenuIdx = 1;
		for (auto& m : menu->appleMenuItems) {
			unsigned char* t = CKC2P(m->text);
			AppendMenu(appleMh, t);
			CKFree(t);
			m->__osMenuHandle = appleMh;
			m->__osMenuItemID = submenuIdx;
			m->ReflectToOS(); // Shortcuts, enable/disable, etc.
			m->SetPropertyObserver(std::bind(&CKApp::__HandleMenuPropertyChange, this, std::placeholders::_1, std::placeholders::_2));
			submenuIdx++;
		}
		AppendMenu(appleMh, "\p(-");
	}

	AppendResMenu(appleMh, 'DRVR');
	InsertMenu(appleMh, 0);

	if (menu == nullptr) {
		// User wants an empty menu, fine!
		DrawMenuBar();
		return CKPass;
	}

	// Now the user menus.

	short menuIdx = kCKUserMenuStartID;

	for (auto& m : menu->items) {
		unsigned char* t = CKC2P(m->text);
		MenuHandle mh = NewMenu(menuIdx, t);
		CKFree(t);
		m->__osMenuHandle = mh;
		m->__osMenuID = menuIdx;
		short submenuIdx = 1;
		auto& vec = m->items.get();
		for (auto& sm : vec) {
			t = CKC2P(sm->text);
			AppendMenu(mh, t);
			CKFree(t);
			sm->__osMenuHandle = mh;
			sm->__osMenuItemID = submenuIdx;
			sm->ReflectToOS(); // Shortcuts, enable/disable, etc.
			submenuIdx++;
			sm->SetPropertyObserver(std::bind(&CKApp::__HandleMenuPropertyChange, this, std::placeholders::_1, std::placeholders::_2));
		}
		InsertMenu(mh, 0);
		menuIdx++;
		// Just to make sure we are tied.
		m->SetPropertyObserver(std::bind(&CKApp::__HandleMenuPropertyChange, this, std::placeholders::_1, std::placeholders::_2));
	}

	DrawMenuBar();
	this->__menubar = menu;
	return CKPass;
}

/**
 * @brief If hidden, bring the menu bar back.
 */
void CKApp::CKShowMenuBar() {
	// TODO: Implement this.
}

/**
 * @brief If shown, hide the menu bar.
 */
void CKApp::CKHideMenuBar() {
	// TODO: Implement this.
}

void CKApp::__HandleMenuPropertyChange(const CKObject* obj, const char* propName) {

	const CKMenuItem* item = dynamic_cast<const CKMenuItem*>(obj);
	if (item) {
		const_cast<CKMenuItem*>(item)->ReflectToOS();
	} else {
		const CKMenuBarItem* menubarItem = dynamic_cast<const CKMenuBarItem*>(obj);
		if (!strcmp(propName, "enabled")) {
			// TODO: Gray out all child items on disable
			// -- but need to store the previous
		}
	}
}