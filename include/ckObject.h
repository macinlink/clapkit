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
#include <unordered_map>

using CKEventHandlerFunc = std::function<void(const CKEvent&)>;

class CKObject {

	public:
		CKObject();
		virtual ~CKObject();
		virtual void AddHandler(CKEventType type, CKEventHandlerFunc cb);
		virtual void RemoveHandler(CKEventType type);
		virtual bool HasHandler(CKEventType type) const;
		virtual bool HandleEvent(const CKEvent& evt);

	protected:
		std::unordered_map<CKEventType, CKEventHandlerFunc> __handlers;
};