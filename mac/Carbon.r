/*
 *  Permit this Carbon application to launch on OS X
 *
 *  © 1997-2000 Metrowerks Corp.
 *
 *  Questions and comments to:
 *       <mailto:support@metrowerks.com>
 *       <http://www.metrowerks.com/>
 */
 
#include "MacTypes.r"
#include "Dialogs.r"
#include "Balloons.r"
#include "Menus.r"
#include "Finder.r"
#include "Quickdraw.r"
#include "Icons.r"
#include "Processes.r"
#include "Controls.r"

/*----------------------------carb • Carbon on OS X launch information --------------------------*/
type 'carb' {
};


resource 'carb'(0) {
};

resource 'ALRT' (129) {
	{55, 39, 153, 407},
	128,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionParentWindowScreen
};

resource 'DITL'(128) {
	{
		{8, 74, 61, 356},
		StaticText {
			disabled,
			"^0"
		},
		
		{70, 299, 90, 357},
		Button {
			enabled,
			"OK"
		}
	}
};

resource 'MENU'(999) {
	999, 63, allEnabled, enabled, "Please Select a Game…",
	{
		"Maniac Mansion (C64)", noIcon, noKey, noMark, plain,
		"Zak McKracken and the Alien Mindbenders (C64)", noIcon, noKey, noMark, plain,
		"Maniac Mansion", noIcon, noKey, noMark, plain,
		"Zak McKracken and the Alien Mindbenders", noIcon, noKey, noMark, plain,
		"Indiana Jones and the Last Crusade", noIcon, noKey, noMark, plain,
		"Indiana Jones and the Last Crusade (256)", noIcon, noKey, noMark, plain,
		"Zak McKracken and the Alien Mindbenders (256)", noIcon, noKey, noMark, plain,
		"Loom", noIcon, noKey, noMark, plain,
		"Monkey Island 1 (EGA)", noIcon, noKey, noMark, plain,
		"Monkey Island 1 (256 color Floppy version)", noIcon, noKey, noMark, plain,
		"Loom (256 color CD version)", noIcon, noKey, noMark, plain,
		"Monkey Island 1", noIcon, noKey, noMark, plain,
		"Monkey Island 1 (alt)", noIcon, noKey, noMark, plain,
		"Monkey Island 2: LeChuck's revenge", noIcon, noKey, noMark, plain,
		"Indiana Jones 4 and the Fate of Atlantis", noIcon, noKey, noMark, plain,
		"Indiana Jones 4 and the Fate of Atlantis (Demo)", noIcon, noKey, noMark, plain,
		"Day Of The Tentacle", noIcon, noKey, noMark, plain,
		"Day Of The Tentacle (Demo)", noIcon, noKey, noMark, plain,
		"Sam & Max", noIcon, noKey, noMark, plain,
		"Sam & Max (Demo)", noIcon, noKey, noMark, plain,
		"Full Throttle", noIcon, noKey, noMark, plain,
		"The Dig", noIcon, noKey, noMark, plain,
		"The Curse of Monkey Island", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Simon the Sorcerer 1 (DOS)", noIcon, noKey, noMark, plain,
		"Simon the Sorcerer 1 (Windows)", noIcon, noKey, noMark, plain,
		"Simon the Sorcerer 2 (Windows)", noIcon, noKey, noMark, plain
	}		
};

resource 'MENU'(1000) {
	1000, 63, allEnabled, enabled, apple,
	{
		"About ScummVM…", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU'(1001) {
	1001, 63, allEnabled, enabled, "File",
	{
		"New Game", noIcon, "N", noMark, plain,
		"Open Game", noIcon, "O", noMark, plain,
		"Save Game", noIcon, "S", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Q", noIcon, "Q", noMark, plain
	}
};