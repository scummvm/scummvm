README FOR THE MAC CARBON PORT

Runs on Mac OS X 10.1 (could also run on 10.0.x, but not tested), and Mac OS 8.6 or later (with CarbonLib
installed).

HOW TO COMPILE?
Launch the scummvm.mcp file in CodeWarrior and choose one of the Targets to compile. For just playing,
you should choose "ScummVM Final", the Debug one is just for Developing.
Binaries will come up to the Website with the release 0.2.0, which will be hopefully soon.

NOTES:
Put the games in a folder in the same directory as ScummVM, which have the name of the Data Files, like this:

-- - ScummVM
  \- monkey2-
             \- monkey2.000
              - monkey2.001


Hope this will help you :)

CHANGES:
- Thanks to Florent Boudet, QuickTime MIDI has been added, although it currently only works with DOTT
- Added SFX Sound Support
- Now draws the Games Cursors

BUGS:
- On OS X it got some Problems with KeyDown-Events (seems to work now, most of the time)
- Bad behaviour of the Application towards other, will be fixed soon.
- Some minor bugs

If you find any bugs, just make a Bug Report on our SourceForge Page.

-- Mutle