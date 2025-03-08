/**
 * 
 * Clapkit
 * ----------------------------------------------------------------------
 * A wrapper for creating a 'generalized' app for Classic MacOS
 * that (hopefully) can be ported easily to other platforms.
 * 
 * CKWindow
 * ----------------------------------------------------------------------
 * Defines a window.
 * 
*/

#pragma once

#include "ckApp.h"
#include "ckObject.h"
#include "ckControl.h"

#include <vector>

struct CKWindowInitParams {
    const char* title = 0;
    int x = 0;
    int y = 0;
    int width = 100;
    int height = 50;
    bool closable = true;
    bool modal = false;
};

class CKWindow: public CKObject {

    public:
        CKWindow(CKWindowInitParams params);
        ~CKWindow();

        void SetTitle(const char* title);
        char* GetTitle();

        void Show();
        void Hide();
        bool IsVisible();
        void Focus();

        CKRect* GetRect(bool getCopy = false);
        void Move(int x, int y);
        void Resize(int width, int height);
        void Center();
        void Close();

        bool AddControl(CKControl* control);
        void RemoveControl(CKControl* control, bool free);
        void Redraw(CKRect rectToRedraw);

        void SetOwner(CKApp* owner);
        CKControl* FindControl(CKPoint point);
        bool ContainsControl(CKControl* control);
        void SetActiveControl(CKControl* control);

        void SetIsActive(bool active);

        bool HandleEvent(CKControlEvent evt);
        void AddHandler(CKControlEventType type, std::function<void(CKWindow*, CKControlEvent)> callback);
        // void AddGenericHandler(std::function<void(CKWindow*, CKControlEvent)> callback);
        void RemoveHandler(CKControlEventType type);
        CKHandlerContainer* HasHandler(CKControlEventType type);

    public:
        CKWindowPtr __windowPtr;

        /**
         * @brief Contains the last control the user has been pushing down on.
        */
        CKControl* latestDownControl;

        /**
         * @brief Contains the active text input (textarea, textbox, etc..) control.
        */
        CKControl* activeTextInputControl;

        /**
         * @brief True if we should receive mouseMove events.
         * We are storing this as a hack to speed things up as HasHandler is pretty slow.
        */
        bool shouldReceiveMouseMoveEvents;

        // /**
        //  * Generic event handler.
        // */
        // std::function<void(CKWindow*, CKControlEvent)> genericEventHandler;

    private:
        CKRect* __rect;
        std::vector<CKControl*> __controls;
        CKApp* __owner;
        bool __visible;
        std::vector<CKHandlerContainer*> __handlers;

};