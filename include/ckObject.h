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

#pragma once

#include "ckProperty.h"
#include <map>

/**
 * @ingroup Utils
 * @brief Simple wrapper for tying an @ref CKProperty::onChange of CKProperty to an object.
 */
#define CKOBSERVEVALUE(name) [this]() { this->RaisePropertyChange(name); };

enum class CKEventType;
class CKEvent;
class CKObject;

/**
 * @ingroup Types
 */
using CKEventHandlerFunc = std::function<void(const CKEvent&)>;

/**
 * @ingroup Types
 */
using CKPropertyObserverFunc = std::function<void(const CKObject*, const char*)>;

/**
 * @ingroup CoreApp
 * @brief Defines the base class for all controls and objects.
 */
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
		std::map<CKEventType, CKEventHandlerFunc> __handlers;
		CKPropertyObserverFunc propertyObserverCB = nullptr;
};