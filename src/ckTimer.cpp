/**
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

#include "ckTimer.h"

CKTimer::CKTimer() {
}

CKTimer::~CKTimer() {
}

/**
 * @brief Let the timer check if its appropriate to run.
 * @return Returns false if timer needs to be removed from the queue / destroyed.
 */
bool CKTimer::Update() {
	if (!this->enabled) {
		return true;
	}
	if (CKMillis() >= this->nextRun) {
		this->nextRun = CKMillis() + this->interval;
		if (this->callback) {
			this->callback(this->userData);
		}
		if (!this->multiRun) {
			return false;
		}
	}
	return true;
}

void CKTimer::Start() {
	this->enabled = true;
	this->nextRun = CKMillis() + this->interval;
}

void CKTimer::Stop() {
	this->enabled = false;
}