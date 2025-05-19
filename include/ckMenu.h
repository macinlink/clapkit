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

#pragma once

#include "ckObject.h"
#include <vector>

class CKApp;
struct CKMenuBarItem;
struct CKMenuItem;

struct CKMenuBar {
		std::vector<CKMenuItem> appleMenuItems;
		std::vector<CKMenuBarItem> items;
};

struct CKMenuBarItem {
	public:
		CKMenuBarItem(const char* text) {
			CKSafeCopyString(this->text, text);
		}
		char* text = nullptr;
		bool enabled = true;
		std::vector<CKMenuItem> items;

	protected:
		friend class CKApp;
		short __osMenuID;
};

struct CKMenuItem {
	public:
		CKMenuItem(const char* text, char shortcut, CKEventHandlerFunc callback) {
			CKSafeCopyString(this->text, text);
			this->shortcut = shortcut;
			this->callback = callback;
		}
		CKMenuItem(const char* text, CKEventHandlerFunc callback) {
			CKSafeCopyString(this->text, text);
			this->shortcut = 0;
			this->callback = callback;
		}
		CKMenuItem(const char* text) {
			CKSafeCopyString(this->text, text);
		}
		char* text = nullptr;
		bool enabled = true;
		char shortcut = 0;
		bool modifierAlt = false;
		bool modifierCtrl = false;
		CKEventHandlerFunc callback;

	protected:
		friend class CKApp;
		MenuHandle __osMenuHandle;
		short __osMenuItemID;
};