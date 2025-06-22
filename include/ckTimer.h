/*
 *
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 *
 * CKTimer
 * ----------------------------------------------------------------------
 * Defines a timer.
 *
 */

#pragma once

#include "ckObject.h"
#include <MacTypes.h>

using CKTimerCallbackFunc = std::function<void(void*)>;

class CKApp;

/**
 * @ingroup CoreApp
 * @brief Defines a timer that can execute code on a pre-defined interval.
 */
class CKTimer : public CKObject {

	public:
		CKTimer();
		virtual ~CKTimer();
		bool Update();
		void Start();
		void Stop();

	public:
		bool enabled = false;		  // Will actually fire?
		bool multiRun = true;		  // Fires more than once?
		UInt16 interval = 1000;		  // Interval in milliseconds
		UInt32 nextRun;				  // When will it run next time?
		CKTimerCallbackFunc callback; // Callback function
		void* userData = nullptr;	  // Optional context pointer
		CKObject* owner = nullptr;	  // Used to stop the timer when the owner is out of scope
		CKApp* app = nullptr;		  // Used to stop the timer when the owner is out of scope
};