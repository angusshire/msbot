# msbot <img src="https://github.com/4148/msbot/blob/master/images/msbotgui.ico" width=2"%" height="2%"/>
This is a game bot for the MMORPG MapleStory v62. Previous bots allowed only keyboard input to be sent to the foreground window (the window with focus). This bot solves that problem by sending keystrokes to the game window in the background.

A view of the bot:

 <p align="center"><img src="https://raw.github.com/4148/msbot/master/msbot.png"/></p>

As shown, the bot supports features like autologin, autpot, and autoCC. This was done by simply capturing the game window into a BMP file and parsing it accordingly. Of course this method is highly dependent and as a result is not guaranteed to work in other versions of the game. All the bot features use the PostMessage() Win32 API function call to send keystrokes to the background window.

To use, download the latest release and run the executable. Since the bot uses the resources in the `images` folder to parse game snapshots, do not modify anything inside that folder.

### Platforms
This bot works on Windows 7 64-bit. It likely works for other versions of Windows. 
