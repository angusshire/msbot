// Copyright (c) 2014 Angus H. (4148)
// Distributed under the GNU General Public License v3.0 (GPLv3).

// header file for MSBot.cpp

#ifndef WINVER
#define WINVER 0x0500
#endif

#ifndef MSBOT_CLS
#define MSBOT_CLS

// string portability
#ifdef _UNICODE
#define tstring wstring
#define tcout wcout
#define _tcscmp wcscmp
#else
#define tstring string
#define tcout cout
#define _tcscmp strcmp
#endif

// include files
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <string>
#include "Pixel.h"
#include "Image.h"
using namespace std;

class MSBot {
public:
	MSBot();
	void run();
	// CC functions
	int getChannel();
	// SETTERS for LOGIN
	void setUsername(string);
	void setPassword(string);
	// SETTERS FOR BOT FEATURES
	void setAutopot(bool);
	void setAutoCC(bool);
	void setAutoattack(bool);
	void setAutoclick(bool);
	void setAutologin(bool);
	void setAutoloot(bool);
	void setAutoquit(bool);
	void setSpam(bool);
	void setSpamText(string);
	void setPercentLimitHP(int);
	void setPercentLimitMP(int);
	void useSpamFile(bool, ifstream* file=NULL);
private:
	// PRIVATE FIELDS
	// FLAGS
	bool MINIMAP_MINIMIZED; // true iff minimap is minimized; default is FALSE
	bool USE_SPAM_FILE; // 12/11/14: true iff want to use opened file for spam
	// flags for bot features
	bool AUTOPOT_ON;
	bool AUTOCC_ON;
	bool AUTOLOGIN_ON;
	bool AUTOATTACK_ON;
	bool AUTOCLICK_ON;
	bool AUTOLOOT_ON;
	bool SPAM_ON;
	bool AUTOQUIT_ON;
	// CONSTANTS
	// constants indicating channel color
	static const Pixel SELECTED_CHANNEL_COLOR;
	static const Pixel UNSELECTED_CHANNEL_COLOR;
	// number of channels game
	static const int NUM_CHANNELS = 8; // only static const integral data members can be initialized within a class
	static const int MESSAGE_BOX_WIDTH = 70; // message box width is 70 characters wide
	// constants indicating number of images that should be initialized
	static const int NUM_IMAGES = 16;
	// constants indicating kind of image
	static const int NUMBER = 2;
	// these contants indicate kind of image and also serve as indices in images[] to respective image
	static const int SLASH = 10;
	static const int LOGIN = 11;
	static const int MINIMAP_HEADER = 12;
	static const int MINIMAP_WORLD = 13;
	static const int MINIMAP_MINIMIZE_PLUS = 14;
	static const int MINIMAP_BOTTOM = 15;
	// KEYS
	static const DWORD HP_KEY = 0x00530000; // DEL key
	static const DWORD MP_KEY = 0x004F0000; // END key
	static const DWORD ATTACK_KEY = 0x001D0000; // CTRL key
	static const DWORD LOOT_KEY = 0x002C0000; // Z key
	// SPAM TEXT
	string spamText;
	// AUTOPOT VARIABLES
	int hp; // current HP and MP
	int mp;
	int totalHP; // total HP and MP
	int totalMP;
	// HP and MP percentage
	int percentLimitHP; // HP and MP percentage; e.g., percentLimitHP = 50 means HP key should be pressed at <= 50 % HP
	int percentLimitMP;
	// CURRENT CHANNEL
	int currentChannel;
	// FILE HANDLE if USE_SPAM_FILE is true
	ifstream* file;
	// HANDLE TO GAME WINDOW
	HWND hwnd;
	// stores USERNAME and PASSWORD for login()
	string username;
	string password;
	// MINIMAP FIELDS
	// default value is -1, because valid values are positive
	int minimap_width; // the width of the minimap in pixels
	int minimap_height; // the height of the minimap in pixels
	int y_minimap_pos;
	int x_minimap_pos;
	int x_minimap_header_pos; // coordinates (X, Y) in snapshot where minimap header begins
	int y_minimap_header_pos;
	// IMAGE FIELDS
	Image images[NUM_IMAGES]; // array of Image objects, which hold the image's color-index bitmap; elements 0-9 correspond to bitmaps for their respective integers,
	// element 10 corresponding to the bitmap of a forward slash
	// element 11 corresponding to the bitmap of part of the login
	// element 12 corresponding to the bitmap of the minimap-header
	// element 13 corresponding to the bitmap of the minimap-world button
	// element 14 corresponding to the bitmap of the minimap-minimize-plus button
	// element 15 corresponding to the bitmap of part of the minimap-bottom
	Image snapshot; // char values representing color-index table of snapshot

	// INTERNAL FUNCTION
	// SPAM functions
	bool say(string, bool focus);
	void sayFromFile();
	// MINIMAP functions
	void setMinimapHeight();
	void setMinimapWidth();
	bool initializeMinimapFields();
	bool characterSpotted();
	// OTHER GAME BEHAVIOR functions
	void login();
	void click();
	void quitGame();
	void attack();
	void loot();
	// helper functions
	void setChannel();
	void initializeImages();
	void minimizeWindow();
	int mapImage(tstring);
	bool takeSnapshot();
	// CC Functions
	bool validChannel(int);
	void cc(int);
	void ccNext();
	// AUTOPOT FUNCTIONS
	void potHP();
	void potMP();
	void matchHP();
	void matchMP();
	int getBarNumber(int&, int&);
	void setTotalHP(int);
	void setTotalMP(int);
	void setHP(int);
	void setMP(int);

};

// helper functions outside class

#endif
