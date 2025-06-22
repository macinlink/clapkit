/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKObject
 * ----------------------------------------------------------------------
 * The base of everything we have.
 *
 */

#include "ckObject.h"
#include "ckApp.h"
#include "ckProperty.h"
#include "ckTypes.h"
#include "ckUtils.h"

CKObject::CKObject() {
}

CKObject::~CKObject() {

	__handlers.clear();
}

void CKObject::AddHandler(CKEventType type, CKEventHandlerFunc cb) {

	__handlers[type] = std::move(cb);
}

void CKObject::RemoveHandler(CKEventType type) {

	if (__handlers.erase(type) == 0) {
		CKLog("No handler %d to remove on object %p", type, this);
	}
}

bool CKObject::HasHandler(CKEventType type) const {

	return __handlers.find(type) != __handlers.end();
}

bool CKObject::HandleEvent(const CKEvent& evt) {

	CKPROFILE

	auto it = __handlers.find(evt.type);
	if (it == __handlers.end()) {
		return false;
	}
	it->second(evt);
	return true;
}

void CKObject::SetPropertyObserver(CKPropertyObserverFunc cb) {

	this->propertyObserverCB = cb;
}

void CKObject::UnsetPropertyObserver() {

	this->propertyObserverCB = nullptr;
}

void CKObject::RaisePropertyChange(const char* propertyName) {

	if (this->propertyObserverCB) {
		propertyObserverCB(this, propertyName);
	}
}