/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKMenuBar & CKMenuBarItem
 * ----------------------------------------------------------------------
 * Defines a menu and its items.
 *
 */

#include "ckMenu.h"
#include "ckApp.h"

/*** ---------------------------------------------------------------------------- */
/*** ----- CKMenuBar ------------------------------------------------------------ */
/*** ---------------------------------------------------------------------------- */

CKMenuBar::CKMenuBar() {
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
void CKMenuBar::AddMenuItem(CKMenuBarItem* item) {
	if (this->HasMenu(item)) {
		this->RemoveMenuItem(item);
	}
	this->items.push_back(item);
}

/**
 * @brief Add the menu to Apple Menu (Classic Mac OS) or App Menu (OS X)
 */
void CKMenuBar::AddSystemMenuItem(CKMenuBarItem* item) {
}

/**
 * @brief Remove item from the menu bar.
 */
void CKMenuBar::RemoveMenuItem(CKMenuBarItem* item) {
}

/**
 * @brief Check if item is already in this menubar.
 */
bool CKMenuBar::HasMenu(CKMenuBarItem* item) {
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
/*** ----- CKMenuBarItem -------------------------------------------------------- */
/*** ---------------------------------------------------------------------------- */

CKMenuBarItem::CKMenuBarItem(const char* text) {
	this->text = nullptr;
	CKSafeCopyString(this->text, text);
	this->enabled.onChange = CKOBSERVEVALUE("enabled");
	this->items.onChange = CKOBSERVEVALUE("items");
}

CKMenuBarItem::~CKMenuBarItem() {
	CKSafeCopyString(this->text, nullptr);
	for (auto* m : this->items.get()) {
		delete m;
	}
}

void CKMenuBarItem::SetText(const char* text) {
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

void CKMenuItem::ReflectToOS() {
	if (!this->__osMenuHandle || this->__osMenuItemID == 0) {
		return;
	}
	unsigned char* t = CKC2P(this->text);
	SetMenuItemText(this->__osMenuHandle, this->__osMenuItemID, t);
	CKFree(t);
	if (this->shortcut) {
		SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, this->shortcut);
	} else {
		// TODO: Check if this is valid.
		SetItemCmd(this->__osMenuHandle, this->__osMenuItemID, 0);
	}
	if (this->enabled) {
		EnableItem(this->__osMenuHandle, this->__osMenuItemID);
	} else {
		DisableItem(this->__osMenuHandle, this->__osMenuItemID);
	}
}