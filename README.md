# msbot
msbot is a game bot for the MMORPG MapleStory v62. Previous bots only allowed keyboard input to be sent to the foreground window (the window with focus). This bot solves that problem by being able to send keystrokes to the window in the background.

A view of the bot:

<p align='center'><img src="https://raw.github.com/4148/msbot/master/msbot.png" height="50%" width="50%"/></p>

### How it Works

As shown in the image above, the bot supports features like autologin, autopot, and autoCC. This was done by simply capturing the game window into a BMP file and parsing it accordingly. Because of this dependence the result is not guaranteed to work in other versions of the game. All the bot features use the `PostMessage` Win32 API function call to send keystrokes to the background window.

Image recognition is done by calculating the squared euclidean distance (SED) using the RGB pixels of the snapshot image and corresponding template. If the SED is below a certain threshold, then it matches the template, and the bot takes an action accordingly.


### How to Use
To use, download the latest release and run the executable. Since the bot uses the resources in the `images` folder to parse game snapshots, do not modify anything inside that folder.

From there, usage is intuitive: simply check the checkboxes corresponding to the desired functionality. 

### Platforms
This bot works on Windows 7 64-bit. It likely works for other versions of Windows. 

### License
msbot is distributed under the GNU General Public License.
