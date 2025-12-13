#include "main.h"
#include <ckButton.h>
#include <ckCheckbox.h>
#include <ckDropdown.h>
#include <ckLabel.h>
#include <ckMenu.h>
#include <ckNetClient.h>
#include <ckNetworking.h>
#include <ckRadiobox.h>
#include <ckTextArea.h>
#include <ckTextField.h>
#include <ckTimer.h>
#include <ckWindow.h>
#include <functional>
#include <map>
#include <string.h>

CKTest* app;

int main() {

	// Create a new app.
	app = CKNew CKTest();

	/*** ---------------------------------------------------------------------- */
	/*** Set up the menu bar.
	/-------------------------------------------------------------------------- */

	// Create the menu bar with the standard File/Edit menus.
	CKMenuBar* menuBar = CKNew CKMenuBar(true);

	menuBar->GetStdFileMenu()->PrependItem(CKNew CKMenuItem(CKMenuItemType::Separator));
	CKMenuItem* meowItem = CKNew CKMenuItem("Meow", 'M', [&meowItem](CKEvent e) {
		app->CKNewMsgBoxError("Hello world!", "This is meow!", "Oh ok", "Bark!");
		meowItem->SetText("Bark");
		meowItem->enabled = false;
	});
	menuBar->GetStdFileMenu()->PrependItem(meowItem);

	CKMenu* menuTests = CKNew CKMenu("Tests");
	menuBar->AddMenu(menuTests);

	std::map<std::string, std::function<void()>> tests;

	/*** ---------------------------------------------------------------------- */
	/*** Show a simple dialog box
	/-------------------------------------------------------------------------- */

	tests["Message box"] = []() {
		app->CKNewMsgBoxNote("If you are seeing this, then compile was fine!", "Hello world", "OK", "Quit", [](int button) {
			// CKLog("Got button index: %d", button);
			if (button == 0) {
				app->CKQuit();
			}
		});
	};

	tests["Message box (No Title)"] = []() {
		app->CKNewMsgBoxNote("If you are seeing this, then compile was fine!", nullptr, "OK", "Quit", [](int button) {
			CKLog("Got button index: %d", button);
			if (button == 0) {
				app->CKQuit();
			}
		});
	};

	/*** ---------------------------------------------------------------------- */
	/*** Show labels being changed by timers
	/-------------------------------------------------------------------------- */

	tests["Timer"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 70)));
		window->SetTitle("Timer");

		CKLabel* label1 = CKNew CKLabel(CKRect(10, 10, 300 - 20, 20));
		label1->SetText("Timer 1 hasn't fired yet..");
		window->AddControl(label1);

		CKLabel* label2 = CKNew CKLabel(CKRect(10, 30, 300 - 20, 20));
		label2->SetText("Timer 2 hasn't fired yet..");
		window->AddControl(label2);

		window->Center();
		window->Show();

		CKTimer* timer = CKNew CKTimer();
		timer->interval = 1000;
		timer->multiRun = true;
		int idx = 0;
		timer->callback = [label1, idx](void* params) mutable {
			char t[256];
			sprintf(t, "Hi! %d (1000ms)", idx);
			idx++;
			label1->SetText(t);
		};
		app->CKAddTimer(timer, window);

		CKTimer* timer2 = CKNew CKTimer();
		timer2->interval = 100;
		timer2->multiRun = true;
		int idx2 = 0;
		timer2->callback = [label2, idx2](void* params) mutable {
			char t[256];
			sprintf(t, "Hi! %d (100ms)", idx2);
			idx2++;
			label2->SetText(t);
		};
		app->CKAddTimer(timer2, window);
	};

	/*** ---------------------------------------------------------------------- */
	/*** Showcase label properties
	/-------------------------------------------------------------------------- */

	tests["Labels"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 10)));
		window->SetTitle("Labels");
		int padding = 10;
		for (int i = 0; i < 10; i++) {
			CKLabel* label = CKNew CKLabel(CKSize(300 - (padding * 2), 20));
			label->rect->origin->x = padding;
			label->rect->origin->y = padding + (i * (20 + padding));
			window->AddControl(label);
			window->rect->size->height += label->rect->size->height + padding;

			switch (i) {
				case 0:
					label->SetText("A bold label!");
					label->bold = true;
					break;
				case 1:
					label->SetText("An italic label!");
					label->italic = true;
					break;
				case 2:
					label->SetText("An underlined label!");
					label->underline = true;
					break;
				case 3:
					label->SetText("A bold + underline label!");
					label->bold = true;
					label->underline = true;
					break;
				case 4:
					label->SetText("A centered label!");
					label->justification = CKTextJustification::Center;
					break;
				case 5:
					label->SetText("A red label!");
					label->color = CKColor({255, 0, 0});
					break;
				case 6:
					label->SetText("Geneva at 12 pt");
					label->SetFont(app->CKFontToId("Geneva"));
					label->fontSize = 12;
					break;
				case 7:
					label->SetText("Geneva at 10 pt");
					label->SetFont(app->CKFontToId("Geneva"));
					label->fontSize = 10;
					break;
				case 8:
					label->SetText("Geneva at 8 pt");
					label->SetFont(app->CKFontToId("Geneva"));
					label->fontSize = 8;
					break;
				case 9:
					label->SetText("Geneva at 20 pt");
					label->SetFont(app->CKFontToId("Geneva"));
					label->fontSize = 20;
					break;
				default:
					break;
			}
		}
		window->Center();
		window->Show();
	};

	tests["Text Area"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 220)));
		window->SetTitle("Text Area");
		int padding = 10;
		CKTextArea* label = CKNew CKTextArea(CKSize(300 - (padding * 2), 100));
		label->rect->origin->x = padding;
		label->rect->origin->y = padding;
		window->AddControl(label);
		label->SetText("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

		CKRadiobox* opt1 = CKNew CKRadiobox(CKSize(300 - (padding * 2), kCKRadioboxHeight));
		opt1->rect->origin->x = padding;
		opt1->rect->origin->y = 110 + padding;
		opt1->SetText("Plain");
		opt1->SetValue(true);
		opt1->groupID = 1;
		window->AddControl(opt1);
		opt1->AddHandler(CKEventType::changed, [label](CKEvent e) {
			label->multiline = true;
			label->scrollType = CKScrollType::none;
		});

		CKRadiobox* opt2 = CKNew CKRadiobox(CKSize(300 - (padding * 2), kCKRadioboxHeight));
		opt2->rect->origin->x = padding;
		opt2->rect->origin->y = 110 + (padding + kCKRadioboxHeight * 1);
		opt2->SetText("One-Line");
		opt2->groupID = 1;
		window->AddControl(opt2);
		opt2->AddHandler(CKEventType::changed, [label](CKEvent e) {
			label->multiline = false;
			label->scrollType = CKScrollType::none;
		});

		CKRadiobox* opt3 = CKNew CKRadiobox(CKSize(300 - (padding * 2), kCKRadioboxHeight));
		opt3->rect->origin->x = padding;
		opt3->rect->origin->y = 110 + (padding + kCKRadioboxHeight * 2);
		opt3->SetText("With Scrollbar (Vertical only)");
		opt3->groupID = 1;
		window->AddControl(opt3);
		opt3->AddHandler(CKEventType::changed, [label](CKEvent e) {
			label->multiline = true;
			label->scrollType = CKScrollType::vertical;
		});

		CKRadiobox* opt4 = CKNew CKRadiobox(CKSize(300 - (padding * 2), kCKRadioboxHeight));
		opt4->rect->origin->x = padding;
		opt4->rect->origin->y = 110 + (padding + kCKRadioboxHeight * 3);
		opt4->SetText("With Scrollbar (Horizontal only)");
		opt4->groupID = 1;
		window->AddControl(opt4);
		opt4->AddHandler(CKEventType::changed, [label](CKEvent e) {
			label->multiline = true;
			label->scrollType = CKScrollType::horizontal;
		});

		CKRadiobox* opt5 = CKNew CKRadiobox(CKSize(300 - (padding * 2), kCKRadioboxHeight));
		opt5->rect->origin->x = padding;
		opt5->rect->origin->y = 110 + (padding + kCKRadioboxHeight * 4);
		opt5->SetText("With Scrollbar (both)");
		opt5->groupID = 1;
		window->AddControl(opt5);
		opt5->AddHandler(CKEventType::changed, [label](CKEvent e) {
			label->multiline = true;
			label->scrollType = CKScrollType::both;
		});

		window->Center();
		window->Show();
	};

	/*** ---------------------------------------------------------------------- */
	/*** Showcase button properties
	/-------------------------------------------------------------------------- */

	tests["Buttons"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 10)));
		window->SetTitle("Buttons");
		int padding = 10;
		for (int i = 0; i < 2; i++) {
			CKButton* button = CKNew CKButton(CKSize(300 - (padding * 2), kCKButtonHeight));
			button->rect->origin->x = padding;
			button->rect->origin->y = padding + (i * (kCKButtonHeight + padding));
			window->AddControl(button);
			window->rect->size->height += button->rect->size->height + padding;

			switch (i) {
				case 0:
					button->SetText("A regular button!");
					break;
				case 1:
					button->SetText("A disabled button!");
					button->enabled = false;
					break;
			}
		}
		window->Center();
		window->Show();
	};

	/*** ---------------------------------------------------------------------- */
	/*** Showcase checkbox properties
	/-------------------------------------------------------------------------- */

	tests["Checkbox and Radios"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 10)));
		window->SetTitle("Checkbox and Radios");
		int padding = 10;
		int startY = 0;
		for (int i = 0; i < 2; i++) {
			CKCheckbox* obj = CKNew CKCheckbox(CKSize(300 - (padding * 2), kCKCheckboxHeight));
			obj->rect->origin->x = padding;
			obj->rect->origin->y = padding + startY;
			window->AddControl(obj);
			window->rect->size->height += obj->rect->size->height + padding;

			switch (i) {
				case 0:
					obj->SetText("A regular checkbox!");
					break;
				case 1:
					obj->SetText("A disabled checkbox!");
					obj->enabled = false;
					break;
			}
			startY += obj->rect->size->height + padding;
		}
		for (int i = 0; i < 4; i++) {
			CKRadiobox* obj = CKNew CKRadiobox(CKSize(300 - (padding * 2), kCKRadioboxHeight));
			obj->rect->origin->x = padding;
			obj->rect->origin->y = padding + startY;
			window->AddControl(obj);
			window->rect->size->height += obj->rect->size->height + padding;

			switch (i) {
				case 0:
					obj->SetText("A regular radiobox! (ungrouped)");
					break;
				case 1:
					obj->SetText("A disabled radiobox! (ungrouped)");
					obj->enabled = false;
					break;
				case 2:
					obj->SetText("Grouped Item A");
					obj->groupID = 1;
					obj->SetValue(true);
					break;
				case 3:
					obj->SetText("Grouped Item B");
					obj->groupID = 1;
					obj->SetValue(false);
					break;
			}
			startY += obj->rect->size->height + padding;
		}
		window->Center();
		window->Show();
	};

	/*** ---------------------------------------------------------------------- */
	/*** Showcase button properties
	/-------------------------------------------------------------------------- */

	tests["Textfield"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 10)));
		window->SetTitle("Textfield");
		int padding = 10;
		for (int i = 0; i < 2; i++) {
			CKTextField* button = CKNew CKTextField(CKSize(300 - (padding * 2), kCKTextFieldHeight));
			button->rect->origin->x = padding;
			button->rect->origin->y = padding + (i * (kCKTextFieldHeight + padding));
			window->AddControl(button);
			window->rect->size->height += button->rect->size->height + padding;

			switch (i) {
				case 0:
					button->SetText("A regular textfield!");
					break;
				case 1:
					button->SetText("A disabled textfield!");
					button->enabled = false;
					break;
			}
		}
		window->Center();
		window->Show();
	};

	/*** ---------------------------------------------------------------------- */
	/*** Networking
	/-------------------------------------------------------------------------- */

	tests["Networking"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 40)));
		window->SetTitle("Networking");
		if (!CKNetworking::IsAvailable()) {
			CKLabel* lbl = CKNew CKLabel(CKRect(10, 10, 280, 20));
			lbl->SetText("MacTCP is not installed/available.");
			window->AddControl(lbl);
			window->Center();
			window->Show();
			return;
		}
		if (!CKNetworking::IsInitialized()) {
			CKError res = CKNetworking::Initialize();
			if (res != CKPass) {
				CKLabel* lbl = CKNew CKLabel(CKRect(10, 10, 280, 20));
				char t[256];
				sprintf(t, "Unable to set up networking, code: %lu", res);
				lbl->SetText(t);
				window->AddControl(lbl);
				window->Center();
				window->Show();
				return;
			}
		}
		window->rect->size->height += 20;

		char t[256];
		CKLabel* lbl1 = CKNew CKLabel(CKRect(10, 10, 280, 20));
		CKIPAddress ip = CKNetworking::GetLocalIP();
		sprintf(t, "IP of this device: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
		lbl1->SetText(t);
		window->AddControl(lbl1);

		CKLabel* lbl2 = CKNew CKLabel(CKRect(10, 30, 280, 20));
		lbl2->SetText("One second, please..");
		window->AddControl(lbl2);

		CKNetClient* socket = CKNew CKNetClient();
		CKError res = socket->Connect("google.com", 80);

		window->Center();
		window->Show();

		if (res != CKPass) {
			sprintf(t, "Failed with code %u", res);
			lbl2->SetText(t);
		} else {
			lbl2->SetText("Connecting, please wait...");

			socket->AddHandler(CKEventType::tcpConnected, [lbl2, socket](CKEvent e) {
				CKLog("Connected!");
				lbl2->SetText("Connected!");
				char data[256];
				sprintf(data, "GET /contact HTTP/1.1\nHost: google.com\nUser-Agent: curl/8.6.0\nccept: */*\n\n");
				socket->Write(data, strlen(data));
			});

			socket->AddHandler(CKEventType::tcpReceivedData, [lbl2, socket](CKEvent e) {
				CKLog("Going to read data...");
				void* buffer = CKMalloc(512);
				short readBytes;
				CKError r = socket->Read(buffer, 512, &readBytes);
				if (r != CKPass) {
					lbl2->SetText("Read failed!");
					return;
				}
				CKLog("Read %d bytes.", readBytes);
				char data[600];
				sprintf(data, "Read: '%s'", buffer);
				lbl2->SetText(data);
			});

			socket->AddHandler(CKEventType::tcpConnectionFailed, [lbl2](CKEvent e) {
				lbl2->SetText("Could not connect to server.");
			});
		}
	};

	/*** ---------------------------------------------------------------------- */
	/*** Showcase dropdowns
	/-------------------------------------------------------------------------- */

	tests["Dropdowns"] = []() {
		CKWindow* window = app->CKNewWindow(CKWindowInitParams(CKSize(300, 40)));
		window->SetTitle("Dropdowns");
		int padding = 10;
		CKLabel* label = CKNew CKLabel(CKSize(300 - (padding * 2), 20));
		label->rect->origin->x = padding;
		label->rect->origin->y = padding;
		label->SetText("Make a selection below.");
		window->AddControl(label);
		for (int i = 0; i < 2; i++) {
			CKDropdown* control = CKNew CKDropdown(CKSize(300 - (padding * 2), kCKDropdownHeight));
			control->rect->origin->x = padding;
			control->rect->origin->y = padding + ((i + 1) * (kCKDropdownHeight + padding));
			control->items->push_back("Hello world!");
			window->AddControl(control);
			window->rect->size->height += control->rect->size->height + padding;
			control->items->push_back("Bye world!");
			control->items->push_back("Hello again, world!");
			switch (i) {
				case 0:
					control->SetText("Select:");
					break;
				case 1:
					control->SetText("A disabled dropdown!");
					control->enabled = false;
					break;
			}
			control->AddHandler(CKEventType::changed, [control, label](CKEvent e) {
				char t[256];
				sprintf(t, "Selection index: %d (\"%s\")", control->selectedIndex.get(), control->items->at(control->selectedIndex));
				label->SetText(t);
			});
		}
		window->Center();
		window->Show();
	};

	for (const auto& [name, test] : tests) {
		CKMenuItem* item = CKNew CKMenuItem(name.c_str(), 0, [&item, test](CKEvent e) {
			test();
		});
		menuTests->AddItem(item);
	}

	app->CKSetMenu(menuBar);

	// Run loop: without this, your app will quit as soon as it launches.
	while (!app->CKLoop(5)) {
		;
	}

	delete app;
	return 0;
}