/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/str.h"
#include "common/system.h"
#include "common/timer.h"

#include "private/grammar.h"
#include "private/private.h"
#include "private/tokens.h"

namespace Private {

static void fChgMode(ArgArray args) {
	// assert types
	assert(args.size() == 2 || args.size() == 3);
	assert(args[0].type == NUM);

	if (args.size() == 2)
		debugC(1, kPrivateDebugScript, "ChgMode(%d, %s)", args[0].u.val, args[1].u.sym->name->c_str());
	else if (args.size() == 3)
		debugC(1, kPrivateDebugScript, "ChgMode(%d, %s, %s)", args[0].u.val, args[1].u.sym->name->c_str(), args[2].u.sym->name->c_str());
	else
		assert(0);

	g_private->_mode = args[0].u.val;
	g_private->_nextSetting = args[1].u.sym->name->c_str();

	if (g_private->_mode == 0) {
		g_private->_origin = Common::Point(kOriginZero[0], kOriginZero[1]);
	} else if (g_private->_mode == 1) {
		g_private->_origin = Common::Point(kOriginOne[0], kOriginOne[1]);
	} else
		assert(0);

	if (args.size() == 3) {
		Symbol *location = g_private->maps.lookupLocation(args[2].u.sym->name);
		setSymbol(location, true);
	}

	// This is the only place where this should be used
	if (g_private->_noStopSounds) {
		g_private->_noStopSounds = false;
	} else {
		g_private->stopSound(true);
	}
}

static void fVSPicture(ArgArray args) {
	assert(args[0].type == STRING);
	debugC(1, kPrivateDebugScript, "VSPicture(%s)", args[0].u.str);
	g_private->_nextVS = args[0].u.str;
}

static void fDiaryLocList(ArgArray args) {
	int x1, y1, x2, y2;
	assert(args[0].type == NUM);
	assert(args[1].type == NUM);
	assert(args[2].type == NUM);
	assert(args[3].type == NUM);

	debugC(1, kPrivateDebugScript, "DiaryLocList(%d, %d, %d, %d)", args[0].u.val, args[1].u.val, args[2].u.val, args[3].u.val);

	x2 = args[0].u.val;
	y2 = args[1].u.val;

	x1 = args[2].u.val;
	y1 = args[3].u.val;

	Common::Rect rect(x1, y1, x2, y2);
	g_private->loadLocations(rect);
}

static void fDiaryGoLoc(ArgArray args) {
	debugC(1, kPrivateDebugScript, "WARNING: DiaryGoLoc not implemented");
}

static void fDiaryInvList(ArgArray args) {
	debugC(1, kPrivateDebugScript, "DiaryInvList(%d, ..)", args[0].u.val);

	const Common::Rect *r1 = args[1].u.rect;
	const Common::Rect *r2 = args[2].u.rect;

	g_private->loadInventory(args[0].u.val, *r1, *r2);
}

static void fgoto(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "goto(%s)", args[0].u.str);
	g_private->_nextSetting = args[0].u.str;
}

static void fSyncSound(ArgArray args) {
	assert(args[0].type == STRING);
	assert(args[1].type == NAME);
	debugC(1, kPrivateDebugScript, "SyncSound(%s, %s)", args[0].u.str, args[1].u.sym->name->c_str());
	g_private->_nextSetting = args[1].u.sym->name->c_str();
	Common::String s = args[0].u.str;

	if (s != "\"\"") {
		g_private->playSound(s, 1, true, false);
		while (g_private->isSoundActive())
			g_private->ignoreEvents();

		uint32 i = 100;
		while (i--) // one second extra
			g_private->ignoreEvents();
	}
}

static void fQuit(ArgArray args) {
	debugC(1, kPrivateDebugScript, "Quit()");
	g_private->quitGame();
}

static void fLoadGame(ArgArray args) {
	assert(args[0].type == STRING);
	assert(args[2].type == NAME);
	debugC(1, kPrivateDebugScript, "LoadGame(%s, %s)", args[0].u.str, args[2].u.sym->name->c_str());
	MaskInfo m;
	if (strcmp(args[0].u.str, "\"\"") == 0) // Not sure why the game tries to load an empty mask
		return;
	m.surf = g_private->loadMask(args[0].u.str, 0, 0, true);
	m.cursor = *args[2].u.sym->name;
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	if (g_private->_loadGameMask.surf)
		g_private->_loadGameMask.surf->free();
	delete g_private->_loadGameMask.surf;
	g_private->_loadGameMask = m;
	g_private->_masks.push_front(m);
}

static void fSaveGame(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "SaveGame(%s, %s)", args[0].u.str, args[1].u.sym->name->c_str());
	MaskInfo m;
	m.surf = g_private->loadMask(args[0].u.str, 0, 0, true);
	m.cursor = *args[1].u.sym->name;
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	if (g_private->_saveGameMask.surf)
		g_private->_saveGameMask.surf->free();
	delete g_private->_saveGameMask.surf;
	g_private->_saveGameMask = m;
	g_private->_masks.push_front(m);
}

static void fRestartGame(ArgArray args) {
	assert(args.size() == 0);
	g_private->restartGame();
}

static void fPoliceBust(ArgArray args) {
	// assert types
	assert(args.size() == 1 || args.size() == 2);
	g_private->_policeBustEnabled = args[0].u.val;
	//debug("Number of clicks %d", g_private->computePoliceIndex());

	if (g_private->_policeBustEnabled)
		g_private->startPoliceBust();

	if (args.size() == 2) {
		if (args[1].u.val == 2) {
			// Unclear what it means
		} else if (args[1].u.val == 3) {
			g_private->_nextSetting = g_private->getMainDesktopSetting();
			g_private->_mode = 0;
			g_private->_origin = Common::Point(kOriginZero[0], kOriginZero[1]);
		} else
			assert(0);
	}
	debugC(1, kPrivateDebugScript, "PoliceBust(%d, ..)", args[0].u.val);
	debugC(1, kPrivateDebugScript, "WARNING: PoliceBust partially implemented");
}

static void fBustMovie(ArgArray args) {
	// assert types
	assert(args.size() == 1);
	debugC(1, kPrivateDebugScript, "BustMovie(%s)", args[0].u.sym->name->c_str());
	uint policeIndex = g_private->maps.variables.getVal(g_private->getPoliceIndexVariable())->u.val;
	int videoIndex = policeIndex / 2 - 1;
	if (videoIndex < 0)
		videoIndex = 0;
	assert(videoIndex <= 5);
	Common::String pv =
		Common::String::format("po/animatio/spoc%02dxs.smk",
							   kPoliceBustVideos[videoIndex]);

	if (kPoliceBustVideos[videoIndex] == 2) {
		Common::String s("global/transiti/audio/spoc02VO.wav");
		g_private->playSound(s, 1, false, false);
	}

	g_private->_nextMovie = pv;
	g_private->_nextSetting = args[0].u.sym->name->c_str();
}

static void fDossierAdd(ArgArray args) {

	assert(args.size() == 2);
	Common::String s1 = args[0].u.str;
	Common::String s2 = args[1].u.str;
	DossierInfo m;
	m.page1 = s1;

	if (s2 != "\"\"") {
		m.page2 = s2;
	} else {
		m.page2 = "";
	}

	g_private->_dossiers.push_back(m);
}

static void fDossierBitmap(ArgArray args) {
	assert(args.size() == 2);
	int x = args[0].u.val;
	int y = args[1].u.val;
	assert(x == 40 && y == 30);
	g_private->loadDossier();
}

static void fDossierChgSheet(ArgArray args) {
	assert(args.size() == 4);
	debugC(1, kPrivateDebugScript, "DossierChgSheet(%s,%d,%d,%d)", args[0].u.str, args[1].u.val, args[2].u.val, args[3].u.val);
	Common::String s(args[0].u.str);
	MaskInfo m;

	int p = args[1].u.val;
	int x = args[2].u.val;
	int y = args[3].u.val;

	m.surf = g_private->loadMask(s, x, y, true);
	m.cursor = g_private->getExitCursor();
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	if (p == 0)
		g_private->_dossierPrevSheetMask = m;
	else if (p == 1)
		g_private->_dossierNextSheetMask = m;
	else
		error("Invalid sheet number in DossierChgSheet %d", p);

	g_private->_masks.push_front(m);
}

static void fDossierPrevSuspect(ArgArray args) {
	assert(args.size() == 3);
	Common::String s(args[0].u.str);
	MaskInfo m;

	int x = args[1].u.val;
	int y = args[2].u.val;

	m.surf = g_private->loadMask(s, x, y, true);
	m.cursor = g_private->getExitCursor();
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	g_private->_dossierPrevSuspectMask = m;
	g_private->_masks.push_front(m);
}

static void fDossierNextSuspect(ArgArray args) {
	assert(args.size() == 3);
	Common::String s(args[0].u.str);
	MaskInfo m;

	int x = args[1].u.val;
	int y = args[2].u.val;

	m.surf = g_private->loadMask(s, x, y, true);
	m.cursor = g_private->getExitCursor();
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	g_private->_dossierNextSuspectMask = m;
	g_private->_masks.push_front(m);
}

static void fNoStopSounds(ArgArray args) {
	assert(args.size() == 0);
	debugC(1, kPrivateDebugScript, "NoStopSounds()");
	g_private->_noStopSounds = true;
}

static void fLoseInventory(ArgArray args) {
	assert(args.size() == 0);
	debugC(1, kPrivateDebugScript, "LoveInventory()");
	g_private->inventory.clear();
}

bool inInventory(Common::String &bmp) {
	for (NameList::const_iterator it = g_private->inventory.begin(); it != g_private->inventory.end(); ++it) {
		if (*it == bmp)
			return true;
	}
	return false;
}

static void fInventory(ArgArray args) {
	// assert types
	Datum b1 = args[0];
	Datum v1 = args[1];
	Datum v2 = args[2];
	Datum e = args[3];
	Datum i = args[4];
	Datum c = args[5];
	Datum snd = args[8];

	assert(v1.type == STRING || v1.type == NAME);
	assert(b1.type == STRING);
	assert(e.type == NAME || e.type == NUM);
	assert(snd.type == STRING);
	assert(i.type == STRING);

	Common::String bmp(i.u.str);
	assert(g_private->isDemo() || bmp != "\"\"");

	if (v1.type == STRING)
		assert(strcmp(v1.u.str, "\"\"") == 0);

	debugC(1, kPrivateDebugScript, "Inventory(...)");
	Common::String mask(b1.u.str);
	if (mask != "\"\"") {
		MaskInfo m;
		m.surf = g_private->loadMask(mask, 0, 0, true);

		if (e.type == NUM) {
			assert(e.u.val == 0);
			m.nextSetting = "";
		} else
			m.nextSetting = e.u.sym->name->c_str();

		m.cursor = g_private->getInventoryCursor();
		m.point = Common::Point(0, 0);

		if (v1.type == NAME) {
			m.flag1 = g_private->maps.lookupVariable(v1.u.sym->name);
		} else
			m.flag1 = nullptr;

		if (v2.type == NAME) {
			m.flag2 = g_private->maps.lookupVariable(v2.u.sym->name);
		} else
			m.flag2 = nullptr;

		g_private->_masks.push_front(m);
		g_private->_toTake = true;
		Common::String sound(snd.u.str);

		if (sound != "\"\"") {
			g_private->playSound(sound, 1, false, false);
		} else {
			g_private->playSound(g_private->getTakeLeaveSound(), 1, false, false);
		}

		if (!inInventory(bmp))
			g_private->inventory.push_back(bmp);
	} else {
		if (v1.type == NAME) {
			v1.u.sym = g_private->maps.lookupVariable(v1.u.sym->name);
			if (strcmp(c.u.str, "\"REMOVE\"") == 0) {
				v1.u.sym->u.val = 0;
				if (inInventory(bmp))
					g_private->inventory.remove(bmp);
			} else {
				v1.u.sym->u.val = 1;
				if (!inInventory(bmp))
					g_private->inventory.push_back(bmp);
			}
		} else {
			if (!inInventory(bmp))
				g_private->inventory.push_back(bmp);
		}
		if (v2.type == NAME) {
			v2.u.sym = g_private->maps.lookupVariable(v2.u.sym->name);
			v2.u.sym->u.val = 1;
		}
	}
}

static void fSetFlag(ArgArray args) {
	assert(args.size() == 2);
	assert(args[0].type == NAME && args[1].type == NUM);
	debugC(1, kPrivateDebugScript, "SetFlag(%s, %d)", args[0].u.sym->name->c_str(), args[1].u.val);
	args[0].u.sym = g_private->maps.lookupVariable(args[0].u.sym->name);
	args[0].u.sym->u.val = args[1].u.val;
}

static void fExit(ArgArray args) {
	// assert types
	assert(args[2].type == RECT || args[2].type == NAME);
	debugC(1, kPrivateDebugScript, "Exit(%d %d %d)", args[0].type, args[1].type, args[2].type); //, args[0].u.str, args[1].u.sym->name->c_str(), "RECT");
	ExitInfo e;

	if (args[0].type == NUM && args[0].u.val == 0)
		e.nextSetting = "";
	else
		e.nextSetting = args[0].u.sym->name->c_str();

	if (args[1].type == NUM && args[1].u.val == 0)
		e.cursor = "";
	else
		e.cursor = *args[1].u.sym->name;

	if (args[2].type == NAME) {
		Symbol *rect = g_private->maps.lookupRect(args[2].u.sym->name);
		assert(rect->type == RECT);
		args[2].u.rect = rect->u.rect;
	}

	e.rect = *args[2].u.rect;
	g_private->_exits.push_front(e);
}

static void fSetModifiedFlag(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "SetModifiedFlag(%d)", args[0].u.val);
	g_private->_modified = args[0].u.val != 0;
}

static void fPaperShuffleSound(ArgArray args) {
	assert(args.size() == 0);
	debugC(1, kPrivateDebugScript, "PaperShuffleSound()");
	g_private->playSound(g_private->getPaperShuffleSound(), 1, false, false);
}

static void fSoundEffect(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "SoundEffect(%s)", args[0].u.str);
	Common::String s(args[0].u.str);
	if (s != "\"\"") {
		g_private->playSound(s, 1, false, false);
	} else {
		g_private->stopSound(true);
	}
}

static void fSound(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "Sound(%s)", args[0].u.str);
	if (args.size() == 4) {
		bool b1 = args[1].u.val != 0;
		bool b2 = args[2].u.val != 0;
		int c = args[3].u.val;

		if (!b1 && !b2 && c == 1) {
			g_private->stopSound(true);
		} else if (!b1 && !b2 && c == 2) {
			g_private->stopSound(false);
		} else
			assert(0);
	}

	Common::String s(args[0].u.str);
	if (s != "\"\"") {
		g_private->playSound(s, 1, false, false);
	} else {
		g_private->stopSound(true);
	}
}

static void fLoopedSound(ArgArray args) {
	// assert types
	assert(args.size() == 1);
	debugC(1, kPrivateDebugScript, "LoopedSound(%s)", args[0].u.str);
	Common::String s(args[0].u.str);

	if (s != "\"\"") {
		g_private->playSound(s, 0, true, true);
	} else {
		g_private->stopSound(true);
	}
}

static void fViewScreen(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "WARNING: ViewScreen not implemented!");
}

static void fTransition(ArgArray args) {
	assert(args[0].type == STRING);
	assert(args[1].type == NAME);
	debugC(1, kPrivateDebugScript, "Transition(%s, %s)", args[0].u.str, args[1].u.sym->name->c_str());
	g_private->_nextMovie = args[0].u.str;
	g_private->_nextSetting = args[1].u.sym->name->c_str();
}

static void fResume(ArgArray args) {
	assert(args[0].type == NUM);
	debugC(1, kPrivateDebugScript, "Resume(%d)", args[0].u.val); // this value is always 1
	g_private->resumeGame();
}

static void fMovie(ArgArray args) {
	// assert types
	assert(args[0].type == STRING);
	assert(args[1].type == NAME);
	debugC(1, kPrivateDebugScript, "Movie(%s, %s)", args[0].u.str, args[1].u.sym->name->c_str());
	Common::String movie = args[0].u.str;
	Common::String nextSetting = *args[1].u.sym->name;

	if (!g_private->_playedMovies.contains(movie) && movie != "\"\"") {
		g_private->_nextMovie = movie;
		g_private->_playedMovies.setVal(movie, true);
		g_private->_nextSetting = nextSetting;
	} else if (movie == "\"\"") {
		g_private->_repeatedMovieExit = nextSetting;
		debugC(1, kPrivateDebugScript, "repeated movie exit is %s", nextSetting.c_str());
	} else {
		debugC(1, kPrivateDebugScript, "movie %s already played", movie.c_str());
		g_private->_nextSetting = g_private->_repeatedMovieExit;
	}
}

static void fCRect(ArgArray args) {
	// assert types
	debugC(1, kPrivateDebugScript, "CRect(%d, %d, %d, %d)", args[0].u.val, args[1].u.val, args[2].u.val, args[3].u.val);
	int x1, y1, x2, y2;
	x1 = args[0].u.val;
	y1 = args[1].u.val;
	x2 = args[2].u.val;
	y2 = args[3].u.val;

	Datum d = Datum();
	Common::Rect *rect = new Common::Rect(x1, y1, x2, y2);
	d.type = RECT;
	d.u.rect = rect;
	Gen::push(d);
}

static void fBitmap(ArgArray args) {
	assert(args.size() == 1 || args.size() == 3);

	int x = 0;
	int y = 0;

	const char *f = args[0].u.str;
	if (args.size() == 3) {
		x = args[1].u.val;
		y = args[2].u.val;
	}

	debugC(1, kPrivateDebugScript, "Bitmap(%s, %d, %d)", f, x, y);
	Common::String s(args[0].u.str);
	g_private->loadImage(s, x, y);
}

static void _fMask(ArgArray args, bool drawn) {
	assert(args.size() == 3 || args.size() == 5);

	int x = 0;
	int y = 0;
	const char *f = args[0].u.str;
	const char *e = args[1].u.sym->name->c_str();
	Common::String *c = args[2].u.sym->name;

	if (args.size() == 5) {
		x = args[3].u.val;
		y = args[4].u.val;
	}

	debugC(1, kPrivateDebugScript, "Mask(%s, %s, %s, %d, %d)", f, e, c->c_str(), x, y);
	const Common::String s(f);

	MaskInfo m;
	m.surf = g_private->loadMask(s, x, y, drawn);
	m.nextSetting = e;
	m.cursor = *c;
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	m.point = Common::Point(x, y);
	g_private->_masks.push_front(m);
}

static void fMask(ArgArray args) {
	_fMask(args, false);
}
static void fMaskDrawn(ArgArray args) {
	_fMask(args, true);
}

static void fAddSound(Common::String sound, const char *t, Symbol *flag = nullptr, int val = 0) {
	if (sound == "\"\"")
		return;

	if (strcmp(t, "AMRadioClip") == 0)
		g_private->_AMRadio.push_back(sound);
	else if (strcmp(t, "PoliceClip") == 0)
		g_private->_policeRadio.push_back(sound);
	else if (strcmp(t, "PhoneClip") == 0) {
		// This condition will avoid adding the same phone call twice,
		// it is unclear why this could be useful, but it looks like a bug
		// in the original scripts
		if (g_private->_playedPhoneClips.contains(sound))
			return;

		g_private->_playedPhoneClips.setVal(sound, true);
		PhoneInfo p;
		p.sound = sound;
		p.flag = flag;
		p.val = val;
		g_private->_phone.push_back(p);
	} else
		error("error: invalid sound type %s", t);
}

static void fAMRadioClip(ArgArray args) {
	assert(args.size() <= 4);
	fAddSound(args[0].u.str, "AMRadioClip");
}

static void fPoliceClip(ArgArray args) {
	assert(args.size() <= 4 || args.size() == 6);
	fAddSound(args[0].u.str, "PoliceClip");
	// In the original, the variable is updated when the clip is played, but here we just update
	// the variable when the clip is added to play. The effect for the player, is mostly the same.
	if (args.size() == 6) {
		assert(args[4].type == NAME);
		assert(args[5].type == NUM);
		Symbol *flag = g_private->maps.lookupVariable(args[4].u.sym->name);
		setSymbol(flag, args[5].u.val);
	}
}

static void fPhoneClip(ArgArray args) {
	if (args.size() == 2) {
		debugC(1, kPrivateDebugScript, "Unimplemented PhoneClip special case");
		return;
	}
	int i = args[2].u.val;
	int j = args[3].u.val;
	Symbol *flag = g_private->maps.lookupVariable(args[4].u.sym->name);

	if (i == j)
		fAddSound(args[0].u.str, "PhoneClip", flag, args[5].u.val);
	else {
		assert(i < j);
		Common::String sound = g_private->getRandomPhoneClip(args[0].u.str, i, j);
		fAddSound(sound, "PhoneClip", flag, args[5].u.val);
	}
}

static void fSoundArea(ArgArray args) {
	// assert types
	//char *n;
	Common::String n;
	if (args[1].type == NAME)
		n = *(args[1].u.sym->name);
	else if (args[1].type == STRING) {
		n = Common::String(args[1].u.str);
		Common::replace(n, "\"", "");
		Common::replace(n, "\"", "");
	} else
		error("Invalid input for SoundArea");

	debugC(1, kPrivateDebugScript, "SoundArea(%s, %s, ..)", args[0].u.str, n.c_str());
	Common::String s = args[0].u.str;
	MaskInfo m;
	if (n == "kAMRadio") {
		m.surf = g_private->loadMask(s, 0, 0, true);
		m.cursor = *args[2].u.sym->name;
		m.nextSetting = "";
		m.flag1 = nullptr;
		m.flag2 = nullptr;
		if (g_private->_AMRadioArea.surf)
			g_private->_AMRadioArea.surf->free();
		delete g_private->_AMRadioArea.surf;
		g_private->_AMRadioArea = m;
		g_private->_masks.push_front(m);
	} else if (n == "kPoliceRadio") {
		m.surf = g_private->loadMask(s, 0, 0, true);
		m.cursor = *args[2].u.sym->name;
		m.nextSetting = "";
		m.flag1 = nullptr;
		m.flag2 = nullptr;
		if (g_private->_policeRadioArea.surf)
			g_private->_policeRadioArea.surf->free();
		delete g_private->_policeRadioArea.surf;
		g_private->_policeRadioArea = m;
		g_private->_masks.push_front(m);
	} else if (n == "kPhone") {
		m.surf = g_private->loadMask(s, 0, 0, true);
		m.cursor = *args[2].u.sym->name;
		m.nextSetting = "";
		m.flag1 = nullptr;
		m.flag2 = nullptr;
		if (g_private->_phoneArea.surf)
			g_private->_phoneArea.surf->free();
		delete g_private->_phoneArea.surf;
		g_private->_phoneArea = m;
		g_private->_masks.push_front(m);
	} else
		error("Invalid type for SoundArea");
}

static void fSafeDigit(ArgArray args) {
	assert(args[0].type == NUM);
	assert(args[1].type == RECT);
	debugC(1, kPrivateDebugScript, "SafeDigit(%d, ..)", args[0].u.val);
	g_private->addSafeDigit(args[0].u.val, args[1].u.rect);
}

static void fAskSave(ArgArray args) {
	// This is not needed, since scummvm will take care of this
	debugC(1, kPrivateDebugScript, "WARNING: AskSave is partially implemented");
	g_private->_nextSetting = *args[0].u.sym->name;
}

static void fTimer(ArgArray args) {
	assert(args.size() == 2 || args.size() == 3);

	if (args.size() == 3)
		debugC(1, kPrivateDebugScript, "Timer(%d, %s, %s)", args[0].u.val, args[1].u.sym->name->c_str(), args[2].u.sym->name->c_str());
	else
		debugC(1, kPrivateDebugScript, "Timer(%d, %s)", args[0].u.val, args[1].u.str);

	int32 delay = 1000000 * args[0].u.val;
	// This pointer is necessary since installTimer needs one
	Common::String *s = new Common::String(args[1].u.sym->name->c_str());
	if (delay > 0) {
		if (!g_private->installTimer(delay, s))
			error("Timer installation failed!");
	} else if (delay == 0) {
		g_private->_nextSetting = *s;
		// No need to keep the pointer alive
		delete s;
	} else {
		assert(0);
	}
}

const FuncTable funcTable[] = {

	// Control flow
	{fChgMode, "ChgMode"},
	{fResume, "Resume"},
	{fgoto, "goto"},
	{fTimer, "Timer"},

	// Variables
	{fSetFlag, "SetFlag"},
	{fSetModifiedFlag, "SetModifiedFlag"},

	// Sounds
	{fSound, "Sound"},
	{fSoundEffect, "SoundEffect"},
	{fLoopedSound, "LoopedSound"},
	{fNoStopSounds, "NoStopSounds"},
	{fSyncSound, "SyncSound"},
	{fAMRadioClip, "AMRadioClip"},
	{fPoliceClip, "PoliceClip"},
	{fPhoneClip, "PhoneClip"},
	{fSoundArea, "SoundArea"},
	{fPaperShuffleSound, "PaperShuffleSound"},

	// Images
	{fBitmap, "Bitmap"},
	{fMask, "Mask"},
	{fMaskDrawn, "MaskDrawn"},
	{fVSPicture, "VSPicture"},
	{fViewScreen, "ViewScreen"},
	{fExit, "Exit"},

	// Video
	{fTransition, "Transition"},
	{fMovie, "Movie"},

	// Diary
	{fDiaryLocList, "DiaryLocList"},
	{fDiaryInvList, "DiaryInvList"},
	{fDiaryGoLoc, "DiaryGoLoc"},

	// Main menu
	{fQuit, "Quit"},
	{fLoadGame, "LoadGame"},
	{fSaveGame, "SaveGame"},
	{fAskSave, "AskSave"},
	{fRestartGame, "RestartGame"},

	// Dossiers
	{fDossierAdd, "DossierAdd"},
	{fDossierChgSheet, "DossierChgSheet"},
	{fDossierBitmap, "DossierBitmap"},
	{fDossierPrevSuspect, "DossierPrevSuspect"},
	{fDossierNextSuspect, "DossierNextSuspect"},

	// Inventory
	{fLoseInventory, "LoseInventory"},
	{fInventory, "Inventory"},

	// PoliceBust
	{fPoliceBust, "PoliceBust"},
	{fBustMovie, "BustMovie"},

	// Others
	{fSafeDigit, "SafeDigit"},
	{fCRect, "CRect"},

	{nullptr, nullptr}};

void call(const char *name, const ArgArray &args) {
	Common::String n(name);
	if (!g_private->_functions.contains(n)) {
		error("I don't know how to execute %s", name);
	}

	void (*func)(ArgArray) = (void (*)(ArgArray))g_private->_functions.getVal(n);
	func(args);
}

} // End of namespace Private
