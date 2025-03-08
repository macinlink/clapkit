/**
 * 
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 * 
 * CKButton
 * ----------------------------------------------------------------------
 * Defines a push-button.
 * 
*/

#pragma once 

#include "ckApp.h"
#include "ckControlToolbox.h"

class CKButton: public CKControlToolbox {

    public:
        CKButton(const CKControlInitParams& params);
        virtual ~CKButton();

};