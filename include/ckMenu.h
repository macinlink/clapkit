/**
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

#pragma once

#include "ckObject.h"
#include "ckProperty.h"
#include <Menus.h>
#include <vector>

class CKApp;
class CKMenu;
class CKMenuItem;

class CKMenuBar : public CKObject {
	public:
		CKMenuBar(bool createStandardItems = false);
		virtual ~CKMenuBar();
		void AddMenu(CKMenu* item);
		void AddSystemMenuItem(CKMenu* item);
		void RemoveMenu(CKMenu* item);
		bool HasMenu(CKMenu* item);
		bool HasMenuItem(CKMenuItem* item);
		CKMenu* GetStdFileMenu() { return this->__stdFile; }
		CKMenu* GetStdEditMenu() { return this->__stdEdit; }

	protected:
		friend class CKApp;
		std::vector<CKMenuItem*> appleMenuItems;
		std::vector<CKMenu*> items;

	private:
		CKMenu* __stdFile = nullptr;
		CKMenu* __stdEdit = nullptr;
};

class CKMenu : public CKObject {
	public:
		CKMenu(const char* text);
		virtual ~CKMenu();
		CKProperty<bool> enabled = true;
		CKProperty<std::vector<CKMenuItem*>> items;
		void SetText(const char* text);
		void AddItem(CKMenuItem* item) {
			this->items.get().push_back(item);
		}
		void PrependItem(CKMenuItem* item) {
			this->items.get().insert(this->items.get().begin(), item);
		}
		void DeleteItem(CKMenuItem* item) {
			auto& vec = this->items.get();
			vec.erase(std::remove(vec.begin(), vec.end(), item), vec.end());
		}

	protected:
		friend class CKApp;
		char* text = nullptr;
		short __osMenuID;
		MenuHandle __osMenuHandle;
};

enum class CKMenuItemType {
	Standard = 0,
	Separator = 1,
	Quit,
	Undo,
	Cut,
	Copy,
	Paste,
	Clear,
};

class CKMenuItem : public CKObject {
	public:
		CKMenuItem(const char* text, char shortcut, CKEventHandlerFunc callback);
		CKMenuItem(CKMenuItemType type);
		virtual ~CKMenuItem();
		void SetText(const char* text);
		void DoCallback(CKApp* app);

	protected:
		friend class CKApp;
		void __ReflectToOS();

	public:
		CKProperty<bool> enabled = true;
		CKProperty<char> shortcut = 0;
		CKProperty<bool> modifierAlt = false;
		CKProperty<bool> modifierCtrl = false;
		CKProperty<CKMenuItemType> type = CKMenuItemType::Standard;
		CKEventHandlerFunc callback = nullptr;

	protected:
		friend class CKApp;
		char* text = nullptr;
		MenuHandle __osMenuHandle;
		short __osMenuItemID;

	private:
		void __AttachObservers();
};