// implementation file for MSBot.h

#include "MSBot.h"

// a signed char value (value <= 0x7F by default) has a max value of 0x7F
// anything greater is promoted to int
// since variables declared char are automatically unsigned, we must cast the int 221 (not signed char because greater than 0x7F) to
// unsigned to get rid of the warning
const Pixel MSBot::SELECTED_CHANNEL_COLOR(221u, 170u, 51u);
const Pixel MSBot::UNSELECTED_CHANNEL_COLOR(221u, 221u, 221u);

// Default constructor for MSBot
MSBot::MSBot() : hwnd(FindWindow(NULL, (LPCSTR) L"MapleStory")), username(""), password(""), spamText("This is the default spam text.") { // initialization list finds window named "MapleStory"
	currentChannel = 1; // 1 by default
	AUTOPOT_ON = AUTOCC_ON = AUTOLOGIN_ON = AUTOATTACK_ON = AUTOCLICK_ON = AUTOLOOT_ON = SPAM_ON = AUTOQUIT_ON = false;
	MINIMAP_MINIMIZED = false;
	hp = mp = totalMP = totalHP = -1;
	percentLimitHP = percentLimitMP = 50; // default percent limit is 50%
	minimap_width = minimap_height = y_minimap_pos = x_minimap_pos = x_minimap_header_pos = y_minimap_header_pos = -1;
	// NO FUNCTIONS inside this constructor require hwnd to be set, so it being NULL is ok... however must use in run()
	initializeImages();

	// 12/11/14
	USE_SPAM_FILE = false;
	file = NULL; // default is null
}

// driver function for bot
void MSBot::run() {
	if (hwnd == NULL) {
	  hwnd = FindWindow(NULL, (LPCSTR) L"MapleRoyals");
		if (hwnd == NULL) {
		  hwnd = FindWindow(NULL, (LPCSTR) L"MapleStory");
		}
		Sleep(3000);
		return;
	}
	if (NULL == FindWindow(NULL, (LPCSTR) L"MapleRoyals") && NULL == FindWindow(NULL, (LPCSTR) L"MapleStory")) { // eliminates problem of using same handle for different window
		hwnd = NULL;
		return;
	}

	if (!takeSnapshot())
		return;

	if (AUTOPOT_ON) {
		matchHP();
		matchMP();
		if (hp != -1 && totalHP != -1) {
			if ((hp*1.0/totalHP * 100) <= percentLimitHP) {
				potHP();
			}
		}
		if (mp != -1 && totalMP != -1) {
			if ((mp*1.0/totalMP * 100) <= percentLimitMP) {
				potMP();
			}
		}
	}
	if (AUTOCC_ON) { // AUTOCC and AUTOQUIT are mutually exclusive
		initializeMinimapFields();
		if (characterSpotted()) {
			ccNext();
			Sleep(3000);
		}
	} else if (AUTOQUIT_ON) {
		initializeMinimapFields();
		if (characterSpotted()) {
			quitGame();
			Sleep(3000);
		}
	}
	if (AUTOATTACK_ON)
		attack(); // 12/11/14: removed sleep(100) from attack(), loot(), click(), potHP(), potMP() and instead put it at the end of run to apply to all
	if (AUTOCLICK_ON)
		click();
	if (AUTOLOOT_ON)
		loot();
	if (SPAM_ON) {
		if (USE_SPAM_FILE) {
			sayFromFile();
		} else {
			say(spamText, true);
			Sleep(1000);
			say(string(spamText).append("!"), true);
		}
	}
	if (AUTOLOGIN_ON) {
		if (snapshot.equals(614, 357, images[LOGIN])) { // 614, 357 is position of "QUIT" button relative to the window; the image was chosen because it is small, and because it doesn't appear in any of the succeeding login windows
			login();
			Sleep(5000); // don't try to login multiple times in a row quickly if one login fails to avoid DC
		}
	}
	Sleep(100); // to make time for main thread
}

// SETTERS for LOGIN
// setter for USERNAME
void MSBot::setUsername(string s) {
	username = s;
}
// setter for PASSWORD
void MSBot::setPassword(string s) {
	password = s;
}

// assumes no pin needed for login
// precondition: atLogin() must return true
void MSBot::login() {
	Sleep(2000);
	PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0); // eliminates need to image rec disconnect window by just exiting it with this key; if not in it, doesn't matter when in login window (does nothing)
	Sleep(2000);

	LPARAM lParam = MAKELPARAM(450, 249); // moves mouse to username text position
	PostMessage(hwnd, WM_MOUSEMOVE, 0, lParam);
	Sleep(50);
	RECT rect;
	GetWindowRect(hwnd, &rect);
	SetCursorPos(rect.left + 450 + 6, rect.top + 249 + 25); // window that contains 800x600 images has an X offset of 6 and Y offset of 25
	PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam); // mouse click
	Sleep(50);
	PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
	Sleep(50);

	// resets text space
	for (int i = 0; i < 12; i++) { // max username is 12 chars
		PostMessage(hwnd, WM_KEYDOWN, VK_RIGHT, 0);
		PostMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
		Sleep(50);
	}
	// enters username
	for (unsigned int i = 0; i < username.length(); i++) {
		PostMessage(hwnd, WM_CHAR, (WPARAM) username.at(i), 0);
		Sleep(50); // solves message block problem
	}

	// goes to next text space
	PostMessage(hwnd, WM_KEYDOWN, VK_TAB, 0);

	// resets text space
	for (int i = 0; i < 12; i++) { // max username is 12 chars
		PostMessage(hwnd, WM_KEYDOWN, VK_RIGHT, 0);
		PostMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
		Sleep(50);
	}
	// enters password
	for (unsigned int i = 0; i < password.length(); i++) {
		PostMessage(hwnd, WM_CHAR, (WPARAM) password.at(i), 0);
		Sleep(50);
	}

	// logs in
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	Sleep(8000); // takes a while to login

	// opens up display of channels on server Scania
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	Sleep(2000); // takes a while for channels to load

	// clicks on first channel
	lParam = MAKELPARAM(267, 298);
	PostMessage(hwnd, WM_MOUSEMOVE, 0, lParam);
	Sleep(50);
	GetWindowRect(hwnd, &rect);
	SetCursorPos(rect.left + 267, rect.top + 301);
	PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, lParam);
	Sleep(2000); // takes a while for characters to load

	// selects first character in list; can easily be modified to support multiple chars by posting arrow key
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	PostMessage(hwnd, WM_KEYDOWN, VK_RIGHT, 0); // just in case not focused
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);

	Sleep(7500); // takes a while to login

	PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0); // if login succesful, will do nothing; if login not succesful, will go back to main page
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);

}
// simulates click in the game
// window must be focused and positioned by the user
void MSBot::click() {
	LPARAM lParam = MAKELPARAM(0, 0); // default position is in upper left of window; must be repositioned by user
	PostMessage(hwnd, WM_LBUTTONDOWN, 0,lParam); // mouse click
	PostMessage(hwnd, WM_LBUTTONUP, 0, lParam);
	PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, lParam);
}
// quits the game with keys ESC, UP ARROW, RETURN in that order
void MSBot::quitGame() {
	PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0x00010000);
	PostMessage(hwnd, WM_KEYDOWN, VK_UP, 0x00010000);
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0x00010000);
}
// SPAM functions
// 12/11/14: added useSpamFile(bool)
// sets USE_SPAM_FILE flag
void MSBot::useSpamFile(bool b, ifstream* filea) {
	USE_SPAM_FILE = b;
	if (NULL != filea) { // if being pass new file
		if (file != NULL) { // delete original file if it exists
			if (file->is_open()) { // close if open
				file->close();
			}
			delete file;
		}
		file = filea;
	}
}
// sets SPAMTEXT
void MSBot::setSpamText(string s) {
	spamText = s;
}
// enters string S into chatbox, breaking it down according to the message box width; make sure chatbox is not in focus before calling
// 12/12/14: modified to return BOOL to allow selective closing of file
// returns true iff say() completed operation
bool MSBot::say(string s, bool focus) {
	int idx = 0;
	int chars_left = s.length();
	if (!focus) {
		PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0x00010000);
	}
	while (chars_left > 0) {
		if (!SPAM_ON) {// added 12/12/14: allows to break from spam from file operation
			return false;
		}

		int cutoff = idx + MESSAGE_BOX_WIDTH;
		if (chars_left < MESSAGE_BOX_WIDTH) {
			cutoff = s.length();
		} else {
			int cutoff_save = cutoff;
			while (s.at(cutoff-1) != ' ') { // can't use s.at(cuttoff) because it may not exist
				if (cutoff <= idx) { // if subline is entirely nonspaces, just print out entire line then
					cutoff = cutoff_save;
					break;
				}
				cutoff--; // truncates early if cutoff ends in middle of a word
			}
		}
		chars_left -= (cutoff - idx);
		for (int i = idx; i < cutoff; i++, idx++) {
			PostMessage(hwnd, WM_CHAR, (WPARAM) s.at(i), 0);
			Sleep(50); // solves message block problem
		}
		PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0x00010000);
		Sleep(500); // 12/12/14
	}
	return true;
}
// calls say() on the contents of the file
// 12/11/14: modified to take nothing instead of string filename/file handle
void MSBot::sayFromFile() {
	std::stringstream buffer;
	buffer << (file->rdbuf());
	string content = buffer.str();
	replace(content.begin(), content.end(), '\n', ' ');
	if (file->is_open()) {
		if (say(content, false)) // assumes chatbox not focused; closes file handle only if say() completes
			file->close();
		else
			file->seekg(0, ios::beg); // sets internal pointer of associate streambuf object back to begginning of stream; since I didn't read past the end, no eofbit was set, and thus no need to clear() the eof state
		return;
	} else {
		return; // returns if unable to open file
	}
}
// loots with specified LOOTKEY
void MSBot::loot() {
	PostMessage(hwnd, WM_KEYDOWN, 0, LOOT_KEY);
}

void MSBot::attack() {
	PostMessage(hwnd, WM_KEYDOWN, 0, ATTACK_KEY);
}

// HELPER functions
// initializes store images into memory
// precondition: templates directory file structure is not changed, and BMP images inside templates are not deleted or renamed
void MSBot::initializeImages() {
	WIN32_FIND_DATA file;
	HANDLE hFindFile = FindFirstFile(TEXT("images\\templates\\*.bmp"), &file);
	WORD images_initialized = 0;
	if (hFindFile != INVALID_HANDLE_VALUE) {
		DWORD rtnFindNext;
		do {
			tstring temp(TEXT("images\\templates\\"));
			temp.append(file.cFileName);
			HBITMAP hBMP = (HBITMAP) LoadImage(NULL, temp.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); // loads handle to DIB

			tstring temp2(file.cFileName); // must match filename to int
			int rep_idx = temp2.find(TEXT(".bmp"));

			if (rep_idx != string::npos) {
				temp2.replace(rep_idx, strlen(".bmp"), TEXT(""));
				int num = mapImage(temp2); // maps image name to corresponding int

				if (-1 != num) {
					images[num] = Image(hBMP);
					images_initialized++;
				} else {
					cout << "Warning: Do not rename or add additional BMP files in MSBot\\images\\bar_numbers directory." << endl;
				}
			} else {
				cout << "Warning: Irrelevant files present in MSBot\\images\\bar_numbers directory." << endl;
			}
			rtnFindNext = FindNextFile(hFindFile, &file);
		} while (rtnFindNext != 0 && rtnFindNext != ERROR_NO_MORE_FILES);
	}
	FindClose(hFindFile);
	if (NUM_IMAGES != images_initialized) {
		cout << "Fatal Internal Error: Unable to initialize all images. " << images_initialized << " out of " << NUM_IMAGES << " images were initialized." << endl;
		exit(1);
	}
}
// maps name of image (without file extension) to proper int; returns -1 if no corresponding int found
int MSBot::mapImage(tstring s) {
	if (s == TEXT("zero")) { return 0; }
	else if (s == TEXT("one")) { return 1; }
	else if (s == TEXT("two")) { return 2; }
	else if (s == TEXT("three")) { return 3; }
	else if (s == TEXT("four")) { return 4; }
	else if (s == TEXT("five")) { return 5; }
	else if (s == TEXT("six")) { return 6; }
	else if (s == TEXT("seven")) { return 7; }
	else if (s == TEXT("eight")) { return 8; }
	else if (s == TEXT("nine")) { return 9; }
	else if (s == TEXT("slash")) { return SLASH; }
	else if (s == TEXT("login")) { return LOGIN; }
	else if (s == TEXT("minimap-header")) { return MINIMAP_HEADER; }
	else if (s == TEXT("minimap-minimize-plus")) { return MINIMAP_MINIMIZE_PLUS; }
	else if (s == TEXT("minimap-world")) { return MINIMAP_WORLD; }
	else if (s == TEXT("minimap-bottom")) { return MINIMAP_BOTTOM; }
	else {
		cout << "Warning: Invalid image name for mapImage()." << endl;
		return -1;
	}
}
// note: bmBits field is a pointer to the color-index table (pixel array); because 600 * 3 bytes is a multiple of 4 (32 bits), the scan lines
// are not padded meaning I don't need to worry about padded zeroes
// takes a snapshot in background; did not use built-in scroll lock snapshot feature because it does not work if the window is in the background
// 12/10/14: Since speed seems fine, I won't be modifying takeSnapshot to not save the image
// 12/10/14: modified to return boolean, making sure that WE HAVE A SNAPSHOT SET before operating
bool MSBot::takeSnapshot() {
	RECT rect;
	if (0 == GetWindowRect(hwnd, &rect)) {
		return false; // game window is not fully set up, so return
	}
	if (rect.right < 0) { // if window is maximized, minimize it
		minimizeWindow();
		Sleep(2000); // 3000 seems too long
		GetWindowRect(hwnd, &rect);
		if (rect.right < 0) {
			return false;
		}
	}

	HDC hDC = GetDC(hwnd); // gets DC associated with game window
	HDC hCaptureDC = CreateCompatibleDC(hDC); // create buffer DC to copy bitmap into
	int width = 800; // assumes that the dimensions of the minimized window are 800 x 600
	int height = 600;
	HBITMAP hCaptureBMP = CreateCompatibleBitmap(hDC, width, height); // creates device-dependent bitmap for capturing game window
	SelectObject(hCaptureDC, hCaptureBMP); // associates bitmap with buffer DC
	// copies bit blocks to buffer DC from source DC
	if (NULL == hDC || NULL == hCaptureDC|| NULL == hCaptureBMP || 0 == BitBlt(hCaptureDC, 0, 0, width, height, hDC, 0, 0, SRCCOPY)) {
		return false;
	}

	// save captured bitmap by:
	// (1) initializing BITMAPINFOHEADER
	// (2) initializing BITMAPFILEHEADER
	// (3) allocating memory for buffer bitmap
	// (4) storing captured bitmap into buffer bitmap
	// (5) retrieving file handle
	// (6) writing to BITMAPFILEHEADER, BITMAPINFOHEADER, and bitmap to file with file handle
	BITMAPINFOHEADER bi; // specifies format of DIB
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // 24-bit bmp
    bi.biCompression = BI_RGB;  // uncompressed

	// bitmap size in bytes to ceiling byte
	DWORD dwBmpSize = (((width * bi.biBitCount + 31) / 32) * 4 * height);
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char *lpbitmap = (char*) GlobalLock(hDIB); // returns pointer to DIB

    // copies bits from hCaptureBMP into a buffer pointed to by lpbitmap.
    GetDIBits(hCaptureDC, hCaptureBMP, 0, (UINT) height, lpbitmap, (BITMAPINFO *) &bi, DIB_RGB_COLORS);

	/*
	strlen(lpbitmap)
	BITMAP bmp
	..
	*/

	// create file to store snapshot
    HANDLE hFile = CreateFile((LPCSTR) L"images\\snapshot.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// total file size of DIB is sum of size of bitmap, size of BITMAPFILEHEADER, and size of BITMAPINFOHEADER
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	BITMAPFILEHEADER bmfHeader;
    // offsets to where bitmap starts
    bmfHeader.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + (DWORD) sizeof(BITMAPINFOHEADER);
	// file size
	bmfHeader.bfSize = dwSizeofDIB;
    //bfType must abe BM (0x4D42) for Bitmaps
    bmfHeader.bfType = 0x4D42;

    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR) &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR) &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR) lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // file cleanup
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    CloseHandle(hFile);

	// DC and BMP cleanup
	ReleaseDC(hwnd, hDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBMP);

	// loads snapshot into memory
	static HBITMAP hBMP = NULL; // since static, will be persistent across all instances of the class; however doesn't matter as behavior still same
	if (hBMP != NULL) { DeleteObject(hBMP); } // 12/10/14: fixed memory leak with HBMP; did not delete it when loading another image;
	hBMP = (HBITMAP) LoadImage(NULL, (LPCSTR) L"images\\snapshot.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); // must use LR_CREATEDDIBSECTION to return associate with a DIB
	if (hBMP == NULL) {
		cout << " IMAGE IS NULL  " << GetLastError() << endl;
		return false;
	}
	bool res = snapshot.setBMP(hBMP);
	return res;
}
// minimizes window by:
// (1) bringing focus to game window
// (2) Sending ALT + RETURN keystrokes accordingly; assumes that game does not have this feature disabled
// since this event should be a one time occurence, I decided not to have the window minimized in the background
void MSBot::minimizeWindow() {
	SetForegroundWindow(hwnd);
	Sleep(1000);
	INPUT in[4];
	in[0].type = INPUT_KEYBOARD;
	in[0].ki.wScan = 0;
	in[0].ki.time = 0;
	in[0].ki.dwExtraInfo = 0;
	in[0].ki.wVk = VK_MENU;
	in[0].ki.dwFlags = 0; // 0 for keypress
	in[1] = in[0];
	in[1].ki.wVk = VK_RETURN;
	// keyups
	in[2] = in[1];
	in[3] = in[0];
	in[2].ki.dwFlags = in[3].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(4, (LPINPUT) &in, sizeof(INPUT));
	Sleep(1000);
}

// AUTOPOT FUNCTIONS
// matchHP() returns true iff CURRENT HP and TOTAL HP are matched
void MSBot::matchHP() {
	int x = 241; // (241, 571) is where HP bar numbers begin
	int y = 571;
	hp = getBarNumber(x, y);
	if (hp == -1) { return; }
	else if (snapshot.equals(x, y, images[SLASH], 4000)) {
		x += 8; // 8 offset for slash
		totalHP = getBarNumber(x, y);
		if (totalHP == -1) { return; }
	} else { return; } // anomaly
}

// matchMP() returns true iff MP and TOTALMP are matched
void MSBot::matchMP() {
	int x = 353; // (353, 571) is where MP bar numbers begin
	int y = 571;
	mp = getBarNumber(x, y);
	if (mp == -1) { return; }
	else if (snapshot.equals(x, y, images[SLASH], 4000)) {
		x += 8; // 8 offset for slash
		totalMP = getBarNumber(x, y);
		if (totalMP == -1) { return; }
	} else { return; } // anomaly
}
// pots HP
void MSBot::potHP() {
	PostMessage(hwnd, WM_KEYDOWN, 0, HP_KEY);
}
// pots MP
void MSBot::potMP() {
	PostMessage(hwnd, WM_KEYDOWN, 0, MP_KEY);
}
// this works only if you do not cover the HP/MP bar with another window inside the game
// note: only works for numbers displayed above the HP/MP bar; does not generalize to anything else;
// a bar number is 7 pixels in height and 5 pixels in width
// returns an int > 0 indicating bar number matched, or -1 if no match was found
// precondition: INCREMENTS X
int MSBot::getBarNumber(int& x, int& y) {
	int res = 0;
	bool MATCH = true;
	int numMatches = 0;
	while (MATCH) {
		MATCH = false;
		for (int i = 0; i <= 9; i++) {
			if (snapshot.equals(x, y, images[i], 4000)) { // last time tested average SED was around 1700
				res = (res * 10) + i;
				MATCH = true;
				numMatches++;
				x += 6; // increment by 6 because bar number is 5 pixels in width
				break;
			}
		}
	}
	if (numMatches == 0) { return -1; }
	return res;
}
// setter for PERCENTLIMITHP
void MSBot::setPercentLimitHP(int p) {
	if (p > 0 && p < 100) {
		percentLimitHP = p;
	} else {
		cout << "Warning: percentLimitHP cannot be <= 0 or >= 100." << endl;
	}
}
// setter for PERCENTLIMITMP
void MSBot::setPercentLimitMP(int p) {
	if (p > 0 && p < 100) {
		percentLimitMP = p;
	} else {
		cout << "Warning: percentLimitMP cannot be <= 0 or >= 100." << endl;
	}
}
// setter for TOTALHP
void MSBot::setTotalHP(int p) {
	if (p < 1 || p < hp) {
		cout << "Warning: totalHP cannot be < 1 or < current hp." << endl;
	} else {
		totalHP = p;
	}
}
// setter for TOTALMP
void MSBot::setTotalMP(int p) {
	if (p < 1 || p < mp) {
		cout << "Warning: totalMP cannot be < 1 or < current mp." << endl;
	} else {
		totalMP = p;
	}
}
// setter for HP
void MSBot::setHP(int p) {
	if (p < 0 || totalHP < p) {
		cout << "Warning: hp cannot be < 0 or > totalHP." << endl;
	} else {
		hp = p;
	}
}
// setter for MP
void MSBot::setMP(int p) {
	if (p < 0 || totalMP < p) {
		cout << "Warning: mp cannot be < 0 or > totalMP." << endl;
	}  else {
		mp = p;
	}
}

// SETTERS FOR BOT FEATURES
// setter for SPAM_ON
void MSBot::setSpam(bool b) {
	SPAM_ON = b;
}
// setter for autoquit
void MSBot::setAutoquit(bool b) {
	AUTOQUIT_ON = b;
}
// setter for AUTOLOOT_ON
void MSBot::setAutoloot(bool b) {
	AUTOLOOT_ON = b;
}
// setter for AUTOCLICK_ON
void MSBot::setAutoclick(bool b) {
	AUTOCLICK_ON = b;
}
// setter for AUTOATTACK_ON
void MSBot::setAutoattack(bool b) {
	AUTOATTACK_ON = b;
}
// setter for AUTOLOGIN_ON
void MSBot::setAutologin(bool b) {
	AUTOLOGIN_ON = b;
}
// setter for AUTOPOT_ON
void MSBot::setAutopot(bool b) {
	AUTOPOT_ON = b;
}
// setter for AUTOCC_ON
void MSBot::setAutoCC(bool b) {
	AUTOCC_ON = b;
}

// CC Functions
// setter for CURRENTCHANNEL field
// first opens up channel window, then examines each channel box for current channel color
// assumes channel window opens up in same place everytime, which for the current version does
// returns current channel, or -1 if getChannel() fails
// functionality fails when screen cannot display channel window (e.g., black screen); but otherwise works moderately well
// not used in run() (or anywhere else) because presently has no practical use
void MSBot::setChannel() {
	currentChannel = 1; // 1 by default
	static const int X_CHANNEL_INCREMENT = 70;  // constant for incrementing horizontally to the next channel
	static const int Y_CHANNEL_INCREMENT = 20; // constant for incrementing vertically to the next channel
	static const int X_CHANNEL_START = 230; // x coordinate for where to start looking for channel pixel
	static const int Y_CHANNEL_START = 305; // y coordinate for where to start looking for channel pixel
	int curX = X_CHANNEL_START, curY = Y_CHANNEL_START; // current coordinates
	PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
	Sleep(50);
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	Sleep(50);
	Pixel currentPixel;
	for (int channel = 1; channel <= NUM_CHANNELS; channel++) {
		currentPixel = snapshot.getPixel(curX, curY);
		if (currentPixel.match(SELECTED_CHANNEL_COLOR, UNSELECTED_CHANNEL_COLOR) == SELECTED_CHANNEL_COLOR) {
			PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
			Sleep(50);
			if (validChannel(channel)) {
				currentChannel = channel;
				return;
			}
		} else if ((channel % 6) == 0) { // if channel is a multiple of 6, then go to next row
			curX = X_CHANNEL_START;
			curY += Y_CHANNEL_INCREMENT;
		} else {
			curX += X_CHANNEL_INCREMENT;
		}
	}
}
// getter for channel
int MSBot::getChannel() {
	return currentChannel;
}
// for MapleRoyals, valid channel is channel between 1 and 8, inclusive
bool MSBot::validChannel(int channel) {
	return (channel >= 1 && channel <= NUM_CHANNELS);
}
// change channels to specified channel with keys ESC, RETURN, RIGHT ARROWS OR LEFT ARROWS, RETURN in that order
void MSBot::cc(int channel) {
	if (validChannel(channel) && validChannel(currentChannel)) {
		PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
		PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
		if (currentChannel > channel) {
			for (int i = 0; i < (currentChannel-channel); i++) {
				PostMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0);
			}
		} else {
			for (int i = 0; i < (channel-currentChannel); i++) {
				PostMessage(hwnd, WM_KEYDOWN, VK_RIGHT, 0);
			}
		}
		PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
		currentChannel = channel;
	} else {
		cout << "Channel must be between 1 to 8, inclusive." << endl;
		return;
	}
}
// change channels to next channel with keys ESC, RETURN, RIGHT ARROW, RETURN in that order
void MSBot::ccNext() {
	PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	PostMessage(hwnd, WM_KEYDOWN, VK_RIGHT, 0);
	PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	if (validChannel(currentChannel)) {
		if (currentChannel == 8) {
			currentChannel = 1;
		} else {
			currentChannel++;
		}
	}
}

// MINIMAP functions
// initializes minimap fields; also calls setMinimapStatus(), setMinimapWidth(), setMinimapHeight()
// returns true iff minimap is identified
bool MSBot::initializeMinimapFields() {
	static const int Y_MINIMAP_MINIMIZED_OFFSET = 23; // the offsets from the y_minimap_header_pos where to start looking for minimap
	static const int Y_MINIMAP_MAXIMIZED_OFFSET = 67;
	for (int y = 0; y < (snapshot.height()-images[MINIMAP_HEADER].height()); y++) {
		for (int x = 0; x < (snapshot.width()-images[MINIMAP_HEADER].width()); x++) {
			if (snapshot.equals(x, y, images[MINIMAP_HEADER])) {
				x_minimap_header_pos = x;
				y_minimap_header_pos = y;
				setMinimapWidth(); // MUST be called here only, after X/Y_MINIMAP_HEADER_POS have been set
				if (MINIMAP_MINIMIZED) {
					x_minimap_pos = x_minimap_header_pos;
					y_minimap_pos = y_minimap_header_pos + Y_MINIMAP_MINIMIZED_OFFSET;
				} else {
					x_minimap_pos = x_minimap_header_pos;
					y_minimap_pos = y_minimap_header_pos + Y_MINIMAP_MAXIMIZED_OFFSET;
				}
				setMinimapHeight(); // MUST be called here only, after X/Y_MINIMAP_POS have been set
				return true;
			}
		}
	}
	cout << "Warning: Unable to identify minimap. Make sure that the minimap is not covered or maximally minimized." << endl;
	return false;
}
// returns true iff the minimap has a Pixel of CHARACTER_COLOR
bool MSBot::characterSpotted() {
	if (x_minimap_pos == -1 || y_minimap_pos == -1) {
		cout << "Warning: characterSpotted() failed because X_MINIMAP_POS and Y_MINIMAP_POS were not set." << endl;
		return false;
	}
	static const Pixel CHARACTER_COLOR = Pixel(0, 0, 238);
	for (int y = y_minimap_pos; y < y_minimap_pos + minimap_height; y++) {
		for (int x = x_minimap_pos; x < x_minimap_pos + minimap_width; x++) {
			if (snapshot.getPixel(x, y) == CHARACTER_COLOR) {
				return true;
			}
		}
	}
	return false;
}
// sets minimap_width and MINIMAP_MINIMIZED fields
void MSBot::setMinimapWidth() {
	for (int x = x_minimap_header_pos; x < (snapshot.width()-images[MINIMAP_MINIMIZE_PLUS].width()); x++) {
		if (snapshot.equals(x, y_minimap_header_pos, images[MINIMAP_MINIMIZE_PLUS])) {
			MINIMAP_MINIMIZED = true;
		} else if (snapshot.equals(x, y_minimap_header_pos, images[MINIMAP_WORLD])) {
			minimap_width = x + 36 - x_minimap_header_pos; // 36 is width of the WORLD button (remember, widths and heights don't have zero index)
			return;
		}
	}
	MINIMAP_MINIMIZED = false;
	minimap_width = -1;
	cout << "Warning: Unable to set minimap width." << endl;
}
// sets minimap_height field
void MSBot::setMinimapHeight() {
	for (int y = y_minimap_pos; y < (snapshot.height()-images[MINIMAP_BOTTOM].height()); y++) {
		if (snapshot.equals(x_minimap_pos, y, images[MINIMAP_BOTTOM])) {
			minimap_height = (y - y_minimap_pos);
			return;
		}
	}
	minimap_height = -1;
	cout << "Warning: Unable to set minimap height." << endl;
}
