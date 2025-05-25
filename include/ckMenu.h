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
#include "ckProperty.h"
#include <Menus.h>
#include <vector>

class CKApp;
class CKMenuBarItem;
class CKMenuItem;

class CKMenuBar : public CKObject {
	public:
		CKMenuBar();
		virtual ~CKMenuBar();
		void AddMenuItem(CKMenuBarItem* item);
		void AddSystemMenuItem(CKMenuBarItem* item);
		void RemoveMenuItem(CKMenuBarItem* item);
		bool HasMenu(CKMenuBarItem* item);
		bool HasMenuItem(CKMenuItem* item);

	protected:
		friend class CKApp;
		std::vector<CKMenuItem*> appleMenuItems;
		std::vector<CKMenuBarItem*> items;
};

class CKMenuBarItem : public CKObject {
	public:
		CKMenuBarItem(const char* text);
		virtual ~CKMenuBarItem();
		CKProperty<bool> enabled = true;
		CKProperty<std::vector<CKMenuItem*>> items;
		void SetText(const char* text);
		void AddItem(CKMenuItem* item) {
			this->items.get().push_back(item);
		}

	protected:
		friend class CKApp;
		char* text = nullptr;
		short __osMenuID;
		MenuHandle __osMenuHandle;
};

class CKMenuItem : public CKObject {
	public:
		CKMenuItem(const char* text, char shortcut, CKEventHandlerFunc callback);
		virtual ~CKMenuItem();
		void SetText(const char* text);
		void ReflectToOS();

	public:
		CKProperty<bool> enabled = true;
		CKProperty<char> shortcut = 0;
		CKProperty<bool> modifierAlt = false;
		CKProperty<bool> modifierCtrl = false;
		CKEventHandlerFunc callback = nullptr;

	protected:
		friend class CKApp;
		char* text = nullptr;
		MenuHandle __osMenuHandle;
		short __osMenuItemID;
};