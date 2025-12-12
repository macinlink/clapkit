/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKMenuBar & CKMenu
 * ----------------------------------------------------------------------
 * Defines a menu and its items.
 *
 */

#include "ckMenu.h"
#include "ckApp.h"
#include "ckWindow.h"
#include "ck_pFocusableControl.h"

/*** ---------------------------------------------------------------------------- */
/*** ----- CKMenuBar ------------------------------------------------------------ */
/*** ---------------------------------------------------------------------------- */

/**
 * @brief Create a new menubar.
 * @param createStandardItems Adds "File" and "Edit" menus and their items if true.
 */
CKMenuBar::CKMenuBar(bool createStandardItems) {

	if (createStandardItems) {

		this->__stdFile = CKNew CKMenu("File");
		this->__stdFile->AddItem(CKNew CKMenuItem(CKMenuItemType::Quit));
		this->AddMenu(this->__stdFile);

		this->__stdEdit = CKNew CKMenu("Edit");
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Undo));
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Separator));
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Cut));
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Copy));
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Paste));
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Separator));
		this->__stdEdit->AddItem(CKNew CKMenuItem(CKMenuItemType::Clear));
		this->AddMenu(this->__stdEdit);
	}
}

CKMenuBar::~CKMenuBar() {

	for (auto* m : appleMenuItems) {
		CKDelete(m);
	}

	for (auto* m : items) {
		CKDelete(m);
	}
}

/**
 * @brief Add a menu item to the bar.
 */
void CKMenuBar::AddMenu(CKMenu* item) {

	if (this->HasMenu(item)) {
		this->RemoveMenu(item);
	}

	this->items.push_back(item);
}

/**
 * @brief Add the menu to Apple Menu (Classic Mac OS) or App Menu (OS X)
 */
void CKMenuBar::AddSystemMenuItem(CKMenu* item) {
}

/**
 * @brief Remove item from the menu bar.
 */
void CKMenuBar::RemoveMenu(CKMenu* item) {
}

/**
 * @brief Check if item is already in this menubar.
 */
bool CKMenuBar::HasMenu(CKMenu* item) {

	for (auto& m : this->items) {
		if (m == item) {
			return true;
		}
	}

	return false;
}

/**
 * @brief Check if item is already in this any of the menus.
 */
bool CKMenuBar::HasMenuItem(CKMenuItem* item) {

	for (auto& sm : this->appleMenuItems) {
		if (sm == item) {
			return true;
		}
	}

	for (auto& m : this->items) {
		auto& vec = m->items.get();
		for (auto& sm : vec) {
			if (sm == item) {
				return true;
			}
		}
	}

	return false;
}

/*** ---------------------------------------------------------------------------- */
/*** ----- CKMenu -------------------------------------------------------- */
/*** ---------------------------------------------------------------------------- */

CKMenu::CKMenu(const char* text) {

	this->text = nullptr;
	CKSafeCopyString(this->text, text);
	this->enabled.onChange = CKOBSERVEVALUE("enabled");
	this->items.onChange = CKOBSERVEVALUE("items");
}

CKMenu::~CKMenu() {

	CKSafeCopyString(this->text, nullptr);
	for (auto* m : this->items.get()) {
		CKDelete(m);
	}
}

void CKMenu::SetText(const char* text) {

	CKSafeCopyString(this->text, text);
	this->RaisePropertyChange("text");
}

/*** ---------------------------------------------------------------------------- */
/*** ----- CKMenuItem ----------------------------------------------------------- */
/*** ---------------------------------------------------------------------------- */

CKMenuItem::CKMenuItem(const char* text, char shortcut, CKEventHandlerFunc callback) {

	this->text = nullptr;
	CKSafeCopyString(this->text, text);
	this->shortcut = shortcut;
	this->callback = callback;
	this->__AttachObservers();
}

CKMenuItem::CKMenuItem(CKMenuItemType type) {
	this->type = type;
	this->callback = nullptr;
	this->__AttachObservers();
}

void CKMenuItem::__AttachObservers() {
	this->enabled.onChange = CKOBSERVEVALUE("enabled");
	this->shortcut.onChange = CKOBSERVEVALUE("shortcut");
	this->modifierAlt.onChange = CKOBSERVEVALUE("modifierAlt");
	this->modifierCtrl.onChange = CKOBSERVEVALUE("modifierCtrl");
}

CKMenuItem::~CKMenuItem() {

	CKSafeCopyString(this->text, nullptr);
}

void CKMenuItem::SetText(const char* text) {

	CKSafeCopyString(this->text, text);
	this->RaisePropertyChange("text");
}

void CKMenuItem::DoCallback(CKApp* app) {

	if (app == nullptr) {
		CKLog("DoCallback called with a null app!");
		return;
	}

	if (!this->enabled) {
		return;
	}

	CKWindow* window = app->CKTopMostWindow();
	CKFocusableControl* fcontrol = nullptr;
	CKObject* control = nullptr;
	if (window) {
		if (window->GetActiveControl()) {
			fcontrol = window->GetActiveControl();
			if (auto c = dynamic_cast<CKObject*>(fcontrol)) {
				control = c;
			}
		}
	}

	switch (this->type.get()) {
		case CKMenuItemType::Undo:
			if (control) {
				if (((CKControl*)control)->HasHandler(CKEventType::dataUndo)) {
					((CKControl*)control)->HandleEvent(CKEventType::dataUndo);
				} else {
					fcontrol->PerformUndo();
				}
			}
			break;
		case CKMenuItemType::Cut:
			if (control) {
				if (((CKControl*)control)->HasHandler(CKEventType::dataCut)) {
					((CKControl*)control)->HandleEvent(CKEventType::dataCut);
				} else {
					fcontrol->PerformCut();
				}
			}
			break;
		case CKMenuItemType::Copy:
			if (control) {
				if (((CKControl*)control)->HasHandler(CKEventType::dataCopy)) {
					((CKControl*)control)->HandleEvent(CKEventType::dataCopy);
				} else {
					fcontrol->PerformCopy();
				}
			}
			break;
		case CKMenuItemType::Paste:
			if (control) {
				if (((CKControl*)control)->HasHandler(CKEventType::dataPaste)) {
					((CKControl*)control)->HandleEvent(CKEventType::dataPaste);
				} else {
					fcontrol->PerformPaste();
				}
			}
			break;
		case CKMenuItemType::Clear:
			if (control) {
				if (((CKControl*)control)->HasHandler(CKEventType::dataClear)) {
					((CKControl*)control)->HandleEvent(CKEventType::dataClear);
				} else {
					fcontrol->PerformClear();
				}
			}
			break;
		case CKMenuItemType::Quit:
			app->CKQuit();
			break;
		default:
			if (this->callback) {
				this->callback(CKEvent(CKEventType::click));
			}
			break;
	}
}

void CKMenuItem::__ReflectToOS() {

	if (!this->__osMenuHandle || this->__osMenuItemID == 0) {
		return;
	}

	if (this->type == CKMenuItemType::Separator) {
		SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\p-");
	} else {
		if (this->text) {
			unsigned char* t = CKC2P(this->text);
			SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, t);
			CKFree(t);
		} else {
			// TODO: Localization, eventually.
			switch (this->type.get()) {
				case CKMenuItemType::Undo:
					SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\pUndo");
					break;
				case CKMenuItemType::Cut:
					SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\pCut");
					break;
				case CKMenuItemType::Copy:
					SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\pCopy");
					break;
				case CKMenuItemType::Paste:
					SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\pPaste");
					break;
				case CKMenuItemType::Clear:
					SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\pClear");
					break;
				case CKMenuItemType::Quit:
					SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, "\pQuit");
					break;
				default:
					break;
			}
		}
	}

	if (this->type == CKMenuItemType::Standard) {
		if (this->shortcut) {
			SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, this->shortcut);
		} else {
			// TODO: Check if this is valid.
			SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 0);
		}
	} else {
		// If menu type is not standard, ignore the user-provided shortcuts for
		// OS-standard ones. AFAIK, 'Clear' does not have one.
		switch (this->type.get()) {
			case CKMenuItemType::Undo:
				this->shortcut = 'Z';
				SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 'Z');
				break;
			case CKMenuItemType::Cut:
				this->shortcut = 'X';
				SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 'X');
				break;
			case CKMenuItemType::Copy:
				this->shortcut = 'C';
				SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 'C');
				break;
			case CKMenuItemType::Paste:
				this->shortcut = 'V';
				SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 'V');
				break;
			case CKMenuItemType::Quit:
				this->shortcut = 'Q';
				SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 'Q');
				break;
			default:
				break;
		}
	}

	if (this->enabled) {
		EnableItem(this->__osMenuHandle, this->__osMenuItemID);
	} else {
		DisableItem(this->__osMenuHandle, this->__osMenuItemID);
	}
}