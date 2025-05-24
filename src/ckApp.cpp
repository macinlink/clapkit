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
#include "ckLabel.h"
#include "ckMenu.h"
#include "ckTimer.h"
#include "ckUtils.h"
#include "ckWindow.h"
#include <Appearance.h>
#include <Devices.h>
#include <Quickdraw.h>

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

	this->workCount = 0;
	this->__windows = std::vector<CKWindow*>();
	this->__gc_windows = std::vector<CKWindow*>();
	this->lastMouseDownWindow = nil;
}

/**
 * Nothing much ere.
 */
CKApp::~CKApp() {
}

/**
 * Main event loop, handling events.
 * Needs to be called as much as possible.
 *
 * Returns:
 *  0   All OK, keep going.
 *  1   App is going to quit, clean up.
 */
int CKApp::Loop(int waitTime) {

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
			this->HandleEvtMouseMove(e);
			lastMousePosX = e.where.h;
			lastMousePosY = e.where.v;
		}

		goto doTimers;
	}

	this->DispatchEvent(e);

doTimers:
	this->DoHousekeepingTasks();
	return 0;
}

/**
 * Flush events, clean things up, quit.
 */
void CKApp::Quit() {

	CKPROFILE

	this->IncreaseWork();

	while (this->__windows.size() > 0) {
		this->CKRemoveWindow(this->__windows.at(0));
	}

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
 * We don't immediately delete a window but rather collect it for future
 * deletion - this is because mouseUp, etc. events might still pop
 * during execution of an event handler.
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
 * @param title
 * @param message
 * @param btnOk 'OK' button text. If set to null, "OK" is used.
 * @param btnCancel Optional 'Cancel' button. If set to null, not displayed.
 * @param callback
 * @return
 */
CKWindow* CKApp::CKNewAlert(const char* title, const char* message, const char* btnOk, const char* btnCancel, std::function<void(int button)> callback) {

	CKPROFILE

	int padding = 13;

	CKWindowInitParams params = CKWindowInitParams(300, 0, title ? title : "Alert", false, true);
	CKWindow* toReturn = this->CKNewWindow(params);

	CKLabel* label = CKNew CKLabel({message, padding, padding, params.size.width - (padding * 2), 0});
	label->AutoHeight(300);
	toReturn->AddControl(label);

	int windowHeight = label->rect->size->height + (padding * 3) + 20;

	int okButtonWidth = 80;
	int okButtonLeft = params.size.width - padding - okButtonWidth;

	int buttonTop = windowHeight - (padding * 2) - 5;

	// TODO: On Classic Mac OS, OK is on the left, Cancel is on the right.
	// We have to flip these but at the same time, add maybe a check on the platform
	// we are compiling for so if it's macOS >= 10, we flip it back.

	if (btnCancel) {
		int cancelButtonLeft = okButtonLeft;
		CKButton* cancelButton = CKNew CKButton({btnCancel, cancelButtonLeft, buttonTop, okButtonWidth, 20});
		toReturn->AddControl(cancelButton);
		cancelButton->AddHandler(CKEventType::click, [callback, toReturn](CKEvent e) {
			toReturn->Close();
			if (callback) {
				callback(false);
			}
		});
		okButtonLeft -= okButtonWidth + padding;
	}

	CKButton* okButton = CKNew CKButton({btnOk ? btnOk : "OK", okButtonLeft, buttonTop, okButtonWidth, 20});
	toReturn->AddControl(okButton);
	okButton->AddHandler(CKEventType::click, [callback, toReturn](CKEvent e) {
		toReturn->Close();
		if (callback) {
			callback(true);
		}
	});
	okButton->SetDefault(true);

	toReturn->rect->size->height = windowHeight;
	toReturn->Center();
	toReturn->Show();
	return toReturn;
}

/**
 * @brief Show the 'Working' cursor.
 */
void CKApp::IncreaseWork() {
	this->workCount++;
	CKLog("increase work.. count is now %d", this->workCount);
	this->RestoreCursor();
}

/**
 * @brief Decrease work count by one, if zero, hide the 'Working' cursor.
 */
void CKApp::DecreaseWork() {
	this->workCount--;
	CKLog("decrease work.. count is now %d", this->workCount);
	if (this->workCount < 0) {
		CKLog("Work count is below zero for some reason, fix.");
		this->workCount = 0;
	}
	this->RestoreCursor();
}

/**
 * @brief Changed the cursor yourself? Call this to get the default/waiting back.
 */
void CKApp::RestoreCursor() {
	if (this->workCount == 0) {
		SetCursor(&qd.arrow);
	} else {
		static Cursor c;
		static CursHandle ch = 0;
		if (!ch) {
			ch = GetCursor(watchCursor);
			HLock((Handle)ch);
			c = **ch;
		}
		if (this->workCount == 1) {
			SetCursor(&c);
		}
	}
}

/**
 * @brief Return the top-most window we have.
 * @return Nil if there are no windows open.
 */
CKWindow* CKApp::TopMostWindow() {

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

void CKApp::DoHousekeepingTasks() {

	CKWindow* tmw = this->TopMostWindow();
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
}

void CKApp::DispatchEvent(EventRecord e) {

	switch (e.what) {
		case nullEvent:
			/* Nada. */
			break;
		case mouseDown:
			this->HandleEvtMouseDown(e);
			break;
		case mouseUp:
			this->HandleEvtMouseUp(e);
			break;
		case keyDown:
		case keyUp:
		case autoKey:
			this->HandleEvtKey(e, e.what == keyUp, e.what == autoKey);
			break;
		case updateEvt:
			this->HandleEvtUpdate(e);
			break;
		case diskEvt:
			break;
		case activateEvt:
			this->HandleEvtActivate(e);
			break;
		case osEvt:
			this->HandleEvtOS(e);
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

void CKApp::HandleEvtKey(EventRecord event, bool isKeyUp, bool isAutoKey) {

	char theChar = event.message & charCodeMask;
	short theKey = event.message & keyCodeMask >> 8;

	bool isCmd = event.modifiers & cmdKey;

	// Check if this is a command we need to handle.

	if (isCmd) {

		if (theChar == 'q' || theChar == 'Q') {
			// Close the app!
			this->Quit();
		}

		if (theChar == 'w' || theChar == 'W') {
			// Close the window.
			CKWindow* topmostWindow = this->TopMostWindow();
			if (topmostWindow) {
				topmostWindow->Close();
			}
		}

		// Input boxes do not get anything if isCmd, of course.
		return;
	}

	CKWindow* activeWindow = this->TopMostWindow();
	if (!activeWindow) {
		return;
	}

	CKEvent evt = CKEvent(isKeyUp ? CKEventType::keyUp : CKEventType::keyDown);
	evt.character = theChar;
	evt.key = theKey;
	activeWindow->HandleEvent(evt);
}

void CKApp::HandleEvtMouseDown(EventRecord event) {

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

	if (where == inMenuBar) {
		long choice = MenuSelect(event.where);
		if (choice) {
			// Handle choice.
			short menuId = choice >> 16;
			short menuItem = choice & 0xFFFF;
			CKLog("Got menu item %d on menu %d", menuItem, menuId);

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

				if (actualItemId > (short)this->__menubar->items.size()) {
					// Should not happen but, just in case.
					CKLog("actualItemId = %d but size = %d", actualItemId, this->__menubar->items.size());
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
							lastControl->HandleEvent(evt);
							ckFoundWindow->SetLastControl(nullptr);
						}
					}
					ckFoundWindow->SetLastControl(control);

					CKPoint p = CKPoint().FromOS(event.where);
					CKEvent evt = CKEvent(CKEventType::mouseDown, p);
					if (control) {
						CKLog("Will call control %x to handle down event", control);
						control->HandleEvent(evt);
					}
					if (this->lastMouseDownWindow == ckFoundWindow) {
						// Send mouseMove instead.
						evt.type = CKEventType::mouseMove;
						evt.mouseButton = CKMouseButton::Left;
					} else {
						// Let the window know to switch the active control.
						ckFoundWindow->SetActiveControl(control);
					}
					this->lastMouseDownWindow = ckFoundWindow;
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

void CKApp::HandleEvtMouseUp(EventRecord event) {

	WindowRef foundWindow;
	WindowPartCode where = 0;

	where = FindWindow(event.where, &foundWindow);

	this->lastMouseDownWindow = nil;

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
						lastControl->HandleEvent(evt);
						ckFoundWindow->SetLastControl(nullptr);
					}
					CKPoint p = CKPoint().FromOS(event.where);
					CKEvent evt = CKEvent(CKEventType::mouseUp, p);
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

void CKApp::HandleEvtMouseMove(EventRecord event) {

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
			// TODO: We shouldn't need the second check here. Find out why lastMouseDownWindow gets 'stuck'
			if (this->lastMouseDownWindow && this->lastMouseDownWindow == ckFoundWindow) {
				evt.mouseButton = CKMouseButton::Left;
			} else {
				evt.mouseButton = CKMouseButton::None;
			}
			ckFoundWindow->HandleEvent(evt);
		}
	}
}

void CKApp::HandleEvtUpdate(EventRecord event) {

	CKLog("Update event received.");

	WindowPtr window = (WindowPtr)event.message;
	if (window == nil || ((WindowPeek)window)->windowKind != userKind) {
		// No window (or it's not ours), so no update.
		CKLog("HandleEvtUpdate called, but window = %x and windowKind = %d", window, ((WindowPeek)window)->windowKind);
		return;
	}

	GrafPtr oldPort;
	GetPort(&oldPort);
	SetPort(window);

	BeginUpdate((WindowRef)event.message);
	CKWindow* ckFoundWindow = this->CKFindWindow(window);
	if (ckFoundWindow) {
		ckFoundWindow->Redraw(CKRect());
	} else {
		CKLog("HandleEvtUpdate called, but ckFoundWindow is nil");
	}

	EndUpdate((WindowRef)event.message);
	SetPort(oldPort);
}

void CKApp::HandleEvtActivate(EventRecord event) {

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

void CKApp::HandleEvtOS(EventRecord event) {
	bool resuming = (event.message & suspendResumeMessage) == resumeFlag;
	CKWindow* top = this->TopMostWindow();
	if (top) {
		top->SetIsActive(resuming);
	}
}

/**
 * @brief Convert a font name to a font Id.
 * @param font
 * @return Returns 0 if not found or if font name == "System"
 */
short CKApp::FontToId(const char* font) {

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
void CKApp::AddTimer(CKTimer* timer, CKObject* owner) {
	this->__timers.push_back(timer);
	timer->owner = owner;
	timer->Start();
}

/**
 * @brief Stop and remove timer.
 * @param timer Timer to stop and remove
 */
void CKApp::RemoveTimer(CKTimer* timer) {
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
void CKApp::RemoveTimersOfOwner(CKObject* owner) {
	std::vector<CKTimer*> toRemove;
	for (auto* timer : __timers) {
		if (timer->owner == owner) {
			toRemove.push_back(timer);
		}
	}
	CKLog("RemoveTimersOfOwner is looking for timers with owner %x, found %d", owner, toRemove.size());
	for (auto* timer : toRemove) {
		// We need this to avoid a loop as Timer's destructor also calls RemoveTimersOfOwner.
		timer->app = nullptr;
		timer->owner = nullptr;
		this->RemoveTimer(timer);
	}
}

/**
 * @brief Set the application's menu bar.
 * @param
 * @return CKPass on success.
 */
CKError CKApp::SetMenu(CKMenuBar* menu) {

	if (this->__menubar) {
		// TODO: Remove all previously added/created menus
		// so we can start all over here.
		CKDelete(this->__menubar);
		this->__menubar = nullptr;
	}

	// Do the Apple Menu, always.

	MenuHandle appleMh = NewMenu(kCKAppleMenuID, "\p\024");

	if (menu && menu->appleMenuItems.size() > 0) {
		short submenuIdx = 1;
		for (auto& m : menu->appleMenuItems) {
			AppendMenu(appleMh, CKC2P(m->text));
			m->__osMenuHandle = appleMh;
			m->__osMenuItemID = submenuIdx;
			m->ReflectToOS(); // Shortcuts, enable/disable, etc.
			m->SetPropertyObserver(std::bind(&CKApp::HandleMenuPropertyChange, this, std::placeholders::_1, std::placeholders::_2));
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
		MenuHandle mh = NewMenu(menuIdx, CKC2P(m->text));
		m->__osMenuID = menuIdx;
		short submenuIdx = 1;
		auto& vec = m->items.get();
		for (auto& sm : vec) {
			AppendMenu(mh, CKC2P(sm->text));
			sm->__osMenuHandle = mh;
			sm->__osMenuItemID = submenuIdx;
			sm->ReflectToOS(); // Shortcuts, enable/disable, etc.
			submenuIdx++;
			sm->SetPropertyObserver(std::bind(&CKApp::HandleMenuPropertyChange, this, std::placeholders::_1, std::placeholders::_2));
		}
		InsertMenu(mh, 0);
		menuIdx++;
		// Just to make sure we are tied.
		m->SetPropertyObserver(std::bind(&CKApp::HandleMenuPropertyChange, this, std::placeholders::_1, std::placeholders::_2));
	}

	DrawMenuBar();
	this->__menubar = menu;
	return CKPass;
}

/**
 * @brief If hidden, bring the menu bar back.
 */
void CKApp::ShowMenuBar() {
}

/**
 * @brief If shown, hide the menu bar.
 */
void CKApp::HideMenuBar() {
}

void CKApp::HandleMenuPropertyChange(const CKObject* obj, const char* propName) {
	CKLog("Menu bar property '%s' of %x has changed!", propName, obj);
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