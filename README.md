# Simple text editor Notefall


Editor is written purely on WIN32 API, no MFC classes used.
So it's fast and simple.

Main advantage is that Notefall editor supports "Chome like" tabs in window title to open multiple files that saves screen space.
Try it out and take a look at the implementation of "Chrome" tabs.

## Functionality
- Open multiple text files in tabs
- Customizable text font and font color on Settings screen
- Find and replace text
- Cut/copy/paste as usual can be done with help of keyboard shortcuts, context menu, and from main menu
- Recent opened files menu (stored in registry)
- Status bar with text modified flag and with usefull statistics (zoom value, caret line and col numbers, ordering number of char under caret, total size of opened file)

## Building

Just open Notefall.sln in Visual Studio and press F5 (or Start Debugging from Debug menu).


