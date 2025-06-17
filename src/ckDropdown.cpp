/**
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKDropdown
 * ----------------------------------------------------------------------
 * Defines a dropdown list control.
 *
 */

#include "ckDropdown.h"
#include "ckWindow.h"

CKDropdown::CKDropdown(const CKControlInitParams& params)
	: CKControlToolbox(params, CKControlType::Dropdown) {

	this->items.onChange = CKOBSERVEVALUE("items");
	this->labelWidth.onChange = CKOBSERVEVALUE("labelWidth");
	this->selectedIndex.onChange = CKOBSERVEVALUE("selectedIndex");
}

CKDropdown::CKDropdown(const CKControlInitParams& params, CKControlType forcedType)
	: CKControlToolbox(params, forcedType) {}

CKDropdown::~CKDropdown() {
	if (this->__menu) {
		DeleteMenu(this->__menuId); // TODO: necessary?
		DisposeMenu(this->__menu);
	}
}

void CKDropdown::AddedToWindow(CKWindow* window) {

	CKPROFILE

	CKControl::AddedToWindow(window);

	if (this->owner == nil || this->owner->GetWindowPtr() == 0) {
		CKLog("CKControlToolbox added to window but owner or it's ptr is missing!");
		return;
	}

	Rect r = this->rect->ToOS();
	unsigned char* title = CKC2P(this->__text);

	const CKWindowPtr windowPtr = this->owner->GetWindowPtr();

	if (this->__menu) {
		DeleteMenu(this->__menuId); // TODO: necessary?
		DisposeMenu(this->__menu);
	}

	this->__menuId = CKFindFreeMenuID();
	this->__menu = NewMenu(this->__menuId, "\p");
	this->__rebuildMenu = true;
	AppendMenu(this->__menu, "\p");
	InsertMenu(this->__menu, kInsertHierarchicalMenu);
	int lw = this->rect->size->width * 0.25;
	if (this->labelWidth != -1) {
		lw = this->labelWidth;
	}
	// +1 to kControlProcPopup makes the popup width fixed.
	this->__ptr = NewControl(windowPtr, &r, title, false, 1, this->__menuId, lw, kControlProcPopup + 1, 0);

	if (!this->__ptr) {
		CKLog("Unable to create new control!");
	}

	CKFree(title);

	this->__ReflectToOS();
}

void CKDropdown::__ReflectToOS() {

	if (!this->__ptr) {
		return;
	}

	if (this->__rebuildMenu) {
		int lastSelection = MAX(GetControlValue(this->__ptr), 1);
		// TODO: This is a horrible hack but not sure what else to do?
		while (CountMItems(this->__menu) > 0) {
			DeleteMenuItem(this->__menu, 1);
		}
		for (auto& c : this->items.get()) {
			unsigned char* t = CKC2P(c);
			AppendMenu(this->__menu, t);
			CKFree(t);
		}
		AppendMenu(this->__menu, "\p");
		SetControlMaximum(this->__ptr, this->items->size());
		SetControlValue(this->__ptr, MIN(lastSelection, this->items->size()));
	}

	CKControlToolbox::__ReflectToOS();
}

bool CKDropdown::HandleEvent(const CKEvent& evt) {

	bool r = CKControlToolbox::HandleEvent(evt);

	if (evt.type == CKEventType::mouseDown) {
		this->selectedIndex.get() = GetControlValue(this->__ptr);
		if (this->selectedIndex != this->__lastRaisedSelectedIndex) {
			CKControlToolbox::HandleEvent(CKEventType::changed);
			this->__lastRaisedSelectedIndex = this->selectedIndex;
		}
	}

	return r;
}

void CKDropdown::RaisePropertyChange(const char* propertyName) {

	if (!strcmp(propertyName, "items")) {
		this->__rebuildMenu = true;
	}
	if (!strcmp(propertyName, "selectedIndex")) {
		SetControlValue(this->__ptr, MIN(this->selectedIndex.get(), this->items->size()));
		this->selectedIndex.get() = GetControlValue(this->__ptr);
	}
	this->__ReflectToOS();
	CKControl::RaisePropertyChange(propertyName);
}