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

#include "ckProperty.h"
#include <unordered_map>

#define CKOBSERVEVALUE(name) [this]() { this->RaisePropertyChange(name); };

enum class CKEventType;
class CKEvent;
class CKObject;

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
		virtual void SetPropertyObserver(CKPropertyObserverFunc cb);
		virtual void UnsetPropertyObserver();
		virtual void RaisePropertyChange(const char* propertyName);

	protected:
		std::unordered_map<CKEventType, CKEventHandlerFunc> __handlers;
		CKPropertyObserverFunc propertyObserverCB = nullptr;
};