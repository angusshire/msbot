# msbot <img src="https://github.com/4148/msbot/blob/master/images/msbotgui.ico" width=2"%" height="2%"/>
msbot is a game bot for the MMORPG MapleStory v62. Previous bots only allowed keyboard input to be sent to the foreground window (the window with focus). This bot solves that problem by being able to send keystrokes to the window in the background.

A view of the bot:

<img src="https://raw.github.com/4148/msbot/master/msbot.png"/>

### How it Works

As shown, the bot supports features like autologin, autopot, and autoCC. This was done by simply capturing the game window into a BMP file and parsing it accordingly. Because of this dependence the result is not guaranteed to work in other versions of the game. All the bot features use the `PostMessage` Win32 API function call to send keystrokes to the background window.


### How to Use

To use, download the latest release and run the executable. Since the bot uses the resources in the `images` folder to parse game snapshots, do not modify anything inside that folder.

### Platforms
This bot works on Windows 7 64-bit. It likely works for other versions of Windows. 

### License
msbot is distributed under the GNU General Public License.
