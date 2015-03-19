// Copyright (c) 2014 Angus H. (4148)
// Distributed under the GNU General Public License v3.0 (GPLv3).

#include <windows.h>
#include <iostream>
//#include <tchar.h> // defines portable _tWinMain
#include <unordered_map>
#include <vector>
#include "MSBot.h"
#include "process.h"
using namespace std;

// VS Express doesn't support editing of resources, so I am not using macros in defining controls
static int start_value = 1; // arbitrary start value
// use dictionary to allow order not matter for TEXTS; less to consider
static const int AUTOPOT = start_value++; // these constants serve as valid indices to both TEXTS and
static const int AUTOATTACK = start_value++;
static const int AUTOLOOT = start_value++;
static const int AUTOCLICK = start_value++;
static const int SPAM = start_value++;
static const int AUTOLOGIN = start_value++;
static const int AUTOCC = start_value++;
static const int AUTOQUIT = start_value++;
static const int COPYRIGHT = start_value++;
// subwindows associated with subwindows
static const int AUTOLOGIN_USERNAME = start_value++;
static const int AUTOLOGIN_PASSWORD = start_value++;
static const int SPAM_TEXT = start_value++;
static const int CURRENT_CHANNEL = start_value++;
static const int AUTOPOT_HP_PERCENTAGE = start_value++;
static const int AUTOPOT_MP_PERCENTAGE = start_value++;
static const int SPAM_OPEN_FILE = start_value++;
static const int NEITHER = start_value++;

// background color
static const COLORREF BACKGROUND_COLOR = RGB(100, 100, 100);
// specifies order or elements in GUI starting from the upper left
const int ORDER[] = { AUTOPOT, AUTOATTACK, AUTOLOOT, AUTOCLICK, SPAM, AUTOLOGIN, AUTOCC, AUTOQUIT, COPYRIGHT };
// container for all relevant subwindows
unordered_map<int, HWND> subwindows;

// the bot
static MSBot bot;

// entry-point
void bot_entry_point(void* a) {
	while(true)
		bot.run();
}

// procedure called by gui window when processing messages sent to it
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH hBrushColor = CreateSolidBrush(BACKGROUND_COLOR); // create once for all

	switch(msg) {
	case WM_COMMAND: {
		if (((HWND) lParam) == subwindows[AUTOCC]) { // radios don't need to be checked for checkness
			bot.setAutoquit(false);
			bot.setAutoCC(true);
		} else if (((HWND) lParam) == subwindows[AUTOQUIT]) {
			bot.setAutoquit(true);
			bot.setAutoCC(false);
		} else if (((HWND) lParam) == subwindows[NEITHER]) { // option to get out of AUTOQUIT or AUTOCC (since they are mutually exclusive)
			bot.setAutoquit(false);
			bot.setAutoCC(false);
		} else if (((HWND) lParam) == subwindows[AUTOPOT] ||
			((HWND) lParam) == subwindows[AUTOPOT_HP_PERCENTAGE] ||
			((HWND) lParam) == subwindows[AUTOPOT_MP_PERCENTAGE]) {
			if (SendMessage(subwindows[AUTOPOT], BM_GETCHECK, 0, 0) == BST_CHECKED) {
				wchar_t hp_digits[3] = {0}; // includes null character
				GetWindowText(subwindows[AUTOPOT_HP_PERCENTAGE], hp_digits, 3); // 3 includes null character
				wchar_t mp_digits[3] = {0};
				GetWindowText(subwindows[AUTOPOT_MP_PERCENTAGE], mp_digits, 3);
				int mp_percentage = 0;
				int hp_percentage = 0;
				for (int i = 0; i < 3; i++) { // translates wchars to ints
					if (hp_digits[i] != '\0')
						hp_percentage = (hp_percentage * 10) + (hp_digits[i] - '0');
					if (mp_digits[i] != '\0')
						mp_percentage = (mp_percentage * 10) + (mp_digits[i] - '0');
				}
				if (mp_percentage == 0) // defaults
					mp_percentage = 50;
				if (hp_percentage == 0)
					hp_percentage = 50;
				bot.setPercentLimitHP(hp_percentage);
				bot.setPercentLimitMP(mp_percentage);
				bot.setAutopot(true);
			} else {
				bot.setAutopot(false);
			}
		} else if ((((HWND) lParam) == subwindows[SPAM_TEXT])) {
			bot.useSpamFile(false);
			const int BUFFER_SIZE = GetWindowTextLength(subwindows[SPAM_TEXT]) + 1; // GetWindowTextLengt() does not count null terminator, so add 1
			wchar_t* buffer = new wchar_t[BUFFER_SIZE]; // because BUFFER_SIZE may vary dynamically, compiler flags so must allocate memory
			GetWindowText(subwindows[SPAM_TEXT], buffer, BUFFER_SIZE);
			wstring w(buffer);
			string s(w.begin(), w.end());
			bot.setSpamText(s);
			delete[] buffer;
		} else if (((HWND) lParam) == subwindows[SPAM]) {
			if (SendMessage(subwindows[SPAM], BM_GETCHECK, 0, 0) == BST_CHECKED) { // how IsDlgButtonChecked() works underneath
				bot.setSpam(true);
			} else {
				bot.setSpam(false);
			}
		} else if (((HWND) lParam) == subwindows[SPAM_OPEN_FILE]) {
			// OPENS FILE DIALOG
			OPENFILENAME ofn = {0}; // USER THIS INITIALIZER FROM NOW ON INSTEAD OF NULLING EVERYTHING INDIVIDUALLY
			ofn.lStructSize = sizeof(OPENFILENAME);
			const int BUFFER_SIZE = 256;
			wchar_t buffer[BUFFER_SIZE] = {0}; // initialize all elms to 0
			ofn.lpstrFile = buffer; // REQUIRED, it seems
			ofn.nMaxFile = BUFFER_SIZE;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
			GetOpenFileName(&ofn);
			ifstream* filedata = new ifstream;
			filedata->open(ofn.lpstrFile); // works on this compiler for wstring paths as well
			SetWindowText(subwindows[SPAM_TEXT], L"[USING FILE UPLOAD FOR SPAM, DO NOT FOCUS THIS TEXTBOX]"); // didn't use Dlg version because too lazy to create resource file
			bot.useSpamFile(true, filedata); // since filedata is dynamic, must delete it
		} else if (((HWND) lParam) == subwindows[AUTOLOGIN]) {
			if (SendMessage(subwindows[AUTOLOGIN], BM_GETCHECK, 0, 0) == BST_CHECKED) { // how IsDlgButtonChecked() works underneath
				wchar_t username[13] = {0}; // 12 + 1 including null terminator
				wchar_t password[13] = {0};
				GetWindowText(subwindows[AUTOLOGIN_USERNAME], username, 13);
				GetWindowText(subwindows[AUTOLOGIN_PASSWORD], password, 13); // works for passwords, even if characters are hidden
				wstring wu(username);
				wstring wp(password);
				string su(wu.begin(), wu.end());
				string sp(wp.begin(), wp.end());
				bot.setUsername(su);
				bot.setPassword(sp);
				bot.setAutologin(true);
			} else {
				bot.setAutologin(false);
			}
		} else if (((HWND) lParam) == subwindows[AUTOCLICK]) {
			bot.setAutoclick(SendMessage(subwindows[AUTOCLICK], BM_GETCHECK, 0, 0) == BST_CHECKED);
		} else if (((HWND) lParam) == subwindows[AUTOLOOT]) {
			bot.setAutoloot(SendMessage(subwindows[AUTOLOOT], BM_GETCHECK, 0, 0) == BST_CHECKED);
		} else if (((HWND) lParam) == subwindows[AUTOATTACK]) {
			bot.setAutoattack(SendMessage(subwindows[AUTOATTACK], BM_GETCHECK, 0, 0) == BST_CHECKED);
		}
		return 0;
	}
	case WM_CTLCOLORSTATIC: { // sent when button is about to be drawn // must use parents inside switch case to avoid scoping issues
		SetBkColor((HDC) wParam, BACKGROUND_COLOR); // default background for all
		if (((HWND) lParam) == (subwindows[COPYRIGHT])) {
			SetTextColor((HDC) wParam, RGB(0, 122, 204));
			return (UINT) hBrushColor; // resolved error in functionality by returning brush color
		} else if (((HWND) lParam) == subwindows[CURRENT_CHANNEL]) { // font for current channel static control
			SetTextColor((HDC) wParam, RGB(255, 132, 0));
			return (UINT) hBrushColor;
		} else { // for copyright control only
			SetTextColor((HDC) wParam, RGB(60, 255, 60));
			return (UINT) hBrushColor; // resolved error in functionality by returning brush color
		}
	}
	case WM_CREATE: {// message received after window is created, but before visible // initializes all subwindows
		// dictionary of texts associated with subwindows
		unordered_map<int, wchar_t*> texts;
		texts[AUTOPOT] = L"Autopot (HP: DEL key, MP: END key)";
		texts[AUTOATTACK] = L"Autoattack (CTRL key)";
		texts[AUTOLOOT] = L"Autoloot (Z key)";
		texts[AUTOCLICK] = L"Autoclick";
		texts[SPAM] = L"Spam";
		texts[AUTOLOGIN] = L"Autologin (logins into first character)";
		texts[AUTOCC] = L"Auto CC";
		texts[AUTOQUIT] = L"Autoquit";
		texts[COPYRIGHT] = L"Copyright";
		texts[NEITHER] = L"Neither";

		// order in which texts are placed
		vector<const int> ORDER;
		ORDER.push_back(AUTOPOT);
		ORDER.push_back(AUTOATTACK);
		ORDER.push_back(AUTOLOOT);
		ORDER.push_back(AUTOCLICK);
		ORDER.push_back(SPAM);
		ORDER.push_back(AUTOLOGIN);
		ORDER.push_back(AUTOCC);
		ORDER.push_back(AUTOQUIT);
		ORDER.push_back(NEITHER);
		ORDER.push_back(COPYRIGHT);

		HWND temp; // checkboxes are windows too
	 	HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, L"Arial"); // arial font

		int y_offset = 10; // y offset for starting subwindow
		// const_iterator doesn't allow changing of values pointed to
		for (vector<const int>::const_iterator it = ORDER.begin(); it != ORDER.end(); it++, y_offset+=20) {
			int i = *it; // holds key
			if (i == AUTOCC) {
				temp = CreateWindowEx(0, L"BUTTON", texts[i], WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 10, y_offset, 75, 20, hwnd, NULL, NULL, NULL); // radio button
			} else if (i == AUTOQUIT) {
				temp = CreateWindowEx(0, L"BUTTON", texts[i], WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, y_offset, 75, 20, hwnd, NULL, NULL, NULL); // radio button is grouped with autocc
			} else if (i == NEITHER) {
				temp = CreateWindowEx(0, L"BUTTON", texts[i], WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 10, y_offset, 75, 20, hwnd, NULL, NULL, NULL); // radio button is grouped with autocc
				y_offset -= 20; // just some formatting idiosyncrasies
			} else if (i == COPYRIGHT) {
				// makeintresource(...) returns string that is what the documentation refers to as the name of the resource
				HWND noob = CreateWindowEx(WS_EX_TOPMOST, L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_CENTERIMAGE, 65, y_offset, 80, 64, hwnd, NULL, GetModuleHandle(NULL), NULL); // halts exec. when 3rd arg. is MAKEINTRESOURCE(IDB_NOOB), don't know why
				HBITMAP hBmp = (HBITMAP) LoadImage(NULL, L"images\\noob.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				SendMessage(noob, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBmp); // re-sets the image to specified image
				temp = CreateWindowEx(0, L"STATIC", L"Version: 0.1 \n Author: 4148 \n All rights reserved", WS_VISIBLE | SS_CENTER | WS_CHILD, 10, y_offset+=70, 200, 100, hwnd, NULL, NULL, NULL);
				HFONT copyrightFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN, L"Arial"); // copyright font
				SendMessage (temp, WM_SETFONT, WPARAM (copyrightFont), TRUE); // sets font for current subwindow
				subwindows[i] = temp; // needed since we continue
				continue;
			} else { // default is checkbox
				temp = CreateWindowEx(0, L"BUTTON", texts[i], WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, y_offset, 200, 20, hwnd, NULL, NULL, NULL);  // default checkbox
				if (i == SPAM) {
					HWND label = CreateWindowEx(0, L"STATIC", L"Spam Text", WS_VISIBLE | SS_CENTER | WS_CHILD, 20, y_offset+=20, 50, 20, hwnd, NULL, NULL, NULL); // labels spam text
					SendMessage(label, WM_SETFONT, WPARAM (hFont), TRUE);
					HWND spambox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, 75, y_offset, 150, 20, hwnd, NULL, NULL, NULL);
					SendMessage(spambox, WM_SETFONT, WPARAM (hFont), TRUE); // sets font for spambox
					HWND openfilebox = CreateWindowEx(0, L"BUTTON", L"Upload File", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 75, y_offset+=25, 100, 15, hwnd, NULL, NULL, NULL);  // default checkbox
					SendMessage(openfilebox, WM_SETFONT, WPARAM (hFont), TRUE); // sets font for button
					subwindows[SPAM_TEXT] = spambox;
					subwindows[SPAM_OPEN_FILE] = openfilebox;
				} else if (i == AUTOLOGIN) {
					HWND label = CreateWindowEx(0, L"STATIC", L"Username", WS_VISIBLE | SS_CENTER | WS_CHILD, 20, y_offset+=20, 50, 20, hwnd, NULL, NULL, NULL); // labels usernamebox
					SendMessage(label, WM_SETFONT, WPARAM (hFont), TRUE);
					HWND username_box = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | WS_TABSTOP, 75, y_offset, 100, 20, hwnd, NULL, NULL, NULL);
					SendMessage(username_box, WM_SETFONT, WPARAM (hFont), TRUE); // sets font for spambox
					SendMessage(username_box, EM_SETLIMITTEXT, 12, 0);
					label = CreateWindowEx(0, L"STATIC", L"Password", WS_VISIBLE | SS_CENTER | WS_CHILD, 20, y_offset+=20, 50, 20, hwnd, NULL, NULL, NULL); // labels passwordbox
					SendMessage(label, WM_SETFONT, WPARAM (hFont), TRUE);
					HWND password_box = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_PASSWORD | WS_TABSTOP, 75, y_offset, 100, 20, hwnd, NULL, NULL, NULL);
					SendMessage (password_box, WM_SETFONT, WPARAM (hFont), TRUE);
					SendMessage(username_box, EM_SETLIMITTEXT, 12, 0); // pw also has max of 12 chars
					subwindows[AUTOLOGIN_USERNAME] = username_box;
					subwindows[AUTOLOGIN_PASSWORD] = password_box;
				} else if (i == AUTOPOT) {
					HWND label = CreateWindowEx(0, L"STATIC", L"HP % Limit", WS_VISIBLE | SS_CENTER | WS_CHILD, 20, y_offset+=20, 60, 20, hwnd, NULL, NULL, NULL); // labels usernamebox
					SendMessage(label, WM_SETFONT, WPARAM (hFont), TRUE);
					HWND hp_percent_box = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"50", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER, 80, y_offset, 30, 20, hwnd, NULL, NULL, NULL);
					SendMessage(hp_percent_box, WM_SETFONT, WPARAM (hFont), TRUE); // sets font for spambox
					SendMessage(hp_percent_box, EM_SETLIMITTEXT, 2, 0); // 0-99
					label = CreateWindowEx(0, L"STATIC", L"MP % Limit", WS_VISIBLE | SS_CENTER | WS_CHILD, 20, y_offset+=20, 60, 20, hwnd, NULL, NULL, NULL); // labels passwordbox
					SendMessage(label, WM_SETFONT, WPARAM (hFont), TRUE);
					HWND mp_percent_box = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"50", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_NUMBER, 80, y_offset, 30, 20, hwnd, NULL, NULL, NULL);
					SendMessage (mp_percent_box, WM_SETFONT, WPARAM (hFont), TRUE);
					SendMessage(hp_percent_box, EM_SETLIMITTEXT, 2, 0); // 0-99
					subwindows[AUTOPOT_HP_PERCENTAGE] = hp_percent_box;
					subwindows[AUTOPOT_MP_PERCENTAGE] = mp_percent_box;
				}
			}
			SendMessage (temp, WM_SETFONT, WPARAM (hFont), TRUE); // sets font for current subwindow
			subwindows[i] = temp; // no need for individual assignment for each button now that indices correspond to map key
		}
		return 0;
	}
    case WM_CLOSE:
        DestroyWindow(hwnd); // closes parent window and child windows
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
	    return 0;
    default:
		return DefWindowProc(hwnd, msg, wParam, lParam); // messages not processed are given to DefWindowProc()
    }
}

// using this entry-point function makes linker default to GUI subsystem in Micrsoft Tool Chains
// APIENTRY specifies calling convention; WINAPI makes the callee instead of the caller clean up the stack
// changing calling convention for debugging purposes
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {

	// this is SECOND thread; main thread is the WINMAIN function (or the main that calls this WINMAIN)
	// each thread gets its own stack
	// in win32 multithreading, once primary thread dies, all other threads die
	// BECAUSE THE ENTRY-POINT FUNCTION DOESN'T ALLOW IMPLICIT THIS ARGUMENT, CANNOT USE MEMBER FUNCTIONS AS ENTRY_POINT
	_beginthread(bot_entry_point, 0, 0);

	const TCHAR* className = TEXT("MSBotGUI"); // specifies class name
    WNDCLASSEX wc; // WNDCLASSEX stores information about the kind of window
    // initialize WNDCLASSEX
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc; // pointer to window procedure
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance; // contains window procedure for class
    wc.hIcon         = (HICON) LoadImage(NULL, TEXT("images\\msbotgui.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(BACKGROUND_COLOR); // I like this grey color
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = className;
    wc.hIconSm       = (HICON) LoadImage(NULL, TEXT("images\\msbotgui.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);

	// registers window clas
    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

	HWND hwnd; // handle to window be created
    // once window class is registered, can create window
	// ^ WS_THICKFRAME prevents window resizing
    hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, className, TEXT("MSBot v0.1"), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 250, 475, NULL, NULL, hInstance, NULL);
    if(hwnd == NULL) {
        MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    ShowWindow(hwnd, nCmdShow); // show window; NCMDSHOW controls how the window is to be shown, so is passed
	UpdateWindow(hwnd); // update window updates client area of window by sending WM_PAINT message directly to window (instead of thread message queue)
	MSG msg;
    // message loop
	// > 0 to filter out error and quit messages
    while(GetMessage(&msg, NULL, 0, 0) > 0) { // since there is only one window, we don't need to specify handle to window
		if (!IsDialogMessage(hwnd, &msg)) { // IsDialogMessage specifically processes VK_TAB to carry out tabstop functionality
			TranslateMessage(&msg); // need translatemessage because intend to use character input
			DispatchMessage(&msg); // since draw operations do not take a long time, can wait for DispatchMessage() to return; no need to peek
		}
	}
    return msg.wParam; // returns exit code to system
}
