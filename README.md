# msbot
Gaming bot for MS v62. Previous bots suffered from the fact that keyboard input must be sent to the game window with focus. This bot solves that problem by sending keystrokes to the game window in the background.

A view of the bot:

![ScreenShot](https://raw.github.com/4148/msbot/master/msbot.png)

As shown, the bot supports features like autologin, autpot, and autoCC. This was done by simply capturing the game window into a BMP file and parsing it accordingly. Of course this method is highly dependent and as a result is not guaranteed to work in other versions. 
