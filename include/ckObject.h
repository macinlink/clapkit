/**
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

#pragma once

#include "ckApp.h"
#include "ckProperty.h"
#include <unordered_map>

#define CKOBSERVEVALUE(name) [this]() { this->RaisePropertyChange(name); };

using CKEventHandlerFunc = std::function<void(const CKEvent&)>;
using CKPropertyObserverFunc = std::function<void(const CKObject*, const char*)>;

class CKObject {

	public:
		CKObject();
		virtual ~CKObject();
		virtual void AddHandler(CKEventType type, CKEventHandlerFunc cb);
		virtual void RemoveHandler(CKEventType type);
		virtual bool HasHandler(CKEventType type) const;
		virtual bool HandleEvent(const CKEvent& evt);
		virtual void SetPropertyObserver(CKPropertyObserverFunc cb) {
			this->propertyObserverCB = cb;
		}
		virtual void UnsetPropertyObserver() {
			this->propertyObserverCB = nullptr;
		}
		virtual void RaisePropertyChange(const char* propertyName) {
			CKLog("[CKObject] Property '%s' of %x has changed, calling propertyObserverCB...", propertyName, this);
			if (this->propertyObserverCB) {
				propertyObserverCB(this, propertyName);
			}
		}

	protected:
		std::unordered_map<CKEventType, CKEventHandlerFunc> __handlers;
		CKPropertyObserverFunc propertyObserverCB;
};