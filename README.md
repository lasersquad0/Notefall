Simple text editor

Written purely on WIN32 API, no MFC classes used.

Main advantage is that Notefall editor supports "Chome like" tabs in window title to open multiple files that saves screen space.
Try it out and take a look at the implementation of "Chrome" tabs.

Functionality:
- open multiple text files in tabs
- customizable text font and font color on Settings screen
- find and replace text
- cut/copy/paste as usual can be done with help of keyboard shortcuts, context menu, and from main menu
- Recent opened files menu (stored in registry)
- Status bar with text modified flag and with usefull statistics (zoom value, caret line and col numbers, ordering number of char under caret, total size of opened file)
