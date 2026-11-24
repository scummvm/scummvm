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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "dune/condit.h"
#include "dune/clock.h"
#include "dune/combat.h"
#include "dune/dialogue.h"
#include "dune/dune.h"
#include "dune/actor_pool.h"
#include "dune/event_scripts.h"
#include "dune/worm_animation.h"
#include "dune/flight_navigation.h"
#include "dune/map_animation.h"
#include "dune/map_mode.h"
#include "dune/hsq.h"
#include "dune/music.h"
#include "dune/movie_stream.h"
#include "dune/resources.h"
#include "dune/room.h"
#include "dune/sprite_sheet.h"
#include "dune/video.h"

namespace Dune {

// 79FA/7A15 put the route card at (92,30) for an upper contact or
// (92,14) for a lower one. 543E insets its 160x89 map by (5,7).
enum {
	kRouteMapLeft = 97,
	kRouteMapWide = 160, kRouteMapHigh = 89,
	kMapScratchOrigin = 0x4c60, kMapScratchBytes = 0x5960
};

static PanelElement readPanelElement(const byte *row) {
	return {
		READ_LE_INT16(row), READ_LE_INT16(row + 2),
		READ_LE_INT16(row + 4), READ_LE_INT16(row + 6),
		READ_LE_INT16(row + 8), READ_LE_UINT16(row + 10), READ_LE_UINT16(row + 12)
	};
}

// Where COMMAND1.HSQ keeps the people's names: the hundred and nineteenth is
// Duke Leto, who is the first of them, and the rest follow in the order the
// dialogue's own groups take them.
enum { kFirstPerson = 119 };

/**
 * 0x9FB2 passes a marked record over only when it has no 0x40 and one of its
 * bits is selected by DS:0x47C2.  Ordinary conversation selects 0x80, making
 * newly spoken records one-shot; replies to requests select 0x20 instead.
 * Some records already carry 0x80 in DIALOGUE.HSQ (the normal Harg acceptance
 * is one), so treating every high bit as an unconditional "already spoken"
 * mark skips valid lines.
 */
bool DuneEngine::dialogueRecordAvailable(const Dialogue::Record &record, byte mask) {
	return !((record.flags & 0x80) && !(record.flags & 0x40) &&
			 (record.flags & mask));
}

/** Whether a named person's dormant palace record is active yet. */
static bool namedPersonAvailable(byte who, byte storyStage) {
	// Thufir's dormant record points at the communication room, but its
	// standing-position word is invalid until stage 32 makes him available. Keep
	// him out of the earlier palace lists; Duke Leto sends Paul to him after
	// Gurney is found, and from then on he must be selectable in room eight.
	return who != 2 || storyStage >= 32;
}

// COMMAND1.HSQ's own words for a list that has nothing in it and for the row
// that takes one down, and the part number that stands for that row.
enum { kNoTroops = 101, kCancel = 162, kGoBack = 0xffff };

/*
 * What the fixed rows of the room's own list stand for, kept apart from the
 * people below them: whoever is here is named by their own number, and a row
 * that is not one of them carries one of these instead.  The list is not
 * always the same length - the row that takes an ornithopter is only there
 * where one can be taken - so a row cannot be told by where it sits.
 */
enum {
	kRoomMap = 0xff00, kRoomOrnithopter = 0xff01, kRoomMixer = 0xff02,
	kRoomMirror = 0xff03, kRoomMirrorAway = 0xff04, kRoomPlanDone = 0xff05,
	kRoomMassiveAttack = 0xff06, kRoomFightDay = 0xff07, kRoomBattleWorm = 0xff08,
	// Person fifteen is a representative for as many as eight local troop
	// chiefs.  Keep the selected representative in the row itself; otherwise
	// every room-menu choice opened the first chief and gave a standing Fremen
	// somebody else's face and troop.
	kRoomChiefBase = 0xfe00
};

/* And the same for the list the maps carry. */
enum { kMapExit = 0xff10, kMapOrders = 0xff11, kMapDensity = 0xff12,
	   kMapOrnithopter = 0xff13, kMapProspectors = 0xff14 };

enum { kAddRouteDestination = 90 };

/* And for the two rows a place pressed on the maps puts up. */
enum { kPlaceFly = 0xff30, kPlaceCancel = 0xff31, kPlaceWorm = 0xff32 };

/* And for the two the open desert carries, on foot and being carried. */
enum { kDesertMap = 0xff20, kDesertWorm = 0xff21, kDesertEvening = 0xff22,
	   kDesertMorning = 0xff23, kDesertMixer = 0xff24, kDesertSkip = 0xff25,
	   kDesertBack = 0xff26, kDesertNearest = 0xff27, kDesertChange = 0xff28 };

/* A carried report interrupts discovery or entry into hostile territory. */
enum { kDiscoveryGo = 0xff40, kDiscoveryWhat = 0xff41, kDiscoveryChange = 0xff42 };

void DuneEngine::setCursor(Cursor shape) {
	const byte *cursorData = _staticState.begin() + static_cast<uint16>(shape);
	// The mouse driver keeps its pointers as a pair of 16x16 masks: the first
	// clears the screen to black where its bits are low, the second writes
	// white where its bits are high, and where neither speaks the screen is
	// left alone.
	byte cursor[16 * 16];
	byte *dst = cursor;

	for (int y = 0; y != 16; y++) {
		uint16 blackMask = READ_LE_UINT16(cursorData + 4 + 2 * y);
		uint16 whiteMask = READ_LE_UINT16(cursorData + 4 + 32 + 2 * y);

		for (int x = 0; x != 16; x++) {
			if (whiteMask & 0x8000)
				*dst++ = 0x0f;
			else if (!(blackMask & 0x8000))
				*dst++ = 0x00;
			else
				*dst++ = kCursorTransparent;

			blackMask <<= 1;
			whiteMask <<= 1;
		}
	}

	const Common::Point hotspot = cursorHotspot(cursorData);
	CursorMan.replaceCursor(cursor, 16, 16, hotspot.x, hotspot.y, kCursorTransparent);
}

/**
 * Writes one of the three faces of the right hand box over the six entries of
 * the panel's table it takes up, which is what 0xD72D does before it draws
 * them.
 */
void DuneEngine::setCompass(CompassKind kind) {
	const uint16 templates[] = { 0x1c76, 0x1cca, 0x1d72, 0x1dc6 };
	const byte *table = _staticState.begin() + templates[kind];
	for (int i = 0; i != kCompassElements; i++)
		_panel[kFirstCompassElement + i] = readPanelElement(table + 14 * i);
}

/**
 * Which of the four arrows the panel shows, and what the box behind them is.
 * 0x2FFB reads the room's own record: a way out that leads nowhere - a nought,
 * or one of the numbers between 0x80 and 0xFA, which are the doors the story
 * has yet to open - has its entry marked 0x20, and 0xD218 passes over an entry
 * marked that way rather than drawing it.
 *
 * The box itself is the thirty-fourth sprite in the first room of a place and
 * the thirty-third everywhere else, and the mark in the middle of it stands
 * for the place the player calls home, so it is shown only there.
 */
void DuneEngine::updateCompass() {
	enum { kBox = 33, kFirstRoomBox = 34, kNoWayOutBox = 35, kHidden = 0x20, kShown = 0x80 };

	setCompass(kCompassRoom);
	if (_state[0x2b]) {
		// 2FFB tests the retained battle layout, not the current troop mix.
		// Massive Attack resolves combat before its effects finish; room
		// navigation stays hidden until the caller releases that layout.
		const byte *table = _staticState.begin() + 0x1d1e;
		for (int i = 0; i < kCompassElements; i++)
			_panel[kFirstCompassElement + i] = readPanelElement(table + 14 * i);
		return;
	}

	const byte kindOfPlace = _state.size() > 8 ? _state[8] : 0;
	const byte room = _state.size() > 0xb ? _state[0xb] : 0;
	const byte *record = roomRecord(kindOfPlace, room);

	if (!record) {
		// Nowhere with rooms of its own: 0x3073 puts up the box that has no
		// ways out drawn on it and shows all four arrows against it.
		_panel[kFirstCompassElement].sprite = kNoWayOutBox;
		return;
	}

	// The mark in the middle belongs to the first of the places, which is the
	// one the player begins at and comes back to.
	const bool atHome = _state.size() > 0x114f
						&& READ_LE_UINT16(_state.begin() + 0x114e) == 0x100;
	int16 here = atHome ? kShown : kHidden;

	_panel[kFirstCompassElement].sprite = kBox;
	if (room == 1) {
		_panel[kFirstCompassElement].sprite = kFirstRoomBox;
		here = kHidden;
	}

	_panel[kFirstCompassElement + 5].flags = here;

	for (int way = 0; way != 4; way++) {
		const byte leads = record[1 + way];

		// A way out is shut when it names no room, and when it names one of
		// the rooms the story has still to open, which the original tells
		// apart by comparing the byte as a signed number against -5.
		const bool open = leads != 0 && (int8)leads >= -5;

		_panel[kFirstCompassElement + 1 + way].flags = open ? kShown : kHidden;
	}
}

/**
 * The panel's table is the game's own and is written over as it runs, so it
 * starts from the copy the data holds and the right hand box is set to the
 * four ways out of a room.  This used to be done as the game itself started,
 * which left the intro drawing the panel out of whatever the memory happened
 * to hold: three of its entries came out with a sprite of their own and
 * ICONES.HSQ was asked for sprites it does not have, differently each run.
 */
void DuneEngine::resetPanel() {
	for (int i = 0; i != ARRAYSIZE(_panel); i++)
		_panel[i] = readPanelElement(_staticState.begin() + 0x1ae6 + 14 * i);

	setCompass(kCompassRoom);
}

void DuneEngine::drawPanel() {
	openSpriteSheet("ICONES.HSQ");

	// The two boxes in the panel's foot hold whoever is walking with Paul:
	// 0xD775 puts the sprite of each of them, which is the sixty-fifth and
	// their own number, into the twenty-second and twenty-third entries, and
	// leaves the empty box of the sixty-fourth where nobody is.
	// 0xD778 widens the byte with its sign before it adds 0x41, so the 0xFF
	// that stands for nobody comes out as the empty box of the sixty-fourth.
	enum { kFirstPortrait = 0x41, kFollowing = 0x1152 };

	for (int i = 0; i != 2; i++) {
		const int8 who = _state.size() > kFollowing + i ? (int8)_state[kFollowing + i] : -1;

		_panel[21 + i].sprite = (uint16)(kFirstPortrait + who);
	}

	// D7B2 changes the first four panel sprites for the globe: the world
	// emblem goes in entry zero, the hollow in entry one, and the room's
	// ordinary side button remains in entry two. The globe's turning arrows
	// are then drawn over the left end by the DS:1DC6 template.
	enum { kHollow = 6, kWorld = 13, kFirstBox = 21, kBoxes = 2 };

	// 0xD200 draws an entry that keeps a sprite and passes over one that does
	// not.  The box in the corner is drawn after the rest, out of the six
	// entries the compass template has just been laid into.
	for (int i = 0; i != ARRAYSIZE(_panel); i++) {
		if (i >= kFirstCompassElement && i < kFirstCompassElement + kCompassElements)
			continue;

		if (_onWorld && !_inCabin && i >= kFirstBox && i < kFirstBox + kBoxes)
			continue;
		if (_menuKind == kMenuBook && (i == 0 || i == 3 || (i >= kFirstBox && i < kFirstBox + kBoxes)))
			continue;

		uint16 sprite = _panel[i].sprite;
		if (_menuKind == kMenuBook && i == 1)
			sprite = 9;
		else if (_onGlobe && i == 0)
			sprite = kWorld;
		else if (_onGlobe && i == 1)
			sprite = kHollow;
		else if (_onWorld && !_inCabin && sprite == 0)
			sprite = kHollow;
		else if (i >= kFirstBox && i < kFirstBox + kBoxes)
			sprite = 64; // D763 paints BOTH blank boxes before D775's portraits.
		if (sprite != 0xffff && !(_panel[i].flags & 0x20))
			drawSprite(sprite, _panel[i].x, _panel[i].y);
	}
	if (!(_onWorld && !_inCabin) && _menuKind != kMenuBook) {
		for (int i = kFirstBox; i < kFirstBox + kBoxes; i++) {
			const PanelElement &box = _panel[i];
			if (box.sprite != 0xffff && !(box.flags & 0x20))
				drawSprite(box.sprite, box.x, box.y);
		}
	}

	if (_retainRoomPanelFace) {
		for (uint y = 15; y < 23; y++)
			memcpy(_roomPanelFaceBacking + y * 20,
					_framebuffer1 + (137 + y) * kScreenWidth + 150, 20);
		_roomPanelFaceValid = true;
		_retainRoomPanelFace = false;
	}
	if (!_state[0xe8]) {
		for (uint y = 0; y < 23; y++)
			memcpy(_openingPanelFaceBacking + y * 20,
					_framebuffer1 + (137 + y) * kScreenWidth + 150, 20);
	}
	drawFace();

	// The world sits in the corner the first entry of the table keeps, which is
	// 22 across and 161 down.
	if (_onWorld && !_inCabin && !_onGlobe)
		drawSprite(kWorld, _panel[0].x, _panel[0].y);
	else if (!_onGlobe && _menuKind != kMenuBook)
		drawTimeOfDay();

	drawCompass();
}

/** D7B7: blink a newly assigned companion without changing the party. */
void DuneEngine::updateCompanionPanelBlink(uint16 ticks) {
	if (_state.size() <= 0xdcf1)
		return;
	const byte phase = (byte)(ticks >> 6);
	if (_state[0xdcf1] == phase)
		return;
	_state[0xdcf1] = phase;
	if (!READ_LE_UINT16(_state.begin() + 0x2222))
		return;
	byte visible[2] = { _state[0x1152], _state[0x1153] };
	for (uint i = 0; i < 2; i++) {
		byte &remaining = _state[0x2222 + i];
		if (remaining && (--remaining & 1))
			visible[i] = 0xff;
	}
	// Closed shutters suppress painting, while the countdown continues.
	if (READ_LE_UINT16(_state.begin() + 0x1afe))
		return;
	drawCompanionPanel(visible);
}

/** D763 erases both boxes before drawing their transparent portraits. */
void DuneEngine::drawCompanionPanel(const byte *visible) {
	Resource icons = _archive.openMember("ICONES.HSQ");
	if (!icons)
		return;
	SpriteSheet portraits(this, icons, false);
	byte *savedPage = _graphics.frameBuffer();
	_graphics.setFrameBuffer(_framebuffer1);
	for (uint i = 0; i < 2; i++) {
		const PanelElement &box = _panel[21 + i];
		if (!(box.flags & 0x20))
			portraits.draw(64, box.x, box.y, 0, 0);
	}
	for (uint i = 0; i < 2; i++) {
		PanelElement &box = _panel[21 + i];
		box.sprite = 0x41 + (int8)visible[i];
		if (!(box.flags & 0x20))
			portraits.draw(box.sprite, box.x, box.y, 0, 0);
	}
	_graphics.setFrameBuffer(savedPage);
	_system->copyRectToScreen(_framebuffer1 + 182 * kScreenWidth + 35,
			kScreenWidth, 35, 182, 43, 15);
}

/**
 * The carved face over the middle of the panel, which the top edge of the box
 * below it leaves a notch for.  0x1797 draws the bar of ICONES.HSQ's
 * fifteenth sprite and then one of the eleven faces from the sixteenth on,
 * picked by the byte at DS:0xE8 - the raising/lowering phase, not Paul's
 * charisma. Neither used to be drawn, which left a hole in the
 * panel where the notch is.
 */
void DuneEngine::drawFace() {
	enum { kBar = 15, kFirstFace = 16, kFaces = 11 };

	const byte phase = _state.size() > 0xe8 ? MIN<byte>(_state[0xe8], kFaces - 1) : 0;

	// 1797/C13E restores the caller's resident bank. openSpriteSheet deletes
	// that sheet, so restoring its old pointer would leave a dangling pointer
	// (or an allocator-reused ICONES object) instead of the caller's graphics.
	Resource icons = _archive.openMember("ICONES.HSQ");
	if (!icons)
		return;
	SpriteSheet panel(this, icons, false);
	panel.draw(kBar, 126, 148, 0, 0);
	panel.draw(kFirstFace + phase, 150, 137, 0, 0);
}

/** 008F/1860/17E6: raise the lowered panel face when a new game opens. */
void DuneEngine::raiseOpeningPanelFace() {
	raisePanelFace(true);
}

/** 17BE: restore the uncovered 20x23 page, then present only that rectangle. */
void DuneEngine::paintPanelFacePhase() {
	for (uint y = 0; y < 23; y++)
		memcpy(_framebuffer1 + (137 + y) * kScreenWidth + 150,
				_openingPanelFaceBacking + y * 20, 20);
	drawFace();
	_system->copyRectToScreen(_framebuffer1 + 137 * kScreenWidth + 150,
			kScreenWidth, 150, 137, 20, 23);
	_system->updateScreen();
	if (!ConfMan.getBool("dune_nodelay")) {
		const uint32 began = _system->getMillis();
		while (!shouldQuit() && _system->getMillis() - began < 40) {
			// E387 enables interrupts, not E3A0's input/actor/music
			// handoff callbacks. The driver's independent fade still runs.
			updateMusicFade();
			_system->delayMillis(5);
		}
	}
}

/** 17E6: opening and conversation return share the painter, not clock credit. */
void DuneEngine::raisePanelFace(bool opening) {
	if (_state[0x11c9])
		return;
	// 17BE restores DS:1E6E from the retained room/panel before each phase.
	// Keep that backing separate: transparent face pixels must not retain the
	// previous phase. Only this rectangle is presented by C4F0/C6F0.
	uint phases = 0;
	while (_state[0xe8] < 10 && !shouldQuit()) {
		_state[0xe8]++;
		paintPanelFacePhase();
		phases++;
	}
	if (!opening && phases) {
		if (ConfMan.getBool("dune_nodelay"))
			advanceClockElapsed(phases * 40, false, false);
		else
			tick(false);
	}
}

/** 1803 guards local paragraphs; boarding calls the unguarded181E painter. */
void DuneEngine::lowerPanelFace(bool local, bool automatic) {
	if ((local && _state[0x28e7]) || !_state[0xe8])
		return;
	const byte phase = _state[0xe8];
	// Automatic 35AD/36D3 replies and 28C5 communications bypass93AA's
	// contact portrait/1834 capture. 9FD8 lowers over the room retained by
	// 2E23 BEFORE 9FEF enlarges the new caller. Only DS:1E76's20x10 area
	// comes from that capture; the remainder belongs to the panel painter.
	const bool retainRoom = local && !_inDesert && _roomPanelFaceValid &&
		(automatic || _localRoomRequestPending || _state[0x47a9]);
	byte retainedRoom[20 * 10];
	if (retainRoom)
		memcpy(retainedRoom, _roomPanelFaceBacking, sizeof(retainedRoom));
	// Explicit93AA contact and boarding retain the current portrait instead.
	// Rebuild without the face or old caption to obtain the uncovered panel;
	// automatic callers keep the room's captured top ten rows separately.
	const Common::String speech = _speech;
	_speech.clear();
	_state[0xe8] = 0;
	if (_inDesert)
		drawDesertDialogue(false);
	else
		drawScene(false);
	if (retainRoom)
		memcpy(_openingPanelFaceBacking, retainedRoom, sizeof(retainedRoom));
	_speech = speech;
	_state[0xe8] = phase;
	lowerRetainedPanelFace();
}

/** 181E: lower the retained page without a scene or palette rebuild. */
void DuneEngine::lowerRetainedPanelFace() {
	uint phases = 0;
	while (_state[0xe8] && !shouldQuit()) {
		_state[0xe8]--;
		paintPanelFacePhase();
		phases++;
	}
	if (phases) {
		if (ConfMan.getBool("dune_nodelay"))
			advanceClockElapsed(phases * 40, false, false);
		else
			tick(false);
	}
}

/**
 * The hour and the day, in the panel's left hand corner.  0x1A34 takes the low
 * nibble of the hour at DS:0x2 into the table at DS:0x1E7E - eight bytes to an
 * hour, where the sun stands and where the moon does, and nowhere at all when
 * one of them is not up - and writes the day beside them, which is the hour
 * and three shifted down by four.
 */
void DuneEngine::drawTimeOfDay() {
	openSpriteSheet("ICONES.HSQ");
	drawTimeOfDay(*currentSpriteSheet);
}

void DuneEngine::drawTimeOfDay(SpriteSheet &icons) {

	enum { kSun = 74, kMoon = 75 };

	const uint16 now = hour();
	const byte *sky = _staticState.begin() + 0x1e7e + 8 * (now & 0xf);

	// 1A9B uses the clipped VGA painter with DS:1EFE, not C22F's plain
	// sprite painter. In particular, the sun must not cover the day below.
	const Common::Rect skyClip(6, 181, 30, 190);
	if (const int16 sunX = READ_LE_INT16(sky))
		icons.drawMapSprite(kSun, sunX, READ_LE_INT16(sky + 2), skyClip);
	if (const int16 moonX = READ_LE_INT16(sky + 4))
		icons.drawMapSprite(kMoon, moonX, READ_LE_INT16(sky + 6), skyClip);

	// The day is written centred: 0x1A77 starts at eleven across and steps two
	// to the left for every figure the number is short of three, and 0xE290
	// always writes three characters, putting a space where a leading nought
	// would be.  A space is four wide, so a single figure lands eight to the
	// right of where it starts.  Writing only the figures themselves left the
	// day hard against the panel's left hand edge.
	const uint day = ((uint16)(now + 3) >> 4) % 365 + 1;
	const Common::String written = Common::String::format("%3u", day);

	int x = 11;
	if (day < 100)
		x -= 2;
	if (day < 10)
		x -= 2;

	// 0x1A5F picks the small face for the day as well.
	_font.setFace(Font::kFaceAlternate);
	// 1A62 installs FA/F1 ink/background: overwrite the previous digits,
	// including leading spaces, rather than leaving old day-number strokes.
	_font.drawString(_framebuffer1, x, 190, written, kMenuColour, 0xf1);
}

/** 1A0F updates the visible clock in the open left panel box. */
void DuneEngine::refreshPanelClock() {
	if ((_onWorld && !_inCabin) || _onGlobe || _menuKind == kMenuBook)
		return;

	// DS:1F06 limits the transfer to the sky and day counter. In particular,
	// the clock must leave the scene and any windows above the panel intact.
	Resource resource = _archive.openMember("ICONES.HSQ");
	if (!resource)
		return;
	SpriteSheet icons(this, resource, false);
	const Common::Rect clockRect(6, 181, 30, 198);
	_graphics.setFrameBuffer(_framebuffer1);
	icons.drawMapSprite(0, _panel[1].x, _panel[1].y, clockRect);
	drawTimeOfDay(icons);
	_system->copyRectToScreen(_framebuffer1 + clockRect.top * kScreenWidth + clockRect.left,
			kScreenWidth, clockRect.left, clockRect.top, clockRect.width(), clockRect.height());
	_system->updateScreen();
}

/**
 * The panel's six active control entries. On the globe the four arrows are
 * at the left, while the plain button is at the right. For a room the entries
 * are the box itself, the four ways out and the mark for where Paul stands:
 * ICONES.HSQ keeps the box at its
 * thirty-third sprite and the four arrows before it, north, east, south and
 * west in turn.  Only the box and the mark used to be drawn, which left the
 * arrows as the dim ones printed on the box rather than the lit ones the game
 * lays over them.
 */
void DuneEngine::drawCompass() {
	// While the panel's right hand box shows one of its first three pictures
	// the corner is filled flat before the compass goes over it, which is what
	// 0xD741 does with the rectangle at DS:0x2458 and the colour 0xF0.  It is
	// what stands behind the arrows; without it they sat on nothing.
	if ((uint16)(_panel[2].sprite - 3) < 3)
		_graphics.fillRect(254, 162, 296, 193, 0xf0);

	openSpriteSheet("ICONES.HSQ");

	for (int i = 0; i != kCompassElements; i++) {
		const PanelElement &e = _panel[kFirstCompassElement + i];

		if (e.sprite != 0xffff && !(e.flags & 0x20))
			drawSprite(e.sprite, e.x, e.y);
	}
}

// DOS 8E16/8E9E is shared by ordinary dialogue and the troop-contact card.
// Each word reserves six pixels after itself, even though the font's actual
// space glyph is narrower.  A row's leftover width goes to its gaps.
struct SpeechRow {
	uint first;
	uint count;
	int gap;
	int spare;
};

struct SpeechLayout {
	Common::Array<Common::String> words;
	Common::Array<SpeechRow> rows;
};

static SpeechLayout layoutSpeech(Font &font, const Common::String &text, int boxWidth, int firstExtra = 0) {
	SpeechLayout layout;
	font.setFace(Font::kFaceNormal);
	Common::Array<int> widths;

	// A line may ask to be broken in the middle of itself, which 0x8E39 does
	// on a carriage return: the row is closed where it stands and not
	// stretched.  A break is kept as a word of no width so that the laying out
	// below can see it.
	Common::Array<bool> breaks;

	for (uint at = 0; at < text.size();) {
		while (at < text.size() && text[at] == ' ')
			at++;

		if (at < text.size() && text[at] == '\r') {
			if (!layout.words.empty()) {
				layout.words.push_back(Common::String());
				widths.push_back(0);
				breaks.push_back(true);
			}

			at++;
			continue;
		}

		uint end = at;
		while (end < text.size() && text[end] != ' ' && text[end] != '\r')
			end++;

		if (end > at) {
			const Common::String word(text.c_str() + at, end - at);
			Font::Face endingFace;
			const int wide = font.wordWidth(word, &endingFace) + (layout.words.empty() ? firstExtra : 0);
			// DOS 8ED3 changes DS:47A0 on codes 6/8 and leaves it changed for
			// the next word (and the next row).  Measuring each word in the
			// initial paragraph face gives the wrong line breaks and gap
			// justification. Within one word BX keeps its entry width table.
			font.setFace(endingFace);

			// A word of no width takes no room and no gap after it.
			if (wide > 0) {
				layout.words.push_back(word);
				widths.push_back(wide);
				breaks.push_back(false);
			}
		}

		at = end;
	}

	if (layout.words.empty())
		return layout;

	SpeechRow row = { 0, 0, kSpaceWidth, 0 };
	int left = boxWidth;

	// Closes the row that is being filled, sharing what is left of it out
	// between the gaps unless it is the last one.
	auto close = [&](int over, bool flush) {
		if (flush && row.count != 0) {
			// 8EB4 skips division for a one-word row, but still records
			// its gap: 8DF0 can then disable justification for the paragraph.
			const int gaps = MAX<int>(1, row.count - 1);
			row.gap = kSpaceWidth + over / gaps;
			row.spare = over % gaps;
		}

		layout.rows.push_back(row);
		row.first += row.count;
		row.count = 0;
		row.gap = kSpaceWidth;
		row.spare = 0;
		left = boxWidth;
	};

	for (uint i = 0; i != layout.words.size();) {
		// A break closes the row where it stands.
		if (breaks[i]) {
			i++;
			close(left, false);
			// 8E39 consumes the carriage return itself. It is only a
			// layout marker, not the first word of the following row.
			row.first = i;
			continue;
		}

		const int wanted = widths[i] + kSpaceWidth;

		if (left - wanted >= 0) {
			left -= wanted;
			row.count++;
			i++;
			continue;
		}

		// The space that would have followed the word is not needed at the
		// end of a row, so the word may still fit without it.
		if (left - wanted + kSpaceWidth >= 0) {
			row.count++;
			i++;
			close(left - widths[i - 1], true);
			continue;
		}

		if (row.count == 0) {
			// A single word wider than the box: it goes on a row of its own
			// rather than holding the whole line up.
			row.count++;
			i++;
		}

		// 8E69 adds back the rejected word's width AND its trailing space.
		// The accepted row likewise needs no trailing space; share those six
		// pixels between its gaps rather than leaving its right edge short.
		close(left + kSpaceWidth, true);
	}

	if (row.count != 0)
		close(left, false);
	// 8E7B also resets a row already closed by 8E64 when the final word
	// fits only without its trailing space. It must not keep that row's
	// calculated gap/remainder merely because there is no pending row.
	layout.rows.back().gap = kSpaceWidth;
	layout.rows.back().spare = 0;
	return layout;
}

static void paintSpeechLayout(Font &font, byte *screen, const SpeechLayout &layout,
		int left, int top, uint rowCount, byte colour, int firstAdvance = 0) {
	font.setFace(Font::kFaceNormal);
	bool justified = true;
	for (uint r = 0; r < layout.rows.size(); r++) {
		// 8DF0 clears justification for the whole paragraph if any nonempty row
		// would have a gap of thirty pixels or more, including one-word rows.
		if (layout.rows[r].count && layout.rows[r].gap >= 30) {
			justified = false;
			break;
		}
	}
	for (uint r = 0; r < rowCount; r++) {
		const SpeechRow &row = layout.rows[r];
		int x = left;
		int spare = justified ? row.spare : 0;
		for (uint w = 0; w < row.count; w++) {
			Common::String word = layout.words[row.first + w];
			if (!r && !w && firstAdvance) {
				word.deleteChar(0);
				x += firstAdvance;
			}
			x = font.drawString(screen, x, top + r * kSpeechRowHeight, word, colour);
			if (w + 1 != row.count) {
				x += justified ? row.gap : kSpaceWidth;
				if (spare > 0) {
					x++;
					spare--;
				}
			}
		}
	}
}

/** 8D22/D0FF: book columns and BOOK's nine ornamental initial capitals. */
int DuneEngine::drawBookText(const Common::String &text) {
	const byte *capitals = _dosCode.begin() + 0xd0d1;
	const byte *advances = capitals + 9;
	int cap = -1;
	for (uint i = 0; i < 9 && !text.empty(); i++)
		if (text[0] == capitals[i])
			cap = i;
	_font.setFace(Font::kFaceNormal);
	const int advance = cap < 0 ? 0 : advances[cap];
	const int extra = cap < 0 ? 0 : advance - _font.charWidth(text[0]);
	const SpeechLayout layout = layoutSpeech(_font, text, 210, extra);
	const int top = 10 + MAX<int>(0, (132 - layout.rows.size() * 10) / 2);
	paintSpeechLayout(_font, _framebuffer1, layout, 60, top, layout.rows.size(), 100, advance);
	if (cap >= 0)
		drawSprite(5 + cap, 60, MAX(0, top - 19));
	return top + layout.rows.size() * 10;
}

/** 8CFB/8D02: narrator/defeat text uses224C, not the ordinary caption foot. */
void DuneEngine::drawDefeatText() {
	const byte *box = _state.begin() + 0x224c;
	const int left = READ_LE_UINT16(box) + 72;
	const int width = READ_LE_UINT16(box + 4) - 72 - 16;
	const int height = READ_LE_UINT16(box + 6) - 8 - 8;
	_font.setFace(Font::kFaceNormal);
	const SpeechLayout layout = layoutSpeech(_font, _speech, width);
	if (layout.words.empty())
		return;
	const int top = READ_LE_UINT16(box + 2) + 8 +
		MAX<int>(0, (height - (int)layout.rows.size() * kSpeechRowHeight) / 2);
	paintSpeechLayout(_font, _framebuffer1, layout, left, top, layout.rows.size(), 240);
}

/**
 * What is being said, over the picture rather than in the panel.  The block
 * is laid so that its foot comes to 146, as in DOS 8E16/8B11.
 */
void DuneEngine::drawSpeech(const Common::String &text, byte colour, int behind, byte *page) {
	// A0E2 restores the preference, not the already displayed page. Keep
	// an owned balloon until 8C8A/new paragraph preparation removes it.
	if (behind < 0 && drawLocalBalloonCaption(text, page))
		return;
	// The mixer panel's third choice is to write nothing out at all.
	if (_state[0x28e7] >= 2 && !_state[0x46d9] && !forcedSpeechCaption())
		return;

	const SpeechLayout layout = layoutSpeech(_font, text, kSpeechWidth);
	if (layout.words.empty())
		return;

	const int y = kSpeechBottom - (int)layout.rows.size() * kSpeechRowHeight;

	// Over the maps the words are written into a box of their own, which is
	// what the original puts beside the place a press has named: the ground
	// there is the same yellow the words are and nothing could be read off it.
	// The two colours are the nearest ONMAP.HSQ keeps to the ones the box is
	// drawn in; where it stands and what its border really looks like are
	// still to be read out of the original.
	if (behind >= 0) {
		enum { kBoxEdge = 149 };

		_graphics.fillRect(kSpeechLeft - 4, y - 3,
						   kSpeechLeft + kSpeechWidth + 3,
						   kSpeechBottom + kSpeechRowHeight - 4, (byte)kBoxEdge);
		_graphics.fillRect(kSpeechLeft - 3, y - 2,
						   kSpeechLeft + kSpeechWidth + 2,
						   kSpeechBottom + kSpeechRowHeight - 5, (byte)behind);
	}

	byte *screen = page ? page : _framebuffer1;
	const bool ordinaryCaption = behind < 0 && colour == kSpeechColour &&
		READ_LE_UINT16(_state.begin() + 0x47c4) != 0xffff &&
		!_state[0x46eb] && !_state[0xc6] && !_state[0x227d] && !_state[0x46d9] && !_state[0x28e7];
	if (!ordinaryCaption) {
		paintSpeechLayout(_font, screen, layout, kSpeechLeft, y, layout.rows.size(), colour);
		return;
	}
	// 9046 chooses the ink once, while the paragraph's event is active.
	// 9025/908C subsequently copy the prepared raster even after that event
	// returns. A checkpoint can retain the ink without retaining this raster.
	const byte shadow = _speechCaptionMode == 0 && _speechCaptionText == text &&
		(_speechCaptionShadow == 8 || _speechCaptionShadow == 240)
		? _speechCaptionShadow : (int8)_state[0xea] > 0 ? 8 : 240;
	if (_speechCaptionMode != 0 || _speechCaptionPixels.empty() || _speechCaptionText != text) {
		_speechCaptionMode = 0;
		// 900B/9015 starts the ordinary transparent caption by clearing the
		// shared C085 page. A later MAP2 must not retain an older flight inset
		// below its prepared rows once this caption has replaced that page.
		memset(_state.begin() + kMapScratchOrigin, 0, kMapScratchBytes);
		_speechCaptionPixels.resize(kScreenWidth * kScreenHeight);
		memset(_speechCaptionPixels.begin(), 0, _speechCaptionPixels.size());
		paintSpeechLayout(_font, _speechCaptionPixels.begin(), layout, kSpeechLeft, y,
			layout.rows.size(), colour);
		_speechCaptionFace = _font.face();
		// Scan the separate transparent raster, as 9046 does. Looking for
		// glyph-coloured pixels on the scene would outline unrelated scenery.
		for (uint i = 0; i < _speechCaptionPixels.size(); i++) {
			if (_speechCaptionPixels[i] != kSpeechColour)
				continue;
			const int neighbours[] = { (int)i - 1, (int)i + 1, (int)i - kScreenWidth, (int)i + kScreenWidth };
			for (uint n = 0; n < ARRAYSIZE(neighbours); n++) {
				const int at = neighbours[n];
				if (at >= 0 && (uint)at < _speechCaptionPixels.size() && !_speechCaptionPixels[at])
					_speechCaptionPixels[at] = shadow;
			}
		}
		_speechCaptionText = text;
		_speechCaptionShadow = shadow;
	}
	_font.setFace(_speechCaptionFace);
	// Ordinary room callers use the default target. 9025 commits the first
	// caption to the clean page before presenting it; later 908C restores
	// dirty intersections there. Explicit targets (mouths, scratch pages)
	// must not alter a separate retained page as a side effect.
	byte *clean = !page && !_onWorld && !_onGlobe && !_inCabin && !_onMixer && !_takingOff &&
		_speakerPerson >= 0 && _localSpeechPerson == _speakerPerson &&
		_localSpeechClean.size() == kScreenWidth * kScreenHeight ? _localSpeechClean.begin() : nullptr;
	for (uint i = 0; i < _speechCaptionPixels.size(); i++)
		if (_speechCaptionPixels[i]) {
			screen[i] = _speechCaptionPixels[i];
			if (clean)
				clean[i] = _speechCaptionPixels[i];
		}
}

/** 8D8A..8DEF: choose a local balloon once, before portrait/voice preparation. */
void DuneEngine::prepareLocalBalloonCaption() {
	// Both 88DA and 94DD prepare a NEW paragraph, even when its text is
	// identical to the preceding page. Only redraws retain the old raster.
	_speechCaptionText.clear();
	_speechCaptionPixels.clear();
	_speechCaptionMode = 0;
	_speechCaptionShadow = 0;
	if (!_state[0x28e7]) {
		// 94DD enters 8B11 directly. A0E2 may have restored Subtitles
		// since the previous balloon: that next paragraph is no longer boxed.
		WRITE_LE_UINT16(_state.begin() + 0x479e, 0x223c);
		if (!_state[0x46eb] && !_state[0xc6] && !_state[0x227d] && !_state[0x46d9] &&
				READ_LE_UINT16(_state.begin() + 0x47c4) < 16) {
			WRITE_LE_UINT16(_state.begin() + 0x4784, 16);
			WRITE_LE_UINT16(_state.begin() + 0x4786, 16);
			WRITE_LE_UINT16(_state.begin() + 0x4788, 1);
			WRITE_LE_UINT16(_state.begin() + 0x478a, 0);
			// 8B11/9046 precedes 2B1A's local-event reset. The native
			// scene can be painted later, but its ink is already selected.
			_speechCaptionText = _speech;
			_speechCaptionShadow = (int8)_state[0xea] > 0 ? 8 : 240;
		}
		return;
	}
	if (_state[0x46eb] ||
			_state[0xc6] || _state[0x227d] || _state[0x46d9] ||
			READ_LE_UINT16(_state.begin() + 0x47c4) >= 16 || _speech.empty())
		return;
	const uint caller = READ_LE_UINT16(_state.begin() + 0x47c4);
	const int picture = fremenPicture(caller, caller == 15 ? _state[0x476c] & 7 : 0);
	if (picture < 0 || picture > 16)
		return;
	// 91CE copies the bank's actual horizontal origin into all three templates.
	const int left = _state[0x22a8 + picture];
	for (uint i = 0; i < 3; i++)
		WRITE_LE_UINT16(_state.begin() + 0x2224 + 8 * i, left);
	uint chosen = 2;
	const int marginLeft = READ_LE_UINT16(_state.begin() + 0x4784);
	const int marginRight = READ_LE_UINT16(_state.begin() + 0x4786);
	for (uint i = 0; i < 3; i++) {
		const byte *box = _state.begin() + 0x2224 + 8 * i;
		const int width = READ_LE_UINT16(box + 4) - marginLeft - marginRight;
		const SpeechLayout candidate = layoutSpeech(_font, _speech, width);
		const int height = candidate.rows.size() * 10 + READ_LE_UINT16(_state.begin() + 0x4788) +
			READ_LE_UINT16(_state.begin() + 0x478a);
		if (height < READ_LE_UINT16(box + 6)) {
			chosen = i;
			if (i < 2 && (troopRandom(true) & 1))
				chosen++; // 8DD8 deliberately need not recheck the larger box's height.
			break;
		}
		if (i == 2) {
			WRITE_LE_UINT16(_state.begin() + 0x4788, 0);
			WRITE_LE_UINT16(_state.begin() + 0x478a, 0);
		}
	}
	const uint owner = 0x2224 + 8 * chosen;
	const byte *box = _state.begin() + owner;
	const int width = READ_LE_UINT16(box + 4);
	const int height = READ_LE_UINT16(box + 6);
	WRITE_LE_UINT16(_state.begin() + 0x479e, owner);
	WRITE_LE_UINT16(_state.begin() + 0x1be2, left);
	WRITE_LE_UINT16(_state.begin() + 0x1be4, READ_LE_UINT16(box + 2));
	WRITE_LE_UINT16(_state.begin() + 0x1be6, MIN<int>(left + width, 320));
	WRITE_LE_UINT16(_state.begin() + 0x1be8, READ_LE_UINT16(box + 2) + height);
	WRITE_LE_UINT16(_state.begin() + 0x1bea, 0x80);
	WRITE_LE_UINT16(_state.begin() + 0x1bee, 0x9468);
	const SpeechLayout layout = layoutSpeech(_font, _speech, width - marginLeft - marginRight);
	byte words = 0;
	for (uint i = 0; i < layout.rows.size(); i++)
		words += layout.rows[i].count;
	_state[0x478c] = words;
	_speechCaptionText.clear();
	_speechCaptionPixels.clear();
}

/** A0E2 leaves 479E and the prepared caption raster intact. */
bool DuneEngine::retainedLocalBalloonCaption(const Common::String &text) const {
	const uint owner = READ_LE_UINT16(_state.begin() + 0x479e);
	return !_state[0x46eb] && !_state[0xc6] && !_state[0x227d] && !_state[0x46d9] &&
		(owner == 0x2224 || owner == 0x222c || owner == 0x2234) &&
		_speechPart < _speechParts.size() && text == _speechParts[_speechPart];
}

/** 8F28/C370: real ICONES28 texture and the retained 8B66 centred text block. */
bool DuneEngine::drawLocalBalloonCaption(const Common::String &text, byte *page) {
	const uint owner = READ_LE_UINT16(_state.begin() + 0x479e);
	if (!retainedLocalBalloonCaption(text))
		return false;
	// A complete local scene already put this box into the portrait's
	// backdrop. Later room/pose/mouth copies must not put it over the hands.
	// Explicit private painter targets still receive the isolated box raster.
	if (_localSpeakerBalloonText == text && _localSpeakerPerson == _speakerPerson &&
			(!page || page == _framebuffer1 || page == _framebuffer2 ||
			 (_localSpeechClean.size() == kScreenWidth * kScreenHeight && page == _localSpeechClean.begin())))
		return true;
	if (_speechCaptionMode != 1 || _speechCaptionPixels.empty() || _speechCaptionText != text) {
		Resource resource = _archive.openMember("ICONES.HSQ");
		if (!resource)
			return false;
		SpriteSheet icons(this, resource, false);
		int tileWidth = 0, tileHeight = 0;
		if (!icons.size(28, &tileWidth, &tileHeight) || tileWidth <= 0 || tileHeight <= 0)
			return false;
		const byte *box = _state.begin() + owner;
		const int left = READ_LE_UINT16(box), top = READ_LE_UINT16(box + 2);
		const int width = READ_LE_UINT16(box + 4), height = READ_LE_UINT16(box + 6);
		const Common::Rect bounds(left, top, MIN<int>(left + width, 320), top + height);
		_speechCaptionPixels.resize(kScreenWidth * kScreenHeight);
		memset(_speechCaptionPixels.begin(), 0, _speechCaptionPixels.size());
		byte *previousPage = _graphics.frameBuffer();
		const int previousOffset = _graphics.globalYOffset();
		_graphics.setFrameBuffer(_speechCaptionPixels.begin());
		_graphics.setGlobalYOffset(0);
		for (int y = top; y < bounds.bottom; y += tileHeight)
			for (int x = left; x < bounds.right; x += tileWidth)
				icons.drawMapSprite(28, x, y, bounds);
		_graphics.setFrameBuffer(previousPage);
		_graphics.setGlobalYOffset(previousOffset);
		const int marginLeft = READ_LE_UINT16(_state.begin() + 0x4784);
		const int marginRight = READ_LE_UINT16(_state.begin() + 0x4786);
		const int marginTop = READ_LE_UINT16(_state.begin() + 0x4788);
		const int marginBottom = READ_LE_UINT16(_state.begin() + 0x478a);
		const SpeechLayout layout = layoutSpeech(_font, text, width - marginLeft - marginRight);
		const int space = MAX<int>(height - marginTop - marginBottom - layout.rows.size() * 10, 0);
		paintSpeechLayout(_font, _speechCaptionPixels.begin(), layout, left + marginLeft,
			top + marginTop + space / 2, layout.rows.size(), 240);
		_speechCaptionText = text;
		_speechCaptionMode = 1;
		_speechCaptionFace = _font.face();
	}
	_font.setFace(_speechCaptionFace);
	byte *screen = page ? page : _framebuffer1;
	for (uint i = 0; i < _speechCaptionPixels.size(); i++)
		if (_speechCaptionPixels[i])
			screen[i] = _speechCaptionPixels[i];
	return true;
}

/**
 * The panel's left hand box holds the mouth of whoever is speaking, drawn
 * over the empty box and moving frame by frame while the line is read.
 */
void DuneEngine::drawSpeaker() {
	if (_speaker.empty())
		return;

	// The box in the panel is left as it is.  What used to go in it was one
	// frame of the speaker's mouth and nothing else, which is why a mouth was
	// seen talking in the corner with no face around it: the first sprites of
	// a speaker's bank are the mouth alone, and the rest of the face is in the
	// pieces after them.  The face belongs on the scene, drawn whole out of
	// the record at the end of the bank, and that is where it is drawn now.
	for (int y = 0; y != kPortraitHeight; y++)
		memcpy(_framebuffer1 + 320 * (kPortraitTop + y) + kPortraitLeft,
			   _portraitBox + kPortraitWidth * y, kPortraitWidth);

	// drawScene/drawBattlefield publish the complete composed page when
	// requested. A partial blit here exposes the next panel even when those
	// callers are preparing a deferred transition with present=false.
}

// The banks a place can name.  The routine that draws a scene adds 19 to the
// high nibble of a place and asks for that resource, and 19 counting from
// ICONES.HSQ - which is where the game's own list of banks begins - is
// GENERIC.HSQ.  The ninth is a gap the game leaves empty.
static const char *const kBanks[] = {
	"GENERIC.HSQ", "PROUGE.HSQ", "COMM.HSQ", "EQUI.HSQ", "BALCON.HSQ", "CORR.HSQ",
	"POR.HSQ", "SIET1.HSQ", "XPLAIN9.HSQ", nullptr, "BUNK.HSQ", "FINAL.HSQ",
	"SERRE.HSQ", "BOTA.HSQ", "PALPLAN.HSQ", "SUN.HSQ"
};

// Puts back whatever the player was looking at, the world or the room they
// stand in, after something else has had the screen.
void DuneEngine::redraw() {
	if (_menuKind == kMenuBook)
		drawBook();
	else if (showingDefeat())
		drawDefeatScreen();
	else if (_menuKind == kMenuPalaceGathering && !_menuParts.empty() &&
			_menuParts[0] == 519 && storyIsOver())
		drawEndingCard();
	else if (_menuKind == kMenuPalaceGathering && !_menuParts.empty() &&
			(_menuParts[0] == 312 || _menuParts[0] == 316))
		drawChaniKissCard(0);
	else if (_menuKind == kMenuPalaceGathering && !_menuParts.empty() &&
			(_menuParts[0] == 313 || _menuParts[0] == 315))
		drawChaniKissCard(1);
	else if (isStageEventMenu() &&
			stageEventHasRoomFrame(_dosCode.begin(), READ_LE_UINT16(_state.begin() + 0x477a))) {
		const byte *layout = roomRecord(_state[5], _state[4]);
		if (layout)
			_place = layout[0] - 1;
		drawScene();
	}
	else if (_onGlobe)
		redrawGlobe();
	else if (_onMixer)
		drawMixerKnobs();
	else if (_menuKind == kMenuPalacePlan)
		drawPalacePlan();
	else if (_menuKind == kMenuTroopNotice)
		drawTroopNotice();
	else if (_menuKind == kMenuWormMap)
		drawWormMap();
	else if (_menuKind == kMenuMirror ||
			 (_menuKind == kMenuLeaving && !_onGlobe && !_onMixer &&
			  _state.size() > 5 && READ_LE_UINT16(_state.begin() + 4) == 0x2009))
		drawMirror();
	// The world put up over a flight - which is what CHANGE DESTINATION does -
	// is asked for on top of the desert and not instead of it, so _inDesert is
	// still set underneath.  Read the map and the cabin first, or the sand
	// paints itself back over the map several times a second and the player
	// never sees what they are being asked to press.
	else if (_inCabin)
		showCabin();
	else if (_onWorld)
		showWorld();
	else if (_inDesert) {
		if (_flightDiscoveryPrompt || carriedPortraitVisible()) {
			if (_speakerPerson < 0 && _state[0x23] == 4)
				showFlightWarning();
			else
				showFlightDiscovery();
		}
		// Ordinary on-foot conversations need the same portrait/caption
		// composition as scripted ones, including after a checkpoint load.
		else if (_speakerPerson >= 0)
			drawDesertDialogue();
		else
			drawDesert();
	}
	else
		drawScene();
}

/** 0F13/09C7: shifted portraits clipped by 9D2D to the scene rectangle. */
void DuneEngine::drawEndingPortrait(int person, int frame, int animation, int down) {
	Common::Array<SpeakerSprite> sprites;
	collectSpeakerSprites(person, frame, animation, -1, sprites);
	const int dx = person == 45 ? 45 : 0;
	const int dy = down + (person == 7 ? 15 : 0);
	const Common::Rect clip(0, 0, kScreenWidth, kSceneHeight);
	for (uint i = 0; i < sprites.size(); i++)
		currentSpriteSheet->drawMapSprite(sprites[i].index,
				sprites[i].x + dx, sprites[i].y + dy, clip);
}

/** The two full-size portraits used on either side of FINAL's reveal. */
void DuneEngine::drawEndingCouple(int down) {
	if (_endingCoupleActive && down == 0 && _endingCouplePage.size() == kScreenWidth * kSceneHeight) {
		memcpy(_framebuffer1, _endingCouplePage.begin(), _endingCouplePage.size());
		return;
	}
	drawEndingPortrait(7, 0, 0, down);
	drawEndingPortrait(45, 32, 0, down);
}

namespace {

uint endingPortraitRecord(const Common::Array<byte> &bank) {
	if (bank.size() < 2)
		return 0;
	const uint index = READ_LE_UINT16(bank.begin());
	if (index + 2 > bank.size())
		return 0;
	const uint count = READ_LE_UINT16(bank.begin() + index) / 2;
	if (!count || index + 2 * count > bank.size())
		return 0;
	const uint record = index + READ_LE_UINT16(bank.begin() + index + 2 * (count - 1)) + 4;
	return record + 12 <= bank.size() ? record : 0;
}

uint endingNextFrame(const Common::Array<byte> &bank, uint at) {
	while (at < bank.size() && bank[at])
		at += bank[at] == 1 ? 2 : 1;
	return at < bank.size() ? at + 1 : bank.size();
}

} // End of anonymous namespace

bool DuneEngine::loadEndingPortraitBank(bool chani) {
	Common::Array<byte> &bank = _endingCoupleBanks[chani ? 0 : 1];
	if (bank.empty()) {
		Resource resource = _archive.openMember(chani ? "CHAN.HSQ" : "PAUL.HSQ");
		if (!resource)
			return false;
		bank.resize(resource->size());
		resource->read(bank.begin(), bank.size());
	}
	return endingPortraitRecord(bank) != 0;
}

void DuneEngine::stopEndingCouple() {
	if (_endingCoupleActive)
		_state[0x47c3] = 0;
	_endingCoupleActive = false;
	_endingCoupleRemainder = 0;
	_endingCoupleBackdrop.clear();
	_endingCouplePage.clear();
}

/** 0F13/9908/998E: initialize Chani first, then Paul, with one registration. */
void DuneEngine::prepareEndingCouple() {
	stopEndingCouple();
	if (!loadEndingPortraitBank(true) || !loadEndingPortraitBank(false))
		return;
	_endingCoupleBackdrop.resize(kScreenWidth * kScreenHeight);
	memcpy(_endingCoupleBackdrop.begin(), _framebuffer1, _endingCoupleBackdrop.size());
	_endingCoupleActive = true;
	_state[0x47c3] = 1;
	for (uint i = 0; i < 2; i++) {
		const bool chani = i == 0;
		const Common::Array<byte> &bank = _endingCoupleBanks[i];
		const uint record = endingPortraitRecord(bank);
		const uint origin = chani ? 0x4c60 : 0x1bf0;
		const uint actor = chani ? 0x4dfc : 0x47c4;
		const uint history = chani ? 0x4c6a : 0x4540;
		const uint variant = chani ? 0 : paulAnimation() + 1;
		const uint table = record + 8 + READ_LE_UINT16(bank.begin() + record + 8);
		const uint choice = chani ? (troopRandom(true) & 6) : 2 * (variant - 1);
		if (table + choice + 2 > bank.size()) {
			stopEndingCouple();
			return;
		}
		uint at = table + READ_LE_UINT16(bank.begin() + table + choice);
		if (!chani)
			for (uint frame = 0; frame < 32; frame++)
				at = endingNextFrame(bank, at);
		for (uint word = 0; word < 4; word++)
			WRITE_LE_UINT16(_state.begin() + origin + 2 * word, READ_LE_UINT16(bank.begin() + record + 2 * word));
		if (chani)
			WRITE_LE_UINT16(_state.begin() + origin + 2, READ_LE_UINT16(_state.begin() + origin + 2) + 15);
		else {
			WRITE_LE_UINT16(_state.begin() + origin, READ_LE_UINT16(_state.begin() + origin) + 45);
			WRITE_LE_UINT16(_state.begin() + origin + 4, MIN<uint>(320, READ_LE_UINT16(_state.begin() + origin + 4) + 45));
		}
		const uint16 words[] = { (uint16)(chani ? 7 : 45), (uint16)at, (uint16)at,
			(uint16)table, (uint16)(record + 10), (uint16)(chani ? 0 : 4), (uint16)(0xc000 | variant) };
		for (uint word = 0; word < ARRAYSIZE(words); word++)
			WRITE_LE_UINT16(_state.begin() + actor + 2 * word, words[word]);
		WRITE_LE_UINT16(_state.begin() + history, 0);
		openSpriteSheet(chani ? "CHAN.HSQ" : "PAUL.HSQ");
		paintEndingActor(chani, true);
		// The second C412 saves the clean Chani page as BOTH actors' shared
		// backdrop. It is not an empty room and does not contain Paul yet.
		if (chani)
			memcpy(_endingCoupleBackdrop.begin(), _framebuffer2, _endingCoupleBackdrop.size());
	}
	_endingCouplePage.resize(kScreenWidth * kSceneHeight);
	memcpy(_endingCouplePage.begin(), _framebuffer1, _endingCouplePage.size());
	_endingCoupleTime = _system->getMillis() / 5;
	_state[0x478c] = 1;
	WRITE_LE_UINT16(_state.begin() + 0x4c68, 7);
	WRITE_LE_UINT16(_state.begin() + 0x22a6, 45);
}

/** Reject invalid saved cursors/history before a retained pair is painted. */
bool DuneEngine::validEndingCoupleState() {
	if (_endingCoupleBackdrop.size() != kScreenWidth * kScreenHeight ||
			_endingCouplePage.size() != kScreenWidth * kSceneHeight || _endingCoupleRemainder >= 16 ||
			_state[0x47c3] != 1 ||
			!loadEndingPortraitBank(true) || !loadEndingPortraitBank(false))
		return false;
	for (uint i = 0; i < 2; i++) {
		const Common::Array<byte> &bank = _endingCoupleBanks[i];
		const uint record = endingPortraitRecord(bank);
		const uint actor = i == 0 ? 0x4dfc : 0x47c4;
		const uint origin = i == 0 ? 0x4c60 : 0x1bf0;
		const uint history = i == 0 ? 0x4c6a : 0x4540;
		const uint table = record + 8 + READ_LE_UINT16(bank.begin() + record + 8);
		const uint flags = READ_LE_UINT16(_state.begin() + actor + 12);
		if (READ_LE_UINT16(_state.begin() + actor) != (i == 0 ? 7 : 45) ||
				READ_LE_UINT16(_state.begin() + actor + 6) != table ||
				READ_LE_UINT16(_state.begin() + actor + 8) != record + 10 ||
				(flags >> 8 != 0xc0 && flags >> 8 != 0xd0) ||
				(i == 0 ? (flags & 255) != 0 : (flags & 255) < 1 || (flags & 255) > 18) ||
				(int16)READ_LE_UINT16(_state.begin() + actor + 10) < -128 ||
				(int16)READ_LE_UINT16(_state.begin() + actor + 10) > 8 ||
				READ_LE_UINT16(_state.begin() + origin) != (i == 0 ? 0 : 50) ||
				READ_LE_UINT16(_state.begin() + origin + 2) != (i == 0 ? 15 : 0) ||
				READ_LE_UINT16(_state.begin() + origin + 4) != (i == 0 ? 175 : 320) ||
				READ_LE_UINT16(_state.begin() + origin + 6) != 152)
			return false;
		const uint count = READ_LE_UINT16(_state.begin() + history);
		openSpriteSheet(i == 0 ? "CHAN.HSQ" : "PAUL.HSQ", false);
		if (count > 133)
			return false;
		for (uint part = 0; part < count; part++) {
			int wide, high;
			if (!currentSpriteSheet->size(_state[history + 2 + 3 * part] - 1, &wide, &high))
				return false;
		}
		bool validCurrent = false, validNext = false;
		for (uint animation = 0; animation < (i == 0 ? 6 : 18); animation++) {
			if (i == 0 && (animation == 3 || animation == 4))
				continue;
			if (table + 2 * animation + 2 > bank.size())
				return false;
			uint at = table + READ_LE_UINT16(bank.begin() + table + 2 * animation);
			while (at < bank.size() && bank[at] != 255) {
				validCurrent |= at == READ_LE_UINT16(_state.begin() + actor + 4);
				validNext |= at == READ_LE_UINT16(_state.begin() + actor + 2);
				at = endingNextFrame(bank, at);
			}
			validNext |= at < bank.size() && at == READ_LE_UINT16(_state.begin() + actor + 2);
		}
		if (!validCurrent || !validNext)
			return false;
	}
	return true;
}

/** 9BEE/9C2D/9D16/C4F0: retain each expanded list and copy only changed parts. */
bool DuneEngine::paintEndingActor(bool chani, bool initial) {
	const Common::Array<byte> &bank = _endingCoupleBanks[chani ? 0 : 1];
	const uint actor = chani ? 0x4dfc : 0x47c4;
	const uint origin = chani ? 0x4c60 : 0x1bf0;
	const uint history = chani ? 0x4c6a : 0x4540;
	const int x = READ_LE_UINT16(_state.begin() + origin), y = READ_LE_UINT16(_state.begin() + origin + 2);
	const uint parts = READ_LE_UINT16(_state.begin() + actor + 8);
	uint at = READ_LE_UINT16(_state.begin() + actor + 4);
	Common::Array<SpeakerSprite> sprites;
	while (at < bank.size() && bank[at] && bank[at] != 255) {
		uint part = bank[at++];
		if (part == 1 && at < bank.size())
			part = 256 + bank[at++];
		if (part < 2 || parts + 2 * (part - 2) + 2 > bank.size())
			return false;
		uint p = parts + READ_LE_UINT16(bank.begin() + parts + 2 * (part - 2));
		while (p + 2 < bank.size() && bank[p]) {
			const SpeakerSprite sprite = { bank[p] - 1, x + bank[p + 1], y + bank[p + 2] };
			sprites.push_back(sprite);
			p += 3;
		}
	}
	if (at >= bank.size() || sprites.size() > 133)
		return false;
	// 9BB1 returns SI after the frame's zero even when no pixel changed.
	if (!initial)
		WRITE_LE_UINT16(_state.begin() + actor + 2, at + 1);
	openSpriteSheet(chani ? "CHAN.HSQ" : "PAUL.HSQ", false);
	int left = 319, top = 199, right = 0, bottom = 0;
	Common::Array<SpeakerSprite> previous;
	const uint count = READ_LE_UINT16(_state.begin() + history);
	if (count > 133)
		return false;
	for (uint i = 0; i < count; i++) {
		const byte *p = _state.begin() + history + 2 + 3 * i;
		const SpeakerSprite sprite = { p[0] - 1, x + p[1], y + p[2] };
		previous.push_back(sprite);
	}
	auto changed = [&](const Common::Array<SpeakerSprite> &from, const Common::Array<SpeakerSprite> &to) {
		for (uint i = 0; i < from.size(); i++) {
			bool found = false;
			for (uint j = 0; j < to.size(); j++)
				if (from[i].index == to[j].index && from[i].x == to[j].x && from[i].y == to[j].y)
					found = true;
			int wide, high;
			if (!found && currentSpriteSheet->size(from[i].index, &wide, &high)) {
				left = MIN(left, from[i].x); top = MIN(top, from[i].y);
				right = MAX(right, from[i].x + wide); bottom = MAX(bottom, from[i].y + high);
			}
		}
	};
	if (initial) {
		left = x; top = y;
		right = READ_LE_UINT16(_state.begin() + origin + 4);
		bottom = READ_LE_UINT16(_state.begin() + origin + 6);
	} else {
		changed(previous, sprites);
		changed(sprites, previous);
	}
	bottom = MIN<int>(bottom, kSceneHeight);
	const uint16 bounds[] = { (uint16)left, (uint16)top, (uint16)right, (uint16)bottom };
	for (uint i = 0; i < 4; i++)
		WRITE_LE_UINT16(_state.begin() + 0xd834 + 2 * i, bounds[i]);
	if (left == 319 || left >= right || top >= bottom)
		return false;
	Common::Rect clip(left, top, right, bottom);
	clip.clip(Common::Rect(0, 0, kScreenWidth, kSceneHeight));
	if (clip.isEmpty())
		return false;
	memcpy(_framebuffer2, _endingCoupleBackdrop.begin(), _endingCoupleBackdrop.size());
	_graphics.setFrameBuffer(_framebuffer2);
	for (uint i = 0; i < sprites.size(); i++)
		currentSpriteSheet->drawMapSprite(sprites[i].index, sprites[i].x, sprites[i].y, clip);
	WRITE_LE_UINT16(_state.begin() + history, sprites.size());
	for (uint i = 0; i < sprites.size(); i++) {
		byte *p = _state.begin() + history + 2 + 3 * i;
		p[0] = sprites[i].index + 1; p[1] = sprites[i].x - x; p[2] = sprites[i].y - y;
	}
	if (!_state[0x227d] && bottom >= 137 && right >= 126 && left < 194)
		drawFace();
	for (int row = clip.top; row < clip.bottom; row++)
		memcpy(_framebuffer1 + row * kScreenWidth + clip.left,
				_framebuffer2 + row * kScreenWidth + clip.left, clip.width());
	_graphics.setFrameBuffer(_framebuffer1);
	return true;
}

/** 99DA/9A1D/9A40/9A7B: one actor, including the silent speech-block phase. */
bool DuneEngine::advanceEndingActor(bool chani) {
	const Common::Array<byte> &bank = _endingCoupleBanks[chani ? 0 : 1];
	const uint actor = chani ? 0x4dfc : 0x47c4;
	uint flags = READ_LE_UINT16(_state.begin() + actor + 12);
	const uint variant = chani ? 0 : paulAnimation() + 1;
	flags = (flags & 0xff00) | variant; // 9187 reselects the fixed age/fatigue variant.
	WRITE_LE_UINT16(_state.begin() + actor + 12, flags);
	if (!(flags & 0x8000))
		return false;
	const uint table = READ_LE_UINT16(_state.begin() + actor + 6);
	int16 remaining = READ_LE_UINT16(_state.begin() + actor + 10);
	uint at = READ_LE_UINT16(_state.begin() + actor + 2);
	if (at >= bank.size())
		return false;
	if (!(flags & 0x1000) && bank[at] == 255) {
		if (remaining < 0) {
			flags |= 0x1000;
			WRITE_LE_UINT16(_state.begin() + actor + 12, flags);
		} else {
			const uint choice = variant ? 2 * (variant - 1) : troopRandom(true) & 6;
			if (table + choice + 2 > bank.size())
				return false;
			at = table + READ_LE_UINT16(bank.begin() + table + choice);
			if (variant)
				for (uint frame = 0; frame < 32; frame++)
					at = endingNextFrame(bank, at);
		}
	}
	if ((flags & 0x1000) && remaining <= 0) {
		const uint animation = variant ? variant - 1 : (storyStage() >= 200 ? 5 : 4);
		if (table + 2 * animation + 2 > bank.size())
			return false;
		// 9A7B sets SI before testing AH. A rejected block keeps the previous
		// retained cursor and page, but still spends the secondary RNG word.
		at = table + READ_LE_UINT16(bank.begin() + table + 2 * animation);
		const uint choice = troopRandom(true) & (variant ? 0x0f18 : 0x0f38);
		if (choice & 0xff00)
			return false;
		for (uint frame = 0; frame < (choice & 0xff); frame++)
			at = endingNextFrame(bank, at);
		remaining = 8;
	}
	if (at >= bank.size() || bank[at] == 255)
		return false;
	WRITE_LE_UINT16(_state.begin() + actor + 10, (uint16)(remaining - 1));
	WRITE_LE_UINT16(_state.begin() + actor + 4, at);
	return paintEndingActor(chani, false);
}

/** D9D2/99BE: one paired callback per due poll, never a catch-up loop. */
bool DuneEngine::pollEndingCouple(uint16 ticks, bool present) {
	if (!_endingCoupleActive)
		return false;
	const uint16 elapsed = ticks + _endingCoupleRemainder;
	_endingCoupleRemainder = elapsed % 16;
	if (elapsed < 16)
		return false;
	bool changed = advanceEndingActor(true); // 998E always services Chani first.
	changed |= advanceEndingActor(false);
	if (changed) {
		memcpy(_endingCouplePage.begin(), _framebuffer1, _endingCouplePage.size());
		if (present) {
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kSceneHeight);
			_system->updateScreen();
		}
	}
	return changed;
}

void DuneEngine::updateEndingCouple() {
	const uint32 now = animationClockTicks();
	const uint16 elapsed = animationElapsedTicks(_endingCoupleTime, now);
	_endingCoupleTime = now;
	if (!_animationClock.active())
		_animationClock.rebase(now * 5); // Standalone E353/DD63 ending polls also sample DC68.
	pollEndingCouple(elapsed);
}

/** 157E/1566/1556: a saved DAC range and 16 timer ticks per update. */
bool DuneEngine::fadeEndingPaletteRange(uint first, uint count, uint frames, bool fadingIn) {
	byte rgb[768], target[768], current[768];
	_system->getPaletteManager()->grabPalette(rgb, 0, 256);
	for (uint i = 0; i < ARRAYSIZE(target); i++)
		target[i] = current[i] = ((uint)rgb[i] * 63 + 127) / 255;
	// 394D saves zero as the room fade's destination; 38BD instead starts
	// the pending range at zero for the silhouettes and title.
	memset((fadingIn ? current : target) + first * 3, 0, count * 3);
	if (fadingIn)
		applyPalette(first, count, current + first * 3);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	uint32 deadline = _system->getMillis();
	_state[0x227d]++; // 1566 suppresses crest restoration during a palette range.
	for (uint remaining = frames; remaining && !shouldQuit(); remaining--) {
		Graphics::fadePaletteRange(target, current, first, count, remaining);
		applyPalette(first, count, current + first * 3);
		_system->updateScreen();
		updateGameMusic();
		if (pollSkip() != kPlaybackFinished) {
			_state[0x227d]--;
			return false;
		}
		deadline += 80; // E353 waits 16 five-ms ticks after each callback.
		const uint32 now = _system->getMillis();
		if (now < deadline)
			_system->delayMillis(deadline - now);
		updateEndingCouple(); // DD63/D9D2 runs inside E353's timer wait.
	}
	_state[0x227d]--;
	return !shouldQuit();
}

void DuneEngine::playEndingTransition() {
	if (ConfMan.getBool("dune_nodelay"))
		return;
	// 14C9 blacks only the room's colours; portrait and panel pixels stay.
	if (!fadeEndingPaletteRange(128, 112, 40, false))
		return;
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet("FINAL.HSQ");
	for (uint figure = 0; figure < 3; figure++)
		drawSprite(figure, 0, 0);
	prepareEndingCouple(); // 14AC reinitializes the actors on the FINAL page.
	drawEndingCouple(0); // 14AC/0F13 retain the couple over the silhouettes.
	if (!fadeEndingPaletteRange(99, 76, 48, true))
		return;
	stopEndingCouple(); // 98B2 removes the portrait callback before effect 22.
	Common::Array<byte> previous, next;
	previous.resize(kScreenWidth * kScreenHeight);
	next.resize(previous.size());
	memcpy(previous.begin(), _framebuffer1, previous.size());
	// 1504 fills DS:1470's scene rectangle with 8F, not colour zero.
	memset(_framebuffer1, 0x8f, kSceneHeight * kScreenWidth);
	openSpriteSheet("FINAL.HSQ");
	drawSprite(3, 52, 0);
	memcpy(next.begin(), _framebuffer1, next.size());
	uint32 deadline = _system->getMillis();
	for (uint rows = 0; rows <= kSceneHeight && !shouldQuit(); rows++) {
		Graphics::scrollEndingPage(previous.begin(), next.begin(), _framebuffer1, rows);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		updateGameMusic();
		if (pollSkip() != kPlaybackFinished)
			return;
		deadline += 30; // DNVGA 2D9D: one row per six 200-Hz ticks.
		const uint32 now = _system->getMillis();
		if (now < deadline)
			_system->delayMillis(deadline - now);
	}
	// E387 holds the uncaptioned face for 300 ticks before drawing FINAL 4.
	for (uint elapsed = 0; elapsed < 1500 && !shouldQuit(); elapsed += 10) {
		updateGameMusic();
		if (pollSkip() != kPlaybackFinished)
			return;
		_system->delayMillis(10);
	}
	openSpriteSheet("FINAL.HSQ");
	drawSprite(4, 90, 64);
	fadeEndingPaletteRange(160, 15, 32, true);
}

/** FINAL.HSQ's settled red face and the original THE END title. */
void DuneEngine::drawEndingCard(bool present) {
	stopEndingCouple();
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	memset(_framebuffer1, 0x8f, kScreenWidth * kSceneHeight);
	openSpriteSheet("FINAL.HSQ");
	// Sprites 0-2 are the transition's successive silhouettes, not layers
	// retained underneath the title. Its settled frame uses only 3 and 4.
	// DOS 14C9 puts FINAL 3 at (34h,0) and FINAL 4 at (5Ah,40h).
	drawSprite(3, 52, 0);
	drawSprite(4, 90, 64);
	drawPanel();
	drawMenu();
	if (present) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** DOS 167C's cast cards, from the callback table at CS:165A. */
void DuneEngine::drawEndingCastFrame(uint index, bool present) {
	if (index > 13)
		return;

	struct CastRoom {
		uint16 room;
		byte place;
		byte person;
	};
	// Entries 2..13 of the table. Entry 1 is Paul's desert, and entry 10
	// calls 09AD to build the Baron's purple BACK.HSQ scene instead of a room.
	static const CastRoom kRooms[] = {
		{ 0x200a,  0,  0 }, // Duke Leto
		{ 0x2003,  0,  1 }, // Jessica
		{ 0x2006,  0,  4 }, // Gurney
		{ 0x2004,  0,  3 }, // Duncan
		{ 0x3002,  1, 11 }, // Emperor
		{ 0x0703, 16,  8 }, // Harah
		{ 0x2008,  0,  2 }, // Thufir
		{ 0x0802, 15,  5 }, // Stilgar
		{ 0,       0,  9 }, // Baron: 09AD
		{ 0x3002,  1, 10 }, // Feyd
		{ 0x0803, 15,  7 }, // Chani
		{ 0x1005, 62,  6 }  // Liet Kynes
	};

	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	if (index == 0) {
		// 15B7 draws the caption with FINAL sprite five at (64, 52).
		openSpriteSheet("FINAL.HSQ");
		drawSprite(5, 64, 52);
	} else if (index == 1) {
		// 095D: the daytime sky and INTDS dune at y=60, then Paul.
		applySkyPalette(true);
		drawSky();
		openSpriteSheet("INTDS.HSQ");
		drawSprite(0, 0, 60);
		drawSpeakerFigure(45, 32);
	} else if (index >= 2 && index <= 13) {
		const CastRoom &cast = kRooms[index - 2];
		if (index == 10) {
			// 09AD reuses the same four-piece backdrop as the introduction.
			drawList(INDEX_BACK_HSQ, 0x153a, -1, 0, 0, false);
			drawSpeakerFigure(cast.person, 0);
		} else {
			// 08F0 sets the room and place, 37B2 paints it, and 978E puts the
			// speaker in front. Cast rooms are not the fourfold dialogue zoom.
			const Common::Array<byte> savedState = _state;
			const byte savedPlace = _place;
			const Common::String savedSal = _salFile;
			const Common::String savedBackground = _background;
			const Common::String savedSpeech = _speech;
			const int savedSpeaker = _speakerPerson;
			const MenuKind savedMenu = _menuKind;
			const bool savedDesert = _inDesert;
			const bool savedWorld = _onWorld;
			const uint16 place = 0x100 + 28 * cast.place;
			WRITE_LE_UINT16(_state.begin() + 4, cast.room);
			WRITE_LE_UINT16(_state.begin() + 6, ((uint16)(cast.place + 1) << 8) | 0x80);
			WRITE_LE_UINT16(_state.begin() + 0x114e, place);
			WRITE_LE_UINT16(_state.begin() + 0x1150, place);
			_state[8] = (byte)(cast.room >> 8);
			_state[9] = cast.place + 1;
			_state[0xb] = (byte)cast.room;
			_state[place + 8] = (byte)(cast.room >> 8);
			// The cast callbacks clear DS:12 before painting the chosen room.
			// Keep the SAL scenery's built-in guards, but do not reintroduce
			// the live game's standing NPC roster in peopleHere().
			for (uint person = 0; person != 16; person++)
				WRITE_LE_UINT16(_state.begin() + 0xfd8 + 16 * person, 0xffff);
			_speakerPerson = -1;
			_menuKind = kMenuRoom; // No ending-event tableau on a cast card.
			_speech.clear();
			_inDesert = true; // Cast cards cannot start a fort battle.
			_onWorld = false;
			roomFor(cast.place);
			drawScene(false);
			drawSpeakerFigure(cast.person, 0);
			_state = savedState;
			_place = savedPlace;
			_salFile = savedSal;
			_background = savedBackground;
			_speech = savedSpeech;
			_speakerPerson = savedSpeaker;
			_menuKind = savedMenu;
			_inDesert = savedDesert;
			_onWorld = savedWorld;
		}
	}

	if (index != 0) {
		// DOS 15A8 starts the name at (10,157). D16A draws each glyph as a
		// GENERIC.HSQ sprite and advances by its width plus two pixels.
		memset(_framebuffer1 + kScreenWidth * kSceneHeight, 0,
				kScreenWidth * (kScreenHeight - kSceneHeight));
		openSpriteSheet("GENERIC.HSQ");
		const Common::String name = command(0x120 + index);
		int x = 10;
		for (uint i = 0; i < name.size(); i++) {
			const byte c = name[i];
			if (c < 0x20 || c > 0x79)
				continue;
			const int sprite = c - 0x20;
			int width = 0, height = 0;
			if (currentSpriteSheet->size(sprite, &width, &height)) {
				drawSprite(sprite, x, 157);
				x += width + 2;
			}
		}
	}
	if (present) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** The cast uses the same DNVGA 3A handoff as the introduction. */
void DuneEngine::fadeEndingCast(const byte *previous, const byte *next) {
	fadePage(previous, next, nullptr, false);
}

void DuneEngine::playEndingCast() {
	byte previous[256 * 3], next[256 * 3];
	_system->getPaletteManager()->grabPalette(previous, 0, 256);
	for (uint index = 0; index != 14 && !shouldQuit(); index++) {
		drawEndingCastFrame(index, false);
		_system->getPaletteManager()->grabPalette(next, 0, 256);
		if (!ConfMan.getBool("dune_nodelay")) {
			_system->getPaletteManager()->setPalette(previous, 0, 256);
			fadeEndingCast(previous, next);
		} else {
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
					kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
		memcpy(previous, next, sizeof(previous));
		const PlaybackResult asked = hold(3000); // DDB0(0x258): 600 five-ms ticks.
		if (asked == kPlaybackAborted)
			break;
	}
}

/** The last silent Paul/Chani still, assembled from the original sprite banks. */
void DuneEngine::drawFinalStill(bool present) {
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	// BACK's three nested rectangles complete the red gradient behind them.
	openSpriteSheet("BACK.HSQ");
	drawSprite(0, 0, 0);
	drawSprite(1, 52, 24);
	drawSprite(2, 105, 50);
	// 0F13 shifts Chani down fifteen and Paul right forty-five pixels.
	prepareEndingCouple();
	drawEndingCouple(0);
	if (present) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/**
 * Who is standing about the place, which the game works out at 0x3DB0 rather
 * than keeping.  The two words at DS:0x10 and DS:0x12 are exclusive-ored
 * together and every bit that comes out is a person, numbered by which bit it
 * is. Extra chiefs have successive identities 16 through 22, following the
 * ordinary chief's identity 15, up to the count at DS:0x476A.
 *
 * Each of them is laid in the list at the place its own number falls on -
 * 0x3DF4 adds the low nibble of DS:0xC5 and takes the remainder against how
 * many may stand here - and where that is taken already, at the first place
 * still free.  The rest of the list stays 0xFF, which is nobody.
 */
Common::Array<byte> DuneEngine::peoplePresent(uint places) const {
	Common::Array<byte> list;
	list.resize(places, 0xff);

	if (places == 0 || _state.size() <= 0x476a)
		return list;

	// The ending scene opcode gives the renderer an explicit roster, beginning
	// at CS:1292. The scene at CS:12C5 removes Chani for the last tableau.
	// Portrait close-ups suppress the floor figures in the original renderer.
	if (_menuKind == kMenuPalaceGathering && !_menuParts.empty() &&
			_menuParts[0] >= 500 && _menuParts[0] <= 519 && storyIsOver()) {
		if (_speakerPerson >= 0 || (_state[0x47a4] & 0x81))
			return list;
		const byte *roster = _dosCode.begin() + (_menuParts[0] >= 518 ? 0x12c7 : 0x1291);
		for (uint i = 0; i < MIN<uint>(places, *roster); i++)
			list[i] = roster[i + 1];
		return list;
	}

	// DOS 3D12 suppresses every standing figure while either the room-hidden
	// bit or the portrait-close-up bit is set. 3B03 redraws the room with the
	// high bit before enlarging the background around the speaker; retaining
	// the figures here blew the companions up behind Kynes' face.
	// A2B0 clears 47A4 over an already enlarged clean room page. Native
	// redraws must still suppress figures for a valid close-up. But3B1F
	// skips magnification entirely for an unplaced speaker (negative X):
	// retain the ordinary floor figures, as in Stilgar's ecology reply.
	if (_state[0x47a4] & 0x81)
		return list;
	if (_speakerPerson >= 0) {
		const uint who = _speakerPerson == 15 ? 15 + _state[0x476c] : (uint)_speakerPerson;
		const uint at = 0x47f8 + 4 * who;
		if (at + 1 < _state.size() && READ_LE_INT16(_state.begin() + at) >= 0)
			return list;
	}

	const byte from = (byte)(_state[0xc5] & 0xf);

	// Lays one person down, at their own place or the first one free.
	struct Lay {
		static void down(Common::Array<byte> &to, byte who, byte from) {
			const uint places = to.size();
			uint at = (who + from) % places;

			if (to[at] != 0xff) {
				for (at = 0; at != places && to[at] != 0xff; at++)
					;

				if (at == places)
					return;
			}

			to[at] = who;
		}
	};
	// DOS 3D9C copies an event's counted inline roster without the ordinary
	// C5-shifted room placement. These lists were checked by executing 3D83
	// on the original code; portrait close-ups remain suppressed above.
	if (_state.size() > 0x4779 && _state[0x4774]) {
		const uint16 offset = READ_LE_UINT16(_state.begin() + 0x4778);
		switch (offset) {
		case 0x12dd:
		case 0x12fd:
		case 0x1323:
		case 0x1334:
		case 0x1343:
		case 0x135b:
		case 0x1374:
		case 0x1389: {
			const byte *roster = _dosCode.begin() + offset;
			for (uint i = 0; i < MIN<uint>(places, *roster); i++)
				list[i] = roster[i + 1];
			return list;
		}
		default:
			break;
		}
	}

	uint16 here = READ_LE_UINT16(_state.begin() + 0x12) ^ READ_LE_UINT16(_state.begin() + 0x10);
	// 3B1F retains the ordinary room for an unplaced panel companion.
	// Accepting STAY changes10, but must not create a new floor mark and
	// magnify a different corner underneath the still-active conversation.
	if (_speakerPerson >= 0 && _speakerPerson < 16 &&
			READ_LE_INT16(_state.begin() + 0x47f8 + 4 * _speakerPerson) < 0)
		here &= (uint16)~(1u << _speakerPerson);

	for (uint who = 0; who != 16; who++) {
		if (here & (1 << who))
			Lay::down(list, (byte)who, from);
	}

	for (uint more = 1; more < _state[0x476a]; more++)
		Lay::down(list, (byte)(15 + more), from);

	return list;
}

/** ACD's ATTACK.HSQ backdrop. The B45 actor/palette callback is separate. */
void DuneEngine::drawBattlefieldBackdrop() {
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	openSpriteSheet("ATTACK.HSQ");
	// C370 tiles sprites 2/3 into DS:1582/158A, then C21B draws the
	// landscape and foreground from DS:11DD. These are not room SAL assets.
	for (int x = 0; x < kScreenWidth; x += 40) {
		drawSprite(2, x, 0);
		drawSprite(3, x, 81);
	}
	const uint place = READ_LE_UINT16(_state.begin() + 0x114e);
	const byte kind = _state[place + 8];
	const uint landscape = kind < 0x20 ? 0x2f : (kind < 0x28 || kind == 0x30) ? 0x30 : 0x33;
	drawSprite(landscape, 0, 76);
	drawSprite(1, 0, 134);
}

void DuneEngine::drawBattlefield(bool present, bool restart) {
	startBattleAnimation(restart);
	drawBattlefieldBackdrop();
	if (!restart)
		drawBattleActors(); // ACD presents the fresh backdrop before B45 draws the retained actors.
	_sceneMarkers.clear(); // No ordinary room figures/parked orni underneath.
	if (_speakerPerson >= 0) {
		drawCurrentSpeakerFigure();
		drawSpeakerReport();
	}
	if (!_onWorld)
		updateCompass();
	drawPanel();
	for (int y = 0; y < kPortraitHeight; y++)
		memcpy(_portraitBox + kPortraitWidth * y,
			_framebuffer1 + kScreenWidth * (kPortraitTop + y) + kPortraitLeft, kPortraitWidth);
	drawSpeaker();
	drawMenu();
	if (!_speech.empty())
		drawSpeech(_speech, kSpeechColour);
	finishLocalSpeakerScene();
	applyPalette(128, 28, _battlePalette);
	if (present) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** 2DB1/2DDA: re-enter an ongoing battle after rebuilding its room. */
void DuneEngine::restoreBattlefield(bool present) {
	_state[0x47a6] = 0xff;
	_state[0x47a4] = _state[0x4732] = _state[0x46df] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x47aa, 0);
	_state[0x11bc] |= 1; // 1F64 suppresses the next eligible raid check.
	drawBattlefield(present, true);
}

void DuneEngine::drawScene(bool present) {
	if (!_inDesert && battleHere() && (_pendingBoardingPlace < 0 || _state[0x2b])) {
		drawBattlefield(present);
		return;
	}
	stopBattleAnimation();
	_graphics.setFrameBuffer(_framebuffer1);

	// The room is drawn onto a page of its own in the original and everything
	// on it is painted, so nothing of what was up before it can show through.
	// The panel below is painted again further down, so the whole of the
	// screen is cleared and not the room's part of it alone - the steps of the
	// intro that lay one thing over another reach into the panel too, and
	// their leavings stayed in its boxes.
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);

	// A room that looks out of doors stands against the sky, and takes the
	// hour's own colours with it.  0x39F8 knows the two of them by the word at
	// DS:0x4: the palace's balcony, which needs colours for its architecture
	// as well as for the sky, and the one room of the tenth kind of place,
	// which needs only the sky's.
	const uint16 where = _state.size() > 5 ? READ_LE_UINT16(_state.begin() + 4) : 0;
	const byte kindOfPlace = (byte)(where >> 8);
	const byte room = (byte)(where & 0xff);
	// 2E0A/388D: every ordinary room render starts indoors; drawing the
	// sky changes the scene-transition flag. Do not retain the last desert's
	// flag when entering an interior, including a scripted meeting room.
	_state[0x46df] = 0;
	_state[0x22e3] = 1; // 39EC resets the bank selector even for rooms without sky.

	if (where == 0x2005 || where == 0x1005) {
		_state[0x46df] = 1;
		applySkyPalette(where == 0x2005);
		drawSky();
	} else if (room == 1) {
		_state[0x46df] = 1;
		// The first room of a place is the place seen from outside: 0x380C
		// takes the hour's sky and 0x3847 draws the whole picture over it.
		// 3829 uses view index zero indoors. For a village that index is
		// VIL1..VIL6 according to the place's type, not always VIL6.
		applySkyPalette(true);
		drawSky();

		const uint16 lastPlace = _state.size() > 0x1151
			? READ_LE_UINT16(_state.begin() + 0x1150) : 0;
		const byte placeType = lastPlace >= 0x100 && lastPlace < _state.size()
			? _state[lastPlace] : 0xff;
		const byte outsideKind = lastPlace >= 0x100 && (uint)lastPlace + 8 < _state.size()
			? _state[lastPlace + 8] : kindOfPlace;
		const char *outside = outsidePicture(outsideKind, 0, placeType);
		if (outside) {
			openSpriteSheet(outside, false);
			drawSprite(0, 0, 0);
		}

	}

	// The picture the room stands on comes next: it carries the colours the
	// rest of the room is drawn in.
	if (!_background.empty()) {
		openSpriteSheet(_background.c_str());
		drawSprite(0, 0, 0);
	}

	// 0x3B68 loads no bank at all where the high nibble of the place is nought:
	// that is the empty room, which stands on a picture and has nothing of its
	// own to draw.  Loading GENERIC.HSQ for it took its palette as well, which
	// put the panel's own colours over the ones the sky had just laid down.
	const uint bank = (_place >> 4) & 0xf;
	if (bank != 0 && kBanks[bank])
		openSpriteSheet(kBanks[bank]);

	Resource sceneFile = _archive.openMember(_salFile);
	if (!sceneFile) {
		warning("Dune: the scene file %s is missing", _salFile.c_str());
		return;
	}
	Room palace(this, sceneFile);
	_sceneMarkers.clear();
	// Ordinary redraws rebuild the physical presence list, but are not new
	// 2318 merchant visits. Close-ups and scripts retain their caller context.
	if (_introPage.empty() && _speakerPerson < 0 && !(_state[0x47a4] & 0x80) && !_state[0x4774])
		peopleHere(false);
	// Keep a selected close-up's corner; ordinary room entry clears all marks.
	if (_speakerPerson < 0 && !(_state[0x47a4] & 0x80))
		memset(_state.begin() + 0x47f8, 0xff, 23 * 4);

	_scene = _place & 0xf;
	if (_scene < palace.count())
		palace.draw(_framebuffer1, _scene, true);

	// 3A21 draws the room and its people before 3A51 draws the parked
	// ornithopters. The aircraft cover only the parts of a figure they overlap.
	if (room == 1)
		drawOrnithoptersHere(kindOfPlace);
	// C412's retained room precedes the magnification and speaking figure.
	// Panel rows152..159 are appended once their clean base is drawn below.
	for (uint y = 0; y < 15; y++)
		memcpy(_roomPanelFaceBacking + y * 20,
				_framebuffer1 + (137 + y) * kScreenWidth + 150, 20);

	// Whoever is speaking stands over the room.  A speaker is one of the
	// seventeen the dialogue is divided between, eight parts to each, and
	// their bank is their own number and two.
	if (_speakerPerson >= 0) {
		// 0978/099D retain the full intro room; 3B03 enlarges gameplay close-ups.
		if (_introPage.empty())
			magnifyRoomAroundSpeaker();
		drawCurrentSpeakerFigure();
		drawSpeakerReport();
	}
	// CS:1349/138C's opcode 06 draws Leto through 978E, then 9B8B freezes
	// that initial portrait before opcode08 waits. It is not a spoken
	// close-up: keep the full room, its explicit roster and no caption or
	// 99BE callback. All four LETO idle tables start with parts 2/3/4, so
	// the frozen first frame is independent of 9908's random idle choice.
	// The persisted script cursor retains this card across redraw/load.
	if (_speakerPerson < 0 && _menuKind == kMenuPalaceGathering && _state[0x4774]) {
		const uint16 cursor = READ_LE_UINT16(_state.begin() + 0x477a);
		if (cursor == 0x134c || cursor == 0x138f)
			drawSpeakerFigure(0, 0);
	}
	// CS:12C7..12D9 invokes 0F13 before the final silent acknowledgement.
	if (_menuKind == kMenuPalaceGathering && !_menuParts.empty() &&
			_menuParts[0] == 518 && storyIsOver()) {
		if (!_endingCoupleActive)
			prepareEndingCouple();
		drawEndingCouple(0);
	}
	// Intro loaders call 37B2/978E, without gameplay's panel/menu pass.
	if (!_introPage.empty()) {
		if (present) {
			copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
		return;
	}
	// Stilgar's and Chani's epidemic directions carry action 13, the same
	// A2B0 remote-map inset used by the automatic Fremen reports. Keep it with
	// the selected line across redraws and save/load; A2A0 suppresses it when
	// the report is local or the dialogue is in display mode one.
	if (_speakerPerson >= 0 && _speakerPerson == (int)(_dialoguePart / 8) &&
		(_dialoguePart == 40 || _dialoguePart == 56) &&
		_dialoguePart < _dialogue.count() &&
		_dialogueLine < _dialogue.entry(_dialoguePart).size() &&
		(_dialogue.entry(_dialoguePart)[_dialogueLine].flags & 0x0f) == 13 &&
		_state[0x28e7] != 1) {
		const uint16 report = READ_LE_UINT16(_state.begin() + 0x47e6);
		if (report != READ_LE_UINT16(_state.begin() + 0x114e) &&
			report >= 0x100 && (report - 0x100) % 28 == 0 &&
			(uint)(report - 0x100) / 28 < _map.places().size())
			drawTroopNoticeMap(report);
	}

	// The ways out of the room the player has come to, before the panel that
	// shows them is drawn.
	if (isStageEventMenu()) {
		// A scripted room frame has no navigation, even when its return
		// context is the map. Do not retain that map's active compass.
		for (int i = 0; i < kCompassElements; i++)
			_panel[kFirstCompassElement + i].flags |= 0x20;
	} else if (!_onWorld)
		updateCompass();

	_retainRoomPanelFace = true;
	drawPanel();

	// Keep the empty box, so that a mouth can be drawn over it again and
	// again without the last one showing through.
	for (int y = 0; y != kPortraitHeight; y++)
		memcpy(_portraitBox + kPortraitWidth * y,
			   _framebuffer1 + 320 * (kPortraitTop + y) + kPortraitLeft, kPortraitWidth);

	drawSpeaker();

	// What there is to choose from goes in the panel's middle box, and what is
	// being said goes over the picture.
	drawMenu();

	if (!_speech.empty())
		drawSpeech(_speech, kSpeechColour);
	finishLocalSpeakerScene();
	// 2E29 publishes the newly drawn scene before resident requests can
	// start an event. 1784 then saves this flag alongside the return room.
	_state[0x46e0] = _state[0x46df];

	if (present) {
		finishPanelMenuTransition();
		_system->copyRectToScreen(_framebuffer1, 320, 0, 0, 320, 200);
		_system->updateScreen();
	}
}

/** 3D12/3D2F: paint one SAL marker inline, before later room geometry. */
void DuneEngine::drawStandingPerson(byte present, uint index, uint16 x, uint16 y,
		uint16 flags, byte palOffset) {
	const Room::Marker mark = { x, y, palOffset, flags };
	const uint16 where = READ_LE_UINT16(_state.begin() + 4);
	const byte room = (byte)where;
	const byte kindOfPlace = (byte)(where >> 8);
	if (present == 0xff)
		return;

	// 3D9C supplies an explicit event cast to 3D2F. Ordinary follower
	// hiding must not discard figures selected by that counted CS roster.
	const bool eventRoster = _state[0x4774] && READ_LE_UINT16(_state.begin() + 0x4778);
	const bool ordinaryFollower = !eventRoster && isFollowing(present);

	// The palace keeps travelling companions in the two portrait boxes and
	// in its TALK TO list, but not on its formal room floor.  The stage-48
	// throne-room recording, for example, shows Duncan and Harah standing
	// while Stilgar remains available solely through the panel. An active
	// event's counted CS roster overrides this ordinary-room rule: 3D9C
	// includes Stilgar at 1374/1389 even while he follows Paul, and 3D2F
	// paints that explicit figure without consulting the following party.
	// Sietches draw companions among their inhabitants too.
	if (ordinaryFollower && kindOfPlace == 0x20) {
		rememberWhereTheyStood(present, mark.x, mark.y);
		return;
	}

	// Followers have boarded the selected machine before its takeoff begins.
	// Leaving them on their outside-room marks made Gurney remain standing
	// through the transparent parts of the wing while the orni flew away;
	// the reference keeps him fully behind/inside it and he is not visible.
	if (_takingOff && ordinaryFollower)
		return;

	// The third sietch room is the empty passage between its inhabited hall
	// and the outside.  Travelling companions remain named in the panel while
	// Paul crosses it, but the DOS game does not put their standing figures on
	// its otherwise-unused marks.  Drawing them there made Stilgar and Chani
	// suddenly overtake Paul on the way to the waiting ornithopter.
	if (ordinaryFollower && kindOfPlace < 0x20 && room == 3) {
		rememberWhereTheyStood(present, mark.x, mark.y);
		return;
	}

	// Celimyn-Timin's elder receives Paul alone. Stilgar and Chani remain
	// available through their panel portraits, but the reference room keeps
	// both followers off the floor while this local Fremen is introduced.
	if (ordinaryFollower && kindOfPlace == 0x0f && room == 2) {
		rememberWhereTheyStood(present, mark.x, mark.y);
		return;
	}

	// 3D3C stores the SAL mark before 127C lowers Gurney's lying pose.
	// The later close-up must magnify that original room corner, not the
	// displaced sprite corner. Chief roster identities have distinct slots.
	rememberWhereTheyStood(present, mark.x, mark.y);

	// Dialogue has one identity for a Fremen and one for every chief, but
	// PERS.HSQ has three visibly different Fremen figures.  Pick the same
	// modulo-three variant here that drawSpeakerFigure later picks from the
	// troop, and remember which chief a pressed figure represents.
	// 3D51 derives the troop slot from the roster identity, not from the
	// order in which SAL marks are painted. Marks consume the roster in
	// reverse, so counting painted chiefs swaps their figures, hit targets
	// and retained close-up corners even in a two-chief room.
	const byte selectedChief = present >= 15 ? present - 15 : 0xff;
	const byte who = present >= 15 ? 15 : present;
	if (who == 15)
		_state[0x476c] = selectedChief; // 3D53 retains the last painted slot.
	int pictured = (who == 14 || who == 15)
		? fremenPicture(who, selectedChief == 0xff ? 0 : selectedChief) : who;
	int standY = mark.y;

	// 0x127C replaces Gurney's ordinary PERS.HSQ figure with sprite 34
	// while the story is from 0x15 through 0x1F.  That sprite is Gurney
	// lying on the floor of the palace's hidden room; BX, the figure's Y,
	// is advanced by 0x35 so that the horizontal body rests at the foot of
	// the same standing mark.  At stage 0x20 Jessica has got him upright
	// and the ordinary figure is used again.
	if (who == 4 && storyStage() >= 0x15 && storyStage() < 0x20) {
		pictured = 17;
		standY += 0x35;
	}
	// 3D65/3D70 selects the captain's alternate PERS figure when 31CF
	// copied his troop's stopped bit into the representative's flags.
	// Keep his dialogue identity; both drawing and bounds use this pose.
	if (who == 12 && (_state[0x10a7] & 0x10))
		pictured = 18;

	const Standing person = { Common::Point(mark.x, mark.y), who, selectedChief };

	debugC(2, kDebugGraphics, "%s stands on mark %u at %d,%d",
		   command(kFirstPerson + who).c_str(), index, mark.x, standY);

	// PERS.HSQ keeps a figure and then a stub of four by three pixels, so
	// the figure of the person numbered n is the sprite numbered twice n.
	//
	// A mark carries how the figure standing on it is to be drawn as well
	// as where: the scene keeps the size in bits 10 to 12 of the mark's
	// own word and which way it faces in bit 14, so that whoever stands at
	// the back of a room is drawn smaller than whoever stands at the
	// front.  Drawing every one of them at full size and facing the same
	// way is what made the people of a room too large.
	drawSprite(2 * pictured, mark.x, standY, mark.flags & ~0x0200, mark.palOffset);

	// 0x3D77 draws the figure and then counts the sprite up by one and
	// draws that over it as well.  For everybody the second sprite is the
	// empty four-by-three stub, but the third of the three Fremen is kept
	// in two pieces - PERS' thirty-third holds nothing but his head and
	// hands, his body being the thirty-second - so leaving the second
	// draw out stood him in the room with no head.
	drawSprite(2 * pictured + 1, mark.x, standY, mark.flags & ~0x0200,
			   mark.palOffset);

	_sceneMarkers.push_back(person);
}

enum {
	kEdge = 24
};

/**
 * Where in the room a person was standing, kept so that speaking to them can
 * find it again.  0x3D3C writes the mark's own corner into two words at
 * DS:0x47F8 and DS:0x47FA, four bytes to a person, as the room is laid out;
 * 0x3B1B reads them back when that person is taken up to be spoken to.
 */
void DuneEngine::rememberWhereTheyStood(uint who, int x, int y) {
	enum { kWhereTheyStood = 0x47f8, kPerPerson = 4 };

	const uint32 at = kWhereTheyStood + kPerPerson * who;

	if (at + 4 > _state.size())
		return;

	WRITE_LE_UINT16(_state.begin() + at, (uint16)x);
	WRITE_LE_UINT16(_state.begin() + at + 2, (uint16)y);
}

/**
 * The room behind whoever is being spoken to, which is not the room as it
 * stands.  0x3B03 draws it with the floor emptied and then blows a corner of
 * it up: the mark the speaker had been standing on is taken out of the words
 * 0x3D3C kept, held to 240 across and 113 down so the corner has a whole
 * window below and right of it, and the far routine through SS:0x3949 makes
 * the eighty by thirty-eight there fill the whole three hundred and twenty by
 * a hundred and fifty-two - four times over, each way.
 *
 * That overlay is not read, but the four falls straight out of the numbers:
 * 240 and eighty across come to the full width, and the recording bears it
 * out.  Duke Leto stands on the eighth mark of the palace, at 186 and 53, and
 * the throne room behind him in the recording is the eighty by thirty-eight
 * from that very corner blown up four times - the door at the room's right
 * hand side filling the right of the screen.
 *
 * Without it the room was drawn as it stands, so a speaker who should have
 * been close up against a wall stood in the middle of a whole room.
 */
void DuneEngine::magnifyRoomAroundSpeaker() {
	enum {
		kWhereTheyStood = 0x47f8, kPerPerson = 4,
		kZoom = 4, kFurthestAcross = 0xf0, kFurthestDown = 0x71,
		kWindowWide = kScreenWidth / kZoom, kWindowHigh = kSceneHeight / kZoom
	};

	if (_speakerPerson < 0)
		return;

	// 0x3B03 turns back where DS:0x47A4 already carries 0x80, because the
	// original draws the room once and keeps the blown-up page it made.  This
	// engine builds the room again for every drawing, so turning back on that
	// bit left the room at its own size behind every speaker. The original's test
	// stands for here is that the corner is only blown up once per drawing,
	// which it is.

	// A chief is one of eight, and DS:0x476C says which, so they have a
	// corner apiece past the fifteenth.
	uint who = (uint)_speakerPerson;

	if (who == 15 && _state.size() > 0x476c)
		who += _state[0x476c];

	const uint32 at = kWhereTheyStood + kPerPerson * who;

	if (at + 4 > _state.size())
		return;

	const int16 stoodAt = (int16)READ_LE_UINT16(_state.begin() + at);

	// 0x3B1F lets a corner that has gone negative be: there is nothing to
	// blow up for somebody who was never stood anywhere.
	if (stoodAt < 0)
		return;

	const int left = MIN<int>(stoodAt, kFurthestAcross);
	const int top = MIN<int>((int16)READ_LE_UINT16(_state.begin() + at + 2), kFurthestDown);

	if (top < 0)
		return;

	// The window is taken out first, because it is blown up over itself.
	byte window[kWindowWide * kWindowHigh];

	for (int y = 0; y != kWindowHigh; y++)
		memcpy(window + y * kWindowWide,
			   _framebuffer1 + (top + y) * kScreenWidth + left, kWindowWide);

	for (int y = 0; y != kSceneHeight; y++) {
		const byte *from = window + (y / kZoom) * kWindowWide;
		byte *to = _framebuffer1 + y * kScreenWidth;

		for (int x = 0; x != kScreenWidth; x++)
			to[x] = from[x / kZoom];
	}

	debugC(2, kDebugGraphics, "the room is blown up about %d,%d, where %s stood",
		   left, top, command(kFirstPerson + (uint)_speakerPerson).c_str());
}

/**
 * Which pointer belongs over a given spot.  The original turns the pointer
 * into an arrow at the edges of the view, to say which way the room can be
 * left, and into a hand over what can be looked at or spoken to.
 */
/**
 * Which of the people standing in the room is under a spot, or -1.
 */
int DuneEngine::standingAt(const Common::Point &position) const {
	// Nobody is standing about while somebody is being spoken to: the figure
	// drawn over the room is theirs and the marks belong to the room behind
	// it.
	if (_speakerPerson >= 0 || position.y >= kSceneHeight)
		return -1;

	// 9285 scans the saved corners in person order. Each corner excludes
	// its own X and Y and includes the next 32 columns and 80 rows.
	int selected = -1;
	uint lowestIdentity = 23;
	for (uint i = 0; i != _sceneMarkers.size(); i++) {
		const Standing &person = _sceneMarkers[i];
		const int dx = position.x - person.at.x;
		const int dy = position.y - person.at.y;
		const uint identity = person.who + (person.chief == 0xff ? 0 : person.chief);
		if (person.at.x >= 0 && dx > 0 && dx <= 32 && dy > 0 && dy <= 80 && identity < lowestIdentity) {
			selected = i;
			lowestIdentity = identity;
		}
	}

	return selected;
}

/**
 * Which of the active compass's four arrows was pressed, or -1. D72D writes
 * a different six-entry template over the panel for room, world and flight;
 * testing the immutable room rectangles misses the visible world-arrow edges.
 */
int DuneEngine::compassWayAt(const Common::Point &position) const {
	const int button = compassButtonAt(position);
	return button >= 1 && button <= 4 ? button - 1 : -1;
}

/** DC6A/D694: an arrow outside the viewport names a virtual compass button. */
int DuneEngine::worldMapScrollDirection(const Common::Point &position) const {
	if (!mapAnimationVisible() || _surveyDragging || hasSurveyMap())
		return -1;
	const byte *frame = _state.begin() + 0x46e3;
	const Common::Rect viewport(READ_LE_UINT16(frame), READ_LE_UINT16(frame + 2),
			READ_LE_UINT16(frame + 4), READ_LE_UINT16(frame + 6));
	return MAX(-1, (int)surveyMapControlAt(viewport, position));
}

/** D6B7: the current edge arrow and matching panel arrow name one control. */
int DuneEngine::mapPanDirection(const Common::Point &position) const {
	const bool compact = _inCabin || _menuKind == kMenuWormMap;
	if ((!compact && !mapAnimationVisible()) || _onGlobe || _onMixer || _surveyDragging)
		return -1;
	const int edge = compact ? cabinMapScrollDirection(position) :
		(hasSurveyMap() ? moveMapScrollDirection(position) : worldMapScrollDirection(position));
	if (edge >= 0)
		return edge;
	const int way = compassWayAt(position);
	if (way >= 0 && (_panel[kFirstCompassElement + 1 + way].flags & 0x4020) == 0x4000)
		return way;
	return -1;
}

/** 432E/DC6A: the cabin registers DS:149C, not an enlarged hover area. */
int DuneEngine::cabinMapScrollDirection(const Common::Point &position) const {
	if ((!_inCabin && _menuKind != kMenuWormMap) || _onGlobe || _onMixer)
		return -1;
	return MAX(-1, (int)surveyMapControlAt(Common::Rect(81, 45, 241, 134), position));
}

static const Common::Rect kGlobeMapArea(96, 25, 224, 134); // DS:2448

/** B827/DC6A/D6B7: globe edges precede its strict six-entry compass. */
int DuneEngine::globeControlAt(const Common::Point &position) const {
	// The exit confirmation replaces the command rows but retains the globe compass.
	if (!_onGlobe || _onMixer || (_menuKind != kMenuGlobe && _menuKind != kMenuLeaving))
		return -1;
	const int edge = MAX(-1, (int)surveyMapControlAt(kGlobeMapArea, position));
	if (edge >= 0)
		return edge + 1;
	for (int i = 0; i < kCompassElements; i++) {
		const PanelElement &e = _panel[kFirstCompassElement + i];
		if ((e.flags & 0xa0) == 0x80 && position.x > e.x && position.x < e.x2 &&
				position.y > e.y && position.y <= e.y2)
			return i;
	}
	return -1;
}

/** D918/D8DA only repeat the four 4080 controls, never centre/return. */
int DuneEngine::repeatCompassDirection(const Common::Point &position) const {
	if (_state[0x4774])
		return -1; // D89B: a scripted click cannot start a held map/globe pan.
	if (!_onGlobe)
		return mapPanDirection(position);
	const int button = globeControlAt(position);
	return button >= 1 && button <= 4 &&
		(_panel[kFirstCompassElement + button].flags & 0x4020) == 0x4000 ? button - 1 : -1;
}

static const Common::Point kMapPointerDirections[] = {
	{0, -1}, {1, 0}, {0, 1}, {-1, 0}, {-1, -1}, {1, -1}, {1, 1}, {-1, 1}
};

int DuneEngine::mapPointerKey(Common::KeyCode key) {
	switch (key) {
	case Common::KEYCODE_UP: case Common::KEYCODE_KP8: return 0;
	case Common::KEYCODE_RIGHT: case Common::KEYCODE_KP6: return 1;
	case Common::KEYCODE_DOWN: case Common::KEYCODE_KP2: return 2;
	case Common::KEYCODE_LEFT: case Common::KEYCODE_KP4: return 3;
	case Common::KEYCODE_KP7: return 4;
	case Common::KEYCODE_KP9: return 5;
	case Common::KEYCODE_KP3: return 6;
	case Common::KEYCODE_KP1: return 7;
	default: return -1;
	}
}

bool DuneEngine::canMoveMapPointer() const {
	const byte mapMode = _state[0x46eb];
	const bool strategic = (mapMode & 0x80) && mapAnimationVisible();
	const bool compact = (mapMode & 1) && (_inCabin || _menuKind == kMenuWormMap) && !_onGlobe && !_onMixer;
	const bool globe = _onGlobe && !_onMixer && (_menuKind == kMenuGlobe || _menuKind == kMenuLeaving);
	return (strategic || compact || globe) && !_surveyDragging &&
			!_state[0x4774] && !_localRoomRequestPending && _pendingBoardingPlace < 0 &&
			_pendingRoomExit < 0 && _pendingRoomCompass < 0;
}

void DuneEngine::updateMapKeyboardPointer() {
	const uint16 ticks = _system->getMillis() / 5;
	if (!canMoveMapPointer()) {
		_mapPointerGlide.steps = 0;
		_mapPointerPendingButtons = 0;
		_mapPointerKeys = 0;
	}
	if (_mapPointerGlide.steps) {
		if (_mapPointerGlide.advance(ticks))
			_system->warpMouse(_mapPointerGlide.position.x, _mapPointerGlide.position.y);
		return;
	}
	Common::Point direction;
	for (uint i = 0; i < ARRAYSIZE(kMapPointerDirections); ++i)
		if (_mapPointerKeys & (1 << i))
			direction += kMapPointerDirections[i];
	if (direction == Common::Point()) {
		_mapPointerDistance = _mapPointerFraction = Common::Point();
		return;
	}
	const Common::Point from = _system->getEventManager()->getMousePos();
	if (_mapPointerControl) {
		const byte elapsed = ticks - _mapPointerTick;
		_mapPointerTick = ticks;
		auto move = [elapsed](int axis, int16 &distance, int16 &fraction, uint16 magnitude, int a, int b, int c) {
			const int scale = 1 << ((magnitude >= a) + (magnitude >= b) + (magnitude >= c));
			const int motion = ABS(axis) * scale * MIN<int>(elapsed, 8) + fraction;
			fraction = motion % 8;
			// E243 sign-extends the motion byte before its logical shifts.
			const int pixels = uint16(int8(motion)) >> 3;
			const int delta = axis < 0 ? -pixels : pixels;
			distance += delta;
			return delta;
		};
		// E1F7 uses the vertical key direction to sign the accumulated
		// distance, so reversing mid-motion selects the highest speed.
		const uint16 verticalMagnitude = direction.y < 0 ? -_mapPointerDistance.y : _mapPointerDistance.y;
		const int dx = move(direction.x, _mapPointerDistance.x, _mapPointerFraction.x, ABS(_mapPointerDistance.x), 4, 12, 36);
		const int dy = move(direction.y, _mapPointerDistance.y, _mapPointerFraction.y, verticalMagnitude, 3, 10, 28);
		if (dx || dy)
			_system->warpMouse(CLIP<int>(from.x + dx, 0, kScreenWidth - 1),
					CLIP<int>(from.y + dy, 0, kScreenHeight - 1));
		return;
	}
	direction.x = CLIP<int>(direction.x, -1, 1);
	direction.y = CLIP<int>(direction.y, -1, 1);
	MapPointerTarget target;
	const byte *data = _state.begin();
	for (int i = 0; i < kFirstCompassElement + kCompassElements; ++i) {
		const PanelElement &element = _panel[i];
		// D7B2 replaces the first four panel flags on the globe.
		const uint16 flags = _onGlobe && i < 4
				? READ_LE_UINT16(data + 0x1c46 + 4 * i) : element.flags;
		if (!(flags & 0x80))
			continue;
		if (i >= 7 && i < 7 + kMenuRows) {
			const int row = menuRowAt(Common::Point(element.x + 1, element.y + 1));
			if (row < 0 || (uint(row) < _menuFlags.size() && (_menuFlags[row] & 0x4000)))
				continue;
		}
		target.consider(from, direction, Common::Rect(element.x, element.y, element.x2, element.y2));
	}
	if (data[0x46eb] & 0x80) {
		const uint actorCount = MIN<uint>(READ_LE_UINT16(data + kSpriteActorCount), kMaxSpriteActors);
		for (uint i = 0; i < actorCount; ++i) {
			const byte *actor = data + kSpriteActorPool + kSpriteActorSize * i;
			if (!(actor[12] & 0xc0))
				target.consider(from, direction, spriteActorBounds(actor));
		}
	}
	if (data[0x46eb]) { // E0DB visits sites only when a flat map is registered.
		for (uint at = 0xa5c0; at + 6 <= _state.size() && READ_LE_UINT16(data + at); at += 6)
			target.consider(from, direction, Common::Point(READ_LE_INT16(data + at + 2), data[at + 4]));
	}
	// E11C adds eight open-sand targets while Paul's marker is in view.
	if ((data[0x46eb] & 1) && READ_LE_UINT16(data + 0x4749)) {
		for (uint at = 0x2462; at < 0x2482; at += 4)
			target.consider(from, direction, Common::Point(READ_LE_INT16(data + at), READ_LE_INT16(data + at + 2)));
	}
	if (target.score < 0x8000) {
		_mapPointerGlide.start(from, target.point, ticks);
		// D9C7 clears the sampled buttons during the glide. A hardware
		// button already held can make a new edge when DF1E resumes.
		_mapPointerPendingButtons = _system->getEventManager()->getButtonState() &
				(Common::EventManager::LBUTTON | Common::EventManager::RBUTTON);
		_pointerClickKeys &= ~kPointerClickSpace; // E043 clears the Space scan latch.
		if (_pointerClickKeys)
			_mapPointerPendingButtons |= Common::EventManager::LBUTTON;
	}
}

Cursor DuneEngine::cursorForPosition(const Common::Point &position) const {
	if (showingDefeat() || _menuKind == kMenuTroopNotice || _menuKind == kMenuBook)
		return Cursor::Pointer;
	if (_onMixer)
		return Cursor::Pointer;
	if (_onGlobe) {
		const int button = globeControlAt(position);
		// Panel arrows keep the ordinary cursor; only DC6A's virtual edges
		// change its artwork. Never forward hovers to the flat map below.
		if (position.y < 155 && button >= 1 && button <= 4) {
			static const Cursor kWays[] = { Cursor::North, Cursor::East, Cursor::South, Cursor::West };
			return kWays[button - 1];
		}
		// BC1F/BC4E's hand is rectangular, even over an empty oval corner.
		if (_menuKind == kMenuGlobe && kGlobeMapArea.contains(position))
			return Cursor::Hand;
		return Cursor::Pointer;
	}
	switch (worldMapScrollDirection(position)) {
	case 0: return Cursor::North;
	case 1: return Cursor::East;
	case 2: return Cursor::South;
	case 3: return Cursor::West;
	default: break;
	}
	if (_onWorld && !_onGlobe && !_inCabin && !_onMixer && hasSurveyMap()) {
		if (_surveyDragging)
			return Cursor::Pointer;
		switch (surveyMapControlAt(surveyFrameRect(), position)) {
		case kSurveyHand: return Cursor::Hand;
		case kSurveyNorth: return Cursor::North;
		case kSurveyEast: return Cursor::East;
		case kSurveySouth: return Cursor::South;
		case kSurveyWest: return Cursor::West;
		default: return Cursor::Pointer;
		}
	}
	// DC6A's y=155 cutoff also permits the cabin's south arrow at 152..154.
	switch (cabinMapScrollDirection(position)) {
	case 0: return Cursor::North;
	case 1: return Cursor::East;
	case 2: return Cursor::South;
	case 3: return Cursor::West;
	default: break;
	}
	if (position.y >= kSceneHeight)
		return Cursor::Pointer;

	// DC6A/D6FE selects the hand across the destination frame, including open
	// sand. The virtual pan controls above already handle the frame edges.
	if (_inCabin || _menuKind == kMenuWormMap) {
		int across = 0, down = 0;
		return cabinPointsAt(position, &across, &down) ? Cursor::Hand : Cursor::Pointer;
	}

	// DC6A/D6FE chooses the hand throughout the registered map frame.
	// Actor/site reach and report/equipment occlusion govern clicks, not the
	// cursor: even empty sand and a covered or unreachable actor use the hand.
	if (_onWorld) {
		const uint frameAt = READ_LE_UINT16(_state.begin() + 0xdc58);
		if (frameAt && frameAt + 8 <= _state.size()) {
			const byte *frame = _state.begin() + frameAt;
			const Common::Rect viewport(READ_LE_INT16(frame), READ_LE_INT16(frame + 2),
					READ_LE_INT16(frame + 4), READ_LE_INT16(frame + 6));
			if (surveyMapControlAt(viewport, position) == kSurveyHand)
				return Cursor::Hand;
		}
		return Cursor::Pointer;
	}

	// Ordinary dialogue has no registered DC58 viewport (DAA3/DC6A).
	// A scene click continues the paragraph, not an exit behind the portrait.
	if (_speakerPerson >= 0)
		return Cursor::Pointer;

	// Local battles have no DC58 navigation viewport: DC6A retains the plain
	// pointer, just as 2FFB removes the room exits from the compass.
	if (!_inDesert && battleHere())
		return Cursor::Pointer;

	// DC6A has no registered viewport in an uncarried desert. Its scene
	// keeps the plain pointer; 4080's compass controls own walking. Treating
	// these edges as room exits also moved Paul on clicks DOS ignores.
	const bool onFootInDesert = _inDesert && _flyingTo < 0 &&
		(_state[0x11c9] & 0x0f) == 0;
	if (!onFootInDesert) {
		if (position.x < kEdge)
			return Cursor::West;
		if (position.x >= kScreenWidth - kEdge)
			return Cursor::East;
		if (position.y < kEdge)
			return Cursor::North;
		if (position.y >= kSceneHeight - kEdge)
			return Cursor::South;
	}

	// The marks of the scene are where the people in the room stand, so they
	// are what there is to speak to.
	if (standingAt(position) >= 0)
		return Cursor::Hand;

	return Cursor::Pointer;
}

/**
 * Reads whichever of a person's parts of the dialogue has something to say.
 * The game keeps four parts to a person, reached through a table of pointers
 * of their own, and a figure of PERS.HSQ stands for two of the sprites in it,
 * so who is being spoken to is the figure halved.
 */
/**
 * Takes up what a person has to say.  0x96F1 is handed the person's own
 * number and works a part of the dialogue out of it as (n << 3) | 4, so the
 * dialogue is seventeen groups of eight parts: sixteen people and one group of
 * its own for what may be done anywhere.  Reading four parts to a person, as
 * this used to, took one person's lines out of another's group.
 */
void DuneEngine::talkTo(uint figure) {
	const uint person = figure / 2;

	// The parts of the person's own group that have something to say become
	// the list to choose from, the one talking starts at first.
	Common::Array<Common::String> choices;
	_menuParts.clear();

	for (uint i = 0; i != 8; i++) {
		const uint part = person * 8 + ((4 + i) & 7);
		if (part >= _dialogue.count() || _dialogue.entry(part).empty())
			continue;

		const Common::Array<Dialogue::Record> &lines = _dialogue.entry(part);
		for (uint line = 0; line != lines.size(); line++) {
			if (!dialogueRecordAvailable(lines[line], 0x80) ||
				!conditionHolds(lines[line].condition))
				continue;

			choices.push_back(fillInLine(phraseOf(person, lines[line].line)));
			_menuParts.push_back(part * 1000 + line);
			break;
		}
	}

	if (choices.empty()) {
		debugC(1, kDebugScript, "figure %u has nothing to say", figure);
		return;
	}

	_menuKind = kMenuLines;

	if (choices.size() == 1) {
		chooseFromMenu(0);
		return;
	}

	debugC(1, kDebugScript, "figure %u has %u things to say", figure, choices.size());
	showMenu(choices);
}

/**
 * Puts a list in the box the panel keeps for what is said, a choice to a row.
 */
/**
 * Who is waiting to speak to Paul.  The story leaves them in the queue at
 * DS:0x1179, and the low six bits of each word name them: the walk at 0x2864
 * adds 0x78 to it, which is where the command file keeps its people, from
 * Jessica through the Fremen chiefs.
 */
void DuneEngine::showWaiting() {
	const uint32 queue = 0x1179;
	const byte waiting = _state.size() > 0xc8 ? _state[0xc8] : 0;

	Common::Array<Common::String> choices;
	_menuParts.clear();

	for (uint i = 0; i != MIN<uint>(waiting, 10); i++) {
		const uint16 what = READ_LE_UINT16(_state.begin() + queue + 2 * i);

		choices.push_back(command(kFirstPerson + (what & 0x3f)));
		_menuParts.push_back(i);
	}

	// Nobody waiting used to leave the list as it was, so the row a press fell
	// on still stood for whoever had been on it; there is nothing to choose
	// from, so the room's own list goes back up.
	if (choices.empty()) {
		showRoomMenu();
		return;
	}

	_menuKind = kMenuWaiting;
	showMenu(choices);
}

/**
 * Takes up with whoever was chosen out of the queue: their part of the
 * dialogue is read, and they are struck off what is waiting.
 */
void DuneEngine::takeUpWaiting(uint which) {
	const uint32 queue = 0x1179;
	const byte waiting = _state.size() > 0xc8 ? _state[0xc8] : 0;

	if (which >= waiting)
		return;

	const uint16 what = READ_LE_UINT16(_state.begin() + queue + 2 * which);

	for (uint i = which; i + 1 < waiting; i++) {
		WRITE_LE_UINT16(_state.begin() + queue + 2 * i,
						READ_LE_UINT16(_state.begin() + queue + 2 * (i + 1)));
	}

	_state[0xc8] = waiting - 1;
	_menu.clear();

	debugC(1, kDebugScript, "%s is taken up with", command(0x78 + (what & 0x3f)).c_str());

	// Their part of the dialogue is the one their number names, four parts to
	// a person as everywhere else.
	talkTo((what & 0x3f) * 2);
}

/** 1EBE: room-chief selection or map-report release can advance the cure chapter. */
void DuneEngine::checkTroopCure(uint troop) {
	const uint32 record = 0x8aa + troop * 27;
	if (troop < _map.troops().size() && record + 27 <= _state.size() &&
			(READ_LE_UINT16(_state.begin() + record + 0x12) & 0x0800) &&
			storyStage() >= 96 && storyStage() < 100)
		advanceStory(100);
}

/** Diagnostic list for the `dune_orders` scripted fixture. In ordinary play
 * the map's CONTACT FREMEN TROOPS row goes through DOS 0x86CC and selects
 * an actor directly; it does not present this text list.
 */
void DuneEngine::showTroopsHere() {
	const Common::Array<Troop> &troops = _map.troops();

	Common::Array<Common::String> choices;
	_menuParts.clear();

	for (uint which = 0; which != troops.size(); which++) {
		if (!troops[which].isOurs())
			continue;

		choices.push_back(Common::String::format("%d of them, %s", troops[which].size * 10,
											 orderName(troops[which].order()).c_str()));
		debugC(2, kDebugScript, "  the list's row %u is troop %u",
			   (uint)_menuParts.size(), which);
		_menuParts.push_back(which);
	}

	// With nobody here to order about the list says so and nothing more, in
	// COMMAND1.HSQ's own words: a row that only names something is not a
	// choice, and the last row goes back to whatever the panel showed before.
	if (choices.empty()) {
		choices.push_back(command(kNoTroops));
		_menuParts.push_back(kGoBack);
		choices.push_back(command(kCancel));
		_menuParts.push_back(kGoBack);

		Common::Array<uint16> flags;
		flags.push_back(0x4000);
		flags.push_back(0);

		_menuKind = kMenuTroops;
		showMenu(choices, &flags);
		return;
	}

	choices.push_back(command(kCancel));
	_menuParts.push_back(kGoBack);

	_menuKind = kMenuTroops;
	showMenu(choices);
}

/**
 * Opens the map-side console used to speak to one rallied troop.  The DOS
 * game does not leave the occupation list over the chief's close-up: 0x8685
 * redraws ONMAP, selects the troop at DS:0x46EF and 0x7C02 writes its report
 * into the blue box at the top.  Keeping that transition also makes it clear
 * which troop an order applies to when several sietches are known.
 */
void DuneEngine::showTroopContact(uint troop, bool fromMap) {
	const Common::Array<Troop> &troops = _map.troops();
	// 8685/6906 tests only whether the troop has been met (state < 0x80).
	// A known non-Fremen figure must not be swallowed by the map click here.
	if (troop >= troops.size() || !troops[troop].isKnown())
		return;
	const bool retainMap = _onWorld && !_onGlobe && !_inCabin &&
			_mapAnimationActive && _mapUnitsLongitude == _worldLongitude &&
			_mapUnitsLatitude == _worldLatitude &&
			_mapCleanPage.size() == kScreenWidth * kScreenHeight;
	_state[0x46d8] = 1; // 8685 suppresses the old report's closing outline.
	if (READ_LE_UINT16(_state.begin() + 0x46ef))
		finishTroopContact(false, false); // 868A/868D remove the ring/report before MAP2.
	if (_menuKind == kMenuPlace)
		dismissMapInformation(); // 8690/5F79 rolls back to the map menu before 780A.
	const Common::Rect troopCard = mapTroopCard() >= 0 ? mapTroopCardBounds() : Common::Rect();
	const byte *placeBox = _state.begin() + 0x1668;
	const Common::Rect placeCard = _pressedPlace >= 0 ?
			Common::Rect(READ_LE_INT16(placeBox), READ_LE_INT16(placeBox + 2),
					READ_LE_INT16(placeBox + 4), READ_LE_INT16(placeBox + 6)) : Common::Rect();
	closeMapTroopCard();
	closeMapPlaceCard();
	if (retainMap) {
		const MenuKind previousMenu = _menuKind;
		_menuKind = kMenuMap;
		paintMapActorChange(placeCard); // 8690/8693 present only their released windows.
		paintMapActorChange(troopCard);
		_menuKind = previousMenu;
	}
	closeSurveyWindow(); // 8696/58FA follows the old chief/place/compact report.
	_mapWindowGeometryValid = false;

	_orderingTroop = troop;
	// 5A0D/8747 select before opening the report: even time spent speaking
	// the first line must not move this traveller out from under the player.
	_state[0x1954] = _state[0x8aa + 27 * troop];
	// 86A5 remembers every accepted contact, not only compact cards or
	// FIND PROSPECTORS. CUT CONTACT releases 1954 but 86CC reuses 1955.
	_state[0x1955] = _state[0x1954];
	WRITE_LE_UINT16(_state.begin() + 0x46ef, 0x8aa + 27 * troop);
	_mapRingTroop = -1;
	_pointedAtPlace = -1;
	_troopPortraitVisible.clear();
	_troopPortraitClean.clear();
	_troopPortraitKey = -1;
	_mapSpeechPerson = -1;
	// 8685 switches the selected troop without clearing 46F3. A map
	// click inside room-started orders must still return to that room.
	_troopOrdersFromMap = fromMap && !_state[0x46f3];
	_state[0x46f3] = _troopOrdersFromMap ? 0 : 1;
	_troopInfoPage = 0;

	// 8685/7C02 opens a remote report without 1EBE's cure/story check.
	// That check belongs to 7B58 when the report is released (or 93A2
	// when selecting a chief in a room). Running it here advances the
	// chapter and moves Chani before the player has even given an order.

	// 7BA3 retains the newly opened record in 46F1 before 79EE paints it.
	// For a Fremen face, 7A96 also replaces chief slot zero and selects it.
	// This is the map report's generic chief context, not the room's retained
	// slot selection; speech and subsequent chief lookups use this cache. The
	// room presence walk rebuilds the real roster when this contact closes.
	const uint record = 0x8aa + 27 * troop;
	WRITE_LE_UINT16(_state.begin() + 0x46f1, record);
	const uint16 place = READ_LE_UINT16(_state.begin() + record + 4);
	const bool validPlace = place >= 0x100 && (uint)place + 0xa < _state.size();
	const bool harkonnenPortrait = (_state[record + 3] & 0x20) && validPlace &&
		((_state[place + 0xa] & 2) ||
		 (_state[place + 8] >= 0x28 && !(_state[place + 0xa] & 8)));
	if (!harkonnenPortrait) {
		WRITE_LE_UINT16(_state.begin() + 0x4758, record);
		_state[0x476c] = 0;
	}
	prepareTroopContactContext(troop);

	// A contact opened from the room needs the same initial map positioning
	// as SEE DUNE MAP. A map-side contact must preserve the player's panning.
	if (!_onWorld)
		lookAtPlayer();
	_onWorld = true;
	_onGlobe = false;
	_inCabin = false;

	// 0x7C02 starts the chief's ordinary report table, even for established
	// troops. The first applicable job report precedes the personnel card.
	_state[0x19] = 0;
	_speechParts.clear();
	_speechPart = 0;
	_speech.clear();
	// 8685 does not call 5A9A/5D82: retain the current terrain and actor
	// phases while replacing contacts. A room entry still needs its map.
	const MenuKind previousMenu = _menuKind;
	_menuKind = kMenuMap;
	if (!retainMap) {
		closeMapTitle();
		showWorld(false);
	}
	// 8685 does not close 5BB0's legend. 79EE replaces its descriptor,
	// but the opening outlines run over the still-displayed legend first.
	// 8685/697C registers the ring before 780A rolls the list; C5CF does
	// not paint it. Its first paint belongs to the next actor callback.
	// Even a distant contact starts with 18FD; 7C24 changes its frame
	// pointers only after the window opens and the report checks reach.
	_state[0x4c] = 0;
	openSpriteSheet("ONMAP.HSQ", false);
	// 697C requires a registered actor, then projects this record for
	// the new ring. Do not rescan chains or restart neighbouring actors.
	if (troop < ARRAYSIZE(_mapUnits) && _mapUnits[troop].initialList) {
		const Common::Point at = troopOnScreen(troops[troop], troop);
		// 686E limits travelling actors. A stationary troop can stand
		// beyond that margin while its registered site remains visible.
		if (!(troops[troop].state & 0x40) || (at.x > -16 && at.y > -16 && at.x < 328 && at.y < 160)) {
			uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828);
			startMapRing(troop, at.x, at.y, seed);
			WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
		}
	}
	_menuKind = previousMenu;
	// 8685 installs 780A's list through D323 before 7C02/7BA3 opens
	// the report window. Paint only the panel here: a full contact redraw
	// would show the chief before the opening outlines have travelled out.
	_state[0x4c] = distanceFromPaul(READ_LE_UINT16(_state.begin() + record + 6),
			(int16)READ_LE_UINT16(_state.begin() + record + 8)) > READ_LE_UINT16(_state.begin() + 0x1176)
		? 0xff : 0;
	showTroopMenu(false, true);
	drawPanel();
	drawMenu();
	finishPanelMenuTransition();
	// 6917 retains the last tested actor when no match is found (3CAF's
	// scratch rectangle when the list is empty). Only a match changes the
	// report's half. This also covers contacts absent from a panned viewport.
	const byte *actor = _state.begin() + kSpriteActorPool - kSpriteActorSize;
	const uint count = (_state[0x46eb] & 0x80) ?
			MIN<uint>(READ_LE_UINT16(_state.begin() + kSpriteActorCount), kMaxSpriteActors) : 0;
	int top = READ_LE_UINT16(_state.begin() + 0x18eb) == 80 ? 80 : 5;
	for (uint entry = 0; entry < count; entry++) {
		actor += kSpriteActorSize;
		if (READ_LE_UINT16(actor + 10) == record && !(actor[12] & 0x40)) {
			top = READ_LE_INT16(actor + 2) < 76 ? 80 : 5;
			WRITE_LE_UINT16(_state.begin() + 0x4710, 92);
			WRITE_LE_UINT16(_state.begin() + 0x4712, top == 80 ? 14 : 30);
			break;
		}
	}
	const Common::Point anchor(READ_LE_INT16(actor), READ_LE_INT16(actor + 2));
	_troopContactTop = top;
	WRITE_LE_UINT16(_state.begin() + 0x18eb, top);
	WRITE_LE_UINT16(_state.begin() + 0x18ef, top + 67);
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0x18e9);
	_state[0x46d8] = 0;
	_troopWindowAnchor = anchor;
	_troopWindowBounds = Common::Rect(5, top, 232, top + 67);
	_troopWindowGeometryValid = true;
	animateTroopWindow();
	sayTroopInformation();
}

/**
 * Expands the remotely selected troop into the scratch bytes read by
 * CONDIT.HSQ.  0x31F6 does this for both a Fremen spoken to in a sietch and a
 * troop contacted through the map.  The first map console only synthesized
 * status text and never prepared this context, making story-bearing reports
 * such as the Habbanya-Timin troop's Stilgar clue impossible to reach.
 */
void DuneEngine::prepareTroopContactContext(uint troop) {
	enum { kTroops = 0x8aa, kTroopSize = 27 };
	const Common::Array<Troop> &troops = _map.troops();
	const uint32 record = kTroops + troop * kTroopSize;
	if (troop >= troops.size() || record + kTroopSize > _state.size())
		return;

	const uint16 tally = READ_LE_UINT16(_state.begin() + record + 0xe);
	const uint16 started = READ_LE_UINT16(_state.begin() + record + 0xa);
	const uint16 worked = (uint16)(hour() - started);
	uint16 producing = 0;
	// 0x329D only estimates production for an actively mining troop. 0x708A
	// updates its last yield and rise/fall flags even when queried between
	// hourly updates. Average production rounds to nearest, with ties down
	// and the doubled remainder wrapping as 32BA's 16-bit SHL does.
	if (_state[record + 3] == 0) {
		producing = spiceProduction(troop);
		const uint16 previous = READ_LE_UINT16(_state.begin() + record + 0xc);
		WRITE_LE_UINT16(_state.begin() + record + 0xc, producing);
		if (previous != producing)
			_state[record + 0x10] = (_state[record + 0x10] & 0xf3) |
				(previous < producing ? 8 : 4);
		uint16 average = 0;
		if (worked != 0)
			average = tally / worked + ((uint16)(2u * (tally % worked)) > worked ? 1 : 0);
		// 329D reaches 32C1 only for exact mining state zero. Other tasks
		// return through 32A9, retaining the previous shared average word.
		WRITE_LE_UINT16(_state.begin() + 0x4a, average);
	} else {
		_state[record + 0x10] &= 0xf3;
	}

	// 31F6 reads the place word, which already holds a traveller's target.
	const uint16 place = READ_LE_UINT16(_state.begin() + record + 4);
	if (place >= 0x100 && (uint32)place + 0x12 < _state.size()) {
		// 0x331E keeps which place it has expanded, so that a line's own action
		// can afterwards say something about it, and ends at 0x5274 with what
		// lies nearest to it.
		WRITE_LE_UINT16(_state.begin() + 0x11ce, place);
		nearestPlaces(place);
		_state[0x4d] = _state[place + 8];
		WRITE_LE_UINT16(_state.begin() + 0x4e,
						(uint16)((_state[place] << 8) | _state[place + 1]));
		_state[0x51] = _state[place + 0x0a];
		_state[0x52] = _state[place + 0x12];
		// Battle odds and sabotage flags belong to the contacted place, not
		// Paul's previous room. Travelling reinforcements are not here yet.
		gatherTroopStrengths(place);
		gatherTroopCounts(place);
		// 0x331E also expands the region, water and available equipment.
		// Order acknowledgements use these to warn about missing machinery,
		// worm risk and ecology prerequisites before considering ability.
		if ((uint32)place + 0x1b < _state.size()) {
			_state[0x50] = _state[0x1141 + _state[place + 1]];
			_state[0x54] = _state[place + 0x1b];
			_state[0x53] = 0;
			for (uint item = 0; item < 7; item++) {
				_state[0x55 + item] = _state[place + 0x14 + item];
				_state[0x46fe + item] = equipmentSpare(place, item);
				if (_state[0x46fe + item])
					_state[0x53] |= 0x80 >> item;
			}
		}
		// 0x3385 reports Gurney, Stilgar and Chani left at this place,
		// excluding the player's own last location.
		_state[0xf7] = 0;
		if (place != READ_LE_UINT16(_state.begin() + 0x1150)) {
			const uint16 standing = (1 + (place - 0x100) / 28) * 256 + 0x80;
			const uint people[] = { 0x1018, 0x1028, 0x1048 };
			for (uint i = 0; i < ARRAYSIZE(people); i++) {
				if (READ_LE_UINT16(_state.begin() + people[i] + 2) == standing)
					_state[0xf7] |= 1 << (_state[people[i] + 0xe] & 7);
			}
		}
	}

	WRITE_LE_UINT16(_state.begin() + 0x2c, place);
	_state[0x2e] = _state[record];
	_state[0x30] = _state[record + 3];
	_state[0x2f] = _state[record + 3] & 0xf;
	// 31F6 prepares the shared COMMAND1 substitution table too. Its words
	// remain available to later dialogue/actions after this report closes;
	// deriving just the displayed text left stale values from earlier use.
	WRITE_LE_UINT16(_state.begin() + 0x11f3, 24 + _state[0x2f]);
	const uint16 duration = (_state[record + 3] & 0x10) ? 116 :
		worked < 3 ? 112 : worked < 16 ? 113 : worked < 32 ? 114 : 115;
	WRITE_LE_UINT16(_state.begin() + 0x11f5, duration); // 32C7/330C.
	WRITE_LE_UINT16(_state.begin() + 0x32,
					READ_LE_UINT16(_state.begin() + record + 0x10));
	WRITE_LE_UINT16(_state.begin() + 0x34,
					READ_LE_UINT16(_state.begin() + record + 0x12));
	_state[0x31] = _state[record + 0x12] & 0xf;
	WRITE_LE_UINT16(_state.begin() + 0x11ff, _state[0x31]);
	_state[0x38] = _state[record + 0x16];
	_state[0x39] = _state[record + 0x17];
	_state[0x3a] = _state[record + 0x18];
	for (uint skill = 0; skill < 3; skill++)
		WRITE_LE_UINT16(_state.begin() + 0x11f7 + 2 * skill,
			209 + (_state[0x38 + skill] >> 4)); // 3310's skill-name table.
	// DOS 31F6/693B does not clamp category three: tasks 12..15 read
	// +0x19, the carried-equipment byte, as this context value.
	_state[0x37] = _state[record + 0x16 + ((_state[record + 3] & 0xf) >> 2)];
	_state[0x3b] = _state[record + 0x19];
	_state[0x3c] = _state[record + 0x1a];
	WRITE_LE_UINT16(_state.begin() + 0x44,
					READ_LE_UINT16(_state.begin() + record + 0xc));
	WRITE_LE_UINT16(_state.begin() + 0x46, tally);
	// 326E clears AH, then adds E8: preserve the low-byte alias even when
	// this table slot was previously the merchant's selected equipment.
	WRITE_LE_UINT16(_state.begin() + 0x11f1, 0xe8 + (byte)tally);

	_state[0x40] = (byte)((hour() >> 4) - _state[record + 0x14]);
	_state[0x41] = (byte)(worked >> 4);
	WRITE_LE_UINT16(_state.begin() + 0x42, worked);
	WRITE_LE_UINT16(_state.begin() + 0x48, producing);
	_state[0x36] = troopRate(troops[troop]);
}

/** 2B2A..2B51: reports may interrupt only the ordinary, grounded game view. */
bool DuneEngine::canOpenTroopNotice(bool allowFirstVision) const {
	if (_pendingRoomExit >= 0 || _pendingBoardingPlace >= 0 || _state.size() <= 0x4774 || _state[0x46d9] || _state[0x4774] ||
			_state[0x11c9] || (int8)_state[0xfb] < 0)
		return false;
	// Stage twenty has its own prescience branch (2BA1); 1B0D stops the
	// dispatcher entirely at the endgame. Neither is an ordinary report.
	if (storyStage() < 20 || (storyStage() == 20 && !allowFirstVision) ||
			storyStage() >= 200)
		return false;
	// These native modal/vehicle fields mirror the menu-stack and FB tests.
	// On foot, the desert uses the same ordinary DOS menu as a room.
	return (_menuKind == kMenuRoom || _menuKind == kMenuDesert) &&
		!_onWorld && !_onGlobe && !_onMixer && !_inCabin && !_takingOff &&
		_speakerPerson < 0 && _flyingTo < 0;
}

/** 96D8/9702/9F8B: automatic events use part 132 with the repeatable mask. */
int DuneEngine::troopNoticeRecord() const {
	if (_dialogue.count() <= 132)
		return -1;
	const Common::Array<Dialogue::Record> &records = _dialogue.entry(132);
	for (uint i = 0; i < records.size(); i++) {
		if (dialogueRecordAvailable(records[i], 0x20) && conditionHolds(records[i].condition))
			return i;
	}
	return -1;
}

/** Opens the oldest hourly troop notice from the four-byte queue at 0x1190. */
bool DuneEngine::showPendingTroopNotice() {
	enum { kQueue = 0x1190, kEntrySize = 4 };
	if (_menuKind == kMenuTroopNotice)
		return true;
	if (!canOpenTroopNotice() || _state.size() <= kQueue + kEntrySize ||
		_state[kQueue] == 0)
		return false;

	const uint16 what = READ_LE_UINT16(_state.begin() + kQueue + 1);
	const uint16 place = READ_LE_UINT16(_state.begin() + kQueue + 3);
	// 2BD8/2C16: the payload is a place, and the high byte names its caller.
	// A named caller may have no place payload, including Leto's stage-48
	// type-four warning. It uses the same report selector and presentation.
	if ((what >> 8) >= 0x10 || (!place && (what >> 8) == 14) ||
			(place && (place < 0x100 || (place - 0x100) % 28 != 0 ||
			(uint)(place - 0x100) / 28 >= _map.places().size())))
		return false;

	const byte oldKind = _state[0xea];
	_state[0xea] = what & 0xff; // 2BF7: retained until the report closes.
	if ((what & 0xff) == 1)
		_state[0xeb] = 1; // 2BFA/2BFE: latch before report selection.
	WRITE_LE_UINT16(_state.begin() + 0x47c4, what >> 8); // 2C16, before 331E.
	// 2C16 expands the reported place before selecting its event record.
	// Names alone leave troop counts, spare equipment and nearby-site
	// context describing the previous conversation instead of this report.
	if (place) {
		expandPlaceContext(place); // 2C1D: 331E.
		preparePlaceNameContext(place); // 2C20: 2E98.
	}
	const uint caller = prepareTroopNoticeCaller(what, place);
	const int selected = troopNoticeRecord();
	if (selected < 0) {
		_state[0xea] = oldKind;
		return false;
	}
	Dialogue::Record &record = _dialogue.entry(132)[selected];
	// 9FF8/A3DE47 retains the selected dialogue header before A097 marks the
	// record. The modal notice redraws that selection; it does not search
	// part 132 again when a condition or queue entry changes during the wait.
	_state[0x47de] = record.flags;
	Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	byte oldPalette[3 * 256];
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	_troopOrdersFromMap = true;
	if (place) {
		const Place &at = _map.places()[(place - 0x100) / 28];
		lookAtWorld(at.longitude, at.latitude);
	}
	_onWorld = false;
	_onGlobe = false;
	_inCabin = false;

	// The event record, not the portrait's dialogue group, selects PHRASE12.
	// Its prepared name words do not require moving Paul to the report place.
	prepareDialogueLine(record, 16);

	// A092/A097 mark the selected resource record. Mask 0x20 still permits
	// later reports of the same kind. Action thirteen requests the inset;
	// drawTroopNotice applies that action without mutating state on redraw.
	_state[0x19] = 0xff;
	rememberBookRecord(record, 16);
	record.flags |= 0x80;
	selectTroopNoticePose();
	// 978E/4ACA pauses the position callback for the remote portrait too.
	// Closing a grounded report does not resume a carried conversation;
	// the next departure explicitly releases this retained pause.
	_state[0x11ca] = 1;
	_state[0xe8] = 0; // 2C9A: the panel's face disappears during a vision.

	debugC(1, kDebugScript, "a troop sends word: %s", _speech.c_str());
	_menuKind = kMenuTroopNotice;
	drawTroopNotice();
	showNoticeTransition(oldPage.begin(), oldPalette);
	// 2C4A follows the opening transition, before consuming the queue head
	// and clearing the old command rows. The reading window starts last.
	playSpeech(record.line, caller, true);
	// 2C4F consumes before the wait. New queued events cannot replace this
	// report, and dismissal must not pop whichever event is now at the head.
	consumeTroopNotice();
	clearTroopNoticeMenu();
	drawMenu();
	startNoticeCloud();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
						  kScreenWidth, kScreenHeight);
	_system->updateScreen();
	_nextMouthTime = _system->getMillis() + 1000 / 12;
	_troopNoticeRemaining = kTroopNoticeDuration;
	_troopNoticeTime = _system->getMillis() / 5 * 5;
	_troopNoticeButtons = _system->getEventManager()->getButtonState() & 3;
	_state[0xcee8] = _state[0xcee9] = 0;
	// 2C5D/B2B9 freezes EF6A's game-clock countdown for DDB0's reading
	// window, not the interrupt ticks driving its timeout/cloud callback.
	// The opening transition is still unlocked; accrue it before the lock.
	tick(false);
	_state[0x2788]++;
	return true;
}

/** 2C52..2C5A: no room commands remain active under the transmission. */
void DuneEngine::clearTroopNoticeMenu() {
	_menu.clear();
	_menuParts.clear();
	_menuFlags.clear();
	_menuFrom = 0;
	_menuUnder = -1;
}

/** 2C16..2C43: named caller, current local chief, or specialist id three. */
uint DuneEngine::prepareTroopNoticeCaller(uint16 what, uint16 place) {
	uint caller = what >> 8;
	_orderingTroop = _map.troops().size();
	if (place && caller >= 14) {
		const uint id = (what & 0xff) == 14 ? 3 : _state[place + 9];
		if (id && id <= _map.troops().size()) {
			_orderingTroop = id - 1;
			WRITE_LE_UINT16(_state.begin() + 0x4756, 0x8aa + 27 * _orderingTroop);
			caller = 14;
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0x47c4, caller);
	return caller;
}

/**
 * Draws a distant transmission over VIS.HSQ's cloud background. Its small
 * map is centred on the retained report location, not the next queued event.
 * The panel's command rows are blank during the original modal wait.
 */
void DuneEngine::drawTroopNotice() {
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	const uint16 payload = READ_LE_UINT16(_state.begin() + 0x47e6);
	const bool placeNotice = payload >= 0x100 && (payload - 0x100) % 28 == 0 &&
			(uint)(payload - 0x100) / 28 < _map.places().size();
	const bool mapAction = (_state[0x47de] & 0x0f) == 13;
	const bool showMap = mapAction && placeNotice &&
			payload != READ_LE_UINT16(_state.begin() + 0x114e) && _state[0x28e7] != 1;
	// ICONES and the speech font need PERS' shared low colours on reload.
	// 2C9F loads VIS directly and retains the landscape's high ramp: taking
	// all of PERS here replaces SIET1's225..232 after an outdoor report.
	Resource people = _archive.openMember("PERS.HSQ");
	if (people && people->readUint16LE() > 2)
		applyPaletteBlock(&*people, 16);
	// A2B0 draws the inset with PALPLAN/ICONES, without loading ONMAP's
	// palette. Preserve the inherited scene colours outside VIS and the
	// portrait; a full ONMAP load overwrites those retained ramps.
	openSpriteSheet("VIS.HSQ");
	drawSprite(0, 0, 0);

	// 2C16 selects once when the notice opens. Later hourly changes to the
	// standing chain must not replace the caller during this transmission.
	// Chani's 0709 is her own portrait, not a Fremen with the cured troop's id.
	const uint caller = READ_LE_UINT16(_state.begin() + 0x47c4);
	drawSpeakerFigure(caller, _speakerFrame, _speakerAnimation);
	// 2CAD..2CB5: VIS and its portrait have replaced the outside sky.
	// Leave3916 registered until the next shared poll observes this flag
	// and cancels it; stopping it here would discard its retained phase.
	_state[0x46df] = 0;

	// Only action thirteen enters A28E/A2A0. Chani's cure has action zero:
	// even a remote cure has no inset. The action's own gates additionally
	// suppress the map at Paul's place or in dialogue display mode one.
	if (showMap)
		drawTroopNoticeMap(payload);

	drawPanel();
	drawMenu();
	if (!_speech.empty())
		drawSpeech(_speech, kSpeechColour);
	composeNoticeCloud();
}

void DuneEngine::drawTroopNoticeMap(uint16 place) {
	enum {
		kFrameLeft = 168, kFrameTop = 23,
		kMapLeft = 176, kMapTop = 32, kMapRight = 288, kMapBottom = 88,
		kFirstPlacePicture = 58, kFocusPicture = 54
	};
	const Common::Rect view(kMapLeft, kMapTop, kMapRight, kMapBottom);
	const Place &reported = _map.places()[(place - 0x100) / 28];

	// A2B0 opens bank 0x21 (PALPLAN.HSQ), not decimal 21 (COMM.HSQ).
	// Its sprite seven is the complete blue map frame seen in the DOS report.
	// Keep VIS' cloud palette while drawing this bank's indexed pixels.
	openSpriteSheet("PALPLAN.HSQ", false);
	drawSprite(7, kFrameLeft, kFrameTop);
	// B6C3 paints only 22F4's terrain rectangle. The earlier flat-world
	// scroll enlarged this inset and misplaced its sites.
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	_map.drawTerrainInset(_framebuffer2, kScreenWidth, reported.longitude,
			reported.latitude, view, _state.begin() + kMapScratchOrigin, kMapScratchBytes);

	// 5DCE paints every visible known place from ICONES, including the distant
	// variants of Fremen sites. The DOS sprite driver clips these marks to the
	// terrain rectangle; staging the composite does the same here.
	openSpriteSheet("ICONES.HSQ", false);
	for (uint i = 0; i != _map.places().size(); i++) {
		const Place &site = _map.places()[i];
		if (!site.isKnown())
			continue;
		const Common::Point at = _map.surveyPosition(reported.longitude,
				reported.latitude, site.longitude, site.latitude, view);
		if (!view.contains(at))
			continue;
		uint sprite = kFirstPlacePicture + placeClass(site.kind);
		if (site.kind < 0x20 && distanceFromPaul(site.longitude, site.latitude) >
				READ_LE_UINT16(_state.begin() + 0x1176))
			sprite += 5;
		drawSpriteCentred(sprite, at.x, at.y);
	}
	// 62FE overlays the reported place with ICONES sprite 54. Its X coordinate
	// is the projected centre; BL is moved up by the sprite's own height.
	const Common::Point focus = _map.surveyPosition(reported.longitude,
			reported.latitude, reported.longitude, reported.latitude, view);
	int width = 0, height = 0;
	if (view.contains(focus) && currentSpriteSheet->size(kFocusPicture, &width, &height))
		drawSprite(kFocusPicture, focus.x, focus.y - height);

	for (int y = view.top; y < view.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + view.left,
				_framebuffer2 + y * kScreenWidth + view.left, view.width());
	_graphics.setFrameBuffer(_framebuffer1);
}

void DuneEngine::dismissTroopNotice() {
	// 2C66/B2B3 releases only the reading window's lock. Account held time
	// before unlocking, so the following transition cannot catch it up.
	tick(false);
	if (_state[0x2788])
		_state[0x2788]--;
	Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	byte oldPalette[3 * 256];
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	_troopNoticeRemaining = 0;
	stopNoticeCloud();
	_speakerFrame = 0;
	_speakerAnimation = 0;
	_state[0xcee8] = 0; // DE4E consumes the acknowledgement.
	releaseLocalSpeakerForScene(); // 2C6F/98E6 removes the report's callbacks and caption.
	// 2C77 releases portrait history before rebuilding the room. Keeping the
	// caption owner above alive would leave B45's movement gate paused.
	WRITE_LE_UINT16(_state.begin() + 0x4540, 0);
	// The report was already consumed by 2C4F when it opened.
	// 0x2C7A puts the byte back to 0xFF once the notice has been read.
	if (_state.size() > 0xea)
		_state[0xea] = 0xff;
	_state[0xe8] = 10; // 2C7F: restore the ordinary panel face.
	WRITE_LE_UINT16(_state.begin() + 0xdc30, 0); // 2C84 releases the old input latch.
	// 2C8A/189A rebuilds the ordinary view through 2DB1/2E07. A remote
	// warning clears the previous room contacts unless battle context keeps
	// that history; returning only its menu left residents marked as spoken to.
	if (!_state[0x2b])
		WRITE_LE_UINT16(_state.begin() + 0x14, 0);

	_speech.clear();
	_onWorld = false;
	const bool localBattle = !_inDesert && battleHere();
	if (_inDesert) {
		showDesertMenu(false);
		drawDesert(true, false);
	} else {
		showRoomMenu(false);
		if (localBattle)
			restoreBattlefield(false);
		else
			drawScene(false);
	}
	if (!localBattle) {
		// 2E52 consumes the inhibited resident pass on the ordinary return.
		_state[0x47a6] = 0xff;
		showLocalRoomRequests();
	}
	showNoticeTransition(oldPage.begin(), oldPalette);
	resetTroopNoticeIdle(); // 18B1 starts the next idle period after the effect.
}

/** 2C60/DDB0: a report stays for 3000 ticks unless input shortens the wait. */
void DuneEngine::advanceTroopNotice(uint32 elapsedMs) {
	if (_menuKind != kMenuTroopNotice)
		return;
	if (elapsedMs < _troopNoticeRemaining)
		_troopNoticeRemaining -= elapsedMs;
	else
		dismissTroopNotice();
}

void DuneEngine::updateTroopNotice() {
	if (_menuKind != kMenuTroopNotice)
		return;
	const uint32 now = _system->getMillis() / 5 * 5;
	const uint32 elapsed = now - _troopNoticeTime;
	_troopNoticeTime = now;
	// 9908 does not register the idle portrait callback when signed EA > 0;
	// A82E/9DE3 likewise expand but do not paint those speech mouth parts.
	// A remote caller holds the selected pose while 2CC7 distorts the page.
	// Retain the existing fallback for EA <= 0; its ordinary portrait/speech
	// scheduler still needs reconstruction, separate from this remote gate.
	const bool animate = (int8)_state[0xea] <= 0;
	const bool speaking = _mixer->isSoundHandleActive(_speechHandle);
	bool repaint = false;
	if (animate && speaking && _system->getMillis() >= _nextMouthTime) {
		_speakerFrame++;
		_nextMouthTime = _system->getMillis() + 1000 / 12;
		repaint = true;
	} else if (animate && !speaking && _speakerFrame) {
		_speakerFrame = 0;
		repaint = true;
	}
	if (repaint)
		drawTroopNotice();
	repaint = pollNoticeCloud((uint16)(elapsed / 5)) || repaint;
	if (repaint) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
	// DDB0 subtracts elapsed ticks after DD63 has serviced the callbacks,
	// including the final callback before the reading window underflows.
	advanceTroopNotice(elapsed);
}

/** 2C60/DDB0/DD63: input first, then random and callbacks, never 1B0D. */
void DuneEngine::pollTroopNotice() {
	_state[0xcee9] = 0; // DE54 clears the previous Escape result on every poll.
	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		// DD63 reads the keyboard directly, before DB4C can translate a held
		// Space/Enter into a logical pointer button. Keep native button history
		// synchronized without filtering out the acknowledgement key.
		updatePointerClickKeys(event);
		if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
			quitGame();
			return;
		}
		if (event.type == Common::EVENT_MAINMENU) {
			// The event manager already ran the global menu before returning
			// this event. Restore the report without opening a second dialog.
			resetPointerButtons();
			redraw();
		} else {
			handleTroopNoticeEvent(event);
		}
		// DD63 returns with carry before E3CC/D9D2 on an acknowledgement.
		// Leave later queued events for the ordinary view, not this wait.
		if (_menuKind != kMenuTroopNotice || shouldQuit())
			return;
	}
	refreshInputRandom();
	updateGameMusic();
	{
		const AnimationClock::Poll poll(_animationClock, _system->getMillis());
		updateBattleAnimation();
		updateMapSpeech();
		updateLocalSpeaker();
		updateSkyFade();
		// D9D2 retains4BB9 during the transmission. Its paint guard
		// skips the portrait page while its sixteen-tick phase advances.
		updateDesertHarvester();
	}
	updateTroopNotice();
	_system->updateScreen();
	_system->delayMillis(10);
}

/** A notice acknowledgement cannot also activate the room underneath it. */
bool DuneEngine::handleTroopNoticeEvent(const Common::Event &event) {
	if (_menuKind != kMenuTroopNotice)
		return false;
	if (event.type == Common::EVENT_KEYDOWN) {
		_state[0xcee9] = event.kbd.keycode == Common::KEYCODE_ESCAPE;
		dismissTroopNotice();
	} else {
		byte button = 0;
		bool down = false;
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN: button = 1; down = true; break;
		case Common::EVENT_RBUTTONDOWN: button = 2; down = true; break;
		case Common::EVENT_MBUTTONDOWN: button = 4; down = true; break;
		case Common::EVENT_LBUTTONUP: button = 1; break;
		case Common::EVENT_RBUTTONUP: button = 2; break;
		case Common::EVENT_MBUTTONUP: button = 4; break;
		default: break;
		}
		// DD63 ignores held buttons, but a release followed by a new press
		// acknowledges the report. All three mouse buttons are accepted.
		if (down && !(_troopNoticeButtons & button))
			dismissTroopNotice();
		if (down)
			_troopNoticeButtons |= button;
		else
			_troopNoticeButtons &= ~button;
	}
	return true;
}

void DuneEngine::showTroopMenu() {
	showTroopMenu(true, false);
}

void DuneEngine::showTroopMenu(bool present, bool armTransition) {
	if (_menuKind == kMenuMoveTroop)
		closeSurveyWindow(); // 58FA retains the parent report and map mode.
	enum {
		kMoveTroop = 79, kNextTroop = 80, kNoMoreOrders = 81,
		kChangeDestination = 87, kCancel = 162
	};
	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();
	auto add = [&](uint label, uint action, bool enabled = true) {
		choices.push_back(command(label));
		_menuParts.push_back(action);
		flags.push_back(enabled ? 0 : 0x4000);
	};
	const uint record = 0x8aa + 27 * _orderingTroop;
	if (_orderingTroop >= _map.troops().size() || record + 27 > _state.size())
		return;
	const byte status = _state[record + 3];
	const bool outOfReach = _state[0x4c] != 0;
	// 780A chooses 2122 for an unreachable contact or a prisoner other than
	// the 22-state recruit. These have only NEXT TROOP and NO MORE ORDERS.
	if (outOfReach || ((status & 0x20) && status != 0x22)) {
		add(kNextTroop, kNextTroop);
		add(kNoMoreOrders, kNoMoreOrders);
	} else if (status & 0x40) {
		// 214A is the travelling troop's CHANGE DESTINATION/next/cancel menu.
		add(kChangeDestination, kMoveTroop);
		add(kNextTroop, kNextTroop);
		add(kCancel, kCancel);
	} else {
		// 780A registers 210A; 7847 updates it again only when an order is
		// actually assigned to the selected troop, not on every child pop.
		WRITE_LE_UINT16(_state.begin() + 0x211c, _troopOrdersFromMap ? 83 : 82);
		updateTroopMenuTemplate(_orderingTroop);
		showSettledTroopMenu(present, armTransition);
		return;
	}
	_menuKind = kMenuTroopContact;
	showMenu(choices, &flags, present, armTransition);
}

/** 7847 changes the retained 210A rows, without presenting the parent. */
void DuneEngine::updateTroopMenuTemplate(uint troop) {
	const uint record = 0x8aa + 27 * troop;
	if (troop >= _map.troops().size() || record + 27 > _state.size())
		return;
	const byte order = _state[record + 3] & 0x0f;
	const bool held = (READ_LE_UINT16(_state.begin() + record + 0x12) & 0x0400) != 0;
	const uint place = READ_LE_UINT16(_state.begin() + record + 4);
	bool equipment = false;
	if (!held && order != 2 && storyStage() >= 4 &&
			!(READ_LE_UINT16(_state.begin() + record + 0x10) & 0x0200) &&
			place >= 0x100 && place + 28 <= _state.size() &&
			((_state[place + 0x0a] & 8) || _state[place + 8] < 0x28)) {
		// 789F/7F27 refreshes the entire spare inventory even when the
		// troop already carries equipment or an earlier item is available.
		equipment = _state[record + 0x19] != 0;
		for (uint item = 0; item < 7; item++) {
			_state[0x46fe + item] = equipmentSpare(place, item);
			equipment |= _state[0x46fe + item] != 0;
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0x2110,
			(order == 2 && !held ? 86 : 79) | (held || order == 1 ? 0x4000 : 0));
	_state[0x2115] = (_state[0x2115] & ~0x40) | (equipment ? 0 : 0x40);
	_state[0x2119] = (_state[0x2119] & ~0x40) |
			(!held && order != 2 && storyStage() >= 5 ? 0 : 0x40);
}

void DuneEngine::showSettledTroopMenu(bool present, bool armTransition) {
	static const uint actions[] = {76, 78, 77, 79, 81};
	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();
	for (uint i = 0; i < ARRAYSIZE(actions); i++) {
		const uint16 row = READ_LE_UINT16(_state.begin() + 0x210c + 4 * i);
		choices.push_back(command((row & 0xfff) - 1));
		_menuParts.push_back(actions[i]);
		flags.push_back(row & 0x4000);
	}
	_menuKind = kMenuTroopContact;
	showMenu(choices, &flags, present, armTransition);
}

/** 0x7B58 consumes transient report notices and records the contact date. */
void DuneEngine::finishTroopContact() {
	finishTroopContact(false);
}

void DuneEngine::finishTroopContact(bool animate) {
	finishTroopContact(animate, true);
}

void DuneEngine::finishTroopContact(bool animate, bool unwindSurvey) {
	const uint32 record = READ_LE_UINT16(_state.begin() + 0x46ef);
	if (!record)
		return;
	if (unwindSurvey)
		closeSurveyWindow(); // Menu teardown unwinds its MAP2 child first; 8685 does not.
	// D338 unwinds the equipment child first, invoking 7D68 just as Done
	// does. Panning or changing contacts must not silently lose transfers.
	if (_menuKind == kMenuEquipment && _state[0x46f5])
		finishEquipment(true, false); // D338's callback, not Done's final D280/flush.
	// 7B58 uses the active record, not the last native menu index. A second
	// close or a passive map refresh must not consume another troop's notices.
	if (_mapRing.initialList)
		openSpriteSheet("ONMAP.HSQ", false); // Its retained extents do not belong to the panel bank.
	const Common::Rect ring = mapUnitBounds(_mapRing);
	removeMapRing(); // 69A3 removes the activation ring from the shared pool.
	if (!ring.isEmpty())
		paintMapActorChange(ring); // C58A presents while the report remains registered.
	_state[0x1954] = 0; // 875B follows the ring removal, not its presentation.
	WRITE_LE_UINT16(_state.begin() + 0x46ef, 0);
	_state[0x4c] = 0;
	_state[0x46f4] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x1bea, 0); // 7B58 before 1EBE's story check.
	if (record >= 0x8aa && (record - 0x8aa) % 27 == 0 &&
			(record - 0x8aa) / 27 < _map.troops().size() && record + 27 <= _state.size()) {
		// 7B79 runs 1EBE on release, not on remote report entry. Keep this
		// before the notice bits are consumed, including the cured bit.
		checkTroopCure((record - 0x8aa) / 27);
		WRITE_LE_UINT16(_state.begin() + record + 0x10,
				READ_LE_UINT16(_state.begin() + record + 0x10) & 0x03f0);
		WRITE_LE_UINT16(_state.begin() + record + 0x12,
				READ_LE_UINT16(_state.begin() + record + 0x12) & 0xe5ff);
		_state[record + 0x14] = (byte)(hour() >> 4);
		_map.readTroops(_state.begin(), _state.size());
	}
	stopSpeech(); // A7A5: the closed report must not keep talking over the map.
	_mapSpeechPerson = -1;
	// 7B94 clears DBE0 on every active-report close, including fast
	// replacement/panning paths that do not run the closing outline.
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
	_troopInfoPage = 0;
	_speech.clear();
	_speechParts.clear();
	_speechPart = 0;
	// 7B36 bypasses effect 4 when replacing contacts, panning or tearing
	// down a modal child. A deliberate NO MORE ORDERS/CUT CONTACT uses it.
	if (_troopWindowGeometryValid) {
		const MenuKind previousMenu = _menuKind;
		_menuKind = kMenuMap;
		paintMapActorChange(_troopWindowBounds); // 7B9A/C6AD presents before effect 4.
		_menuKind = previousMenu;
		// 7B2B's fast-close gate bypasses only effect 4, not 7B9A's restore.
		if (animate && !_state[0x46d8])
			animateTroopWindow(true);
	}
	_troopWindowGeometryValid = false;
}

/** 8763 returns a room-started conversation to the room; 8770 stays on map. */
void DuneEngine::dismissTroopContact(bool keepMap) {
	dismissTroopContact(keepMap, false);
}

void DuneEngine::dismissTroopContact(bool keepMap, bool restoreParent) {
	if (keepMap) {
		finishTroopContact(true);
		if (!restoreParent) {
			_state[0x46f3] = 0; // 877A clears the origin on Cut Contact.
			_troopOrdersFromMap = true;
		}
		const bool retainedMap = restoreParent || !_panelMenuOldPage.empty();
		showMapMenu(!retainedMap, restoreParent);
		if (retainedMap) {
			// Roll the restored rows over 7B9A's retained map page.
			drawMenu();
			finishPanelMenuTransition();
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
					kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
	} else {
		// 8763 returns through 186B/5ADF, not just 7B58. Release the
		// retained selection, windows and map callback before drawing the
		// room; a later map entry must start with a fresh actor pool.
		teardownStrategicMap(true);
		_state[0x28e7] = _state[0x28e8]; // 186B/187D restores the room preference.
		_state[0xfb] = groundViewMode(_state[0xfb]); // 186E retains the map mode's magnitude.
		letGoOfPerson();
		_onWorld = false;
		showMapReturnView(false);
	}
}

/** 186B/2DB1/C108: restore the room or desert after releasing a map. */
void DuneEngine::showMapReturnView(bool desert) {
	Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	byte oldPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	if (!_state[0x2b])
		WRITE_LE_UINT16(_state.begin() + 0x14, 0); // 2E07 clears room contact history.
	const bool transition = !_state[0x46d9]; // 189D's terminal-return bypass.
	if (transition)
		_state[0xdce6] = 0x80; // C108 suppresses command rolls in its callback.
	if (desert) {
		drawDesert(true, false);
		showDesertMenu(false);
	} else {
		if (battleHere())
			restoreBattlefield(false);
		else
			drawScene(false);
		showRoomMenu(false);
	}
	drawMenu();
	if (transition) {
		showMapEntryTransition(oldPage.begin(), oldPalette, false); // 1898/34h.
		_state[0xdce6] = 0;
	} else {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
	if (!_state[0x2b]) {
		_state[0x47a6] = 0xff; // 2DB1 inhibits the following 35AD resident pass.
		if (_state[0x11c9])
			prepareCarriedRoomRequests(); // 35E9 also consumes it before departure.
		else
			showLocalRoomRequests();
	}
}

/**
 * 0x7C02/0x9719 walks DIALOGUE part 122 with mask 0x20. DS:47BA keeps
 * the next record, and action six makes the next request start over. These
 * conditional records also carry warnings, production and story discoveries;
 * a synthesized page cycle cannot reproduce their order or applicability.
 */
void DuneEngine::sayTroopInformation() {
	sayTroopInformation(true);
}

void DuneEngine::sayTroopInformation(bool refreshMenu) {
	sayTroopInformation(refreshMenu, true);
}

void DuneEngine::sayTroopInformation(bool refreshMenu, bool present) {
	enum { kReportPart = 122 };
	const uint32 troop = 0x8aa + _orderingTroop * 27;
	if (_orderingTroop >= _map.troops().size() || troop + 27 > _state.size() ||
			_dialogue.count() <= kReportPart)
		return;

	auto presentReport = [&]() {
		if (!present)
			return; // 82B7 restores the report before unwinding the route list.
		if (refreshMenu && _menuKind != kMenuTroopContact) {
			showTroopMenu();
		} else {
			// Same-contact 7C02 skips 79EE and never calls 5D82 or 780A.
			// Update only the report: retain terrain, registered actors and
			// the existing contact/Done list, including an open spare card.
			const MenuKind previousMenu = _menuKind;
			_menuKind = kMenuTroopContact;
			_graphics.setFrameBuffer(_framebuffer1);
			drawTroopStatus(_orderingTroop);
			_menuKind = previousMenu;
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
					kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
	};

	// A PHRASE continuation belongs to this same report, not to another
	// table record or to the room dialogue underneath the map.
	if (_speechPart + 1 < _speechParts.size()) {
		_speech = _speechParts[++_speechPart];
		updateSpeechContinuation();
		updateSpeechWordCount(); // 8E16 lays out this report page, not its predecessor.
		presentReport();
		return;
	}

	prepareTroopContactContext(_orderingTroop);
	// 7C08/2E98 refresh the active report place and COMMAND substitutions,
	// including an unreachable chief or a traveller's destination.
	const uint16 place = READ_LE_UINT16(_state.begin() + troop + 4);
	preparePlaceNameContext(place);
	// 7C17 latches distant contact until 7B58 releases the chief. Only a
	// report that is currently beyond reach restarts the distant ring.
	if (distanceFromPaul(READ_LE_UINT16(_state.begin() + troop + 6),
			(int16)READ_LE_UINT16(_state.begin() + troop + 8)) > READ_LE_UINT16(_state.begin() + 0x1176)) {
		_state[0x4c] = 0xff;
		if (_mapRingTroop == (int)_orderingTroop) {
			_mapRing.list = _mapRing.next = 0x1916; // 7C24
			updateMapActor(ARRAYSIZE(_mapUnits), true);
		}
	}
	// 9719 acknowledges this place's Fremen notices before selecting a
	// report. Out-of-range contacts retain them; other callers/places and
	// 2A51's unused queue tail must survive. Do this on ASK as well as entry.
	if ((int8)_state[0x4c] >= 0)
		consumeCallerNotices(15, place);

	// 9719 selects the generic chief before 9F9E/94F3 refreshes the
	// caller's flags and visit clock. Do this for every new report scan,
	// including ASK: the room's previous speaker is not this caller.
	// DS:23 is a request context and is deliberately left untouched.
	WRITE_LE_UINT16(_state.begin() + 0x47c4, 15);
	WRITE_LE_UINT16(_state.begin() + 0x47a2, 0x10c8);
	_state[0x47c2] = 0x20;
	preparePersonDialogueContext(15);

	const byte previousMode = _state[0xfb];
	_state[0xfb] = 0xff; // Map reports include the equipment card (DOS 0x5A3D).
	_state[0x47a5] = 0xff;
	// 9719/A1C4 resets the request/font latch, not the preceding equipment
	// report kind. 7C3B changes 46F4 only after 7C37 starts the new voice.
	// A09F consumes the action-six latch after the selected record's action,
	// making the following ASK restart even when this record has no action six.
	Common::Array<Dialogue::Record> &records = _dialogue.entry(kReportPart);
	// End-of-table restarts the search in 0x7C2D. Limit a corrupt/empty table
	// to one full traversal rather than hanging when no condition can match.
	for (uint tested = 0; tested < records.size(); tested++) {
		if (_troopInfoPage >= records.size())
			_troopInfoPage = 0;
		Dialogue::Record &record = records[_troopInfoPage++];
		if (!dialogueRecordAvailable(record, 0x20) || !conditionHolds(record.condition))
			continue;

		prepareDialogueLine(record, 15, troop);
		const int previousSpeaker = _speakerPerson;
		_speakerPerson = 15;
		runLineAction(record);
		_speakerPerson = previousSpeaker;
		_state[0x19] = 0xff;
		rememberBookRecord(record, 15);
		record.flags |= 0x80;
		if (_state[0x47a8]) {
			_troopInfoPage = 0;
			_state[0x47a8] = 0;
		}
		playSpeech(record.line, 15); // A09F consumes the latch before 7C37's voice.
		_state[0x46f4] = _state[0x47a5] == 0x80;
		if (_state[0x46f4]) {
			// 7C47/7EFB refreshes the carried row before 7E1E paints it.
			// Do this on selection, not passive redraw: that row can retain
			// another troop's kit, or an equipment picker's pending edits.
			const byte carried = _state[troop + 0x19];
			for (uint item = 0; item < 7; item++)
				_state[0x4705 + item] = (carried >> (7 - item)) & 1;
		}
		debugC(1, kDebugScript, "troop %u report line %u: %s",
				_orderingTroop, record.line, _speech.c_str());
		break;
	}
	_state[0xfb] = previousMode;
	if ((int8)_state[0x46eb] < 0)
		WRITE_LE_UINT16(_state.begin() + 0x479e, 0); // 7C56/9901.
	presentReport();
}

/** 13B6/9761: select and acknowledge the chief's next event paragraph. */
uint16 DuneEngine::prepareProspectorBriefingLine() {
	const uint part = 15 * 8 + 7;
	if (part >= _dialogue.count())
		return 0;
	WRITE_LE_UINT16(_state.begin() + 0x47c4, 15);
	WRITE_LE_UINT16(_state.begin() + 0x47a2, 0x10c8);
	_state[0x47c2] = 0x80;
	Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
	for (uint i = 0; i < records.size(); i++) {
		Dialogue::Record &record = records[i];
		if (!dialogueRecordAvailable(record, 0x80) || !conditionHolds(record.condition))
			continue;
		prepareDialogueLine(record, 15);
		// These single-page event records pass through A042/A097 too. Keep
		// the retained map scene: sayChosenLine would redraw the room here.
		runLineAction(record);
		_state[0x19] = 0xff;
		rememberBookRecord(record, 15);
		record.flags |= 0x80;
		WRITE_LE_UINT16(_state.begin() + 0x47ba, _state[0x47a8] ? 0xffff : 0);
		_state[0x47a8] = 0;
		return record.line;
	}
	return 0;
}

/** The short two-choice sequence in which the prospectors hand Paul MAP2. */
void DuneEngine::showProspectorBriefing() {
	showProspectorBriefing(false);
}

void DuneEngine::showProspectorBriefing(bool armTransition) {
	Common::Array<Common::String> choices;
	choices.push_back(command(161));
	choices.push_back(command(148));

	_menuParts.clear();
	_menuParts.push_back(0);
	_menuParts.push_back(1);
	_menuKind = kMenuProspectorBriefing;
	// D2E2/D323 changes the command rows over the already presented request
	// or inset. It does not redraw terrain, re-register actors or repaint the
	// chief. 13A0/13AA perform their own report/window paints before this.
	showMenu(choices, nullptr, false, armTransition);
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** Prepares the continuation panel used by the scripted gatherings. */
void DuneEngine::preparePalaceGathering(uint step) {
	// Unspoken room tableaux select 1392's single Continue menu. Chani's
	// poses change the retained script table, but their active menu stack
	// still holds 1FAE's Continue/WHAT until the next room rebuild.
	const bool singleRow = step == 401 || step == 407 ||
		step == 1 || step == 4 || step == 8 ||
		step == 105 || step == 107 || step == 112 ||
		step == 203 || step == 208 || step == 211 ||
		step == 302 ||
		step == 500 || step == 502 || step == 508 ||
		step == 513 || step == 518 || step == 519 ||
		step == 606;
	_menu.clear();
	_menu.push_back(command(161)); // 1FAE/1FBA's original COMMAND1 word 00A2.
	// The talking beats offer WHAT; the FINAL card does not.
	if (!singleRow)
		_menu.push_back(command(148)); // " WHAT ? "

	// The next beat is kept in the ordinary persisted menu payload, so a save
	// made halfway through this conversation resumes at the exact speaker.
	_menuParts.clear();
	_menuParts.push_back(step);
	if (!singleRow)
		_menuParts.push_back(step);
	_menuFlags.clear();
	_menuFlags.resize(_menu.size(), 0);
	_menuFrom = 0;
	_menuKind = kMenuPalaceGathering;
}

/** Carry the old, shortened scene's saved next beat into CS:1313's sequence. */
void DuneEngine::restoreChaniSequence() {
	if (_menuKind != kMenuPalaceGathering || _menuParts.empty() ||
			_menuParts[0] < 300 || _menuParts[0] > 303)
		return;
	static const uint kSteps[] = { 310, 311, 313, 317 };
	static const uint16 kCursors[] = { 0x1313, 0x1315, 0x1317, 0x131e };
	const uint old = _menuParts[0] - 300;
	preparePalaceGathering(kSteps[old]);
	WRITE_LE_UINT16(_state.begin() + 0x477a, kCursors[old]);
	if (old == 2) {
		letGoOfPerson();
		_speech.clear();
	}
}

/** DOS 1422/1442: CHANKISS's two tables and the second pose's scene crop. */
void DuneEngine::drawChaniKiss(uint pose) {
	_graphics.setFrameBuffer(_framebuffer1);
	if (pose == 1) {
		// BP=3 selects DNVGA 3A9D's 3:2 expansion, NOT a fourfold zoom.
		// Horizontal pairs duplicate their first pixel; vertical pairs their
		// second row. The driver paints 150 rows, leaving rows 150/151 alone.
		// Copy first because the crop at (0,50) overlaps the output page.
		byte crop[213 * 100];
		for (uint y = 0; y < 100; y++)
			memcpy(crop + y * 213, _framebuffer1 + (y + 50) * 320, 213);
		for (uint y = 0; y < 150; y++)
			for (uint x = 0; x < 320; x++)
				_framebuffer1[y * 320 + x] = crop[((2 * y + 1) / 3) * 213 + (2 * x) / 3];
	}
	openSpriteSheet("CHANKISS.HSQ");
	// DS:2290 and 2298. Neither is the old hand-positioned sprite at (98,3).
	drawSprite(pose, pose == 0 ? 78 : 26, pose == 0 ? 33 : 4);
}

/** Rebuild a silent pose without advancing its clock, actions or saved cursor. */
void DuneEngine::drawChaniKissCard(uint pose) {
	// 1422/1442 call 37B2 before the pose and C4DD afterwards. The
	// interface must not enter 1442's (0,50) landscape crop; otherwise
	// its centre ornament is enlarged into the lower-right of the scene.
	drawDesert(false, false, true);
	drawChaniKiss(pose);
	drawPanel();
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * Continues the first conversation in the communication room.  These are the
 * game's own DIALOGUE records (and therefore retain their conditions, actions,
 * voices and one-shot marks), merely joined in the order shown by the original
 * event rather than invented as interface strings.
 */
void DuneEngine::continuePalaceGathering(uint step) {
	if (step == kStageEventContinuation) {
		continueStageEvent();
		return;
	}
	// 1707/9761 selects the event record before A0AA/978E prepares its
	// portrait. Bind scripted callers without preparing the previous line;
	// sayChosenLine owns the new paragraph and its one 9908 selection.
	struct Beat {
		byte person;
		uint16 line;
	};

	// Meeting Kynes is another event-script conversation.  His greeting's
	// action advances the story to 88, after which Chani and Stilgar introduce
	// Paul before Kynes agrees to explain his experiments.  Those replies live
	// in each speaker's event part and ordinary WHAT traversal cannot join them.
	enum { kChaniSequence = 310, kKynesSequence = 400,
		kEndingSequence = 500, kGurneySequence = 600 };
	// CS:12DB gathers five advisers in room two before the final assault.
	// The opcode-04/08 pause is a room frame, not another spoken line.
	static const Beat kGurneyBeats[] = {
		{ 2, 198 }, { 1, 135 }, { 2, 199 }, { 7, 94 }, { 5, 478 },
		{ 0xff, 0 },
		{ 4, 381 }, { 2, 200 }, { 1, 136 }, { 2, 201 }
	};
	static const uint16 kGurneyCursor[] = {
		0x12e5, 0x12e7, 0x12e9, 0x12eb, 0x12ed,
		0x12ef, 0x12f1, 0x12f3, 0x12f5, 0x12f7
	};
	static const Beat kKynesBeats[] = {
		{ 7,  90 }, // Chani has brought Paul to meet her father
		{ 6,  18 }, // Kynes recognizes the famous Muad'Dib
		{ 7,  91 }, // Chani says the meeting was her own idea
		{ 5, 475 }, // Stilgar suggests telling Paul about the experiments
		{ 6,  19 }, // Kynes considers it
		{ 7,  92 }, // Chani describes what Paul has achieved
		{ 6,  20 }  // Kynes agrees to show him everything
	};

	// CS:128F begins with the 16-person throne-room tableau. Speaker opcodes
	// at 12A3..12C3 select these exact dialogue records. Opcode 04 redraws
	// the room after the first, sixth and tenth speakers; opcode 12 draws it
	// again without Chani after Jessica speaks. Those pauses are saved as
	// Continue steps just as the spoken lines are.
	static const Beat kEndingBeats[] = {
		{ 4, 380 }, { 0xff, 0 },
		{ 11, 171 }, { 3, 286 }, { 2, 195 }, { 11, 172 }, { 2, 196 },
		{ 0xff, 0 },
		{ 5, 476 }, { 9, 146 }, { 7, 93 }, { 10, 155 },
		{ 0xff, 0 },
		{ 5, 477 }, { 11, 173 }, { 2, 197 }, { 1, 134 },
		{ 0xff, 0 }
	};
	static const uint16 kEndingCursor[] = {
		0x12a5, 0x12a7,
		0x12a9, 0x12ab, 0x12ad, 0x12af, 0x12b1, 0x12b3,
		0x12b5, 0x12b7, 0x12b9, 0x12bb, 0x12bd,
		0x12bf, 0x12c1, 0x12c3, 0x12c5, 0x12d9
	};

	if (step >= kGurneySequence) {
		const uint index = step - kGurneySequence;
		releaseEventSpeaker();
		if (index >= ARRAYSIZE(kGurneyBeats)) {
			// FF restores 1771's room/mode after the last warning about the
			// Emperor. The DOS cursor remains on the FF byte at 12F7.
			const uint16 saved = READ_LE_UINT16(_state.begin() + 0x4776);
			const byte room = (byte)saved;
			_state[0xb] = room;
			_state[0xc] = room;
			WRITE_LE_UINT16(_state.begin() + 4,
				(READ_LE_UINT16(_state.begin() + 4) & 0xff00) | room);
			_state[0x46e0] = (byte)(saved >> 8);
			const byte *layout = roomRecord(_state[8], room);
			if (layout && layout[0])
				_place = (byte)(layout[0] - 1);
			releaseEventScript();
			carryFollowersAlong();
			_speech.clear();
			drawScene();
			showRoomMenu();
			return;
		}

		preparePalaceGathering(step + 1);
		if (index == 0) {
			// 00 02 05 enters the sietch's room two; the five inline bytes
			// name the adviser roster before the first speaker opcode.
			const byte room = 2;
			_state[0xb] = room;
			_state[0xc] = room;
			WRITE_LE_UINT16(_state.begin() + 4,
				(READ_LE_UINT16(_state.begin() + 4) & 0xff00) | room);
			WRITE_LE_UINT16(_state.begin() + 0x4778, 0x12dd);
			WRITE_LE_UINT16(_state.begin() + 0x477a, 0x12e3);
			const byte *layout = roomRecord(_state[8], room);
			if (layout && layout[0])
				_place = (byte)(layout[0] - 1);
			carryFollowersAlong();
			drawScene();
		}
		WRITE_LE_UINT16(_state.begin() + 0x477a, kGurneyCursor[index]);
		const Beat &beat = kGurneyBeats[index];
		if (beat.person == 0xff) {
			_speech.clear();
			drawScene();
			return;
		}

		selectLocalSpeaker(beat.person, false);
		_dialogueMask = 0x80;
		bool found = false;
		for (uint part = (uint)beat.person * 8;
				part != (uint)beat.person * 8 + 8 && part < _dialogue.count() && !found; part++) {
			Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line < records.size(); line++) {
				if (records[line].line != beat.line)
					continue;
				_dialoguePart = part;
				_dialogueLine = line;
				found = true;
				break;
			}
		}
		if (!found) {
			warning("Dune: Gurney gathering step %u has no line %u", index, beat.line);
			releaseEventScript();
			showRoomMenu();
			return;
		}
		sayChosenLine();
		return;
	}

	if (step >= kEndingSequence) {
		const uint index = step - kEndingSequence;
		if (index == ARRAYSIZE(kEndingBeats)) {
			// CS:12D9 enters 14C9, which builds the FINAL.HSQ card and
			// returns at the title for another acknowledgement.
			stopSpeech();
			preparePalaceGathering(step + 1);
			WRITE_LE_UINT16(_state.begin() + 0x477a, 0x12da);
			playEndingTransition();
			drawEndingCard();
			return;
		}
		if (index > ARRAYSIZE(kEndingBeats)) {
			// Opcode 16 runs the cast cards before the credits movie.
			WRITE_LE_UINT16(_state.begin() + 0x477a, 0x12db);
			playEndingCast();
			playVideo(HNM_CREDITS);
			drawFinalStill();
			// DOS 16F1 leaves this picture up until a key/button is pressed.
			// The test-only no-delay mode finishes without a wall-clock wait.
			if (!ConfMan.getBool("dune_nodelay")) {
				while (!shouldQuit() && pollSkip() == kPlaybackFinished) {
					updateEndingCouple();
					_system->delayMillis(10);
					_system->updateScreen();
				}
			}
			quitGame();
			return;
		}

		releaseEventSpeaker();
		const Beat &beat = kEndingBeats[index];
		if (beat.person == 0xff) {
			preparePalaceGathering(step + 1);
			if (index == ARRAYSIZE(kEndingBeats) - 1)
				WRITE_LE_UINT16(_state.begin() + 0x4778, 0x12c7);
			WRITE_LE_UINT16(_state.begin() + 0x477a, kEndingCursor[index]);
			_speech.clear();
			drawScene();
			return;
		}

		selectLocalSpeaker(beat.person, false);
		_dialogueMask = 0x80;
		bool found = false;
		for (uint part = (uint)beat.person * 8;
				part != (uint)beat.person * 8 + 8 && part < _dialogue.count() && !found; part++) {
			Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line != records.size(); line++) {
				if (records[line].line != beat.line)
					continue;
				_dialoguePart = part;
				_dialogueLine = line;
				found = true;
				break;
			}
		}
		if (!found) {
			warning("Dune: ending step %u has no dialogue line %u", index, beat.line);
			releaseEventSpeaker();
			preparePalaceGathering(step + 1);
			drawScene();
			return;
		}

		preparePalaceGathering(step + 1);
		WRITE_LE_UINT16(_state.begin() + 0x477a, kEndingCursor[index]);
		sayChosenLine();
		return;
	}

	if (step >= kKynesSequence) {
		// CS:12FB starts with opcode 00 (the three-person room roster) and
		// opcode 08 (a room-frame pause). CS:130C has another 04/08 redraw
		// after Kynes' fifth reply. These are two real Continue presses,
		// not speech records; omitting them skips the recorded tableau.
		if (step == kKynesSequence || step == kKynesSequence + 6) {
			releaseEventSpeaker();
			if (step == kKynesSequence) {
				WRITE_LE_UINT16(_state.begin() + 0x4778, 0x12fd);
				WRITE_LE_UINT16(_state.begin() + 0x477a, 0x1302);
			} else {
				WRITE_LE_UINT16(_state.begin() + 0x477a, 0x130e);
			}
			_speech.clear();
			preparePalaceGathering(step + 1);
			// Both opcode 00 and 04 return through 13DB/2DB1. Rebuild
			// contact state and consume 35AD's inhibited resident pass;
			// drawing the tableau alone leaves the previous request armed.
			if (!_state[0x2b])
				WRITE_LE_UINT16(_state.begin() + 0x14, 0);
			_state[0x47a6] = 0xff;
			drawScene();
			showLocalRoomRequests();
			return;
		}
		const uint beatIndex = step < kKynesSequence + 6
			? step - kKynesSequence - 1 : step - kKynesSequence - 2;
		if (beatIndex >= ARRAYSIZE(kKynesBeats)) {
			releaseEventSpeaker();
			if (_state.size() > 0x477b) {
				releaseEventScript();
				// 171F sees the FF at CS:1312 and jumps to 1736 before
				// 1729 can advance the cursor. Keep that terminating offset
				// even after the event-active byte has been released.
				WRITE_LE_UINT16(_state.begin() + 0x477a, 0x1312);
			}
			carryFollowersAlong();
			_speech.clear();
			drawScene();
			showRoomMenu();
			raisePanelFace(); // 175F/0FA7 restores the ordinary panel after FF.
			return;
		}

		releaseEventSpeaker();
		const Beat &beat = kKynesBeats[beatIndex];
		selectLocalSpeaker(beat.person, false);
		_dialogueMask = 0x80;

		bool found = false;
		const uint firstPart = (uint)beat.person * 8;
		for (uint part = firstPart; part != firstPart + 8 && !found; part++) {
			if (part >= _dialogue.count())
				break;
			Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line != records.size(); line++) {
				if (records[line].line != beat.line)
					continue;
				_dialoguePart = part;
				_dialogueLine = line;
				found = true;
				break;
			}
		}

		if (!found) {
			debugC(1, kDebugScript,
				   "Kynes meeting step %u has no dialogue line %u",
				   beatIndex, beat.line);
			releaseEventSpeaker();
			drawScene();
			showRoomMenu();
			return;
		}

		preparePalaceGathering(step + 1);
		static const uint16 kKynesCursor[] = {
			0x1304, 0x1306, 0x1308, 0x130a, 0x130c, 0x1310, 0x1312
		};
		WRITE_LE_UINT16(_state.begin() + 0x477a, kKynesCursor[beatIndex]);
		sayChosenLine();
		return;
	}

	// Old saves used 300..303 for a shortened sequence. Keep those meanings
	// distinct from the complete script's persisted 310..318 next-beat values.
	if (step >= 300 && step <= 303) {
		preparePalaceGathering(step);
		restoreChaniSequence();
		step = _menuParts[0];
	}
	// CS:1313: 02/07, 0A, 0C, 04/02/07, 0C, 0A, 02/05, 02/07, FF.
	if (step >= kChaniSequence) {
		const uint beat = step - kChaniSequence;

		static const uint16 kCursors[] = {
			0x1315, 0x1316, 0x1317, 0x131a, 0x131b, 0x131c, 0x131e, 0x1320
		};
		if (beat == 1 || beat == 2 || beat == 4 || beat == 5) {
			stopSpeech();
			releaseEventSpeaker();
			_speech.clear();
			const uint pose = (beat == 1 || beat == 5) ? 0 : 1;
			// 1422 waits until hour 13 via FD9, including ordinary hourly work.
			// Redraw/load calls only the painter, never this gameplay wait.
			while (pose == 0 && (hour() & 15) < 13 &&
					!shouldQuit())
				advanceWaitHours(1);
			if (pose == 0)
				finishSkyFade(); // 1434, before37B2 prepares the unzoomed scene.
			releaseLocalSpeakerForScene(); // 37B2/98E6 removes the old portrait timer.
			preparePalaceGathering(step + 1);
			WRITE_LE_UINT16(_state.begin() + 0x477a, kCursors[beat]);
			drawChaniKissCard(pose);
			return;
		}

		if (beat >= 8) {
			stopSpeech();
			releaseEventSpeaker();
			releaseEventScript();
			_state[4] = _state[0x4776];
			_state[0x46e0] = _state[0x4777];
			// 171F branches at FF before 1729 stores a post-opcode cursor.
			WRITE_LE_UINT16(_state.begin() + 0x477a, 0x1320);
			_speech.clear();
			showDesertMenu(false);
			drawDesert(true, false);
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
			_system->updateScreen();
			raisePanelFace(); // 175F/0FA7 restores the ordinary panel after FF.
			return;
		}

		const byte person = beat == 6 ? 5 : 7;
		const uint16 wantedLine = beat == 0 ? 87 : beat == 3 ? 88 : beat == 6 ? 474 : 89;
		releaseEventSpeaker();
		if (beat == 3) {
			// CS:1317's 04 goes through 13DB/2DB1, not directly to the
			// next speaker. Rebuild room contact through 2E07 and consume
			// the synchronous redraw's request inhibit at 35AD.
			if (!_state[0x2b]) {
				WRITE_LE_UINT16(_state.begin() + 0x14, 0);
				_state[0x46df] = 0;
			}
			_state[0x47a6] = 0xff;
			releaseLocalSpeakerForScene();
			drawDesert(false, false);
			showLocalRoomRequests();
		}
		selectLocalSpeaker(person, false);
		_dialogueMask = 0x80;

		bool found = false;
		const uint firstPart = (uint)person * 8;
		for (uint part = firstPart; part != firstPart + 8 && !found; part++) {
			if (part >= _dialogue.count())
				break;
			Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line != records.size(); line++) {
				if (records[line].line != wantedLine)
					continue;
				_dialoguePart = part;
				_dialogueLine = line;
				found = true;
				break;
			}
		}

		if (!found) {
			debugC(1, kDebugScript, "Chani sequence step %u has no line %u",
				   beat, wantedLine);
			releaseEventSpeaker();
			drawDesert();
			showDesertMenu();
			return;
		}

		preparePalaceGathering(step + 1);
		WRITE_LE_UINT16(_state.begin() + 0x477a, kCursors[beat]);
		sayChosenLine();
		return;
	}

	// Bringing Stilgar home is the palace's next ordered gathering.  These
	// records live in Leto's making-off/event parts and Stilgar's event part;
	// ordinary TALK TO traversal never joins them and used to skip directly to
	// Leto's generic "Keep on going".  The event reader calls each record in
	// order, exactly as it does for the earlier communication-room gathering.
	enum { kVisionGathering = 101, kStilgarGathering = 200 };
	static const Beat kStilgarBeats[] = {
		{ 0,  33 }, // Leto asks who Paul has brought home
		{ 1, 132 }, // Jessica identifies the leader every Fremen respects
		{ 0xff, 0 }, // 1372/1378: enter throne room and wait
		{ 5, 470 }, // Stilgar offers the Fremen's help
		{ 5, 471 }, // and explains their quarrel with the Harkonnens
		{ 0,  48 }, // Leto accepts their aid
		{ 5, 472 }, // Stilgar praises Paul
		{ 0xff, 0 }, // 1381: redraw the room, then wait
		{ 5, 473 }, // and greets Jessica through the old prophecy
		{ 1, 133 }, // Jessica recognizes his strength
		{ 0xff, 0 }, // 1387/138C/138E: restore the smaller roster
		{ 0,  49 }, // Leto entrusts Paul to Stilgar and advances the story
	};
	static const uint16 kStilgarCursor[] = {
		0x1370, 0x1372, 0x1379, 0x137b, 0x137d, 0x137f,
		0x1381, 0x1383, 0x1385, 0x1387, 0x138f, 0x1391
	};
	// Event opcode 00 replaces only the room byte in DS:4, saves the cursor
	// following that operand and skips its inline roster. Native room drawing
	// also needs the room mirrors and layout index updated before opcode 04's
	// scene redraw, which opcode 00 falls through to in DOS.
	auto enterEventRoom = [&](byte room, uint16 alternate, uint16 cursor) {
		_state[0xb] = room;
		_state[0xc] = room;
		WRITE_LE_UINT16(_state.begin() + 4,
			(READ_LE_UINT16(_state.begin() + 4) & 0xff00) | room);
		WRITE_LE_UINT16(_state.begin() + 0x4778, alternate);
		WRITE_LE_UINT16(_state.begin() + 0x477a, cursor);
		const byte *layout = roomRecord(0x20, room);
		if (layout && layout[0])
			_place = (byte)(layout[0] - 1);
	};

	if (step >= kStilgarGathering) {
		const uint beatIndex = step - kStilgarGathering;
		if (beatIndex >= ARRAYSIZE(kStilgarBeats)) {
			releaseEventSpeaker();
			if (_state.size() > 0x477b) {
				// Script FF restores the room/mode saved by 1771 before it
				// releases the event. Its counted room roster is not a new
				// physical destination for Paul or the palace residents.
				const uint16 saved = READ_LE_UINT16(_state.begin() + 0x4776);
				const byte room = (byte)saved;
				_state[0xb] = room;
				_state[0xc] = room;
				WRITE_LE_UINT16(_state.begin() + 4,
					(READ_LE_UINT16(_state.begin() + 4) & 0xff00) | room);
				_state[0x46e0] = (byte)(saved >> 8);
				const byte *layout = roomRecord(0x20, room);
				if (layout && layout[0])
					_place = (byte)(layout[0] - 1);
				releaseEventScript();
				// 1736 clears the active event, not its terminal FF cursor.
			}
			carryFollowersAlong();
			_speech.clear();
			drawScene();
			showRoomMenu();
			raisePanelFace(); // 175F/0FA7 restores the ordinary panel after FF.
			return;
		}
		releaseEventSpeaker();
		if (beatIndex == 2 || beatIndex == 10) {
			// 1372: 00 0A 03; 1387: 00 0A 02. Select the throne-room
			// tableau and counted roster without moving its residents.
			enterEventRoom(10, beatIndex == 2 ? 0x1374 : 0x1389,
				beatIndex == 2 ? 0x1378 : 0x138c);
			carryFollowersAlong();
			if (beatIndex == 10 && _state.size() > 0x47c5)
				WRITE_LE_UINT16(_state.begin() + 0x47c4, 0); // Opcode 06's argument.
		}

		const Beat &beat = kStilgarBeats[beatIndex];
		preparePalaceGathering(step + 1);
		if (beatIndex)
			WRITE_LE_UINT16(_state.begin() + 0x477a, kStilgarCursor[beatIndex]);
		if (beat.person == 0xff) {
			_speech.clear();
			drawScene();
			return;
		}
		selectLocalSpeaker(beat.person, false);
		_dialogueMask = 0x80;

		bool found = false;
		const uint firstPart = (uint)beat.person * 8;
		for (uint part = firstPart; part != firstPart + 8 && !found; part++) {
			if (part >= _dialogue.count())
				break;
			Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line != records.size(); line++) {
				if (records[line].line != beat.line)
					continue;
				_dialoguePart = part;
				_dialogueLine = line;
				found = true;
				break;
			}
		}

		if (!found) {
			debugC(1, kDebugScript,
				   "Stilgar gathering step %u has no dialogue line %u",
				   beatIndex, beat.line);
			releaseEventSpeaker();
			drawScene();
			showRoomMenu();
			return;
		}

		sayChosenLine();
		return;
	}

	// Jessica's line 104 starts CS:134F before the first Continue press. The
	// script then interleaves ten spoken records with three opcode-08 room
	// pauses. The old unused index-zero copy of line 104 is not a script beat.
	static const Beat kVisionBeats[] = {
		{ 1, 130 }, // Jessica links the Emperor's answer to Paul's vision
		{ 0,  42 }, // Leto cannot explain the vision
		{ 1, 131 }, // Jessica promises to explain it later
		{ 0xff, 0 }, // 1355: redraw the hall, then wait at 1357
		{ 0,  43 }, // Leto calls everyone to the balcony
		{ 0xff, 0 }, // 1359/1360: enter balcony room five and wait
		{ 0,  44 }, // look around and say what can be seen
		{ 3, 285 }, // Duncan sees only dunes
		{ 0,  45 }, // Leto credits Paul's Fremen diplomacy
		{ 8, 139 }, // Harah repeats the old prophecy
		{ 0xff, 0 }, // 1369: redraw the balcony, then wait at 136B
		{ 0,  46 }, // eleven sietchs and more troops
		{ 0,  47 }  // Gurney is missing
	};
	static const uint16 kVisionCursor[] = {
		0x1351, 0x1353, 0x1355, 0x1357, 0x1359, 0x1361, 0x1363,
		0x1365, 0x1367, 0x1369, 0x136b, 0x136d, 0x136f
	};

	if (step >= kVisionGathering) {
		const uint beatIndex = step - kVisionGathering;
		if (beatIndex >= ARRAYSIZE(kVisionBeats)) {
			releaseEventSpeaker();
			if (_state.size() > 0x477b) {
				releaseEventScript();
				// Retain 136F, as the shared DOS FF handler does.
			}

			// The balcony's inline roster is drawing-only. Neither opcode 00
			// nor FF relocates people, dismisses Duncan or installs an E7 latch.
			// 1736 restores the room/mode saved by 1771. The ordinary
			// greeting can begin in the throne room as well as the hall;
			// always returning to room four loses that real caller context.
			const byte returnRoom = _state[0x4776];
			_state[0x46e0] = _state[0x4777];
			_state[0xb] = returnRoom;
			_state[0xc] = returnRoom;
			_state[4] = returnRoom;
			const byte *hall = roomRecord(0x20, returnRoom);
			if (hall && hall[0])
				_place = (byte)(hall[0] - 1);
			_speech.clear();
			drawScene();
			showRoomMenu();
			raisePanelFace(); // 175F/0FA7 returns to the ordinary room parent.
			return;
		}

		// 1359's 00 05 04 opcode takes the gathering onto the palace balcony,
		// room five. The old room-three approximation showed an indoor scene.
		releaseEventSpeaker();
		if (beatIndex == 5) {
			enterEventRoom(5, 0x135b, 0x1360);
		}

		const Beat &beat = kVisionBeats[beatIndex];
		preparePalaceGathering(step + 1);
		WRITE_LE_UINT16(_state.begin() + 0x477a, kVisionCursor[beatIndex]);
		if (beat.person == 0xff) {
			_speech.clear();
			drawScene();
			return;
		}
		selectLocalSpeaker(beat.person, false);
		_dialogueMask = 0x80;

		bool found = false;
		const uint firstPart = (uint)beat.person * 8;
		for (uint part = firstPart; part != firstPart + 8 && !found; part++) {
			if (part >= _dialogue.count())
				break;
			Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line != records.size(); line++) {
				if (records[line].line != beat.line)
					continue;
				_dialoguePart = part;
				_dialogueLine = line;
				found = true;
				break;
			}
		}

		if (!found) {
			debugC(1, kDebugScript,
				   "vision gathering step %u has no dialogue line %u",
				   beatIndex, beat.line);
			releaseEventSpeaker();
			drawScene();
			showRoomMenu();
			return;
		}

		sayChosenLine();
		return;
	}

	static const Beat kBeats[] = {
		{ 0xff, 0 }, // 1321/132D: first communication-room tableau
		{ 0,  38 }, // Leto calls everyone together
		{ 1, 128 }, // Jessica answers him
		{ 0xff, 0 }, // 1332/133A: Duncan and Gurney join the roster
		{ 0,  39 }, // Leto begins congratulating Paul
		{ 0,  40 },
		{ 1, 129 },
		{ 0xff, 0 }, // 1341/1349/134B: the final room tableau
		{ 0,  41 }
	};
	static const uint16 kPalaceCursor[] = {
		0x132e, 0x1330, 0x1332, 0x133b, 0x133d,
		0x133f, 0x1341, 0x134c, 0x134e
	};

	if (step >= ARRAYSIZE(kBeats)) {
		releaseEventSpeaker();
		if (_state.size() > 0x477b) {
			releaseEventScript();
			// Retain 134E; a subsequent 1771 installs its own new cursor.
		}
		// The inline event roster never relocated Leto or Duncan. Their
		// physical room records still belong to the ordinary room menus.
		_speech.clear();
		drawScene();
		showRoomMenu();
		raisePanelFace(); // 175F/0FA7 restores the ordinary room and its face.
		return;
	}

	releaseEventSpeaker();

	// 1321/1332/1341 select counted CS rosters, not physical residents.
	// Keep Leto in the throne room and Duncan in the hall throughout; the
	// independent DOS gathering leaves all sixteen person records in place.

	const Beat &beat = kBeats[step];
	if (step == 0 || step == 3 || step == 7) {
		const uint16 alternate = step == 0 ? 0x1323 : step == 3 ? 0x1334 : 0x1343;
		enterEventRoom(8, alternate, step == 0 ? 0x132d : step == 3 ? 0x133a : 0x1349);
		carryFollowersAlong();
		if (step == 7 && _state.size() > 0x47c5)
			WRITE_LE_UINT16(_state.begin() + 0x47c4, 0); // Opcode 06's argument.
	}
	preparePalaceGathering(step + 1);
	WRITE_LE_UINT16(_state.begin() + 0x477a, kPalaceCursor[step]);
	if (beat.person == 0xff) {
		_speech.clear();
		drawScene();
		return;
	}
	selectLocalSpeaker(beat.person, false);
	_dialogueMask = 0x80;

	bool found = false;
	const uint firstPart = (uint)beat.person * 8;
	for (uint part = firstPart; part != firstPart + 8 && !found; part++) {
		if (part >= _dialogue.count())
			break;
		Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
		for (uint line = 0; line != records.size(); line++) {
			if (records[line].line != beat.line ||
				!dialogueRecordAvailable(records[line], _dialogueMask) ||
				!conditionHolds(records[line].condition))
				continue;
			_dialoguePart = part;
			_dialogueLine = line;
			found = true;
			break;
		}
	}

	if (!found) {
		debugC(1, kDebugScript, "palace gathering step %u has no available line %u",
			   step, beat.line);
		releaseEventSpeaker();
		drawScene();
		showRoomMenu();
		return;
	}

	sayChosenLine();
}

/** The equipment at a troop's place which it can take or put back. */
void DuneEngine::showEquipment(uint troop) {
	const uint32 record = 0x8aa + troop * 27;
	if (troop >= _map.troops().size() || record + 27 > _state.size() ||
			_state.size() < 0x4c98)
		return;

	_orderingTroop = troop;
	_state[0x46f5] = 1;
	Common::Array<Common::String> choices;
	_menuParts.clear();
	choices.push_back(command(160));
	_menuParts.push_back(7);
	armPanelMenuTransition();
	_menuKind = kMenuEquipment;
	showMenu(choices, nullptr, false);
	// 7CC6 rolls the Done list before 7CCB advances reports and 7D0F/7D15
	// paint the equipment cards. D410 changes only command rows: repainting
	// the whole panel would erase the lower chief card with its raised face.
	drawMenu();
	finishPanelMenuTransition();

	// 0x7CCB restarts the reports and advances to the personnel/equipment
	// card, unless that card is already showing. The first two reports are
	// followed by at most two more when a warning precedes the personnel.
	if (!_state[0x46f4]) {
		_troopInfoPage = 0;
		_speechParts.clear();
		_speechPart = 0;
		for (uint report = 0; report < 4; report++) {
			sayTroopInformation(false);
			if (report >= 1 && _state[0x46f4])
				break;
		}
	}

	prepareEquipmentCardGeometry(); // 7DD9 registers the spare window before inventory expansion.
	// DOS edits these two scratch inventories, not the troop's equipment
	// byte. Nothing is assigned until Done (0x7D68). They live in the saved
	// data segment so a checkpoint in this picker retains pending transfers.
	const Troop &selected = _map.troops()[troop];
	_state[0x3d] = _state[record + 0x19];
	for (uint item = 0; item < 7; item++) {
		_state[0x4705 + item] = (_state[record + 0x19] >> (7 - item)) & 1;
		_state[0x46fe + item] = equipmentSpare(selected.place, item);
	}

	// 7D0C paints inventory over the retained report/map. Entry must not
	// re-render terrain or reset the already registered actor pool.
	_graphics.setFrameBuffer(_framebuffer1);
	drawEquipmentCard(troop, true);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** DOS 7F27: total inventory minus equipment carried by the standing chain. */
uint DuneEngine::equipmentSpare(uint32 place, byte item) const {
	if (item >= 7 || place < 0x100 || place + 28 > _state.size())
		return 0;

	const byte bit = (byte)(0x80 >> item);
	uint spare = _state[place + 0x14 + item];
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint record = 0x8aa + standing[i] * 27;
		if ((_state[record + 0x19] & bit) && spare)
			spare--;
	}

	return spare;
}

/** Which of the equipment pictures in the report was pressed. */
int DuneEngine::equipmentAt(uint troop, const Common::Point &at) const {
	if (troop >= _map.troops().size() || _state.size() < 0x4c98 || !_state[0x46f5])
		return -1;

	for (uint row = 0; row < 2; row++) {
		// 0x7E97/0x7EB8 first test the row's frame, then the actual sprite
		// columns recorded while drawing. The panel opens ICONES afterwards,
		// so looking up widths in the current sprite bank here is incorrect.
		const Common::Rect box = equipmentCardRect(row != 0);
		if (at.x <= box.left || at.x >= box.right || at.y <= box.top || at.y >= box.bottom)
			continue;
		const uint table = row ? 0x4c60 : 0x4c7c;
		for (uint item = 0; item < 7; item++) {
			const uint left = READ_LE_UINT16(_state.begin() + table + 4 * item);
			const uint right = READ_LE_UINT16(_state.begin() + table + 4 * item + 2);
			if (left != 0 && at.x >= (int)left && at.x < (int)right)
				return row * 7 + item;
		}
	}
	return -1;
}

void DuneEngine::changeEquipment(uint troop, byte item) {
	if (troop >= _map.troops().size() || item >= 14 || _state.size() < 0x470c ||
			_menuKind != kMenuEquipment || !_state[0x46f5])
		return;

	const bool take = item >= 7;
	item %= 7;
	byte &carried = _state[0x4705 + item];
	byte &unused = _state[0x46fe + item];
	if (take) {
		if (carried)
			return;
		// 7EDA uses the painted hit column even after hourly work refreshes the
		// spare count. A pending return can still be picked up when it is zero.
		carried++;
		unused--;
	} else {
		if (!carried)
			return;
		carried--;
		unused++;
	}
	// 7EDE/7D0C repaints only the two inventory cards. A transfer must not
	// rebuild strategic terrain, restart the chief or re-register actors.
	_graphics.setFrameBuffer(_framebuffer1);
	drawEquipmentCard(troop, true);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 0x7D68 commits the picker, then selects request twelve from DIALOGUE. */
void DuneEngine::finishEquipment(bool announce) {
	finishEquipment(announce, true);
}

void DuneEngine::finishEquipment(bool announce, bool presentMenu) {
	const uint32 record = 0x8aa + _orderingTroop * 27;
	if (_orderingTroop >= _map.troops().size() || record + 27 > _state.size() ||
			_state.size() < 0x470c || !_state[0x46f5])
		return;
	const bool picker = _menuKind == kMenuEquipment;
	if (announce && picker && presentMenu)
		armPanelMenuTransition(); // D2E2's D316 precedes the 7D68 callback.
	const Common::Rect spare = equipmentCardRect(true);
	_state[0x46f5] = 0;
	WRITE_LE_UINT16(_state.begin() + 0xdbe2, 0); // 7D6F removes the spare window, not the chief's report.
	if (announce)
		paintMapActorChange(spare); // 7D78/C6AD before committing/replacing the actor.
	else
		restoreMapInformationBackground(spare); // Silent repair of an old native globe save.
	byte carried = 0;
	for (uint item = 0; item < 7; item++)
		if (_state[0x4705 + item])
			carried |= 0x80 >> item;
	_state[record + 0x19] = carried;
	finishEquipmentChange(false);
	if (!announce)
		return;
	prepareTroopContactContext(_orderingTroop);
	const bool replied = sayTroopRequest(12, false); // Reply under the Done list.
	_state[0x46f4] = 0; // 7DC2 clears the personnel flag after the request.
	if (!replied) {
		// 7BE0 falls through to 7C02, then 7DD5 calls 7C02 again. This
		// two-report fallback may finish on a new personnel card.
		sayTroopInformation(false);
		sayTroopInformation(false);
	}
	if (picker)
		showSettledTroopMenu(false, false); // D2FD restores retained 210A words.
	else
		showTroopMenu(false, false);
	drawMenu();
	if (!presentMenu)
		return; // D2BD pops/redraws the parent, without Done's D280/final flush.
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 7D81 is shared by the picture picker and equipment found locally. */
void DuneEngine::finishEquipmentChange(bool announce) {
	const uint32 record = 0x8aa + _orderingTroop * 27;
	// 7D81 replaces the actor before deriving the new/returned-item masks.
	// Its paint callbacks still observe the preceding equipment context.
	replaceMapUnit(_orderingTroop);
	const byte carried = _state[record + 0x19];
	const byte changed = carried ^ _state[0x3d];
	_state[0x3e] = _state[0x3d] & changed; // returned equipment
	_state[0x3d] = carried & changed;      // newly assigned equipment
	_state[0x3f] = (_state[0x3d] & 0x40) &&
		READ_LE_UINT16(_state.begin() + record + 4) == READ_LE_UINT16(_state.begin() + 0x1150)
		? 0x40 : 0;
	// 7D81 does not mark a harvester as seen. Only the exterior painter
	// (4D06) sets F6 when the machine is actually visible to Paul.
	// Bulbs can start/stop an irrigation job immediately (0x7DB1).
	if ((_state[record + 3] & 0xf) == 8)
		updateTroopWorkState(_orderingTroop);
	_map.readTroops(_state.begin(), _state.size());
	if (announce) {
		prepareTroopContactContext(_orderingTroop);
		const bool replied = sayTroopRequest(12, false); // Local FIND EQUIPMENT retains its occupation list.
		_state[0x46f4] = 0; // 7DC2 follows the request, not its painter.
		// 7DC7 keeps an existing paragraph when no request record matches.
		if (!replied && _speech.empty()) {
			sayTroopInformation(false); // 7DCE/7BE0 then 7DD5/7C02.
			sayTroopInformation(false);
		}
	}
}

/** 7BB9: equipment changes and failed searches use the original requests. */
void DuneEngine::sayEquipmentRequest(byte request) {
	// 77CC/D2FD restores the registered parent before a failed search's
	// request. Unlike a task assignment this pop does not arm a menu roll.
	if (_menuKind == kMenuOrders) {
		showSettledTroopMenu(false, false);
		drawMenu();
	}
	prepareTroopContactContext(_orderingTroop);
	sayTroopRequest(request, false); // 7BB9 keeps terrain, actors and report kind.
}

/** 7BBE selects the request against the already expanded dialogue context. */
bool DuneEngine::sayTroopRequest(byte request) {
	return sayTroopRequest(request, true);
}

/** 7BBE/96F1/9F40/9F8B: a troop request temporarily selects the chief. */
uint16 DuneEngine::prepareTroopRequestContext(uint16 record, byte request) {
	WRITE_LE_UINT16(_state.begin() + 0x46f1, record);
	_state[0x23] = request;
	WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47c4, 15);
	WRITE_LE_UINT16(_state.begin() + 0x47a2, 0x10c8);
	const uint16 previousMask = READ_LE_UINT16(_state.begin() + 0x47c2);
	_state[0x47c2] = 0x20;
	preparePersonDialogueContext(15);
	return previousMask; // 9F97 restores this after either selection result.
}

bool DuneEngine::sayTroopRequest(byte request, bool refreshMenu) {
	const uint32 record = 0x8aa + _orderingTroop * 27;
	_troopInfoPage = 0;
	if (refreshMenu)
		_state[0x46f4] = 0;
	const uint16 previousMask = prepareTroopRequestContext(record, request);
	// 7BBE retains the report/action flag. Equipment replies without an
	// action must not clear the preceding personnel report's 0x80 value.
	// Failed selection also retains the paragraph's continuation; only
	// sayLineInParts below replaces it when a new reply was actually found.
	if (_dialogue.count() > 124) {
		Common::Array<Dialogue::Record> &responses = _dialogue.entry(124);
		for (uint i = 0; i < responses.size(); i++) {
			Dialogue::Record &response = responses[i];
			if (!dialogueRecordAvailable(response, 0x20) || !conditionHolds(response.condition))
				continue;
			prepareDialogueLine(response, 15, record);
			const int previousSpeaker = _speakerPerson;
			_speakerPerson = 15;
			runLineAction(response);
			_speakerPerson = previousSpeaker;
			rememberBookRecord(response, 15);
			response.flags |= 0x80;
			_state[0x19] = 0xff;
			WRITE_LE_UINT16(_state.begin() + 0x47c2, previousMask); // 9F97, before 7BD6's map voice.
			playSpeech(response.line, 15); // 96F1/A042 precedes 7BD6.
			debugC(1, kDebugScript, "troop %u request response line %u: %s",
					_orderingTroop, response.line, _speech.c_str());
			if (refreshMenu) {
				showTroopMenu();
			} else {
				// 7BBE paints the reply over the retained chief/map under
				// its child list. The route/equipment caller owns the pop.
				const MenuKind previousMenu = _menuKind;
				_menuKind = kMenuTroopContact;
				const byte reportKind = _state[0x46f4];
				_state[0x46f4] = 0; // 7BBE paints no kit; the caller owns its flag.
				_graphics.setFrameBuffer(_framebuffer1);
				drawTroopStatus(_orderingTroop);
				_state[0x46f4] = reportKind;
				_menuKind = previousMenu;
				_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
						kScreenWidth, kScreenHeight);
				_system->updateScreen();
			}
			return true;
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0x47c2, previousMask); // Failed scans unwind through 9F97 too.
	if (!refreshMenu) {
		// Failed 96F1 selection still reaches 7C56/C07C with the retained
		// card. The equipment caller then selects 7DCE's fallback reports.
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
	return false;
}

/** 82DA: assess the proposed place without committing a new journey. */
bool DuneEngine::requestTroopMove(uint16 destination) {
	return requestTroopMove(destination, true);
}

bool DuneEngine::requestTroopMove(uint16 destination, bool refreshMenu) {
	const uint record = 0x8aa + _orderingTroop * 27;
	if (_orderingTroop >= _map.troops().size() || destination < 0x100 ||
			(uint)destination + 28 > _state.size())
		return false;
	const uint16 previous = READ_LE_UINT16(_state.begin() + record + 4);
	byte request = 16;
	if (_orderingTroop == 2 || destination != previous) {
		// 82EC temporarily names the destination for 31F6/2E98, then restores
		// only the place word. Production estimates/flags from that query are
		// real changes; refusal does not reset the occupation or work clock.
		WRITE_LE_UINT16(_state.begin() + record + 4, destination);
		_map.readTroops(_state.begin(), _state.size());
		prepareTroopContactContext(_orderingTroop);
		preparePlaceNameContext(destination);
		WRITE_LE_UINT16(_state.begin() + record + 4, previous);
		_map.readTroops(_state.begin(), _state.size());
		request = 11;
	}
	// 82FA/82FD changes the font/request latch, not the existing paragraph.
	// 7BBE replaces text only when it actually selects a movement reply.
	_state[0x47a5] = 0xff; // 82FD calls A1C4 before the movement request.
	sayTroopRequest(request, refreshMenu);
	return _state[0x47a5] == 0xff;
}

/** 77D7: try an unused local item, retaining Paul's last local ornithopter. */
bool DuneEngine::takeLocalEquipment(uint troop, byte item) {
	const uint at = 0x8aa + troop * 27;
	if (troop >= _map.troops().size() || at + 27 > _state.size() || item >= 7)
		return false;
	const uint place = READ_LE_UINT16(_state.begin() + at + 4);
	for (uint i = 0; i < 7; i++)
		_state[0x46fe + i] = equipmentSpare(place, i);
	const uint needed = item == 1 && place == READ_LE_UINT16(_state.begin() + 0x1150) ? 2 : 1;
	if (_state[0x46fe + item] < needed)
		return false;
	_state[0x3d] = _state[at + 0x19];
	_state[at + 0x19] |= 0x80 >> item;
	return true;
}

/** 7F90/8018: nearest eligible supplier, accounting for other collectors. */
uint16 DuneEngine::findEquipmentSupplier(uint troop, byte item) {
	const uint at = 0x8aa + troop * 27;
	WRITE_LE_UINT16(_state.begin() + 0x470c, 0xffff);
	if (troop >= _map.troops().size() || at + 27 > _state.size() || item >= 7)
		return 0;
	const uint home = READ_LE_UINT16(_state.begin() + at + 4);
	if (home < 0x100 || home + 28 > _state.size())
		return 0;
	const uint16 longitude = READ_LE_UINT16(_state.begin() + home + 2);
	const uint16 latitude = READ_LE_UINT16(_state.begin() + home + 4);
	// The original excludes both palaces, even when they have unused stock.
	for (uint place = 0x138; place + 28 <= _state.size(); place += 28) {
		if (READ_LE_UINT16(_state.begin() + place) == 0xffff)
			break;
		if (place == home || (_state[place + 0xa] & 0x80) || _state[place + 8] >= 0x28)
			continue;
		const uint dx = ABS((int)(int16)(READ_LE_UINT16(_state.begin() + place + 2) - longitude)) >> 8;
		const uint dy = ABS((int)(int16)(READ_LE_UINT16(_state.begin() + place + 4) - latitude));
		uint distance = dx >= (byte)dy ? dx : dy;
		if ((byte)distance >= 50)
			continue;
		// Villages are preferred, but still have to be within the original range.
		if (_state[place + 8] >= 0x21)
			distance >>= 2;
		if (distance >= READ_LE_UINT16(_state.begin() + 0x470c))
			continue;
		for (uint i = 0; i < 7; i++)
			_state[0x4c60 + i] = equipmentSpare(place, i);
		if (!_state[0x4c60 + item])
			continue;
		// 8018 uses DEC/JNS, not an unsigned saturating subtraction. Preserve
		// that byte behavior even for unusually large inventory counts.
		auto reserve = [&](uint scratch) {
			const byte remaining = _state[scratch] - 1;
			if ((int8)remaining >= 0)
				_state[scratch] = remaining;
		};
		for (uint other = 0x8aa; other < 0xfbb; other += 27) {
			if ((_state[other + 3] & 0x43) == 0x43 &&
					READ_LE_UINT16(_state.begin() + other + 4) == place &&
					READ_LE_UINT16(_state.begin() + other + 0xc) != place &&
					_state[other + 0xe] == item)
				reserve(0x4c60 + item);
		}
		if (place == READ_LE_UINT16(_state.begin() + 0x1150) && storyStage() < 80)
			reserve(0x4c61);
		if (!_state[0x4c60 + item])
			continue;
		WRITE_LE_UINT16(_state.begin() + 0x470c, distance);
		WRITE_LE_UINT16(_state.begin() + 0x470e, place);
	}
	return READ_LE_UINT16(_state.begin() + 0x470c) == 0xffff ? 0
		: READ_LE_UINT16(_state.begin() + 0x470e);
}

/** 7734/775C/776D: FIND EQUIPMENT chooses an item before changing the job. */
void DuneEngine::findTroopEquipment(uint troop) {
	const uint at = 0x8aa + troop * 27;
	if (troop >= _map.troops().size() || at + 27 > _state.size())
		return;
	_orderingTroop = troop;
	const byte category = _state[at + 3] & 0xc;
	const uint first = category == 0 ? 0 : category == 4 ? 2 : 6;
	const uint end = category == 0 ? 2 : category == 4 ? 6 : 7;
	uint item = first;
	while (item < end && (_state[at + 0x19] & (0x80 >> item)))
		item++;
	if (item == end) {
		sayEquipmentRequest(15);
		return;
	}
	WRITE_LE_UINT16(_state.begin() + 0x1203, 0xe8 + item);
	if (takeLocalEquipment(troop, item)) {
		finishEquipmentChange();
		return;
	}
	const uint16 supplier = findEquipmentSupplier(troop, item);
	if (!supplier) {
		sayEquipmentRequest(14);
		return;
	}
	// Request ten can refuse the new job. Do not reserve or depart until
	// that dialogue has accepted it (A1E2 tests DS:47A5, not the task byte).
	giveOrder(troop, category | 3);
	if (_state[0x47a5] != 0xff)
		return;
	// 6A89 has already presented the acknowledgement and rolled the parent
	// menu through D2E2/D280. 77BD departs immediately after that roll; it
	// does not clear the rows or advance the game while waiting for speech.
	WRITE_LE_UINT16(_state.begin() + at + 0xe, item | ((0x80 >> item) << 8));
	WRITE_LE_UINT16(_state.begin() + at + 0xc, READ_LE_UINT16(_state.begin() + at + 4));
	_troopRoutes.resize(67 * 3);
	_troopRoutes[troop * 3] = (supplier - 0x100) / 28 + 1;
	_troopRoutes[troop * 3 + 1] = _troopRoutes[troop * 3 + 2] = 0;
	beginTroopRoute(troop);
	// 77C0 sets the fast-close latch before 8770 unwinds the contact.
	// This automatic departure is not CUT CONTACT's effect-4 contraction.
	_state[0x46d8] = 1;
	finishTroopContact(true);
	_state[0x46f3] = 0;
	_troopOrdersFromMap = true;
	showMapMenu(false); // 8770/878C changes rows without rebuilding terrain.
	drawMenu();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** ONMAP equipment columns, including DOS 0x61D3's stacked spare items. */
void DuneEngine::drawEquipmentRow(const byte *counts, int x, int y, int bottom,
		byte ink, uint hitTable, Font::Face noneFace, byte textBackground) {
	openSpriteSheet("ONMAP.HSQ", false);
	bool empty = true;
	if (hitTable)
		memset(_state.begin() + hitTable, 0, 7 * 4);
	for (uint item = 0; item < 7; item++) {
		uint count = counts[item];
		if (!count)
			continue;
		empty = false;
		const uint sprite = _state[0x192f + item];
		int width = 0, height = 0;
		if (!currentSpriteSheet->size(sprite, &width, &height))
			continue;
		int step = height + 2;
		const int room = bottom - y;
		if (room / (int)count < step) {
			step = MAX(2, (room - height - 2) / (int)count);
			if ((room - height - 2) / (int)count < 2)
				count = MAX(0, (room - height - 2) / 2);
		}
		for (uint copy = 0; copy < count; copy++)
			drawSprite(sprite, x, y + copy * step);
		if (hitTable) {
			WRITE_LE_UINT16(_state.begin() + hitTable + 4 * item, x);
			WRITE_LE_UINT16(_state.begin() + hitTable + 4 * item + 2, x + width + 1);
		}
		x += width + 1;
	}
	if (empty) {
		_font.setFace(noneFace);
		_font.drawString(_framebuffer1, x + 12, y + 5, command(104), ink, textBackground);
	}
}

/** 7DD9 registers both equipment rectangles and the independent spare window. */
void DuneEngine::prepareEquipmentCardGeometry() {
	if (!_state[0x46f5])
		return; // A closing request must not resurrect the window during a redraw.
	const uint16 top = _troopContactTop ? _troopContactTop : 5;
	const uint16 unusedTop = top + 68 >= 112 ? top + 68 - 109 : top + 68;
	const uint16 bounds[2][4] = {
		{ 130, (uint16)(top + 43), 232, (uint16)(top + 67) },
		{ 53, unusedTop, 232, (uint16)(unusedTop + 40) }
	};
	for (uint row = 0; row < 2; row++)
		for (uint edge = 0; edge < 4; edge++)
			WRITE_LE_UINT16(_state.begin() + 0x1936 + 10 * row + 2 * edge, bounds[row][edge]);
	WRITE_LE_UINT16(_state.begin() + 0xdbe2, 0x1940);
}

/** The same retained descriptors drive 7E97/7EB8 hits and card painting. */
Common::Rect DuneEngine::equipmentCardRect(bool spare) const {
	const byte *frame = _state.begin() + (spare ? 0x1940 : 0x1936);
	return Common::Rect(READ_LE_UINT16(frame), READ_LE_UINT16(frame + 2),
			READ_LE_UINT16(frame + 4), READ_LE_UINT16(frame + 6));
}

/** The carried and unused equipment rows which stand over the Dune map. */
void DuneEngine::drawEquipmentCard(uint troop, bool refreshStock) {
	if (troop >= _map.troops().size() || _state.size() < 0x4c98)
		return;
	// Only 7D0C at entry and after a transfer uses the current scratch.
	// Hourly work can refresh 46FE without repainting the equipment card.
	if (refreshStock)
		memcpy(_equipmentCardSpare, _state.begin() + 0x46fe, sizeof(_equipmentCardSpare));
	// Rebuild the existing DS fields on load too: older checkpoints drew
	// these cards without retaining 7DD9's geometry/window identity.
	prepareEquipmentCardGeometry();
	// 0x7DD9 places the unused store below an upper contact, or above a lower
	// one. Colours and the seven nonconsecutive icons come from the DS tables.
	const Common::Rect carried = equipmentCardRect(false), unused = equipmentCardRect(true);
	_graphics.fillRect(carried.left, carried.top, carried.right, carried.bottom, _state[0x193e]);
	_graphics.fillRect(carried.left + 1, carried.top + 1, carried.right - 1, carried.bottom - 1, _state[0x193f]);
	_graphics.fillRect(unused.left, unused.top, unused.right, unused.bottom, _state[0x1948]);
	_graphics.fillRect(unused.left + 1, unused.top + 1, unused.right - 1, unused.bottom - 1, _state[0x1949]);
	drawEquipmentRow(_state.begin() + 0x4705, carried.left + 3, carried.top + 2, carried.bottom, 240, 0x4c7c);
	drawEquipmentRow(_equipmentCardSpare, unused.left + 80, unused.top + 4,
			unused.bottom, _state[0x1948], 0x4c60);
	_font.setFace(Font::kFaceNormal);
	const Common::String label = command(110);
	uint start = 0;
	// 7E58 advances BX by five for NONE. Unlike the stacked-icon path,
	// that branch leaves the changed baseline for 7D59's store label too.
	bool spareEmpty = true;
	for (byte count : _equipmentCardSpare)
		spareEmpty &= count == 0;
	int y = unused.top + 4 + (spareEmpty ? 5 : 0);
	for (uint end = 0; end <= label.size(); end++) {
		if (end != label.size() && label[end] != '\r')
			continue;
		_font.drawString(_framebuffer1, unused.left + 8, y,
				Common::String(label.c_str() + start, end - start), _state[0x1948]);
		start = end + 1;
		y += 10;
	}
}

/** Draws the troop's face and report in ONMAP's top contact box. */
void DuneEngine::drawTroopStatus(uint troop) {
	enum {
		kLeft = 5, kUpperTop = 5, kLowerTop = 80, kRight = 232, kHigh = 67,
		kFaceLeft = 10, kFaceInsetTop = 4, kFaceWide = 59, kFaceHigh = 59,
		kTextLeft = 78, kTextInsetTop = 3, kTextRight = 231, // DS:2248: width 153
		kRowHeight = 10, kEdge = 245, kInside = 251, kInk = 240,
		kEquipmentLeft = kLeft + 128, kEquipmentInsetTop = 45
	};

	// 0x79EE puts the transmission in the opposite half of the screen from
	// the selected unit, not according to its distance from Paul. The equipment
	// picker and route editor retain that half throughout the contact.
	const bool unitAbove = troop < _troopRects.size() &&
		!_troopRects[troop].isEmpty() && _troopRects[troop].top < kSceneHeight / 2;
	if (_troopContactTop == 0)
		_troopContactTop = unitAbove ? kLowerTop : kUpperTop;
	const int top = _troopContactTop;
	const int bottom = top + kHigh;
	const int faceTop = top + kFaceInsetTop;
	const bool surveyInstruction = _menuKind == kMenuMoveTroop ||
		((_menuKind == kMenuProspectorBriefing || isStageEventMenu()) && _troopInfoPage == 1);
	// 7A53..7A64 retains this source for 80DF/553E's MAP2 opening.
	WRITE_LE_UINT16(_state.begin() + 0x18f3, kFaceLeft - 1);
	WRITE_LE_UINT16(_state.begin() + 0x18f5, faceTop - 1);
	WRITE_LE_UINT16(_state.begin() + 0x18f7, kFaceLeft + kFaceWide + 1);
	WRITE_LE_UINT16(_state.begin() + 0x18f9, faceTop + kFaceHigh + 1);
	// 80F2 moves a lower route instruction below the raised map inset.
	const int textTop = top + kTextInsetTop +
			(surveyInstruction && top >= 50 ? 38 : 0);

	_graphics.fillRect(kLeft, top, kRight, bottom, kEdge);
	// 7B1B/C551 outlines both retained rectangles with one pixel, not two.
	_graphics.fillRect(kLeft + 1, top + 1, kRight - 1, bottom - 1, kInside);
	if (ConfMan.hasKey("dune_fremenid") && 0x8aa + troop * 27 < _state.size())
		_state[0x8aa + troop * 27] = (byte)ConfMan.getInt("dune_fremenid");

	// DOS 7A6E/7A82 uses HARK.HSQ (person 12) for a prisoner during an
	// active battle or at an uncaptured fort. 5D36 leaves carry set for a
	// sietch or captured fort, selecting the Fremen portrait instead. The
	// Harkonnen head has its own fourth crop entry in DS:22B9.
	const uint record = 0x8aa + troop * 27;
	const uint16 place = READ_LE_UINT16(_state.begin() + record + 4);
	const bool validPlace = place >= 0x100 && (uint)place + 0xa < _state.size();
	const int retainedBank = _troopPortraitKey / 256;
	const bool retainedPortrait = (retainedBank == 12 || (retainedBank >= 14 && retainedBank <= 16)) &&
		(_troopPortraitVisible.size() == kFaceWide * kFaceHigh ||
		 (hasTroopContact() && READ_LE_UINT16(_state.begin() + 0x46ef) == record));
	// 7BA3 skips 79EE for the same 46EF record. Capture/battle changes do
	// not replace its already opened face, including after a save and load.
	const bool harkonnenPortrait = retainedPortrait ? retainedBank == 12 :
		(_state[record + 3] & 0x20) && validPlace &&
		((_state[place + 0xa] & 2) ||
		(_state[place + 8] >= 0x28 && !(_state[place + 0xa] & 8)));
	const int person = harkonnenPortrait ? 12 : 15; // 7A96 selects the map chief, not the room's generic Fremen.
	const int pictured = fremenPicture(person);
	const uint crop = 0x22b9 + (harkonnenPortrait ? 12 : 4 * (pictured - 14));
	if ((!harkonnenPortrait && (pictured < 14 || pictured > 16)) ||
		crop + 4 > _state.size())
		return;
	// 0x7AC1/0x9D94 crop the complete sprite to a 59x59 card; neither bank
	// is scaled to the visible face's own bounding box.
	const int sourceLeft = READ_LE_UINT16(_state.begin() + crop);
	const int sourceTop = READ_LE_UINT16(_state.begin() + crop + 2);
	if (sourceLeft + kFaceWide > kScreenWidth || sourceTop + kFaceHigh > kScreenHeight)
		return;

	// Compose the room portrait off-screen so the same animation parts can be
	// copied into the DOS window without painting over the surrounding map.
	const int diagnosticAnimation = ConfMan.hasKey("dune_fremenanimation")
		? ConfMan.getInt("dune_fremenanimation") : 0;
	drawTroopPortrait(person, sourceLeft, sourceTop, diagnosticAnimation, speechMouth());
	_graphics.setFrameBuffer(_framebuffer1);
	// 91A0/C1AA installs the chief's palette. ONMAP leaves 32..79 free for
	// these colours; their shared 20..31 entries already agree. Quantizing to
	// the previous room's palette changes the face when contacting a new chief.

	_graphics.fillRect(kFaceLeft - 1, faceTop - 1,
					   kFaceLeft + kFaceWide + 1, faceTop + kFaceHigh + 1, kEdge);
	_graphics.fillRect(kFaceLeft, faceTop,
					   kFaceLeft + kFaceWide, faceTop + kFaceHigh, 228);
	for (int y = 0; y < kFaceHigh; y++) {
		for (int x = 0; x < kFaceWide; x++) {
			const byte pixel = _framebuffer2[(sourceTop + y) * kScreenWidth + sourceLeft + x];
			_framebuffer1[(faceTop + y) * kScreenWidth + kFaceLeft + x] = pixel;
		}
	}

	// 79FA installs a five-pixel right inset before 8E16 lays out the
	// contact text. The old candidate-string wrapper used the font's four-
	// pixel space, admitting words which DOS's six-pixel spacing wraps.
	const SpeechLayout layout = layoutSpeech(_font, _speech,
			kTextRight - kTextLeft - 5);

	// 69B3 changes only the panel menu. Keep the personnel report's carried
	// kit (or NONE) during occupation selection and actor-driven repaints.
	// Order replies suppress the kit painter without clearing the stored kind.
	const bool equipmentReport = (_menuKind == kMenuTroopContact ||
		_menuKind == kMenuOrders || _menuKind == kMenuEquipment) &&
		_state.size() > 0x46f4 && _state[0x46f4] != 0;
	const uint rowCount = MIN<uint>(layout.rows.size(), 6);
	// 80DF temporarily reduces the route instruction's text window to 25
	// pixels. Centring it in the usual 63 pixels hides it under the inset.
	const int textHigh = surveyInstruction ? 25 : 63;
	const int firstRow = textTop + MAX(0, (textHigh - (int)rowCount * kRowHeight) / 2);
	// The final dashed PHRASE row is painted too: 7E1E adds the equipment
	// icons afterward without erasing the part of that row to their left.
	paintSpeechLayout(_font, _framebuffer1, layout, kTextLeft, firstRow,
			rowCount, kInk);

	// Action seven requests the picture-based card (0x7EFB/0x7E1E).
	// DS:192F supplies its ONMAP sprites; they are not seven consecutive
	// frames. Each displayed sprite advances x by its actual width plus one.
	if (equipmentReport && troop < _map.troops().size()) {
		const uint32 record = 0x8aa + troop * 27;
		const byte carried = record + 0x19 < _state.size()
			? _state[record + 0x19] : 0;
		openSpriteSheet("ONMAP.HSQ", false);
		int x = kEquipmentLeft;
		for (byte item = 0; item != 7; item++) {
			if ((carried & (byte)(0x80 >> item)) == 0)
				continue;
			const uint sprite = _state[0x192f + item];
			int width = 0, height = 0;
			if (currentSpriteSheet->size(sprite, &width, &height)) {
				drawSprite(sprite, x, top + kEquipmentInsetTop);
				x += width + 1;
			}
		}
		if (carried == 0)
			_font.drawString(_framebuffer1, kEquipmentLeft + 12,
					top + kEquipmentInsetTop + 5, command(104), kInk);
	}
}

/**
 * What a troop may be told to do.  The table of task routines at 0x6C26 has
 * sixteen ways in it, but the ones that do anything are these; the rest all
 * point at the same empty routine.
 */
void DuneEngine::showOrders(uint troop) {
	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();

	const Common::Array<Troop> &troops = _map.troops();
	if (troop >= troops.size())
		return;

	auto add = [&](uint commandNumber, byte order, bool available) {
		choices.push_back(command(commandNumber));
		_menuParts.push_back(order);
		flags.push_back(available ? 0 : 0x4000);
	};

	const byte current = troops[troop].order();
	const bool ecology = _state.size() > 0xa && (_state[0xa] & 0x20) != 0;
	// DOS 69F6 greys the first FIND EQUIPMENT row in the ordinary
	// specialization templates until story stage 16. The first-assignment
	// and prospectors' templates take separate paths through 69B3.
	const bool canFindEquipment = storyStage() >= 16;

	// These are the five original templates at DS:0x215A..0x21A6.  A troop
	// awaiting its first job receives the three specialisations seen in the
	// recording; an experienced troop gets the commands for its current field.
	if (current == 2) {
		add(116, 0, true);
		add(117, 4, true);
		add(118, 8, ecology);
	} else if (current < 4) {
		add(92, 3, canFindEquipment);
		add(117, 4, true);
		add(118, 8, ecology);
	} else if (current < 8) {
		if (current == 5) {
			add(95, 6, true);
		} else {
			add(92, 7, canFindEquipment);
			// 0x69D5 clears this row's greyed bit and 0x69E2 puts it back
			// when the word at DS:0xE2 stands at thirty or over, so the row
			// is offered while that number is small and refused once it
			// grows - the other way about from the test here, which offered
			// it only once it had grown.  DS:0xE2 is how far off the nearest
			// village still to be found lies: there is somewhere worth
			// sending a spy while one is close, and nothing to spy on when
			// the last of them is far away.
			add(94, 5, _state.size() > 0xe3
					   && READ_LE_UINT16(_state.begin() + 0xe2) < 30);
			add(116, 0, true);
			add(118, 8, ecology);
		}
	} else {
		add(92, 11, canFindEquipment);
		add(96, 9, true);
		add(116, 0, true);
		add(117, 4, true);
	}

	// The game's own lists of what a troop may be set to end in a way out,
	// which is COMMAND1.HSQ's hundred and sixty-third word; without it the
	// player has to give an order to be rid of the list.
	choices.push_back(command(kCancel));
	_menuParts.push_back(kGoBack);
	flags.push_back(0);

	_orderingTroop = troop;
	_menuKind = kMenuOrders;
	showMenu(choices, &flags, false, true); // 6A28 -> D323, including D316.
	// 69B3 changes only the panel: it neither registers map actors nor
	// repaints the terrain and retained chief report.
	drawMenu();
	finishPanelMenuTransition();
	g_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	g_system->updateScreen();
}

/** 6AB5/D2E2 pops the occupation list without replaying the chief's report. */
void DuneEngine::finishTroopOccupationMenu() {
	if (_menuKind == kMenuOrders) {
		armPanelMenuTransition(); // D316 precedes the 0F66 (RET) close callback.
		showSettledTroopMenu(false, false);
	} else {
		showTroopMenu(false, false);
	}
	// D2EA/D2FD restore the parent command list without a scene refresh.
	drawMenu();
	finishPanelMenuTransition();
	g_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	g_system->updateScreen();
}

/** DOS 6A45: assign espionage and send the troop to the nearest hidden fort. */
void DuneEngine::startTroopEspionage(uint troop) {
	const uint record = 0x8aa + 27 * troop;
	if (troop >= _map.troops().size() || record + 27 > _state.size())
		return;
	// 6A35 preserves the current specialization. 6A89 owns the initial
	// acknowledgment/refusal and occupation-menu pop, but not this journey.
	giveOrder(troop, (_state[record + 3] & 0xc) | 1);
	_state[0x46d8] = 1; // 6A4A suppresses subsequent report-window effects.
	_state[record + 0x10] &= 0xbf; // Clear the preceding espionage result.
	expandPlaceContext(READ_LE_UINT16(_state.begin() + record + 4));
	const uint16 destination = READ_LE_UINT16(_state.begin() + 0xe4);
	if (!requestTroopMove(destination, false)) {
		// 6AC5 reassigns the field's base task after a movement refusal.
		setTroopOrder(troop, _state[record + 3] & 0xfc);
		_map.readTroops(_state.begin(), _state.size());
		return;
	}
	// Unlike a manual route or equipment search, 6A65/8770 closes contact
	// BEFORE 6A68/84A6 departs. The destination can still be undiscovered;
	// do not route it through the known-place-only MAP2 destination picker.
	finishTroopContact(true);
	_state[0x46f3] = 0;
	_troopOrdersFromMap = true;
	showMapMenu(false);
	drawMenu();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
	_troopRoutes.resize(67 * 3);
	_troopRoutes[troop * 3] = (destination - 0x100) / 28 + 1;
	_troopRoutes[troop * 3 + 1] = _troopRoutes[troop * 3 + 2] = 0;
	beginTroopRoute(troop);
	_map.readTroops(_state.begin(), _state.size());
}

/** DOS 0x6C15/0x6BB6: job prerequisites and the waiting-to-working transition. */
bool DuneEngine::updateTroopWorkState(uint troop) {
	const uint32 record = 0x8aa + troop * 27;
	if (record + 27 > _state.size())
		return false;
	const uint16 place = READ_LE_UINT16(_state.begin() + record + 4);
	if (place < 0x100 || (uint32)place + 28 > _state.size())
		return false;
	const byte order = _state[record + 3] & 0xf;
	const byte previousState = _state[record + 3];
	bool blocked = false;
	if (order == 0) {
		blocked = (READ_LE_UINT16(_state.begin() + record + 0x10) & 0x200) ||
			(_state[record + 0x12] & 0x30) || _state[place + 0x12] == 0 ||
			(_state[place + 0xa] & 0x41) != 0x40;
	} else if (order == 1) {
		blocked = (_state[place + 0xa] & 0x41) != 0;
	} else if (order == 8) {
		blocked = (_state[record + 0x12] & 0x30) || _state[place + 0x1b] == 0 ||
			!(_state[place + 0xa] & 0x20) || !(_state[record + 0x19] & 2);
	} else {
		return true;
	}
	if (blocked) {
		_state[record + 3] |= 0x10;
	} else if (_state[record + 3] & 0x10) {
		_state[record + 3] &= (byte)~0x10;
	}
	// 6BB6 calls 8461 when the waiting bit changes. That is a fresh actor,
	// not merely a different sprite on the next map redraw.
	if ((_state[record + 3] ^ previousState) & 0x10) {
		replaceMapUnit(troop);
		// 6BD2 resets resumed work AFTER the replacement's paint callbacks.
		// They observe the new task with its previous clock and production.
		if (!blocked) {
			WRITE_LE_UINT16(_state.begin() + record + 0xa, hour());
			WRITE_LE_UINT16(_state.begin() + record + 0xc, 0);
			WRITE_LE_UINT16(_state.begin() + record + 0xe, 0);
		}
	}
	return !blocked;
}

/** DOS 0x6ACB; restart enters at 0x6AD4 when movement releases a strike. */
bool DuneEngine::setTroopOrder(uint troop, byte order, bool restart) {
	const uint32 record = 0x8aa + troop * 27;
	if (troop >= _map.troops().size() || record + 27 > _state.size())
		return false;
	// The requested byte is deliberately not masked: rollback passes the
	// previous task's waiting/travel bits back through the prerequisite check.
	if (!restart && (_state[record + 3] & 0xf) == order)
		return false;
	const uint16 place = READ_LE_UINT16(_state.begin() + record + 4);
	if (place < 0x100 || (uint32)place + 28 > _state.size())
		return false;
	if (order == 8 && place == 0x7c8 && _state[place + 0x1a] == 0)
		order = 10;

	_state[record + 3] = order;
	_state[record + 0x12] &= 0xcf;
	_state[record + 0x11] &= 0xfe;
	if (updateTroopWorkState(troop))
		_state[record + 0x11] |= 1;
	// 6B00 calls 8461 unconditionally. Comparing only list and position on
	// the next redraw misses reassignments whose two tasks share a sprite list.
	replaceMapUnit(troop);
	WRITE_LE_UINT16(_state.begin() + record + 0xa, hour());
	WRITE_LE_UINT16(_state.begin() + record + 0xc, 0);
	WRITE_LE_UINT16(_state.begin() + record + 0xe, 0);
	// 0x6B06 records the assigned field, even while waiting for equipment.
	if (_state[record + 3] != 2) {
		const uint category = (_state[record + 3] & 0xf) >> 2;
		WRITE_LE_UINT16(_state.begin() + record + 0x12,
				READ_LE_UINT16(_state.begin() + record + 0x12) | (uint16)(0x2000 << category));
	}
	// 6B19/6B21 updates 210A before the reply (and again on rollback).
	// Background assignment to a different troop must not change this menu.
	if (_state[record] == _state[0x1954])
		updateTroopMenuTemplate(troop);
	return true;
}

void DuneEngine::giveOrder(uint troop, byte order) {
	const uint32 record = 0x8aa + troop * 27;
	if (troop >= _map.troops().size() || record + 27 > _state.size())
		return;

	auto repaintChangedActor = [&]() {
		if (!_mapActorNeedsRepaint)
			return;
		// Replacement already presented its own C58A/C6AD rectangles.
		// Flush only an independently pending host composite, without 5D82.
		const bool previousRedraw = _mapAnimationRedraw;
		_mapAnimationRedraw = true;
		repaintMapActors(false);
		_mapAnimationRedraw = previousRedraw;
	};
	auto presentReply = [&](bool paintReport) {
		if ((int8)_state[0x46eb] < 0)
			WRITE_LE_UINT16(_state.begin() + 0x479e, 0); // 7C56/9901.
		if (paintReport) {
			_graphics.setFrameBuffer(_framebuffer1);
			// 7BB9 does not clear 46F4 or invoke the inventory painter.
			// Replace the paragraph without changing the last report kind.
			const byte reportKind = _state[0x46f4];
			_state[0x46f4] = 0;
			drawTroopStatus(troop);
			_state[0x46f4] = reportKind;
		}
		// 7BB9 updates the paragraph, not terrain, actors or command rows.
		// Failed selection skips its painter and keeps the previous card.
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
	};

	order &= 0xf;
	const bool firstProspectors = _state[record] == 3;
	// SPECIALIZE IN SPICE means prospecting for this one specialist troop;
	// treating the same menu row as mining made it immediately refuse the job
	// for lack of a survey, even though this is the troop that performs it.
	if (firstProspectors && order == 0)
		order = 1;

	// 0x6A89 ignores an unchanged task; reselecting it must not restart its
	// work clock or discard accumulated production.
	if ((_state[record + 3] & 0xf) == order) {
		finishTroopOccupationMenu();
		return;
	}
	const byte previousOrder = _state[record + 3];
	const uint16 previousStatus = READ_LE_UINT16(_state.begin() + record + 0x12);

	if (!setTroopOrder(troop, order)) {
		finishTroopOccupationMenu();
		return;
	}
	repaintChangedActor();
	order = _state[record + 3] & 0xf;
	_map.readTroops(_state.begin(), _state.size());
	prepareTroopContactContext(troop);

	// 0x6A89 tries the task, then 0x7BB9 asks the chief's request dialogue
	// (part 124) with request 10 and mask 0x20. The first applicable record
	// supplies either the acknowledgement or a refusal (action two). In
	// particular, army refusal depends on the story unlock, not troop zero's
	// motivation; mining acknowledgements depend on equipment and ability.
	const uint16 previousMask = prepareTroopRequestContext(record, 10);
	_state[0x47a5] = 0xff;
	Dialogue::Record *response = nullptr;
	if (_dialogue.count() > 124) {
		Common::Array<Dialogue::Record> &responses = _dialogue.entry(124);
		for (uint i = 0; i < responses.size(); i++) {
			if (dialogueRecordAvailable(responses[i], 0x20) &&
					conditionHolds(responses[i].condition)) {
				response = &responses[i];
				break;
			}
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0x47c2, previousMask); // 9F97, before the map reply/voice.
	// 7BD4 skips both 88AF and 9EFD when selection fails. Keep the old
	// paragraph and its continuation, not just the already painted pixels.
	if (response) {
		_speechParts.clear();
		_speechPart = 0;
	}
	if (response && (response->flags & 0xf) == 2) {
		prepareDialogueLine(*response, 15, record);
		rememberBookRecord(*response, 15);
		response->flags |= 0x80;
		_state[0x19] = 0xff;
		_state[0x47a5] = 0;
		playSpeech(response->line, 15);
		// 7BBE/7C56 presents this reply while the trial task and the old
		// occupation list still stand, before 6AAF rolls the task back.
		presentReply(true);
		// 0x6AAF restores only the status word, then reapplies the old task
		// through 0x6ACB. Its prerequisites, work flag and clock are evaluated
		// again; copying the entire record resurrected stale waiting/strike
		// state and production-change notices cleared by the trial report.
		WRITE_LE_UINT16(_state.begin() + record + 0x12, previousStatus);
		setTroopOrder(troop, previousOrder);
		repaintChangedActor();
		_map.readTroops(_state.begin(), _state.size());
		// The old job is expanded on the next report, not while this refusal
		// is still visible: querying production now would refill its reset
		// last-yield word, unlike 0x6AAF/0x6B25.
		_troopInfoPage = 0;
		debugC(1, kDebugScript, "troop %u refused order %u: %s", troop, order, _speech.c_str());
		finishTroopOccupationMenu();
		return;
	}

	if (firstProspectors && order == 1 && response && (response->flags & 0xf) == 3) {
		prepareDialogueLine(*response, 15, record);
		// 7BB9/96F1 runs the selected record's action, not a hard-coded
		// MAP2 handover. A naturally delayed first order can select134F.
		runLineAction(*response);
		rememberBookRecord(*response, 15);
		response->flags |= 0x80;
		_state[0x19] = 0xff;
		playSpeech(response->line, 15); // Action/read mark precede 7BD6's voice.
		_troopInfoPage = 0;
		debugC(1, kDebugScript, "troop %u is set to %s", troop, orderName(order).c_str());
		presentReply(true); // 7BBE precedes 6AB5/D2E2's return.
		if (READ_LE_UINT16(_state.begin() + 0x477a) == 0x12f8)
			showProspectorBriefing(_menuKind == kMenuOrders);
		else
			showStageEventMenu(true, _menuKind == kMenuOrders);
		return;
	}

	_troopInfoPage = 0;
	debugC(1, kDebugScript, "troop %u is set to %s", troop, orderName(order).c_str());

	if (response) {
		prepareDialogueLine(*response, 15, record);
		rememberBookRecord(*response, 15);
		response->flags |= 0x80;
		_state[0x19] = 0xff;
		playSpeech(response->line, 15);
		_troopInfoPage = 0;
		debugC(1, kDebugScript, "troop %u acknowledges order %u with line %u: %s",
				troop, order, response->line, _speech.c_str());
	}
	presentReply(response != nullptr); // Before 6AB8's equipment release and D2E2's pop.
	// The place's total already includes assigned equipment, so only the
	// carried bit changes. Other troops can now take this spare machine.
	if (order >= 4)
		_state[record + 0x19] &= 0x7f;
	// A blocked mining assignment retains its warning and troop controls so
	// ASK FOR MORE INFORMATION can explain the job's missing prerequisites.

	finishTroopOccupationMenu();
}

/**
 * Who is at the place the player is standing in.  Sixteen records of sixteen
 * bytes lie at DS:0xFD8, one to a person: the word at +0 is the kind of
 * arrival and the word at +2 the place, and the walk at 0x36EE takes the
 * records whose two words are the ones the game keeps at DS:0x4 and DS:0x6.
 * The byte at +0xE is the person's own number and the byte at +0xF their
 * state; 0x30A0 walks the table twice, once for those without 0x40 in that
 * byte and once for those with it, so the marked ones come last.
 *
 * The walk is also what fills the word at DS:0x12, a bit to a person, which
 * is what the room draws its standing figures from.
 */
/**
 * Where the four who are not anybody in particular stand.  0x3127 puts the
 * Harkonnen Captain, the Smuggler, the Fremen and the Fremen Chief nowhere -
 * the place 0x7F80, which is no place at all - and then walks the troops
 * standing at the place the player is in and stands one of the four in for
 * each of them.
 *
 * A troop with 0x20 in its byte at +3 waits in the second room of a place, or
 * the third where the place is Harkonnen; one without it waits in the second
 * as well, or the first where somebody is already waiting there, and is passed
 * over altogether if 0x80 is set in its byte at +0x10 - unless the 0x20 put it
 * down the other road, where that same bit makes it the Harkonnen Captain.
 * Whichever of them it is only stands anywhere if that room is the one the
 * player is in.  A troop the player has still to meet - 0x80 in its +3 - is
 * the Fremen, and one they have met is the Fremen Chief.
 *
 * This is why a sietch has Fremen standing about in it: the troop that lives
 * there is one.  None of it was written, so every sietch was empty and the
 * only people anywhere were the twelve the game names.
 */
void DuneEngine::placeTheWandering() {
	placeTheWandering(true);
}

void DuneEngine::placeTheWandering(bool prepareMerchant) {
	enum {
		kPeople = 0xfd8, kRecordSize = 16,
		kCaptain = 12, kSmuggler = 13, kFremen = 14, kChief = 15,
		kNowhere = 0x7f80,
		kChiefsPlaced = 0x476a, kWhichIsOurs = 0x476b,
		kTroops = 0x8aa, kTroopSize = 27,
		kOurTroop = 0x8e0, kSomebodyWaiting = 0x2b,
		// 0x31ED keeps, beside each of the four, the troop they were taken
		// out of: the Fremen's in one word, the Captain's in another, and
		// the chiefs' in eight words in a row.  Without them there was no
		// way back from the person spoken to to the troop they belong to.
		kFremensTroop = 0x4756, kChiefsTroops = 0x4758, kCaptainsTroop = 0x4768
	};

	if (_state.size() < kPeople + 16 * kRecordSize || _state.size() <= kWhichIsOurs)
		return;

	_state[kChiefsPlaced] = 0;
	_state[kWhichIsOurs] = 0;

	for (uint who = kCaptain; who <= kChief; who++)
		WRITE_LE_UINT16(_state.begin() + kPeople + who * kRecordSize + 2, kNowhere);

	// Out in the desert there is no place and no troops to walk.
	const uint16 place = READ_LE_UINT16(_state.begin() + 6);

	if ((place & 0xff) != 0x80)
		return;

	const uint32 at = READ_LE_UINT16(_state.begin() + 0x114e);

	if (at < 0x100 || at + 28 > _state.size())
		return;

	const uint16 room = READ_LE_UINT16(_state.begin() + 4);
	const byte kindOfPlace = _state[at + 8];

	for (byte troop = _state[at + 9]; troop != 0;) {
		const uint32 record = kTroops + (uint32)(troop - 1) * kTroopSize;

		if (record + kTroopSize > _state.size())
			break;

		const byte what = _state[record + 3];
		const byte more = _state[record + 0x10];
		const byte next = _state[record + 1];

		int waits;

		if ((what & 0x20) != 0) {
			waits = kindOfPlace >= 0x28 ? 3 : 2;
		} else if ((more & 0x80) != 0) {
			troop = next;
			continue;
		} else {
			waits = _state[kSomebodyWaiting] != 0 ? 1 : 2;
		}

		if (waits != (room & 0xff)) {
			troop = next;
			continue;
		}

		uint who;
		uint32 belongsTo;

		if ((more & 0x80) != 0) {
			who = kCaptain;
			belongsTo = kCaptainsTroop;

			// 0x31CF: the Captain carries the troop's own 0x10 in his flags,
			// and how many of them there are goes into DS:0xED - or 0xFF,
			// which is nothing to show, where that bit stands.
			const byte hidden = (byte)(what & 0x10);
			byte &flags = _state[kPeople + who * kRecordSize + 15];

			flags = (byte)((flags & ~0x10) | hidden);

			// 31CF initializes the encounter on the room-presence pass, not
			// while 3B28 repaints the hidden floor behind a conversation.
			// Native redraws still walk this list: preserve both resistance and
			// per-person attempts until the floor is returned by STOP TALKING.
			if (!(_state[0x47a4] & 0x80)) {
				WRITE_LE_UINT16(_state.begin() + 0xee, 0);
				_state[0xed] = hidden != 0 ? 0xff : _state[record + 0x15];
			}
		} else if (what >= 0x80) {
			who = kFremen;
			belongsTo = kFremensTroop;
		} else {
			who = kChief;

			// 0x31AA counts the chiefs standing here, eight at the most, and
			// keeps which of them is the player's own.
			_state[kChiefsPlaced] &= 7;

			const byte nth = _state[kChiefsPlaced];

			if (record == kOurTroop)
				_state[kWhichIsOurs] = (byte)(nth + 1);

			_state[kChiefsPlaced] = (byte)(nth + 1);
			belongsTo = kChiefsTroops + 2 * (uint32)nth;
		}

		WRITE_LE_UINT16(_state.begin() + belongsTo, (uint16)record);
		WRITE_LE_UINT16(_state.begin() + kPeople + who * kRecordSize, room);
		WRITE_LE_UINT16(_state.begin() + kPeople + who * kRecordSize + 2, place);

		troop = next;
	}

	// 0x3157: the one place of the kind 0x21 keeps a smuggler, and he is in
	// whichever of its rooms the player walks into.  0x2318 also prepares the
	// seventeen-byte account belonging to this village.  Its five inventory
	// counts and five base prices drive the offers; omitting this left byte 9D
	// at zero, so every smuggler immediately said that he had nothing to trade.
	if (kindOfPlace == 0x21) {
		WRITE_LE_UINT16(_state.begin() + kPeople + kSmuggler * kRecordSize, room);
		WRITE_LE_UINT16(_state.begin() + kPeople + kSmuggler * kRecordSize + 2, place);

		// While he is speaking, the offer/account scratch prepared on entry must
		// survive redraws of his portrait and price card.  The DOS room walker is
		// suppressed by its talking-person flag; drawScene() deliberately still
		// calls peopleHere(), so make that suppression explicit here.
		if (_speakerPerson == kSmuggler)
			return;

		enum { kFirstAccount = 0x10d8, kAccountSize = 17 };
		uint32 account = kFirstAccount;
		while (account + kAccountSize <= _state.size() &&
				_state[account] != 0xff && _state[account] != _state[at])
			account += kAccountSize;

		if (prepareMerchant && account + kAccountSize <= _state.size() && _state[account] == _state[at]) {
			WRITE_LE_UINT16(_state.begin() + 0x10b4, (uint16)account);
			_state[0x1c] = _state[account + 2];
			WRITE_LE_UINT16(_state.begin() + 0x20,
						READ_LE_UINT16(_state.begin() + account + 0x0e));
			_state[0x1f] = READ_LE_UINT16(_state.begin() + account + 0x0e) != 0
				? (byte)((hour() >> 4) - _state[account + 0x10]) : 0;
			_state[0x1d] = _state[account + 1];
			_state[0x1e] = (byte)((hour() >> 4) - _state[account + 3]);
			const bool firstVisit = (_state[account + 2] & 8) == 0;
			if (firstVisit) {
				_state[0x1e] = 1;
				_state[account + 2] |= 8;
			}

			_state[0x9d] = 0;
			const byte equipmentKinds = _state[0x1141];
			if (equipmentKinds != 0) {
				uint item = READ_LE_UINT16(_state.begin()) & 7;
				while (item >= equipmentKinds)
					item -= equipmentKinds;
				WRITE_LE_UINT16(_state.begin() + 0x11f1, (uint16)(0xe8 + item));
			}
			_state[0x9f] = 0;
		}
	}

	// 316A follows the standing-troop walk and the merchant's 2318 setup.
	// Returning from a remote map query/report must refresh every expanded
	// site field, not only the counts or packed name used by a few replies.
	expandPlaceContext(at);
}

Common::Array<byte> DuneEngine::peopleHere() {
	return peopleHere(true);
}

Common::Array<byte> DuneEngine::peopleHere(bool prepareMerchant) {
	enum {
		kPeople = 0xfd8, kRecords = 16, kRecordSize = 16,
		kLater = 0x40
	};

	Common::Array<byte> here;

	if (_state.size() < kPeople + kRecords * kRecordSize)
		return here;

	// 0x3093 stands the four who are not anybody in particular where they
	// belong before it walks the sixteen.
	placeTheWandering(prepareMerchant);

	const uint16 kind = READ_LE_UINT16(_state.begin() + 4);
	const uint16 place = READ_LE_UINT16(_state.begin() + 6);
	uint16 present = 0;

	for (uint i = 0; i != kRecords; i++)
		debugC(3, kDebugScript, "%s is at room %04x place %04x",
			   command(kFirstPerson + i).c_str(),
			   READ_LE_UINT16(_state.begin() + kPeople + i * kRecordSize),
			   READ_LE_UINT16(_state.begin() + kPeople + i * kRecordSize + 2));

	for (int pass = 0; pass != 2; pass++) {
		for (uint i = 0; i != kRecords; i++) {
			const byte *at = _state.begin() + kPeople + i * kRecordSize;
			if (pass == 0)
				debugC(3, kDebugScript, "person record %u (who %u): %04x at %04x, flags %02x",
					   i, at[14], READ_LE_UINT16(at), READ_LE_UINT16(at + 2), at[15]);

			if (!namedPersonAvailable(at[14], storyStage()) ||
					READ_LE_UINT16(at) != kind || READ_LE_UINT16(at + 2) != place)
				continue;

			if (((at[15] & kLater) != 0) != (pass == 1))
				continue;

			here.push_back(at[14]);
			present |= 1 << (at[14] & 0xf);
		}
	}

	WRITE_LE_UINT16(_state.begin() + 0x12, present);

	return here;
}

/**
 * Takes a person up to speak to.  Whoever is being spoken to is drawn large
 * over the room instead of standing on a mark, so 0x95FC sets 0x40 in their
 * own byte at +0xF. DS:0x10 records followers and is not changed by contact.
 */
void DuneEngine::takeUpPerson(uint who) {
	selectLocalSpeaker(who, true);
}

/** 9381..93AA: both a floor hit and a menu row resolve the local chief. */
void DuneEngine::startRoomConversation(uint who) {
	bool localChief = false;
	if (who == 15 && _state.size() > 0x476c) {
		const uint troop = READ_LE_UINT16(_state.begin() + 0x4758 + 2 * (_state[0x476c] & 7));
		if (troop >= 0x8aa && troop + 27 <= _state.size()) {
			checkTroopCure((troop - 0x8aa) / 27);
			localChief = true;
		}
	}

	takeUpPerson(who);
	recordPersonConversation(who);
	enterDialogue(who, kWhatHeSays);
	if (localChief)
		_listAs = 15;
	showTalkMenu(false);
	sayNextLine();
}

/** Bind the local caller before text/pose preparation in its actual order. */
void DuneEngine::selectLocalSpeaker(uint who, bool preparePortrait) {
	enum { kPeople = 0xfd8, kRecords = 16, kRecordSize = 16 };

	if (_state.size() < kPeople + kRecords * kRecordSize)
		return;
	if (who == 12)
		prepareCaptainContactContext(); // 932E, before the captive's dialogue.

	// DS:0x10 is not who is being spoken to.  The whole of the original writes
	// it in two places only - 0x9616, which is COME WITH ME, and 0x9568, which
	// is STAY HERE - so it says who is travelling with Paul and nothing else.
	// Setting it here made every conversation claim the other party had joined
	// him, and conditions which ask that somebody is NOT with him could then
	// never hold: Jessica's "So, Thufir told you about something hidden in
	// this palace" is one, and 0x6C4F's test for Gurney is another.
	// 92F2..93AA selects the caller without writing DS:23. That byte is
	// request/event context (room transition, sighting, recruitment, etc.),
	// not a one-based person identity. Preserve it for CONDIT.HSQ.

	preparePersonMusterContext(who);

	preparePersonDialogueContext(who);

	// 3B1F skips the room magnification for a caller with no standing
	// corner, notably a companion already in the panel. Only 3B28's
	// magnified room sets the high bit; 97CF uses that distinction when
	// returning from the conversation. Native redraws suppress floor figures
	// through _speakerPerson independently of this DOS latch.
	const uint cornerPerson = who == 15 ? who + _state[0x476c] : who;
	if (!_state[0x2b] && cornerPerson < 23 &&
			(int16)READ_LE_UINT16(_state.begin() + 0x47f8 + 4 * cornerPerson) >= 0)
		_state[0x47a4] |= 0x80;

	_speakerPerson = (int)who;
	_speakerFrame = 0;
	// 93AA/93B3 bind the raw caller before 91A0/9908 select the initial
	// portrait. Waiting for playSpeech leaves the previous person's caller
	// and report selector visible to the first idle-pose preparation.
	WRITE_LE_UINT16(_state.begin() + 0x47e1, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47c4, (uint16)who);
	if (preparePortrait)
		prepareLocalSpeakerIdle();
	// DS:0x19 is cleared when a person is first taken up and set after each
	// record is read.  Several continuations use it to distinguish the first
	// pass through a conversation from the lines that follow.
	_state[0x19] = 0;
	_listAs = -1;
}

/** 93DF records ordinary contact independently of the follower mask at 10. */
void DuneEngine::recordPersonConversation(uint who) {
	if (who >= 16 || _state.size() < 0x16)
		return;
	const uint16 bit = (uint16)(1u << who);
	WRITE_LE_UINT16(_state.begin() + 0x0e,
			READ_LE_UINT16(_state.begin() + 0x0e) | bit);
	WRITE_LE_UINT16(_state.begin() + 0x14,
			READ_LE_UINT16(_state.begin() + 0x14) | bit);
}

/** 932E..936B: expand the captive and the fort he can disclose. */
void DuneEngine::prepareCaptainContactContext() {
	const uint16 record = READ_LE_UINT16(_state.begin() + 0x4768);
	if (record < 0x8aa || record >= 0xfbb || (record - 0x8aa) % 27)
		return;
	prepareTroopContactContext((record - 0x8aa) / 27);
	uint16 slot = 0x44;
	if (!READ_LE_UINT16(_state.begin() + slot)) {
		// 9344 only caches the nearest hidden fort when it is within thirty.
		if (READ_LE_UINT16(_state.begin() + 0xe2) >= 30)
			return;
		slot = 0xe4;
		WRITE_LE_UINT16(_state.begin() + record + 0xc,
			READ_LE_UINT16(_state.begin() + slot));
	}
	const uint16 target = READ_LE_UINT16(_state.begin() + slot);
	if (target < 0x100 || target >= 0x8aa || (target - 0x100) % 28)
		return; // A valid captain's intelligence is a place-record pointer.
	// 407E takes Paul's actual position, not the captive troop's expanded
	// place. 5323 writes the bearing beside the selected pointer before
	// 331E rebuilds the nearest-site cache around the disclosed fort.
	uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	int16 latitude = (int8)_state[6];
	if (_state[6] == 0x80) {
		const uint16 home = READ_LE_UINT16(_state.begin() + 0x114e);
		longitude = READ_LE_UINT16(_state.begin() + home + 2);
		latitude = (int16)READ_LE_UINT16(_state.begin() + home + 4);
	}
	const byte bearing = placeBearing(longitude, latitude, target);
	_state[slot + 2] = bearing;
	expandPlaceContext(target);
	preparePlaceNameContext(target);
	WRITE_LE_UINT16(_state.begin() + 0x11fd, 0xda + bearing);
}

/** 9F40/1243: the current caller checks the fortress muster before selection. */
void DuneEngine::preparePersonMusterContext(uint who) {
	// 0x9F40 asks, whenever Thufir is the one taken up and the war stands at
	// exactly four, what Paul has garrisoned in the three fortresses whose
	// records begin at 0x138.  0x1269 counts the men of each troop there
	// which is set to the fourth order and carries the third piece of
	// equipment, and 0x124D moves the war on once they come to a thousand.
	//
	// That is the walkthrough's own advice put into numbers - four troops
	// expert in army, equipped, working towards the palace - and it is one of
	// the four things which move DS:0xC2 at all.
	enum { kThufir = 2, kWar = 0xc2, kFirstFortress = 0x138, kFortresses = 3,
		   kTroops = 0x8aa, kTroopSize = 27, kAtArms = 4, kEquipped = 4 };

	if (who == kThufir && _state.size() > kWar && _state[kWar] == 4) {
		uint16 mustered = 0;

		for (uint i = 0; i != kFortresses; i++) {
			const uint32 fortress = kFirstFortress + i * 28;
			if (fortress + 28 > _state.size())
				break;
			byte id = _state[fortress + 9];
			for (uint visited = 0; id && visited < 67; visited++) {
				if (id > _map.troops().size())
					break;
				const uint32 troopRecord = kTroops + (id - 1) * kTroopSize;
				if (troopRecord + kTroopSize > _state.size())
					break;
				const byte next = _state[troopRecord + 1];
				// 661D walks the standing links and 1269 compares the entire
				// task byte. The place word at +4 alone can name an incoming unit.
				if (_state[troopRecord + 3] == kAtArms &&
						(_state[troopRecord + 0x19] & kEquipped))
					mustered = (uint16)(mustered + _state[troopRecord + 0x1a]);
				id = next;
			}
		}

		debugC(1, kDebugScript, "the fortresses hold %u of the thousand wanted", mustered);

		if (mustered >= 1000)
			_state[kWar]++;
	}
}

/** 9F9E/94F3 refreshes person flags and elapsed visit time before selection. */
void DuneEngine::preparePersonDialogueContext(uint who) {
	if (who >= 16 || _state.size() < 0xfd8 + 16 * 16)
		return; // 94F3 excludes shared/non-person dialogue callers.
	const byte *selected = _state.begin() + 0xfd8 + 16 * who;
	_state[0x18] = selected[15];
	// A follower uses the join clock (+8); a resident uses the leave clock
	// (+A). Both are unsigned 16-bit differences, not the last speech date.
	const uint16 last = READ_LE_UINT16(selected + ((selected[15] & 0x40) ? 8 : 10));
	WRITE_LE_UINT16(_state.begin() + 0x16, (uint16)(hour() - last));
	// 94F3/952F expands the current epidemic sietch for the nine early
	// named speakers. Their "go to ..." line may refer to a remote place;
	// leaving the ordinary room context here substitutes Paul's own sietch
	// and gives action thirteen no report location to display.
	if (_state.size() > 0x47e7 && storyStage() < 100 && who < 9) {
		const uint16 struck = READ_LE_UINT16(_state.begin() + 0x11db);
		if (struck >= 0x100 && (uint)struck + 1 < _state.size()) {
			preparePlaceNameContext(struck);
		}
	}
}

/** 93DF starts ordinary news, independently of the request context at 23. */
void DuneEngine::beginPersonNews(uint person) {
	_personNewsPart = 0xffff;
	_dialogueActionPending = false;
	_dialoguePart = person * 8;
	_dialogueLine = 0xffff;
	_dialogueMask = 0x80;
	WRITE_LE_UINT16(_state.begin() + 0x47be, _dialoguePart);
	WRITE_LE_UINT16(_state.begin() + 0x47a2, 0xfd8 + 16 * person);
	WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47b6, 0);
	_state[0x47c2] = 0x80;
	_state[0x19] = 0;
	recordPersonConversation(person);
}

/**
 * Ordinary selection reaches 93DF, then 9472/9486 searches the first four
 * news parts in resource order. Request parts enter separately through 96F1
 * or 9F31; selecting a person must not manufacture their request context.
 */
void DuneEngine::enterDialogue(uint person, uint wanted) {
	_dialogueActionPending = false;
	WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
	_state[0x47a8] = 0;
	preparePersonDialogueContext(person);

	// 0x96F8/0x31F6 expands the troop behind a generic Fremen or selected chief
	// into the scratch bytes that CONDIT.HSQ reads before choosing a line.
	// Chiefs use one of the eight words at 0x4758, selected by 0x476C.  Expanding
	// only person 14 left those bytes describing the last un-sworn Fremen Paul
	// had met: the chief at Tuek consequently claimed that his troop lived in a
	// different sietch and offered that other troop's dialogue.
	if ((person == 14 || person == 15) && _state.size() > 0x476c) {
		enum {
			kTroops = 0x8aa, kTroopSize = 27,
			kFremensTroop = 0x4756, kChiefsTroops = 0x4758,
			kSelectedChief = 0x476c
		};
		const uint32 troop = person == 14
			? READ_LE_UINT16(_state.begin() + kFremensTroop)
			: READ_LE_UINT16(_state.begin() + kChiefsTroops
								  + 2 * (_state[kSelectedChief] & 7));

		if (troop >= kTroops && troop + kTroopSize <= _state.size() &&
				(troop - kTroops) % kTroopSize == 0) {
			// DOS 96F8 calls 31F6 on this exact troop pointer. The map path
			// already mirrors 31F6, including 331E's place/equipment summary
			// and 32C7's worked-time fields. Keeping a shorter room-only copy
			// left CONDIT reading stale or incomplete context during dialogue.
			prepareTroopContactContext((troop - kTroops) / kTroopSize);
		}
	}

	if (wanted == kWhatHeSays) {
		beginPersonNews(person);
		return; // 9486 selects only after the talk menu has been registered.
	}

	// 9F8B uses the request mask for departure and follower answers.
	_dialogueMask = (wanted == kMakingOff || wanted == kComingAlong ||
			wanted == kStayingBehind) ? 0x20 : 0x80;

	const uint group = person * 8;
	const uint attempts = wanted == kMakingOff ? 1 : 8;

	for (uint tried = 0; tried != attempts; tried++) {
		const uint part = group + ((wanted + tried) & 7);

		if (part >= _dialogue.count() || _dialogue.entry(part).empty())
			continue;

		const Common::Array<Dialogue::Record> &lines = _dialogue.entry(part);
		for (uint line = 0; line != lines.size(); line++) {
			debugC(3, kDebugScript, "dialogue part %u record %u: line %u, flags %02x, condition %u (%s): %s -- %s",
				   part, line, lines[line].line, lines[line].flags, lines[line].condition,
				   _condit.describe(lines[line].condition).c_str(),
				   conditionHolds(lines[line].condition) ? "true" : "false",
				   fillInLine(phraseOf(person, lines[line].line)).c_str());
			if (!dialogueRecordAvailable(lines[line], _dialogueMask) ||
				!conditionHolds(lines[line].condition))
				continue;

			_dialoguePart = part;
			_dialogueLine = line == 0 ? 0xffff : line - 1;
			return;
		}
	}

	_dialoguePart = group + wanted;
	_dialogueLine = 0xffff;
}

/**
 * Done with whoever was being spoken to: they go back to standing on a mark,
 * which is 0x9673's work - it strikes them off the pair the game keeps at
 * DS:0x1152 and takes their bit out of DS:0x10 again, so 0x3DB0 lays them
 * back among the marks.
 */
/*
 * The pair of bytes at DS:0x1152 says who is walking with Paul; a slot holds
 * 0xFF where nobody is.  drawPanel widens the byte with its sign and adds
 * 0x41, so 0xFF gives the empty box.
 */
enum { kFollowing = 0x1152, kNobody = 0xff };

bool DuneEngine::isFollowing(uint person) const {
	if (_state.size() <= kFollowing + 1)
		return false;

	return _state[kFollowing] == (byte)person || _state[kFollowing + 1] == (byte)person;
}

// 9608/9556 change the person's travel flag and clock when the answer is
// spoken. 9673/9655 do not change the panel slots until the conversation is
// released, so these two pieces of state must not be updated as one action.
// 94F3's dialogue flags at 18 are refreshed by the next dialogue selection.
void DuneEngine::markPersonFollowing(uint person, bool following) {
	if (_state.size() <= kFollowing + 1)
		return;

	const uint16 bit = (uint16)(1 << (person & 0xf));
	const uint16 mask = READ_LE_UINT16(_state.begin() + 0x10);
	WRITE_LE_UINT16(_state.begin() + 0x10, following ? mask | bit : mask & (uint16)~bit);

	enum { kPeople = 0xfd8, kRecords = 16, kRecordSize = 16, kAlong = 0x40 };
	const uint16 now = READ_LE_UINT16(_state.begin() + 2);
	for (uint i = 0; i != kRecords; i++) {
		byte *at = _state.begin() + kPeople + i * kRecordSize;
		if (at[14] != person)
			continue;
		if (following) {
			at[15] |= kAlong;
			if ((uint16)(now - READ_LE_UINT16(at + 10)) >= 2)
				WRITE_LE_UINT16(at + 8, now);
		} else {
			at[15] &= (byte)~kAlong;
			if ((uint16)(now - READ_LE_UINT16(at + 8)) >= 2)
				WRITE_LE_UINT16(at + 10, now);
		}
	}
}

void DuneEngine::comeAlong(uint person) {
	comeAlong(person, false);
}

void DuneEngine::comeAlong(uint person, bool recordPrepared) {
	if (_state.size() <= 0x2223 || isFollowing(person))
		return;

	for (int slot = 0; slot != 2; slot++) {
		if (_state[kFollowing + slot] != kNobody)
			continue;

		_state[kFollowing + slot] = (byte)person;
		_state[0x2222 + slot] = 16; // 96AD arms this portrait's blink counter.
		if (!recordPrepared)
			markPersonFollowing(person, true);
		carryFollowersAlong();

		debugC(1, kDebugScript, "%s walks with Paul now",
			   command(kFirstPerson + person).c_str());
		return;
	}

	// 0x968C does not reject a third companion.  It asks the person in the
	// first slot to remain, shifts the second portrait left and puts the new
	// companion in the second slot. Preserve the saved portrait order: DOS
	// does not test the stage, shipment count or person joining here.
	// Before changing the pair it selects the
	// old companion's making-off record with request byte 100 + person; that is
	// why Duncan joining for a shipment makes Stilgar say the otherwise hidden
	// "I'll stay here." in the reference.
	const byte leaving = _state[kFollowing];
	const byte firstBlink = _state[0x2222];
	letGoOfPerson();
	stayBehind(leaving);
	// 968C marks the leaver at9556, not9655: shifting the second slot
	// does not clear or move the first slot's existing blink counter.
	_state[0x2222] = firstBlink;

	// There is now a free second slot, so the ordinary path supplies the new
	// person's record flag, portrait and current room.
	comeAlong(person, recordPrepared);

	// 968C queues the departure request; A0AA prepares its portrait once,
	// when the making-off paragraph is selected, rather than at contact too.
	_state[0x23] = (byte)(leaving + 100);
	selectLocalSpeaker(leaving, false);
	enterDialogue(leaving, kMakingOff);
	showTalkMenu(false);
	sayNextLine();
}

/**
 * A Fremen asked along brings his troop rather than himself.  0x9603 looks at
 * who agreed and keeps the fourteenth - the one who is nobody in particular -
 * out of 0x9673 altogether: there is no picture of him among ICONES.HSQ's ten
 * boxes, which run from the sixty-fifth for the nine the game names, so the
 * panel had nothing to draw and asked the bank for a sprite past its end
 * every time it was drawn.
 *
 * What 0x961B does instead is swear the troop he was standing out of, kept in
 * the word at DS:0x4756, to Paul: 0x66CE takes the 0x80 that says the troop
 * has still to be met out of its +3 and puts 2 there in its place, gives it
 * nothing to do, counts it at DS:0x28, and Paul is thought a little better of
 * for it.  The place it lives in goes on the map where it was not on it yet.
 */
void DuneEngine::enlistTroop(uint32 record) {
	enum {
		kTroops = 0x8aa, kTroopSize = 27,
		kSworn = 0x28, kAllowed = 0x1178,
		kNotMet = 0x80, kKept = 0x20, kOurs = 0x02
	};

	if (record < kTroops || record + kTroopSize > _state.size())
		return;

	// Only a troop still to be met is sworn, and one whose +0x10 is set is
	// nobody's to swear.
	if ((_state[record + 3] & kNotMet) == 0 || (_state[record + 0x10] & 0x80) != 0)
		return;

	_state[kSworn]++;

	// 0x66E7 moves the story to 0x4C as soon as the newly enlarged rally quota
	// at DS:1178 is met.  Merely logging this condition stranded the campaign at
	// stage 72 no matter how many of Chani's new troops Paul recruited, leaving
	// the Water-of-Life chapter and every later system unreachable.
	debugC(1, kDebugScript, "sworn %u of %u allowed", _state[kSworn], _state[kAllowed]);
	if (_state[kAllowed] != 0 && _state[kSworn] >= _state[kAllowed] &&
			storyStage() < 76)
		advanceStory(76);

	addCharisma(1);
	_state[record + 3] = (byte)((_state[record + 3] & kKept) | kOurs);

	// 0x6B25 leaves the troop with nothing to do: the hour it was last seen
	// to, and the two words that say what it is at.
	const uint16 now = READ_LE_UINT16(_state.begin() + 2);

	WRITE_LE_UINT16(_state.begin() + record + 0xa, now);
	WRITE_LE_UINT16(_state.begin() + record + 0xc, 0);
	WRITE_LE_UINT16(_state.begin() + record + 0xe, 0);

	// 0x1AC5 is the game's own throw of the dice, the clock shifted down.
	_state[record + 0x14] = (byte)(now >> 4);

	// The place the troop lives in is shown on the map where it was not shown
	// already, which is the drawing 0x644E does.
	const uint16 lives = READ_LE_UINT16(_state.begin() + record + 4);

	if (lives != 0 && (uint32)lives + 0xb < _state.size() && _state[lives + 0xb] == 0) {
		_state[lives + 0xb] = 2;
		_map.paintRegion(READ_LE_UINT16(_state.begin() + lives + 2),
				(int16)READ_LE_UINT16(_state.begin() + lives + 4), 2, 0x20);
	}

	// The map-side troop console reads this cache immediately when GIVE
	// ORDERS is chosen.  Leaving the pre-recruitment 0x80 state in it made the
	// newly sworn troop look unavailable until an unrelated reload or tick.
	_map.readTroops(_state.begin(), _state.size());

	// 0x961B refreshes the room's wandering people through 0x3093 before
	// looking for this troop in the eight chief slots at DS:4758.  Without
	// that refresh and scan, DS:476C still points at the previously spoken-to
	// chief: the newly recruited one's portrait and later orders can then
	// belong to another troop at the same sietch.
	peopleHere(); // 9626 calls the full 3093 presence walk, not only 3127.
	// 962A rewards the prospectors after 3093 has refreshed the place's
	// strength/count context. Applying it before that walk changes the
	// retained strength and can change which subsequent reports are eligible.
	if (record == 0x8e0) {
		_state[record + 0x15] += 24;
		_state[0x36] += 24;
		_map.readTroops(_state.begin(), _state.size());
	}
	_state[0x476c] = 7; // DOS's exhausted eight-word scan also leaves seven.
	for (uint chief = 0; chief < 8; chief++) {
		if (READ_LE_UINT16(_state.begin() + 0x4758 + 2 * chief) == record) {
			_state[0x476c] = (byte)chief;
			break;
		}
	}

	debugC(1, kDebugScript, "the troop at %04x is Paul's now, and %d are sworn to him",
		   (uint)record, _state[kSworn]);
}

void DuneEngine::stayBehind(uint person) {
	stayBehind(person, false);
}

void DuneEngine::stayBehind(uint person, bool recordPrepared) {
	if (_state.size() <= 0x2223)
		return;

	// 0x9655 starts at the second slot.  Removing that one simply clears it;
	// removing the first moves the second companion left before clearing the
	// vacancy.  Leaving a hole here reversed the party the next time somebody
	// joined and eventually made the wrong companion leave for Duncan.
	if (_state[kFollowing + 1] == (byte)person) {
		_state[kFollowing + 1] = kNobody;
		_state[0x2223] = 0; // 966B clears the removed slot only.
	} else if (_state[kFollowing] == (byte)person) {
		_state[kFollowing] = _state[kFollowing + 1];
		_state[kFollowing + 1] = kNobody;
		_state[0x2222] = 0;
	}

	if (!recordPrepared)
		markPersonFollowing(person, false);

	debugC(1, kDebugScript, "%s stays behind", command(kFirstPerson + person).c_str());
}

/**
 * Reconciles native/scripted companion records with the panel's party.
 * Ordinary movement uses moveMatchingFollowers instead: DOS 40C3 does not
 * synchronize the panel or bring distant flagged people to Paul. The old
 * 3EC9 attribution was incorrect; that address belongs to a drawing loop.
 */
void DuneEngine::carryFollowersAlong() {
	enum { kPeople = 0xfd8, kRecords = 16, kRecordSize = 16 };

	if (_state.size() < kPeople + kRecords * kRecordSize || _state.size() <= 7)
		return;

	const uint16 where = READ_LE_UINT16(_state.begin() + 4);
	const uint16 whose = READ_LE_UINT16(_state.begin() + 6);

	for (uint i = 0; i != kRecords; i++) {
		byte *at = _state.begin() + kPeople + i * kRecordSize;

		if (!isFollowing(at[14])) {
			at[15] &= (byte)~0x40;
			continue;
		}

		at[15] |= 0x40;

		WRITE_LE_UINT16(at, where);
		WRITE_LE_UINT16(at + 2, whose);
	}
}

void DuneEngine::letGoOfPerson() {
	clearPersonDisplay(true);
}

/** 1707/9761: changing scripted speakers is not an ordinary 97CF STOP. */
void DuneEngine::releaseEventSpeaker() {
	clearPersonDisplay(false);
}

void DuneEngine::clearPersonDisplay(bool finishConversation) {
	stopEndingCouple();
	_personNewsPart = 0xffff;
	enum { kPeople = 0xfd8, kRecords = 16, kRecordSize = 16 };
	_dialogueActionPending = false;
	_localRoomRequestPending = false;
	_pendingRoomExit = -1;
	_pendingRoomLead = 0;

	// 97CF calls A7A5 even when there is no selected person. End the old
	// recording and its mouth cues before any deferred departure reply.
	stopSpeech();
	_speakerCardValueValid = false;
	WRITE_LE_UINT16(_state.begin() + 0x479e, 0); // 97CF/98E6/8C8A releases caption ownership, not A7A5 alone.
	WRITE_LE_UINT16(_state.begin() + 0x47e1, 0); // 97E6 gives back the portrait selector.
	_speakerReportLowered = false;
	_speakerCardText.clear();

	// Both the magnified-room return at 9858 and the battlefield return
	// at 9840 remove the portrait timer through 9B8B.
	const bool magnified = (_state[0x47a4] & 0x80) != 0;
	if (finishConversation && !_state[0x11c9] && (magnified || (!_inDesert && _state[0x2b]))) {
		if (magnified) {
			WRITE_LE_UINT16(_state.begin() + 0x4540, 0);
			WRITE_LE_UINT16(_state.begin() + 0x47c8, 0);
			_state[0x47d1] &= 0x3f;
		}
		WRITE_LE_UINT16(_state.begin() + 0x47c6, 0);
		WRITE_LE_UINT16(_state.begin() + 0x47ce, 0);
		_state[0x47c3] = 0;
		_state[0x47d1] &= 0x7f;
		_speakerIdleActive = false;
		_speakerIdlePerson = -1;
		_speakerVoiceActive = false;
		_speakerSpeechMode = false;
		_speakerVoiceRemaining = 0;
	}

	if (_speakerPerson < 0 || _state.size() < kPeople + kRecords * kRecordSize)
		return;
	const uint person = (uint)_speakerPerson;
	bool recordAlong = false;
	for (uint i = 0; i != kRecords; i++) {
		byte *at = _state.begin() + kPeople + i * kRecordSize;
		if (at[14] == person) {
			// 97DD/97E1 mark the completed conversation and clear its
			// temporary flag before settling the companion portraits.
			if (finishConversation)
				at[15] = (byte)((at[15] | 0x20) & ~0x04);
			recordAlong = (at[15] & 0x40) != 0;
			break;
		}
	}
	const bool panelAlong = isFollowing(person);

	// Done talking is not the same as walking away: 0x9868 gives the floor
	// back and leaves DS:0x10 alone, because that word is STAY HERE's to
	// clear.  See takeUpPerson.

	// The floor is given back, which 0x9868 does as the talk is done.
	if (_state.size() > 0x47a4)
		_state[0x47a4] &= 0x7f;

	debugC(1, kDebugScript, "done with person %d, who stands on a mark again",
		   _speakerPerson);

	_speakerPerson = -1;
	_speakerFrame = 0;
	_listAs = -1;
	_speech = "";

	// 982B/982E settle the portrait slots only when the conversation ends.
	// The answer already changed the record flag, travel word and clock.
	if (finishConversation && recordAlong && !panelAlong) {
		comeAlong(person, true);
	} else if (finishConversation && !recordAlong && panelAlong) {
		stayBehind(person, true);
	}
}

/** D2E2 gives an ordinary conversation back to its room/desert parent. */
void DuneEngine::finishRoomConversation() {
	// With no magnified room to restore, 982B jumps (not calls) to the
	// follower-slot routine. 967B/9685 then returns immediately for a
	// companion already in the panel, bypassing 9879's room rebuild and
	// face raise. The parent menu returns over the retained portrait, with
	// its caption removed by 8C8A. Explicit STOP uses this same callback.
	const int retainedPerson = _speakerPerson;
	const uint retainedFrame = _speakerFrame;
	bool retainPortrait = false;
	if (retainedPerson >= 0 && !_state[0x11c9] && !_state[0x2b] &&
			!(_state[0x47a4] & 0x80) && isFollowing((uint)retainedPerson)) {
		for (uint i = 0; i < 16; i++) {
			const byte *person = _state.begin() + 0xfd8 + 16 * i;
			if (person[14] == retainedPerson) {
				retainPortrait = (person[15] & 0x40) != 0;
				break;
			}
		}
	}
	// 9821 returns through 9840 to repaint the existing field. D2FD
	// then pops the talk menu and restores the battlefield menu's page.
	const bool retainBattleMenu = !_inDesert && !_state[0x11c9] &&
		_state[0x2b] && !(_state[0x47a4] & 0x80);
	letGoOfPerson();
	if (retainPortrait) {
		_speakerPerson = retainedPerson;
		_speakerFrame = retainedFrame;
	}
	if (retainPortrait || (retainBattleMenu && _speakerPerson < 0)) {
		_speechParts.clear();
		if (!_conversationParentMenu.empty()) {
			_menuKind = _conversationParentKind;
			_menuParts = _conversationParentParts;
			_state[0x1f0f] = 4 * _conversationParentFrom;
			showMenu(_conversationParentMenu, &_conversationParentFlags);
		} else if (_inDesert)
			showDesertMenu(true, true); // Older saves lack the retained commands.
		else
			showRoomMenu(true, true);
		_conversationParentMenu.clear();
		_conversationParentParts.clear();
		_conversationParentFlags.clear();
		return;
	}
	if (_speakerPerson >= 0) {
		// Settling a full party can present the displaced companion's reply.
		// 9870/9898 then waits 200 timer ticks and returns through 9858,
		// clearing the paragraph before restoring the parent room controls.
		// Keeping that caller selected under a room menu made doorway clicks
		// read their ordinary news instead of moving Paul. This is not another
		// conversation close: do not mark or settle the displaced person again.
		if (ConfMan.getBool("dune_nodelay")) {
			advanceClockElapsed(1000, false, false);
		} else {
			const uint32 began = _system->getMillis();
			while (!shouldQuit() && _system->getMillis() - began < 1000) {
				// E3A0 services D9D2, not the ordinary hourly dispatcher
				// or another menu click. Keep the departure voice animated.
				updateGameMusic();
				{
					const AnimationClock::Poll poll(_animationClock, _system->getMillis());
					updateMapSpeech();
					updateLocalSpeaker();
					updateSkyFade();
				}
				_system->updateScreen();
				_system->delayMillis(5);
			}
			tick(false);
		}
		stopSpeech();
		_speakerPerson = -1;
		_speakerFrame = 0;
		_listAs = -1;
		_speech.clear();
		_speechParts.clear();
		_dialogueActionPending = false;
		_state[0x23] = 0; // 36E8 consumes the companion-departure request.
		WRITE_LE_UINT16(_state.begin() + 0x4540, 0);
		WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
		_state[0x47d1] &= 0x3f;
		WRITE_LE_UINT16(_state.begin() + 0x47c8, 0);
		_state[0x47a4] &= 0x7f;
		_state[0x47c3] = 0; // 9B8B removes the departure's pose callback.
		WRITE_LE_UINT16(_state.begin() + 0x47ce, 0);
		WRITE_LE_UINT16(_state.begin() + 0x47c6, 0);
		_speakerIdleActive = false;
		_speakerIdlePerson = -1;
	}
	if (_inDesert) {
		drawDesert();
		showDesertMenu();
	} else {
		peopleHere(); // 9879 enters 3093 once before painting the restored room.
		drawScene();
		// 9879 rebuilds the resident's room list through 2EFB, clearing
		// 1F0F before D2FD restores the rebuilt parent.
		showRoomMenu();
	}
	raisePanelFace(); // 988F/9895, after the parent's uncovered page is rebuilt.
}

/**
 * What there is to do where the player is standing, which is the panel the
 * game puts up as it begins: the two things that may be done anywhere, and
 * then everybody who is here.  The command file keeps the people from its
 * hundred and nineteenth string on, in the order they are numbered by, so the
 * name of the person numbered n is the string 119 + n.
 */
/**
 * Which song belongs where the player is.  0xAA96 works a number out of what
 * is going on and 0xAD75 reads the song out of the fourteen bytes at DS:0x375C
 * at that number, taking the low six bits of what it finds; 0xAE6F loads that
 * as the resource 0xA4 and the song's own number, so one is SEKENCE.HSQ and
 * nine ARRAKIS.HSQ.  So the palace has ARRAKIS, its balcony MORNING, a sietch
 * SIETCHM, a village BAGDAD, a Harkonnen place WORMSUIT, the desert on foot
 * WORMINTR and a flight WARSONG.
 * Positive map/outdoor entries enable the driver's loop flag rather than
 * replacing a playing track; their number is also the stopped-song fallback.
 *
 * This is what DS:0x3810 calls music game relative, which is how the game
 * starts; 0xAC90 turns the songs into a playlist instead, which is what it
 * calls CD-style. That separate path does not use this context selector.
 */
// The nine songs, in the order 0xAE6F loads them: the resource is 0xA4 and
// the song's own number, so one is SEKENCE.HSQ and nine ARRAKIS.HSQ.
static const char *const kSongs[10] = {
	nullptr, "SEKENCE", "WATER", "WORMSUIT", "WORMINTR",
	"WARSONG", "MORNING", "SIETCHM", "BAGDAD", "ARRAKIS"
};

// Where the game keeps how the music is to be played.  0xAEC6 reads whether
// it is off at all out of 0x10 in DS:0x2943; DS:0x3810 carries 1 where the
// songs are taken as a CD player's rather than by the place, and 2 where they
// are shuffled.  The nine of them stand at DS:0x37FA with 0xFF after them, an
// untouched copy of the same nine at DS:0x3804 for putting them back in their
// own order, and DS:0x380E points at the next one to be played.
enum {
	kMusicIsOff = 0x2943, kMusicOffBit = 0x10,
	kHowMusicGoes = 0x3810, kCdStyle = 1, kShuffled = 2,
	kPlaylist = 0x37fa, kPlaylistKept = 0x3804, kPlaylistAt = 0x380e,
	kPlaylistSongs = 9
};

byte DuneEngine::musicContextEntry() const {
	if (_state.size() <= 0x4774)
		return 0;

	const uint16 where = READ_LE_UINT16(_state.begin() + 4);
	const uint16 whose = READ_LE_UINT16(_state.begin() + 6);
	const byte room = (byte)(where & 0xff);
	const byte kindOfPlace = (byte)(where >> 8);
	const byte carried = (byte)(_state[0x11c9] & 3);
	const byte story = _state.size() > 0x2a ? _state[0x2a] : 0;

	uint which;

	if (_state[0x4774]) {
		// AA98: an active bytecode event takes priority even over defeat
		// and the globe. Only stage 72's Chani gathering uses MORNING.
		which = story == 72 ? 10 : 0;
	} else if (_state[0x46d9]) {
		which = 13; // AA96: the defeat theme takes priority over the location.
	} else if (_onGlobe || (_state.size() > 0xdd03 && _state[0xdd03])) {
		// 0xAAB1: the fresco the planet opens has its own song.
		which = 1;
	} else if ((int8)_state[0xfb] < 0 || (_onWorld && !_inCabin)) {
		// AAB8 uses signed FB. Native map entry keeps a separate view flag;
		// a positive-mode destination cabin is not the strategic map.
		which = 2;
	} else if (_state[0xc6]) {
		which = 3; // AABF: the pending-message context precedes remote EA.
	} else if ((int8)_state[0xea] > 0) {
		which = 4; // AAC6: hold the remote-report theme above any location.
	} else if ((whose & 0xff) != 0x80 || room == 1) {
		// 0xAADF: out in the open, or standing outside a place, where what
		// carries the player picks the song.
		which = carried == 0 ? 5 : carried == 1 ? 6 : 7;
	} else if (kindOfPlace < 0x20) {
		// 0xAAF4: a sietch is under the seventh kind and a village over it,
		// and past the seventy-second of the story both take the same song.
		which = kindOfPlace < 7 ? 8 : 9;

		if (story >= 0x48 && ((whose & 1) == 0))
			which = 10;
	} else if (kindOfPlace == 0x20) {
		// 0xAB0C: the palace, whose third room is the balcony.
		which = room == 3 ? 10 : 11;
	} else {
		which = 12;
	}

	return _state[0x375c + which]; // AD75's actual context table, including its command bit.
}

Common::String DuneEngine::songForHere() const {
	const uint song = musicContextEntry() & 0x3f;

	return song < 10 && kSongs[song] ? Common::String(kSongs[song]) : Common::String();
}

/**
 * One of the closing narration's screens. The screen is cleared, the
 * picture named is drawn, and the line of the command file is read over it
 * the way the game's own lines are written - laid out flush at both edges
 * over the foot of the picture - and the whole is held while the line would
 * be read.
 */
PlaybackResult DuneEngine::narrate(uint picture, uint16 line, uint32 holdMs) {
	stopSpeech();
	keepScreen();
	_speakerPerson = -1;
	_speaker.clear();
	_introSpeakerX = 0;
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	int narrationSpin = 0, narrationTilt = 0;

	switch (picture) {
	case kNarrStars:
	case kNarrPlanet: {
		// 02DE/02E3 enter A44 with CX=0/32. The same STARS bank holds
		// all three strips and the sun/planet's smaller approach frames.
		const int position = picture == kNarrPlanet ? 32 : 0;
		const int offset = -position * position / 2;
		const Common::Rect scene(0, 0, kScreenWidth, kSceneHeight);
		openSpriteSheet("STARS.HSQ");
		for (int i = 0; i < 3; i++)
			currentSpriteSheet->drawMapSprite(i, offset + 304 * i, 0, scene);
		auto centre = [&](int sprite, int x, int y) {
			int width = 0, height = 0;
			if (currentSpriteSheet->size(sprite, &width, &height))
				currentSpriteSheet->drawMapSprite(sprite, x - width / 2, y - height / 2, scene);
		};
		centre(36, 4 * offset + 69, 78);
		centre((position >> 2) + 37,
				position <= 20 ? 4 * position + 242 : 2 * offset + 994, 103);
		centre(position + 3, ((-offset) >> 4) + 128, 79);
		if (picture == kNarrPlanet) {
			// 02E9/B8A7 prepares the actual globe; B85A draws its mask
			// and the 129x110 world over STARS' final approach frame.
			openSpriteSheet("FRESK.HSQ");
			drawSprite(2, 91, 20);
			const int spin = (uint32)READ_LE_UINT16(_state.begin() + 0x197c) * Map::kEquatorWidth / 65536u;
			const int latitude = (int16)READ_LE_UINT16(_state.begin() + 0x197e);
			const int tilt = latitude < 0 ? MIN(-32, latitude) : MAX(32, latitude);
			narrationSpin = spin;
			narrationTilt = tilt;
			_map.drawGlobe(_framebuffer1, kScreenWidth, 96, 25, 129, 110, spin, tilt);
			// 02EF/02F2 reloads STARS after B85A's FRESK bank. Its
			// palette restores the star colours overwritten by the mask.
			openSpriteSheet("STARS.HSQ");
		}
		break;
	}

	case kNarrDesert:
		// 094A/38B4 draws the sky, then INTDS sprite zero at (0,60).
		applySkyPalette(false);
		drawSky();
		openSpriteSheet("INTDS.HSQ");
		drawSprite(0, 0, 60);
		break;

	case kNarrPaul:
		// 02F8/07EE: Paul's composite portrait over BACK's palace list.
		drawList(48, 0x1526, -1, 19, 74, false);
		prepareIntroSpeaker(45);
		_state[0x478c] = 1; // 096A: Paul's single-frame intro pose block.
		drawCurrentSpeakerFigure();
		break;

	case kNarrBaron:
		// 02FB/09AD uses the same shifted Baron scene as his introduction.
		drawList(48, 0x153a, -1, 19, 74, false);
		prepareIntroSpeaker(9, 82);
		drawCurrentSpeakerFigure();
		break;

	case kNarrPalace:
		showPicture(97, false);
		break;

	default:
		break;
	}

	// The line, written the way the game's own lines are.
	const Common::String said = fillInLine(command(line));
	// 8D43/8D4F selects 2275's full-width box below the 152-line scene,
	// vertically centres its rows, and uses colour six, not room subtitles.
	const byte *box = _state.begin() + 0x2275;
	_font.setFace(Font::kFaceNormal);
	const SpeechLayout layout = layoutSpeech(_font, said, READ_LE_UINT16(box + 4));
	const int top = READ_LE_UINT16(box + 2) + MAX<int>(0,
			((int)READ_LE_UINT16(box + 6) - (int)layout.rows.size() * kSpeechRowHeight) / 2);
	paintSpeechLayout(_font, _framebuffer1, layout, READ_LE_UINT16(box), top, layout.rows.size(), 6);

	// 0235 completes C102's transition before 023D starts the voice.
	PlaybackResult asked = applyWipe(0x3a);
	if (asked != kPlaybackFinished)
		return asked;

	// Text words 279..286 are zero-based COMMAND1 entries. AB4F uses
	// independent global recordings 1..8 and clears their variant byte.
	// 0238/ADE0 fades toward the narration volume and balance before AB4F.
	if (_music)
		_music->fadeDriverVolume(100, MAX<byte>(4, _state[0x289e]), _state[0x28b6]);
	_state[0x47e0] = 0;
	playSpeech((uint16)(line - 278), -1);
	if (picture == kNarrPlanet) {
		// 02F5/B8EA registers B9AE once per 200-Hz input poll. B85A's
		// initial full draw does not advance longitude; B9A8 does so only
		// after a completed incremental frame. Keep partial rows off screen.
		Common::Array<byte> drawingPage;
		drawingPage.resize(kScreenWidth * kScreenHeight);
		memcpy(drawingPage.begin(), _framebuffer1, drawingPage.size());
		GlobeRenderState progress;
		uint32 previous = animationClockTicks();
		const uint32 until = _system->getMillis() + holdMs;
		while (_system->getMillis() < until && !shouldQuit()) {
			asked = pollSkip();
			if (asked != kPlaybackFinished)
				break;
			refreshInputRandom();
			updateGameMusic();
			const uint32 now = animationClockTicks();
			const uint16 ticks = animationElapsedTicks(previous, now);
			previous = now;
			if (ticks && _map.stepGlobe(drawingPage.begin(), narrationSpin, narrationTilt, false, progress)) {
				// B98E copies D834's dirty inset. DD03 is clear in the intro,
				// so it draws no player marker and leaves the caption intact.
				const Common::Rect dirty(96, 15, 234, 134);
				for (int y = dirty.top; y < dirty.bottom; y++)
					memcpy(_framebuffer1 + y * kScreenWidth + dirty.left,
							drawingPage.begin() + y * kScreenWidth + dirty.left, dirty.width());
				copyFrameRectToScreen(_framebuffer1 + dirty.top * kScreenWidth + dirty.left,
						kScreenWidth, dirty.left, dirty.top, dirty.width(), dirty.height());
				_system->updateScreen();
				narrationSpin = (narrationSpin + 1) % Map::kEquatorWidth;
			}
			if (ConfMan.getBool("dune_nodelay"))
				break;
			_system->delayMillis(5);
		}
		if (shouldQuit())
			asked = kPlaybackAborted;
	} else {
		asked = holdSpeaking(holdMs, _speakerPerson < 16 ? _speakerPerson : -1);
	}
	stopSpeech(); // 0254 retires the shared digital channel after the wait.
	if (_music) // 0257/ADED restores the ordinary music volume and balance.
		_music->fadeDriverVolume(400, MAX<byte>(4, _state[0x2896]), _state[0x28ae]);
	return asked;
}

/** DDB0/DD63 waits poll input and the sky, never the campaign hour body. */
bool DuneEngine::waitForDisplayInput(uint32 ms) {
	const uint32 started = _system->getMillis();
	do {
		// DD63 checks a new button/key before E3CC/D9D2. Callers finish
		// their transitions even when input shortens this hold.
		if (pollSkip() != kPlaybackFinished)
			break;
		refreshInputRandom();
		updateGameMusic();
		{
			const AnimationClock::Poll poll(_animationClock, _system->getMillis());
			updateSkyFade();
		}
		if (ConfMan.getBool("dune_nodelay") || shouldQuit())
			break;
		const uint32 elapsed = _system->getMillis() - started;
		if (elapsed >= ms)
			break;
		_system->delayMillis(MIN<uint32>(5, ms - elapsed));
	} while (!shouldQuit());
	return !shouldQuit();
}

/** E353 measures from before the draw and checks input only in intro mode. */
void DuneEngine::waitForDisplayFrame(uint32 started, uint32 ms) {
	if (_state[0x227d]) {
		const uint32 elapsed = _system->getMillis() - started;
		if (elapsed < ms)
			waitForDisplayInput(ms - elapsed);
		return;
	}

	while (!shouldQuit()) {
		// Keep the host responsive and discard clicks during the frame.
		// DDB0 starts its next input wait with a fresh latch.
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event))
			updatePointerClickKeys(event);
		const uint32 elapsed = _system->getMillis() - started;
		if (elapsed >= ms)
			break;
		_system->delayMillis(MIN<uint32>(5, ms - elapsed));
	}
}

/** 0264..0292 follows the eight spoken slides, including INT15, separately. */
bool DuneEngine::finishIntroNarration() {
	stopSkyFade(); // 0911 removed any earlier narration animation.
	_state[0x22e3] = 1;
	_state[0x227d] = 1; // The cutscene still owns the panel until 029D.
	_state[0x46df] = 1;
	keepScreen();
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	showPicture(INDEX_XPLAIN9_HSQ, false); // 0301/C0AD/0304/C2F2.
	applyWipe(0x10);
	bool finished = !shouldQuit() && waitForDisplayInput(200 * 5);
	if (finished) {
		startSkyFade(12); // 0272 supplies BL=0Ch, not a calendar-derived sky.
		finished = waitForDisplayInput(1200 * 5);
	}
	stopSkyFade(); // 0282/3950 cancels, rather than draining to its target.
	_state[0x46df] = 0;
	if (finished) {
		keepScreen();
		memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
		applyWipe(0x10); // 028A/C0AD: clear the page, retain its palette.
	}
	_state[0x227d] = 0;
	_state[0xc5] = 0;
	_state[0x28e7] = 0;
	return finished && !shouldQuit();
}

/**
 * Whether the music is off altogether, which 0xAEC6 reads out of 0x10 in
 * DS:0x2943 - the bit 0xAEAF sets as MUSIC OFF is chosen and the sound's own
 * opening at 0xE820 clears.
 */
bool DuneEngine::musicIsOff() const {
	return _state.size() <= kMusicIsOff || (_state[kMusicIsOff] & kMusicOffBit) != 0;
}

/**
 * Takes the song off, which 0xAEB7 does: nothing is playing from then on.
 */
void DuneEngine::stopTheSong() {
	// AEB7 stops the driver even when the retained current-song label is
	// empty. A loaded/pending fade may still have live audio to stop.
	_song.clear();
	_wantedSong.clear();

	if (_music)
		_music->stop();
}

/**
 * Puts the nine songs back in the order they belong in, out of the untouched
 * copy 0x3804 keeps of them, which is what 0xAC97 does as STANDARD ORDER is
 * chosen.
 */
void DuneEngine::orderThePlaylist() {
	if (_state.size() < kPlaylistKept + kPlaylistSongs)
		return;

	memcpy(_state.begin() + kPlaylist, _state.begin() + kPlaylistKept, kPlaylistSongs);
}

/**
 * Shuffles them instead, which 0xACBF does with eighteen swaps of two of the
 * nine taken at random.
 */
void DuneEngine::shuffleThePlaylist() {
	if (_state.size() < 0xd82a)
		return;
	// ACBF draws each index through E3DF (inclusive bound eight), then
	// mixes the 200-Hz timer and remaining swap count into DS:D828 before
	// drawing the second index. This seed also drives map actors and daily
	// production, so host randomness changes later game state.
	uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828);
	const uint16 ticks = READ_LE_UINT16(_state.begin() + 0xce7a);
	auto drawIndex = [&seed]() -> uint16 {
		uint16 index;
		do {
			index = combatRandom(seed, 0xcbd1) & 15;
		} while (index > 8);
		return index;
	};
	for (uint16 remaining = 18; remaining != 0; remaining--) {
		const uint a = drawIndex();
		seed = (uint16)(seed + (ticks ^ remaining));
		const uint b = drawIndex();
		SWAP(_state[kPlaylist + a], _state[kPlaylist + b]);
	}
	WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
}

/** DOS AD00..AD18: leave a 200-tick pause after a CD-style track ends. */
bool DuneEngine::playlistDelayElapsed(uint16 ticks) {
	if (_state.size() < 0xdbd4)
		return false;
	uint16 started = READ_LE_UINT16(_state.begin() + 0xdbd2);
	if (!started) {
		started = ticks;
		WRITE_LE_UINT16(_state.begin() + 0xdbd2, started);
	}
	return (uint16)(ticks - started) >= 200;
}

/** Retain the unplayed part of AD00's gap across a native save/load. */
void DuneEngine::rebasePlaylistWait(uint16 ticks) {
	if (_state.size() < 0xdbd4)
		return;
	const uint16 started = READ_LE_UINT16(_state.begin() + 0xdbd2);
	if (!started)
		return;
	const uint16 savedTicks = READ_LE_UINT16(_state.begin() + 0xce7a);
	const uint16 elapsed = MIN<uint16>((uint16)(savedTicks - started), 200);
	WRITE_LE_UINT16(_state.begin() + 0xdbd2, (uint16)(ticks - elapsed));
	WRITE_LE_UINT16(_state.begin() + 0xce7a, ticks);
}

/**
 * The next of the nine, which is what the music comes to as a CD player's:
 * 0xACE6 takes the song DS:0x380E points at once the last has run out, and
 * 0xAD21 goes back to the first of them at the end of the row - shuffling
 * them again first where DS:0x3810 asks for that.
 */
void DuneEngine::playNextOfPlaylist(bool fromTheTop) {
	if (_state.size() < kPlaylistAt + 2)
		return;

	uint at = fromTheTop ? kPlaylist : READ_LE_UINT16(_state.begin() + kPlaylistAt);

	if (at < kPlaylist || at > kPlaylist + kPlaylistSongs)
		at = kPlaylist;

	byte song = _state[at++];

	if (song == 0xff || fromTheTop) {
		if ((_state[kHowMusicGoes] & kShuffled) != 0) {
			// The DOS timer interrupt keeps CE7A current independently of the
			// gameplay hour. Sample it at the actual shuffle boundary.
			WRITE_LE_UINT16(_state.begin() + 0xce7a, (uint16)(_system->getMillis() / 5));
			shuffleThePlaylist();
		}

		at = kPlaylist;
		song = _state[at++];
	}

	WRITE_LE_UINT16(_state.begin() + kPlaylistAt, (uint16)at);

	if (song == 0 || song >= ARRAYSIZE(kSongs))
		return;

	_song = kSongs[song];

	// 0xADAC hands DS:0x3810's own bit to the player, so a song taken this
	// way is not looped: it runs out and the next of the nine follows it.
	if (_music)
		_music->play(_song, false);
	WRITE_LE_UINT16(_state.begin() + 0xdbd2, 0); // ADB8: playback resets the wait.

	debugC(1, kDebugSound, "%s is next on the playlist", _song.c_str());
}

/**
 * Puts the song on that belongs where the player is, and leaves the one
 * already playing alone where it is the same one - 0xAD9C will not start a
 * song over itself.  0xAD5E asks first whether the music is off at all, and
 * then whether it is going by the place or as a CD player's, which is what
 * the mixer's own list is for.
 */
void DuneEngine::playTheRightSong() {
	if (musicIsOff()) {
		stopTheSong();
		return;
	}

	// 0xAD66: taken as a CD player's the songs are not chosen by the place at
	// all; whichever is playing runs to its end and the next follows it.
	if (_state.size() > kHowMusicGoes && (_state[kHowMusicGoes] & kCdStyle) != 0) {
		// A loaded checkpoint can be between CD-style tracks. Its saved
		// DBD2 wait belongs to updateGameMusic, not an immediate room redraw.
		if (_song.empty() && !READ_LE_UINT16(_state.begin() + 0xdbd2))
			playNextOfPlaylist(false);

		return;
	}

	const Common::String wanted = songForHere();

	if (wanted.empty())
		return;
	if (!(musicContextEntry() & 0x80)) {
		// AD7D/3979 calls the driver's status/loop entry, not its song
		// loader. DNADG 0109/0610 enables looping and returns the playing
		// status without changing tracks. Retain the requested fallback:
		// AD37 can still start it when a pre-existing fade finishes.
		_wantedSong = wanted;
		if (_music) {
			_music->setLooping(true);
			if (_music->playing())
				return;
		}
	} else if (wanted == _song) {
		return;
	}

	// 0xAD8E keeps what the place asks for beside what is playing, and where
	// they differ 0xADBE takes the one playing down to nothing rather than
	// cutting it off.  Only once it has gone does 0xAD37 put the other on.
	// The three hundred it fades over is counted in the timer's own
	// interrupts, of which there are two hundred to the second.
	enum { kFadeMs = 300 * 1000 / 200 };

	if (_music && _music->playing()) {
		if (_wantedSong != wanted)
			debugC(1, kDebugSound, "%s is fading out, and then %s",
				   _song.c_str(), wanted.c_str());

		_wantedSong = wanted;
		_music->fadeOut(kFadeMs);
		return;
	}

	_wantedSong = wanted;

	_song = wanted;

	if (_music)
		_music->play(wanted);

	debugC(1, kDebugSound, "the song here is %s", wanted.c_str());
}

/**
 * How many ornithopters stand where the player is: the count 0x7F27 leaves in
 * the second of the seven bytes it copies out of the place's record, which is
 * what the row that takes one is greyed out by and what draws them outside.
 */
uint DuneEngine::ornithoptersHere() const {
	enum { kHowMany = 0x46ff };

	return _state.size() > kHowMany ? _state[kHowMany] : 0;
}

void DuneEngine::showRoomMenu(bool present) {
	showRoomMenu(present, false);
}

void DuneEngine::showRoomMenu(bool present, bool restorePage) {
	// 2EFB rebuilds the room list on its first page. D2FD instead restores
	// the parent's existing 1F0F byte offset when a conversation closes.
	// Losing that offset turns an empty second-page row into a room action.
	if (!restorePage)
		_state[0x1f0f] = 0;
	preparePlaceNameContext(READ_LE_UINT16(_state.begin() + 0x114e)); // 2EC9.
	resetTroopNoticeIdle(); // 18B1/2E55: returning to the ordinary view.
	// 2EFB only rebuilds the commands. Map exits own 186B's signed-mode
	// transition; an ordinary conversation return must retain the room's
	// existing positive FB byte (e.g. 14 in the later Duncan scene).
	WRITE_LE_UINT16(_state.begin() + 0x1954, 0); // 5ADF clears both selected and preferred troop.
	playTheRightSong();
	enum {
		kSeeDuneMap = 151,
		kLookAtMirror = 152,
		kMixerPanel = 157,
		kTakeOrnithopter = 166,
		kViewNewMessages = 214,
		kMessagesAlreadySeen = 215
	};

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();

	countThingsHere();

	choices.push_back(command(kSeeDuneMap));
	_menuParts.push_back(kRoomMap);
	flags.push_back(0);

	// 0x2F18 offers an ornithopter only in the first room of a place, which is
	// the place seen from outside and where the machines stand; 0x2F4A greys
	// the row out where none of them does.  A greyed row is one that only
	// names something, which is the flag 0x4000 the game writes over it.
	//
	// This row was never built at all, so the machine standing outside could
	// be looked at and not got into.
	const byte room = _state.size() > 5 ? (byte)(READ_LE_UINT16(_state.begin() + 4) & 0xff) : 0;

	const bool localBattle = !_inDesert && battleHere();
	if (!_inDesert && localBattle != (_state[0x2b] != 0))
		updateBattleContext();
	if (localBattle && room == 1) {
		// 2F1D/2218: MASSIVE ATTACK, FIGHT FOR A WHOLE DAY, CALL A WORM.
		// Original command words are one-based; command() takes zero-based ids.
		const uint commands[] = { 153, 154, 167 };
		const uint parts[] = { kRoomMassiveAttack, kRoomFightDay, kRoomBattleWorm };
		for (uint i = 0; i < ARRAYSIZE(commands); i++) {
			choices.push_back(command(commands[i]));
			_menuParts.push_back(parts[i]);
			flags.push_back(i == 2 && storyStage() < 79 ? 0x4000 : 0);
		}
	} else if (!_inDesert && room == 1) {
		choices.push_back(command(kTakeOrnithopter));
		_menuParts.push_back(kRoomOrnithopter);
		flags.push_back(ornithoptersHere() != 0 ? 0 : 0x4000);
	}

	// Palace room nine carries the mirror as a real room action.  Omitting it
	// not only made the mirror unusable, it shifted Jessica and every follower
	// one row upward compared with the original interface.
	if (_state.size() > 8 && _state[8] == 0x20 && room == 9) {
		choices.push_back(command(kLookAtMirror));
		_menuParts.push_back(kRoomMirror);
		flags.push_back(0);
	}

	// The palace communication room retains every transmission at DS:0x1179.
	// DS:0xC8 is the total count and DS:0xC9 the number whose low-byte 0x80
	// mark has not yet been set.  The original shows both controls whenever
	// the history is non-empty, greying the one whose corresponding subset is
	// empty.  Omitting these rows left the Emperor's queued stage-16 message
	// present in memory but completely unreachable from the interface.
	if (_state.size() > 0xc9 && _state[8] == 0x20 && room == 8 && _state[0xc8] != 0) {
		// 2F62..2F83 selects the communications display while rebuilding
		// the room requests: no unread messages, unread messages, or an
		// active remote caller use PALACE scenes 5, 6, or 7 respectively.
		// The descriptor is mutable DOS state, not a fixed room layout.
		if (_state[6] == 0x80 && _state[7] == 1) {
			_state[0x1248] = _state[0x47a9] ? 0x28 : (_state[0xc9] ? 0x27 : 0x26);
			_place = _state[0x1248] - 1;
		}
		choices.push_back(command(kViewNewMessages));
		_menuParts.push_back(kViewNewMessages);
		flags.push_back(_state[0xc9] != 0 ? 0 : 0x4000);

		choices.push_back(command(kMessagesAlreadySeen));
		_menuParts.push_back(kMessagesAlreadySeen);
		flags.push_back(_state[0xc9] < _state[0xc8] ? 0 : 0x4000);
	}

	choices.push_back(command(kMixerPanel));
	_menuParts.push_back(kRoomMixer);
	flags.push_back(0);

	// And then whoever is here, in the order the walk at 0x30A0 takes them,
	// which is the order it stores their names in.
	// 3093/2318 belongs to the logical room entry, not each menu repaint.
	// Keep its first-visit flags/date and any current merchant bargain intact.
	const Common::Array<byte> here = peopleHere(false);

	for (uint i = 0; i != here.size(); i++) {
		if (here[i] == 15 && _state.size() > 0x476a) {
			// 0x30A0 puts person fifteen in the ordinary presence list once,
			// while 0x476A says how many chiefs that one representative stands
			// for.  COMMAND1 follows "Fremen Chief" with its 2nd..8th forms;
			// the unique troop whose id is three is named "Prospector Chief"
			// instead, without changing the ordinal of the following chief.
			const uint chiefs = MIN<uint>(_state[0x476a], 8);
			for (uint chief = 0; chief != chiefs; chief++) {
				const uint16 troop = READ_LE_UINT16(_state.begin() + 0x4758 + 2 * chief);
				const bool prospector = troop >= 0x8aa && troop < 0xfbb &&
						troop < _state.size() && _state[troop] == 3;
				choices.push_back(command(prospector ? 142 : kFirstPerson + 15 + chief));
				_menuParts.push_back(kRoomChiefBase + chief);
				flags.push_back(0);
			}
			continue;
		}

		choices.push_back(command(kFirstPerson + here[i]));
		_menuParts.push_back(here[i]);
		flags.push_back(0);
	}

	_menuKind = kMenuRoom;
	showMenu(choices, &flags, present);
}

void DuneEngine::fightHere(bool massive) {
	if (!battleHere())
		return;
	if (massive) {
		massiveAttack();
		finishBattleRound();
		// 7396 returns through 1B8D: consume the room-change request once,
		// including its normal entry cleanup. A continuing battle keeps its
		// current scene; defeat returns without briefly painting another room.
		if (!shouldQuit())
			refreshBattleMenu();
		return;
	} else {
		// 0FC5/0FD9: run real hourly work, stopping when the local battle ends.
		// Unlike MASSIVE ATTACK, this advances every troop and the economy.
		for (uint i = 0; i < 16 && _state[0x2b] && !shouldQuit(); i++)
			advanceWaitHours(1);
	}
	if (!shouldQuit())
		finishFullDayBattle();
}

/** 0FA7/189A rebuilds the room before applying DNVGA effect 2A. */
void DuneEngine::finishFullDayBattle() {
	const bool defeat = _state[0x46d9] != 0;
	Common::Array<byte> oldPage;
	byte oldPalette[256 * 3];
	if (!defeat) {
		oldPage.resize(kScreenWidth * kScreenHeight);
		memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
		_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	}

	placeTheWandering();
	showRoomMenu(false);
	if (!_inDesert && battleHere()) {
		restoreBattlefield(false);
	} else {
		_state[0x47a4] = 0;
		WRITE_LE_UINT16(_state.begin() + 0x47aa, 0);
		WRITE_LE_UINT16(_state.begin() + 0x14, 0); // 2E07 clears old room contacts.
		_state[0x47a6] = 0xff;
		drawScene(false);
		showLocalRoomRequests(); // 2E52 consumes the inhibited resident pass.
	}
	if (defeat) {
		// 189D calls the room renderer directly when defeat is pending.
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	} else {
		showRoomConversionTransition(oldPage.begin(), oldPalette);
	}
}

void DuneEngine::refreshBattleMenu() {
	// 1B8D consumes two kinds of change: a positive byte refreshes people
	// after an arrival (83D6) or ends a battle;
	// a negative byte means the place itself changed (6DBB's converted
	// fort). Testing only bit zero left the old fort picture on screen.
	if (_state[0x46d9]) {
		_state[0x473b] = 0;
		return;
	}
	if (_onGlobe || _onMixer)
		return;
	if (_inCabin || _menuKind == kMenuWormMap) {
		// 5D6D dispatches positive map mode to 4377. It replaces only
		// DS:149C's map window, retaining the backdrop, hover text and menu.
		const auto refreshCompactMap = [this]() {
			_state[0x46ec] = 0;
			_map.readPlaces(_state.begin(), _state.size());
			drawCabinMap(true);
			_system->copyRectToScreen(_framebuffer1 + 45 * kScreenWidth + 81,
					kScreenWidth, 81, 45, 160, 89);
			_system->updateScreen();
		};
		if (_state[0x46ec])
			refreshCompactMap();
		const bool placeChanged = READ_LE_UINT16(_state.begin() + 0x114e) &&
			(_state[0x473b] & 0x80);
		_state[0x473b] = 0;
		if (placeChanged)
			refreshCompactMap(); // 1BE9 runs separately after the pending request.
		return;
	}
	if (_onWorld) {
		// 1BE9/5D6D redraw the map in place, retaining the selected troop
		// and its contact/menu. A battle ending alone must not close it.
		const auto refreshMap = [this]() {
			_state[0x46ec] = 0;
			_map.readPlaces(_state.begin(), _state.size());
			_map.readTroops(_state.begin(), _state.size());
			rebuildMapUnits(); // 5DA5/6715 resets phases even in the same viewport.
			_mapRebuildRedraw = true;
			redraw();
			_mapRebuildRedraw = false;
			refreshMapInformation(); // 5D6D also invokes 1C18 after rebuilding the map.
			if ((_state[0x46eb] & 0x40) && hasSurveyMap()) {
				// 5DC7/542F follows the large-map copy and 1C18. Keep the
				// old inset in that first presentation, then refresh its own
				// rectangle with an unhighlighted legend and no continuation.
				resetSurveyHover();
				WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
				// 5484/5584 also consumes captures queued since MAP2 opened.
				// Here it follows the old large-map presentation. Other
				// 542F callers consume captures too; passive painting does not.
				updateSurveyTerritory();
				drawSpiceDensityCard();
				const Common::Rect box = surveyFrameRect();
				_system->copyRectToScreen(_framebuffer1 + box.top * kScreenWidth + box.left,
						kScreenWidth, box.left, box.top, box.width(), box.height());
				_system->updateScreen();
			}
		};
		// 1B97 consumes survey changes before testing Paul's current place.
		// The strategic map can cover a flight/walk, with 114E == 0 and the
		// native desert still active underneath. Neither blocks this refresh.
		if (_state[0x46ec])
			refreshMap();
		const bool placeChanged = READ_LE_UINT16(_state.begin() + 0x114e) &&
			(_state[0x473b] & 0x80);
		_state[0x473b] = 0;
		// Both requests run 5D6D separately, in order: combining them changes
		// the actor phases and third RNG seed after the second registration.
		if (placeChanged)
			refreshMap();
		return;
	}
	if (!READ_LE_UINT16(_state.begin() + 0x114e)) {
		_state[0x473b] = 0;
		return;
	}
	if (_inDesert)
		return;
	const byte changed = _state[0x473b];
	const bool conversion = (changed & 0x80) != 0;
	if (_menuKind != kMenuRoom || _speakerPerson >= 0) {
		// 1BD2/1BE3 unwinds the active talk callback before a conversion.
		// A positive room-change request follows the ordinary refresh path.
		if (!conversion || (_menuKind != kMenuRoom && _menuKind != kMenuTalk) ||
				_state[0x46da] || _state[0x4774])
			return;
		_state[0x473b] = 0;
		if (_menuKind == kMenuTalk)
			finishRoomConversation();
		// 2DB1's new scene releases even a retained companion portrait.
		_speakerPerson = -1;
		_speakerFrame = 0;
		_speech.clear();
		_speechParts.clear();
		releaseLocalSpeakerForScene();
	}
	_state[0x473b] = 0;
	if (_state[0x46da] || (battleHere() == (_state[0x2b] != 0) && !changed))
		return;
	if (battleHere() != (_state[0x2b] != 0))
		updateBattleContext();
	Common::Array<byte> oldPage;
	byte oldPalette[256 * 3];
	if (conversion) {
		oldPage.resize(kScreenWidth * kScreenHeight);
		memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
		_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	}
	const bool roomRefresh = changed != 0;
	const bool normalRoomRefresh = roomRefresh && !_state[0x2b];
	if (roomRefresh) {
		stopBattleAnimation(); // Discard the old room's battle actors.
		// 2DB1 starts a new room render, not an idle/portrait repaint.
		// Clear its old portrait/selection and arm 35AD's one-render inhibit.
		_state[0xdce6] = _state[0x47a4] = 0;
		WRITE_LE_UINT16(_state.begin() + 0x47aa, 0);
		_state[0x47a6] = 0xff;
		_state[0x46df] = 0; // Both 2DE7's battle and 2E0A's ordinary path.
		if (normalRoomRefresh)
			WRITE_LE_UINT16(_state.begin() + 0x14, 0); // 2E07: room contact history, not 0E.
	}
	if (conversion)
		roomFor((READ_LE_UINT16(_state.begin() + 0x114e) - 0x100) / 28);
	placeTheWandering();
	const bool restoringBattle = roomRefresh && battleHere();
	if (conversion) {
		// 0FA7/189A/C108 composes 2DB1 offscreen before effect 2A.
		showRoomMenu(false);
		if (restoringBattle)
			restoreBattlefield(false);
		else
			drawScene(false);
	} else {
		if (restoringBattle)
			restoreBattlefield(true);
		else
			drawScene();
		showRoomMenu();
	}
	if (normalRoomRefresh)
		showLocalRoomRequests(); // 35AD consumes the inhibit without selecting a new reply.
	if (conversion)
		showRoomConversionTransition(oldPage.begin(), oldPalette);
}

/** 27C9: four independently coloured stars and the paired console lamps. */
void DuneEngine::paintCommunicationsIndicators(uint phase) {
	// DNVGA leaves DL=15 before each group. E3B7 rejects the preceding
	// colour, including that first 15; every rejected draw advances D824.
	byte previous = 15;
	for (int y = 103; y <= 112; y += 3) {
		byte colour;
		do {
			colour = troopRandom(true) & 15;
		} while (colour == previous);
		previous = colour;
		drawSprite(23 + colour, 163, y);
	}
	const byte side = 19 + (phase & 3);
	_state[0x14ce] = _state[0x14d4] = side;
	drawSprite(side, 84, 92);
	drawSprite(side, 221, 92, 0x4000);
}

/** 2806/281C or 2795: select/scan the retained communications display. */
void DuneEngine::animateMessageConsole(bool scan) {
	const bool fast = ConfMan.getBool("dune_nodelay");
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet("COMM.HSQ");
	if (!scan) {
		drawSprite(18, 156, 100); // 274E's DS:14C8 list, not a room redraw.
		drawSprite(_state[0x14ce], 84, 92);
		drawSprite(_state[0x14d4], 221, 92, 0x4000);
		drawSprite(12, 100, 86);
		drawSprite(13, 100, 86);
	}
	const uint frames = scan ? 13 : 20;
	for (uint phase = 0; phase < frames && !shouldQuit(); ++phase) {
		const uint32 started = _system->getMillis();
		WRITE_LE_UINT16(_state.begin() + 0xd816, phase);
		if (scan) {
			const uint wave = phase & 7;
			drawSprite(13 + (wave < 5 ? wave : 8 - wave), 100, 86);
		}
		paintCommunicationsIndicators(phase);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
		// E3A0 waits nine 200-Hz ticks. It neither queries the mouse nor
		// dispatches pending hourly work as an ordinary room poll would.
		if (fast) {
			advanceClockElapsed(45, false, false);
		} else if (scan) {
			waitForDisplayFrame(started, 45); // 2795 uses E353.
			tick(false);
		} else {
			const uint32 waitStarted = _system->getMillis();
			while (!shouldQuit() && _system->getMillis() - waitStarted < 45) {
				_system->delayMillis(5);
				tick(false);
				updateGameMusic();
			}
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0xd816, frames);
	// ABA9 finishes the existing digital recording; it does not restart it.
	const uint32 started = _system->getMillis();
	while (!fast && !shouldQuit() && _mixer->isSoundHandleActive(_speechHandle) &&
			_system->getMillis() - started < 5000) {
		_system->delayMillis(5);
		tick(false);
		updateGameMusic();
	}
	if (scan)
		return; // 2795/28AA leave the final band (17), stars and lamps19.
	// Only the scan band returns to rest. Keep the twentieth frame's
	// stars and side lamps (22), rather than restoring their initial 19.
	drawSprite(12, 100, 86);
}

/** 2773/27B6: five closing scan frames, then 274E's resting display. */
void DuneEngine::finishMessageConsole() {
	const bool fast = ConfMan.getBool("dune_nodelay");
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet("COMM.HSQ");
	playSoundEffect(9);
	for (int phase = 4; phase >= 0 && !shouldQuit(); --phase) {
		const uint32 started = _system->getMillis();
		WRITE_LE_UINT16(_state.begin() + 0xd816, phase);
		drawSprite(13 + phase, 100, 86);
		paintCommunicationsIndicators(phase);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
				kScreenWidth, kScreenHeight);
		_system->updateScreen();
		// E353 invokes each scan callback, retaining its star RNG draws.
		if (fast)
			advanceClockElapsed(45, false, false);
		else {
			waitForDisplayFrame(started, 45);
			tick(false);
		}
	}
	WRITE_LE_UINT16(_state.begin() + 0xd816, 0xffff);
	drawSprite(18, 156, 100);
	drawSprite(_state[0x14ce], 84, 92);
	drawSprite(_state[0x14d4], 221, 92, 0x4000);
	drawSprite(12, 100, 86);
}

/** Lists the new or already-viewed palace transmissions, newest first. */
void DuneEngine::showMessageList(bool seen) {
	enum { kQueue = 0x1179, kDepth = 10, kCancelCommand = 162 };

	// 283A/283E share 2841: raw PZ audio, stop the old idle figure, then
	// select the console. This is an entry effect, not a list-row repaint.
	_state[0xdb] = seen ? 0xff : 0;
	_state[0x47e0] = 0;
	if (!_mixer->isSoundHandleActive(_speechHandle)) {
		stopSpeech();
		const Common::String filename = Common::String::format("PZ\\PZ%03XI.VOC", seen ? 43 : 42);
		Resource recording = _archive.openMember(filename);
		if (recording)
			playVoice(recording, filename, -1, false);
	}
	if (!_state[0x47c3]) { // 98B2 leaves an active speech figure alone.
		WRITE_LE_UINT16(_state.begin() + 0x4540, 0);
		_state[0x47d1] &= 0x3f;
		const bool registered = READ_LE_UINT16(_state.begin() + 0x47c8) != 0;
		WRITE_LE_UINT16(_state.begin() + 0x47c8, 0);
		if (registered) {
			WRITE_LE_UINT16(_state.begin() + 0x1bf8, 0);
			WRITE_LE_UINT16(_state.begin() + 0x1c06, 0);
			WRITE_LE_UINT16(_state.begin() + 0x47ce, 0);
			WRITE_LE_UINT16(_state.begin() + 0x47c6, 0);
			_speakerIdleActive = false;
			_speakerIdlePerson = -1;
			stopSpeech(); // 98E2/9B8B, only when there was an old figure.
		}
	}
	animateMessageConsole();
	// 2A7F removes automatic request-kind 1, not caller number 1.
	const uint notices = MIN<uint>(_state[0x1190], 10);
	uint kept = 0;
	for (uint i = 0; i < notices; ++i) {
		const byte *entry = _state.begin() + 0x1191 + 4 * i;
		if (entry[0] != 1)
			memmove(_state.begin() + 0x1191 + 4 * kept++, entry, 4);
	}
	_state[0x1190] = kept;
	if (kept != notices)
		memset(_state.begin() + 0x1191 + 4 * kept, 0, 4);
	memset(_state.begin() + 0x47f8, 0xff, 23 * 4); // 3AE9's person corners.
	_state[0xe9] = 0;

	Common::Array<Common::String> choices;
	_menuParts.clear();

	const uint count = _state.size() > 0xc8 ? MIN<uint>(_state[0xc8], kDepth) : 0;
	for (uint reverse = count; reverse != 0; reverse--) {
		const uint which = reverse - 1;
		const uint16 message = READ_LE_UINT16(_state.begin() + kQueue + 2 * which);
		if (((message & 0x80) != 0) != seen)
			continue;

		choices.push_back(command(kFirstPerson + (message & 0x3f)));
		_menuParts.push_back(which);
	}

	choices.push_back(command(kCancelCommand));
	_menuParts.push_back(kGoBack);
	_menuKind = seen ? kMenuSeenMessages : kMenuNewMessages;
	// 289E enters the retained list through D323 after the room is ready.
	// The console effect above changes the room locally, without restarting it.
	showMenu(choices, nullptr, false, true);
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** The two-row panel kept under a remote speaker's transmission. */
void DuneEngine::showMessageMenu() {
	enum { kViewed = 216, kWhat = 148 };

	Common::Array<Common::String> choices;
	choices.push_back(command(kViewed));
	choices.push_back(command(kWhat));

	_menuParts.clear();
	_menuParts.push_back(0);
	_menuParts.push_back(1);
	_menuKind = kMenuMessageView;
	// DS:1FF2 binds Viewed to 2993 and WHAT to 9ED5. Both are actions.
	// 298B rolls this list after 96F1/98F5 has presented the transmission.
	showMenu(choices, nullptr, false, true);
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * Builds the four shipment amounts at DS:0xB4..0xBA.  This is 0x22B1: the
 * first is the Emperor's demand (or all available spice when that is short),
 * and ARGUE walks through three increasingly different alternatives.  The
 * low two status bits at DS:0xBF say which surplus range those alternatives
 * came from and are used directly by Duncan's following dialogue conditions.
 */
void DuneEngine::prepareShipmentOptions() {
	if (_state.size() <= 0xbf)
		return;

	_state[0x9f] = spice() != 0 ? 3 : 0;
	WRITE_LE_UINT16(_state.begin() + 0x20, 0);
	_state[0x1a] = 0;
	_state[0xbf] &= 0xf9;

	const uint16 stock = spice();
	const uint16 demand = READ_LE_UINT16(_state.begin() + 0xbc);
	const uint16 halfStock = stock >> 1;
	const uint16 threeQuarterStock = (stock >> 2) + halfStock;
	const uint16 oneAndHalfDemand = (demand >> 1) + demand;
	const uint16 twiceDemand = (uint16)(demand + demand);

	if (stock < demand) {
		WRITE_LE_UINT16(_state.begin() + 0xb4, stock);
		WRITE_LE_UINT16(_state.begin() + 0xb6, threeQuarterStock);
		WRITE_LE_UINT16(_state.begin() + 0xb8, halfStock);
		WRITE_LE_UINT16(_state.begin() + 0xba, threeQuarterStock - halfStock);
	} else {
		WRITE_LE_UINT16(_state.begin() + 0xb4, demand);
		WRITE_LE_UINT16(_state.begin() + 0xb6, stock);
		if (stock < oneAndHalfDemand) {
			WRITE_LE_UINT16(_state.begin() + 0xb8, threeQuarterStock);
			WRITE_LE_UINT16(_state.begin() + 0xba, halfStock);
			_state[0xbf] |= 2;
		} else if (stock < twiceDemand) {
			WRITE_LE_UINT16(_state.begin() + 0xb8, threeQuarterStock);
			WRITE_LE_UINT16(_state.begin() + 0xba, oneAndHalfDemand);
			_state[0xbf] |= 4;
		} else {
			WRITE_LE_UINT16(_state.begin() + 0xb8, oneAndHalfDemand);
			WRITE_LE_UINT16(_state.begin() + 0xba, twiceDemand);
			_state[0xbf] |= 6;
		}
	}

	// 0x2253 continues past the Emperor-shipment figures and selects one
	// unpaid village account for Duncan to present.  Prefer the oldest bill
	// whose refused/argued flags are clear; if none has aged for a full day,
	// rotate from the account remembered at 0x113f.  The selected account is
	// expanded into the same scratch words peopleHere() prepares in a village.
	// Leaving this half out kept byte 0x22 saying that a bill existed while
	// word 0x20 was zero, so Duncan stopped after "Let me find it...".
	enum { kFirstAccount = 0x10d8, kAccountSize = 17, kLastAccount = 0x113f };
	Common::Array<uint16> accounts;
	for (uint32 account = kFirstAccount; account + kAccountSize <= _state.size();
			 account += kAccountSize) {
		if (_state[account] == 0xff)
			break;
		accounts.push_back((uint16)account);
	}

	const byte today = (byte)(hour() >> 4);
	uint16 selected = 0;
	byte oldest = 0;
	for (uint i = 0; i != accounts.size(); i++) {
		const uint16 account = accounts[i];
		if (READ_LE_UINT16(_state.begin() + account + 0x0e) == 0 ||
				(_state[account + 2] & 0x60) != 0)
			continue;

		const byte age = (byte)(today - _state[account + 0x10]);
		if (age > oldest) {
			oldest = age;
			selected = account;
		}
	}

	if (selected == 0 && !accounts.empty()) {
		const uint16 previous = READ_LE_UINT16(_state.begin() + kLastAccount);
		uint start = 0;
		for (uint i = 0; i != accounts.size(); i++) {
			if (accounts[i] == previous) {
				start = i + 1;
				break;
			}
		}

		for (uint tried = 0; tried != accounts.size(); tried++) {
			const uint16 account = accounts[(start + tried) % accounts.size()];
			if (READ_LE_UINT16(_state.begin() + account + 0x0e) == 0)
				continue;
			selected = account;
			WRITE_LE_UINT16(_state.begin() + kLastAccount, selected);
			break;
		}
	}

	if (selected != 0) {
		WRITE_LE_UINT16(_state.begin() + 0x10b4, selected);
		_state[0x1c] = _state[selected + 2];
		const uint16 bill = READ_LE_UINT16(_state.begin() + selected + 0x0e);
		WRITE_LE_UINT16(_state.begin() + 0x20, bill);
		_state[0x1f] = bill != 0 ? (byte)(today - _state[selected + 0x10]) : 0;
		_state[0x1d] = _state[selected + 1];
		WRITE_LE_UINT16(_state.begin() + 0x11f7, _state[selected]);
	}
}

/** A244/A248's modal answers for shipments, invoices and other negotiations. */
void DuneEngine::showShipmentResponse() {
	// 9F9E paints the spoken line before A244/A248 enter D323. Compose
	// that scene under the existing list, without flushing a pending outer
	// menu roll. A25B selects the deferred report gesture only after this
	// answer list has finished rolling in.
	if (_inDesert)
		drawDesertDialogue(false);
	else
		drawScene(false);

	Common::Array<Common::String> choices;
	// A252 selects DS:1FFE: COMMAND words 241,239,240,149.
	choices.push_back(command(240)); // ARGUE
	choices.push_back(command(238)); // ACCEPT
	choices.push_back(command(239)); // REFUSE
	choices.push_back(command(148)); // " WHAT ? "

	_menuParts.clear();
	for (uint i = 0; i != choices.size(); i++)
		_menuParts.push_back(i);
	_menuKind = kMenuShipmentResponse;
	showMenu(choices, nullptr, false, true); // A258 -> D323/D316/D280.
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * The long-distance flight of a spice load from Arrakis.  STARS.HSQ is not
 * merely the closing narration's background: after its three star-field
 * pieces it holds the sun, progressively larger planets and the
 * turquoise transport's departure and approach frames.  The original
 * shipment routine at 0x2566 walks those same pieces while the communication
 * panel remains at the foot of the screen.
 */
void DuneEngine::playSpiceShipment() {
	enum { kTop = 152, kSun = 36 };
	const bool fast = ConfMan.getBool("dune_nodelay");

	_graphics.setFrameBuffer(_framebuffer1);
	auto restoreRoom = [&]() {
		// 267C/2690 both redraw through 2DB1 while 47A9 is temporarily
		// nonzero. 2F62 therefore selects PALACE scene7 (descriptor28),
		// including COMM sprite17, not the inactive display's scene5.
		openSpriteSheet("PERS.HSQ");
		openSpriteSheet("COMM.HSQ");
		showRoomMenu(false);
		drawScene(false);
		if (!_state[0x2b]) {
			_state[0x47a6] = 0xff;
			showLocalRoomRequests(); // 35AD consumes the one-render inhibit.
			if (!_state[0x4774])
				raisePanelFace(); // 2E5B/17E6; already raised on the second render.
		}
	};
	auto present = [&]() {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
		_system->updateScreen();
	};
	auto drawShipmentGlobe = [&]() {
		// 2555 plays the console sound on both entrances to the globe.
		playSoundEffect(10);
		drawSprite(27, 140, 39);
	};
	auto waitForFlightFrame = [&](uint32 started) {
		if (fast)
			advanceClockElapsed(60, false, false);
		else
			waitForDisplayFrame(started, 60);
		return !shouldQuit();
	};
	// C108 effect 8 reveals pairs of pixels in DNVGA's feedback sequence.
	// The driver copies 150 pairs before waiting for the next 200-Hz tick.
	auto reveal = [&](const byte *oldPage) {
		Common::Array<byte> target(kScreenWidth * kScreenHeight);
		memcpy(target.begin(), _framebuffer1, target.size());
		memcpy(_framebuffer1, oldPage, target.size());
		const bool cursorShown = CursorMan.showMouse(false);
		_noticeTransitionActive = true;
		uint16 sequence = 1;
		uint ticks = 0;
		while (!shouldQuit()) {
			const bool finished = Graphics::revealScene(target.begin(), _framebuffer1, sequence);
			present();
			if (finished)
				break;
			ticks++;
			if (!fast)
				_system->delayMillis(5);
		}
		if (fast)
			advanceClockElapsed(ticks * 5, false, false);
		_noticeTransitionActive = false;
		CursorMan.showMouse(cursorShown);
	};

	// 25DD/25E0 use the message console's 2806 and 2795 routines. Each
	// sprite leaves the transparent pixels of the preceding frame intact.
	animateMessageConsole(false);
	if (shouldQuit())
		return;
	animateMessageConsole(true);
	if (shouldQuit())
		return;
	Common::Array<byte> roomPage(kScreenWidth * kScreenHeight);
	memcpy(roomPage.begin(), _framebuffer1, roomPage.size());
	openSpriteSheet("STARS.HSQ");
	drawShipmentGlobe(); // Called through C097 before effect 8.
	reveal(roomPage.begin());
	if (!waitForDisplayInput(500))
		return;
	if (fast)
		advanceClockElapsed(500, false, false);

	// DOS 0A44 paints each moving sky frame. Its first three sprites are
	// adjacent 304-pixel pieces, shifted by half the square of the frame
	// counter. The sun and two planets have separate centred trajectories.
	// 2608 walks the counter down to zero; 2664 walks it back up after the
	// transport has crossed. This is not a scaled copy of sprite 35.
	auto drawShipmentSky = [&](int frame) {
		memset(_framebuffer1, 0, kScreenWidth * kTop);
		const int shift = -(frame * frame / 2);
		drawSprite(0, shift, 0);
		drawSprite(1, shift + 304, 0);
		drawSprite(2, shift + 608, 0);
		drawSpriteCentred(kSun, shift * 4 + 69, 78);
		const int farPlanetX = frame <= 20 ? 242 + frame * 4 : 994 + shift * 2;
		drawSpriteCentred(37 + frame / 4, farPlanetX, 103);
		drawSpriteCentred(3 + frame, 128 - shift / 16, 79);
		if (!_state[0x227d])
			drawFace(); // 0A44 finishes through C4DD before the transport saves the sky.
	};
	Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	byte oldPalette[768];
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	drawShipmentSky(24);
	showNoticeTransition(oldPage.begin(), oldPalette); // 2605, effect 6.
	for (int frame = 23; frame >= 0 && !shouldQuit(); frame--) {
		const uint32 started = _system->getMillis();
		drawShipmentSky(frame);
		present();
		if (!waitForFlightFrame(started))
			return;
	}
	// 2619/AB4F starts the global launch recording without a speaking
	// portrait. ABA9 waits for A93F to queue the final buffer, then AB15
	// replaces the launch cue with SN1 on the same digital channel.
	_state[0x47e0] = 0;
	bool launchClockCredited = false;
	if (!_mixer->isSoundHandleActive(_speechHandle)) {
		const char *const filename = "PZ\\PZ027I.VOC";
		Resource launch = _archive.openMember(filename);
		if (launch) {
			// Five 8 KiB DMA buffers have finished at that point. The first
			// buffer includes the six-byte VOC block prefix.
			const uint32 launchSamples = 5 * 0x2000 - 6;
			playVoice(launch, filename, -1, false, launchSamples);
			// With no delays, playVoice already credits the audible portion.
			launchClockCredited = fast && _mixer->isSoundHandleActive(_speechHandle);
		}
	}
	if (!waitForDisplayInput(1000)) {
		stopSpeech();
		return;
	}
	if (fast && !launchClockCredited)
		advanceClockElapsed(1000, false, false);
	const uint32 launchWaitStarted = _system->getMillis();
	while (!fast && !shouldQuit() && _mixer->isSoundHandleActive(_speechHandle) &&
			_system->getMillis() - launchWaitStarted < 5000) {
		_system->delayMillis(5);
		tick(false);
		updateGameMusic();
	}
	stopSpeech();
	_state[0xdc2b] = 0;
	if (shouldQuit())
		return;

	playSoundEffect(1); // 2638/AB15 starts SN1 when the transport animates.

	// DOS 263F walks the 6-byte records at DS:15B2, drawing one transport
	// sprite per frame at 26AC.  The craft first rocks in place (46/47),
	// then approaches and passes the viewer (48..60).
	// Keep the sky from the preceding pass behind each frame, as in the
	// recording, rather than moving a second planet through the scene.
	const byte *transport = _staticState.begin() + 0x15b2;
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kTop);
	for (uint frame = 0; frame != 30 && !shouldQuit(); frame++) {
		const uint32 started = _system->getMillis();
		memcpy(_framebuffer1, _framebuffer2, kScreenWidth * kTop);
		const byte *ship = transport + 6 * frame;
		drawSprite(READ_LE_UINT16(ship), READ_LE_INT16(ship + 2), READ_LE_INT16(ship + 4));
		present();
		if (!waitForFlightFrame(started)) {
			stopSoundEffect();
			return;
		}
		if (frame == 18)
			finishSoundEffectLoop(); // 2653/AC30 lets the current loop finish.
	}
	// 2661/26AC presents the restored sky without a timed hold.
	memcpy(_framebuffer1, _framebuffer2, kScreenWidth * kTop);
	present();

	// 2664 runs A44 in reverse and waits on frame 25 before the room transition.
	for (int frame = 1; frame <= 25 && !shouldQuit(); frame++) {
		const uint32 started = _system->getMillis();
		drawShipmentSky(frame);
		present();
		if (!waitForFlightFrame(started))
			return;
	}

	// 268D first pixelates back to the room with 2555's globe still visible;
	// 26A0 then reveals the restored room through effect 8.
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	_state[0x47a9] = 1; // 2677, before BOTH retained-room renders.
	restoreRoom();
	openSpriteSheet("STARS.HSQ");
	drawShipmentGlobe();
	showNoticeTransition(oldPage.begin(), oldPalette);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	restoreRoom();
	_state[0x47a9] = 0; // 2696: keep the layout selected during those renders.
	reveal(oldPage.begin());
	finishMessageConsole(); // 26A3/2773 starts the sound and closes the display.
	if (shouldQuit())
		return;
	present();

	// Rebuild from the retained descriptor without another2F62 menu walk:
	// recomputing it after2696 would replace scene7 with inactive scene5.
	drawScene();
}

/** DOS 35AD: an accepted load ships when Duncan is present in room eight. */
bool DuneEngine::checkSpiceShipment() {
	if (_state.size() <= 0x118e || _inDesert || _onWorld || _onGlobe || _inCabin || _onMixer ||
			READ_LE_UINT16(_state.begin() + 4) != 0x2008 ||
			!(READ_LE_UINT16(_state.begin() + 0xc0) & READ_LE_UINT16(_state.begin() + 0x1158)))
		return false;

	// 35D8 tests presence (DS:12), not the travelling party (DS:10).
	// In particular Duncan may already be here after the previous shipment.
	peopleHere();
	if (!(READ_LE_UINT16(_state.begin() + 0x12) & 8))
		return false;

	sendSpiceShipment();
	takeUpPerson(3);
	enterDialogue(3, kMakingOff);
	showTalkMenu(false);
	sayNextLine();
	return true;
}

/**
 * Commits the amount Duncan and Paul selected.  This is the state half of
 * 0x2566: remove it from palace stock and add it to the day's adjustments
 * at 1172, so dailyRound does not count shipment spending as lost production.
 * This is not a lifetime shipment total: 1C46 consumes and clears it. Turn the
 * amount/demand ratio into the Emperor's reaction byte and disarm the one-shot
 * room-entry trigger before playing the flight.
 */
void DuneEngine::sendSpiceShipment() {
	if (_state.size() <= 0x118e)
		return;

	const uint16 amount = READ_LE_UINT16(_state.begin() + 0xc0);
	const uint16 demand = MAX<uint16>(READ_LE_UINT16(_state.begin() + 0xbc), 1);
	if (!(amount & READ_LE_UINT16(_state.begin() + 0x1158)))
		return;

	// DOS 0x2566 puts Duncan's record at 0x1008 in SI, then calls 0x40E6
	// directly. Only a following Duncan with the temporary errand bit (0x02)
	// stays behind after dispatch. 0x40E0 is the separate room-wide entry and
	// is not used here; neither the room nor another companion's flag matters.
	if ((_state[0x1017] & 0x42) == 0x42) {
		stayBehind(_state[0x1016]);
		// 40E6/9655 redraws the companion boxes before the movie keeps
		// the panel underneath the launch and flight.
		drawCompanionPanel(_state.begin() + kFollowing);
	}

	const uint16 stock = spice();
	// 2566 calls 2524's word SUB directly. Preserve its wrapping ledger if
	// an already accepted load exceeds the stock remaining at dispatch.
	WRITE_LE_UINT16(_state.begin() + 0xa0, (uint16)(stock - amount));
	WRITE_LE_UINT16(_state.begin() + 0x1172,
		(uint16)(READ_LE_UINT16(_state.begin() + 0x1172) + amount));

	uint ratio = MIN<uint32>(((uint32)amount << 8) / demand, 511) >> 1;
	ratio = MAX<uint>(ratio, 1);
	_state[0xbe] = (byte)ratio;

	const byte previousFlags = _state[0xbf];
	byte reactionFlags = 0x40;
	uint responseHours = 7;
	if (ratio < 0xc0)
		responseHours = 6;
	if (ratio <= 0x80) {
		responseHours = 5;
		reactionFlags = 0;
		if (ratio != 0x80) {
			responseHours = 4;
			reactionFlags = 8;
			// Two consecutive short shipments make the next demand use the
			// harsher edition-three scaling path.
			if (previousFlags & 8)
				_state[0xbe] = 0;
		}
	}
	_state[0xbf] = reactionFlags | 0x80;
	// 25BD updates the previous deadline BEFORE E3B7, including its word
	// wrap. The RNG caller already observes this response interval applied.
	const uint16 due = (uint16)(READ_LE_UINT16(_state.begin() + 0x118d) + responseHours);
	WRITE_LE_UINT16(_state.begin() + 0x118d, due);
	const uint extraRange = (_state[0xc3] >> 1) & 3;
	// DOS 0x25c8 calls E3B7 even for a zero mask, advancing the saved D824
	// stream.  For demand two the one-bit mask allows five or six days.
	const uint extraDays = troopRandom(true) & extraRange;
	const uint16 today = hour() >> 4;
	// 25BD adds to the previous deadline, including a shipment on one of
	// the reminder days. Rebasing a late shipment on today grants extra days
	// which the original never grants; preserve the word's wrapping arithmetic.
	const uint16 next = (uint16)(due + extraDays);
	WRITE_LE_UINT16(_state.begin() + 0x118d, next);
	_state[0xcf] = (byte)(next - today);
	WRITE_LE_UINT16(_state.begin() + 0x1158, 0);

	debugC(1, kDebugScript,
		   "%u tenths of spice shipped against a demand of %u; reaction %u",
		   amount, demand, ratio);
	playSpiceShipment();
}

/** Opens one retained transmission and, for a new one, marks it viewed. */
void DuneEngine::showMessage(uint which, bool seen) {
	enum { kQueue = 0x1179, kDepth = 10 };

	const uint count = _state.size() > 0xc8 ? MIN<uint>(_state[0xc8], kDepth) : 0;
	if (which >= count || kQueue + 2 * which + 1 >= _state.size())
		return;

	uint16 message = READ_LE_UINT16(_state.begin() + kQueue + 2 * which);
	if (!seen && !(message & 0x80)) {
		message |= 0x80;
		WRITE_LE_UINT16(_state.begin() + kQueue + 2 * which, message);
		if (_state[0xc9] != 0)
			_state[0xc9]--;

		// 0x2945 recognizes the Emperor's second and third retained messages.
		// They are his shipment demands; reading either puts the viewed bit into
		// the same byte Duncan's stock-report conditions inspect.
		const uint person = message & 0x3f;
		const byte edition = (byte)(message >> 8);
		if (person == 11 && edition >= 2 && edition < 4)
			_state[0xbf] |= 0x20;
	}

	// The high byte selects which remote message this person is sending; the
	// communication records live in the fourth local dialogue part.  For the
	// first Emperor transmission this chooses PHRASE12 line 158 and retains its
	// action, which advances the story from stage 16 to 17.
	_state[0x24] = (byte)(message >> 8);
	const uint person = message & 0x3f;
	// 2958/295F: changing remote callers first runs 28E1. Its edition-twelve
	// branch calls 215F even without another game hour. 28A1 then retains the
	// new caller, which defers D83E's defeat dispatch until Viewed/cleanup.
	if (_state[0x47a9] != person) {
		if (_state[0x24] == 12)
			_state[0x46d9] = 7;
		_state[0x47a9] = (byte)person;
		// 2964/28A1: identify the remote caller over raw PZ audio, then
		// run 2795's thirteen scan frames before selecting the paragraph.
		_state[0x47e0] = 0;
		if (!_mixer->isSoundHandleActive(_speechHandle)) {
			stopSpeech();
			const Common::String filename = Common::String::format("PZ\\PZ%03XI.VOC", person + 26);
			Resource recording = _archive.openMember(filename);
			if (recording)
				playVoice(recording, filename, -1, false);
		}
		animateMessageConsole(true);
		playSoundEffect(10); // 28B5/28BB, after ABA9 releases raw caller audio.
	}
	// The remote speaker remains the subject of the next local reaction.  The
	// original communication path stores that identity at DS:0xE9; Thufir's
	// post-Feyd line, for example, explicitly requires ten here.  Without it
	// the threat itself played but every character behaved as if no caller had
	// just been seen.
	_state[0xe9] = (byte)person;
	for (uint part = person * 8; part != person * 8 + 8 && part < _dialogue.count(); part++) {
		const Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
		for (uint line = 0; line != records.size(); line++) {
			const Dialogue::Record &record = records[line];
			debugC(2, kDebugScript,
				   "remote part %u record %u: line %u flags %02x condition %u (%s): %s",
				   part, line, record.line, record.flags, record.condition,
				   conditionHolds(record.condition) ? "holds" : "-",
				   fillInLine(phraseOf(person, record.line)).c_str());
		}
	}
	// 0x28C5 reads the remote caller's communication-room corner from the
	// packed x/y table at DS:0x225D and stores it in the same per-person words
	// used by an ordinary standing figure.  The Emperor's (157,12) corner is
	// the purple stepped backdrop in the recording; an untouched (0,0) corner
	// enlarged the wrong side of the console behind him.
	const uint32 remoteAt = 0x225d + 2 * person;
	if (remoteAt + 1 < _state.size()) {
		const uint16 packed = READ_LE_UINT16(_state.begin() + remoteAt);
		rememberWhereTheyStood(person, packed & 0xff, packed >> 8);
	}
	// 297A enters 96F1 directly, not ordinary room-contact 92F2/93AA.
	// Bind the caller without preparing an idle pose from the old paragraph.
	selectLocalSpeaker(person, false);
	// 2973..298F masks the pending defeat while the communication record and
	// controls are prepared, then restores it. Conditions see the same state
	// as DOS; the actual visible caller continues to inhibit the defeat.
	const byte pendingDefeat = _state[0x46d9];
	_state[0x46d9] = 0;
	enterDialogue(person, kMakingOff);
	sayNextLine();
	showMessageMenu();
	_state[0x46d9] = pendingDefeat;
}

/** 2993/2997: Viewed supplies request six, menu cleanup supplies zero. */
void DuneEngine::closeMessage(byte request) {
	// 2999..299D retains the remaining unread count, not the old incoming
	// notice latch. This applies to history views as well as new messages.
	_state[0xeb] = _state[0xc9];
	if (!_state[0x47a9]) // 29A1/29A6: the viewer's later cleanup is inert.
		return;
	armPanelMenuTransition();
	letGoOfPerson();
	// 29B5/28E1 first restores the room with 35AD inhibited. Its 17E6
	// face rise still runs, even when the second render immediately lowers
	// the face for a resident's reply. Do not collapse the two renders.
	WRITE_LE_UINT16(_state.begin() + 0x14, 0);
	_state[0x46df] = 0;
	_state[0x47a6] = 0xff;
	drawScene();
	showLocalRoomRequests(); // Consume 28F5's one-render inhibit.
	raisePanelFace();
	// 29B5/28E1 applies the fatal-edition check again, then 2902 releases the
	// remote caller before restoring the ordinary room's request/menu walk.
	if (_state[0x24] == 12)
		_state[0x46d9] = 7;
	_state[0x47a9] = 0;
	finishMessageConsole(); // 2907/2773, before 29B8 clears the edition.
	_state[0x24] = 0;
	_state[0x23] = request;
	// 2993/2997 restores the ordinary room through 2DD3, including its
	// fresh contact context and 35AD resident request walk. The resource
	// conditions decide whether anyone reacts to this transmission; retaining
	// the remote contact or only restoring the menu skips those replies.
	WRITE_LE_UINT16(_state.begin() + 0x14, 0); // 2E07, ordinary room path.
	_state[0x46df] = 0;
	placeTheWandering(); // 29C4/2EB2 restores local representatives too.
	showRoomMenu(false);
	drawScene();
	if (!showLocalRoomRequests())
		raisePanelFace(); // 2E5B/17E6: only when no automatic reply owns the room.
}

/**
 * The mark in the centre of the palace compass opens a plan of its rooms.
 * 0x18EE admits only palace rooms other than the outside one, covers the
 * rectangle at DS:0x143C, frames DS:0x1444 and draws PALPLAN.HSQ's first
 * sprite at the point kept at DS:0x120D.  The little marks laid over it say
 * which rooms hold people, while the larger one says where Paul is.
 */
void DuneEngine::drawPalacePlan() {
	// Redraw the room first: the plan is a temporary overlay and its uncovered
	// left and lower portions remain the room the player was looking at.
	drawScene();
	_graphics.setFrameBuffer(_framebuffer1);

	enum {
		kPlanLeft = 160, kPlanTop = 0, kPlanRight = 320, kPlanBottom = 116,
		kFrameLeft = 164, kFrameTop = 4, kFrameRight = 316, kFrameBottom = 112,
		kPlanGround = 0xf1, kPlanLine = 0xf7,
		kPlanX = 182, kPlanY = 12,
		kObjectRecords = 0xfd8, kObjectRecordSize = 16, kObjectCount = 16,
		kPositionDeltas = 0x1426
	};

	_graphics.fillRect(kPlanLeft, kPlanTop, kPlanRight, kPlanBottom, kPlanGround);
	// 0x5B6E expands the inner rectangle four times.  Each outline is two
	// palette entries darker than the last, making the same graduated frame
	// visible in the recording rather than a single flat line.
	int frameLeft = kFrameLeft;
	int frameTop = kFrameTop;
	int frameRight = kFrameRight;
	int frameBottom = kFrameBottom;
	byte frameColour = kPlanLine;
	for (uint edge = 0; edge != 4; edge++, frameColour -= 2) {
		--frameLeft;
		--frameTop;
		++frameRight;
		++frameBottom;
		_graphics.drawLine(frameLeft, frameTop, frameRight, frameTop, frameColour);
		_graphics.drawLine(frameRight, frameTop, frameRight, frameBottom, frameColour);
		_graphics.drawLine(frameRight, frameBottom, frameLeft, frameBottom, frameColour);
		_graphics.drawLine(frameLeft, frameBottom, frameLeft, frameTop, frameColour);
	}

	openSpriteSheet("PALPLAN.HSQ", false);
	drawSprite(0, kPlanX, kPlanY);

	// 0x1948 divides its 36 counters into twelve upstairs occupants, twelve
	// downstairs occupants and twelve possible player rooms.  Each object at
	// this palace whose condition holds contributes to the first or second set.
	byte counts[36] = { 0 };
	if (_state.size() > kObjectRecords + kObjectRecordSize * kObjectCount) {
		for (uint record = kObjectRecords;
			 record < kObjectRecords + kObjectRecordSize * kObjectCount;
			 record += kObjectRecordSize) {
			if (!namedPersonAvailable(_state[record + 0xe], storyStage()))
				continue;
			if (_state[record + 3] != _state[7])
				continue;
			// This byte is not a CONDIT.HSQ index.  The tiny predicate at
			// 0x127C accepts every value except four, and accepts four as well
			// outside story stages 21 through 31.  Treating it as ordinary
			// dialogue condition data removed almost every palace occupant.
			if (_state[record + 0xe] == 4 && storyStage() >= 21 && storyStage() < 32)
				continue;

			// The first word is the same packed place-kind/room pair kept at
			// DS:4.  0x196F deliberately discards its high byte after taking
			// one from the room number.
			const uint room = _state[record];
			if (room == 0 || room > 12)
				continue;

			const uint floor = (_state[record + 0xf] & 0x40) ? 12 : 0;
			counts[floor + room - 1]++;
		}
	}

	const byte room = _state.size() > 5
		? (byte)(READ_LE_UINT16(_state.begin() + 4) & 0xff) : 0;
	if (room >= 1 && room <= 12)
		counts[23 + room]++;

	// DS:0x1426 keeps eleven unsigned x/y offsets from 182,12.  The original
	// loop reloads that origin for every pair.  Its counter pointer has also
	// been advanced once, because room one is the outside view and is absent
	// from the plan: the eleven points therefore stand for rooms two to twelve.
	for (uint i = 0; i != 11 && kPositionDeltas + 2 * i + 1 < _state.size(); i++) {
		const int x = kPlanX + _state[kPositionDeltas + 2 * i];
		const int y = kPlanY + _state[kPositionDeltas + 2 * i + 1];

		const uint upstairs = MIN<uint>(counts[i + 1], 5);
		for (uint mark = 0; mark != upstairs; mark++)
			drawSprite(2, x + 2 + 4 * mark, y + 3);

		const uint downstairs = MIN<uint>(counts[i + 13], 5);
		for (uint mark = 0; mark != downstairs; mark++)
			drawSprite(2, x + 2 + 4 * mark, y + 10);

		if (counts[i + 25])
			drawSprite(1, x + 9, y + 6);
	}

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

void DuneEngine::showPalacePlan() {
	Common::Array<Common::String> choices;
	choices.push_back(command(160)); // Done

	_menuParts.clear();
	_menuParts.push_back(kRoomPlanDone);
	_menuKind = kMenuPalacePlan;
	_speech = "";
	_speakerPerson = -1;
	showMenu(choices);
}

/** Paul's reflection over the bedroom, behind the ordinary game controls. */
void DuneEngine::drawMirror() {
	drawScene();

	// 0x0ED0 lays the reflected bedroom down in three pieces: the first two
	// before Paul, then the last over him as the glass/frame foreground.
	openSpriteSheet("MIRROR.HSQ");
	drawSprite(0, 0, 0);
	drawSprite(1, 0, 0);
	// Speaker banks are numbered two after their owner; using the synthetic
	// owner 45 selects resource 47, PAUL.HSQ, and lets the bank's own composite
	// record assemble head, body and mouth.  Sprite zero alone is merely the
	// hair silhouette which the old mirror code mistakenly put on screen.
	// 9908/996C skips thirty-two frames of fixed portraits. At age five
	// PAUL's frame zero is a different composition, not the entry reflection.
	drawSpeakerFigure(45, 32);
	openSpriteSheet("MIRROR.HSQ", false);
	drawSprite(2, 0, 0);

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * The mirror is also the original game's in-room save/load menu.  Its five
 * entries are the template at DS:0x2038, in the order visible in the video.
 */
void DuneEngine::showMirror() {
	static const uint16 kRows[] = {
		185, // Restart game
		179, // Load game
		178, // Save game
		186, // Exit game
		156  // Look away from the mirror
	};

	Common::Array<Common::String> choices;
	_menuParts.clear();
	for (uint i = 0; i != ARRAYSIZE(kRows); i++) {
		choices.push_back(command(kRows[i]));
		_menuParts.push_back(kRows[i]);
	}

	_speech = "";
	_speakerPerson = -1;
	_menuKind = kMenuMirror;
	showMenu(choices);
}

/*
 * The mixer panel, which is what the game calls its own settings: MIXR.HSQ's
 * first picture, 210 across and 142 down, holding three sliders - the voices,
 * the music, and the music while a voice is speaking - a VOICE TEST, the seven
 * languages the release was made in, and whether what is said is written out.
 *
 * The picture carries the ladders the sliders run on; the game lays its second
 * sprite, a bar 22 across and 5 down, on each of them at the height its value
 * stands at. A3F0/A408 loads the origin from DS:2886/2888: (40, 1).
 * Drawing and hit testing must use that same origin.
 */
enum {
	kMixerLeft = 40,
	kMixerTop = 1,
	kMixerKnob = 2,
	// The ladders, by the middle of each, and how far the bar may run: the
	// loudest at the top and the quietest at the foot.
	kMixerLadderTop = 33,
	kMixerLadderFoot = 96,
	kMixerSliders = 3,

	// Whether what is said is written out, which the picture offers as
	// Subtitles, Balloons and No Subtitles down its right hand side.
	// A5B0 maps rows 10..12 of DS:28C7's choice rectangle to modes 0..2
	// at 28E7. A541 retains that setting at 28E8. 4738 is unrelated:
	// it holds the prospectors' pending route length.
	kMixerSaidLeft = 134,
	kMixerSaidRight = 200,
	kMixerSaidTop = 39 + 10 * 7,
	kMixerSaidRowHeight = 7,
	kMixerSaidRows = 3,
	kSaidHow = 0x28e7,
	kRetainedSaidHow = 0x28e8,

	// VOICE TEST, across the foot of the picture under the three sliders.
	kMixerTestLeft = 14,
	kMixerTestRight = 118,
	kMixerTestTop = 120,
	kMixerTestFoot = 130
};

static const int kMixerLadder[kMixerSliders] = { 24, 62, 100 };

/**
 * Lays the bar of each slider where its value stands.  A press anywhere in a
 * ladder's column sets that value, so the whole run is a control and not just
 * the bar.
 */
void DuneEngine::drawMixerKnobs() {
	openSpriteSheet("MIXR.HSQ");
	drawSprite(0, kMixerLeft, kMixerTop);

	const int loud[kMixerSliders] = {
		ConfMan.getInt("speech_volume"),
		ConfMan.getInt("music_volume"),
		ConfMan.getInt("sfx_volume")
	};

	for (int i = 0; i != kMixerSliders; i++) {
		const int run = kMixerLadderFoot - kMixerLadderTop;
		const int down = kMixerLadderFoot - CLIP(loud[i], 0, 255) * run / 255;

		drawSprite(kMixerKnob, kMixerLeft + kMixerLadder[i] - 11, kMixerTop + down);
	}

	// A44C/A465 highlights the selected subtitle row with its MIXR asset.
	const byte saidHow = _state[kSaidHow];
	if (saidHow < kMixerSaidRows)
		drawSprite(44 + 2 * saidHow, kMixerLeft + kMixerSaidLeft,
			kMixerTop + kMixerSaidTop + saidHow * kMixerSaidRowHeight);
}

void DuneEngine::showMixer() {
	_onMixer = true;
	_menuKind = kMenuMixer;

	if (_mixerOrdering)
		showMixerOrder();
	else
		showMixerMusic();

	_graphics.setFrameBuffer(_framebuffer1);
	drawPanel();
	// A3F0 paints MIXR over the retained room panel before D32F rolls
	// its command rows. The panel's raised centre crest overlaps the bottom
	// of MIXR: repainting ICONES afterward punches a hole in that frame.
	drawMixerKnobs();
	drawMenu();

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * The list the panel carries while the mixer is up.  It is the five entries
 * of the template at DS:0x201A, in the order they stand there: the three ways
 * the music may be played, the way out of the game, and the row that takes
 * the mixer down.  0xAC3A picks out whichever of the three the music is going
 * by - the first where it is off, the second where the place chooses the
 * song, and the third where the nine are taken as a CD player's.
 */
void DuneEngine::showMixerMusic() {
	enum {
		kMusicOff = 269, kByThePlace = 266, kAsACd = 265,
		kExitGame = 186, kDone = 160
	};

	const bool off = musicIsOff();
	const bool cd = !off && _state.size() > kHowMusicGoes
					&& (_state[kHowMusicGoes] & kCdStyle) != 0;

	static const uint16 kRows[] = { kMusicOff, kByThePlace, kAsACd, kExitGame, kDone };

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();

	for (int i = 0; i != ARRAYSIZE(kRows); i++) {
		choices.push_back(command(kRows[i]));
		_menuParts.push_back(kRows[i]);

		const bool on = kRows[i] == kMusicOff ? off
						: kRows[i] == kByThePlace ? (!off && !cd)
						: kRows[i] == kAsACd ? cd : false;

		flags.push_back(on ? 0x8000 : 0);
	}

	_menu = choices;
	_menuFlags = flags;
	_menuFrom = 0;
	_menuUnder = menuRowAt(_system->getEventManager()->getMousePos(), true);

	debugMenuIfChanged();
}

/**
 * What choosing MUSIC ON (CD-STYLE) puts up instead: the two entries of the
 * template at DS:0x206A and a way back, which ask whether the nine are to be
 * taken in the order they stand in or shuffled.  0xAC7E picks out whichever
 * of them DS:0x3810 carries.
 */
void DuneEngine::showMixerOrder() {
	enum { kStandardOrder = 268, kShuffle = 267, kCancel = 162 };

	const bool shuffled = _state.size() > kHowMusicGoes
						  && (_state[kHowMusicGoes] & kShuffled) != 0;

	static const uint16 kRows[] = { kStandardOrder, kShuffle, kCancel };

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();

	for (int i = 0; i != ARRAYSIZE(kRows); i++) {
		choices.push_back(command(kRows[i]));
		_menuParts.push_back(kRows[i]);
		flags.push_back(kRows[i] == (shuffled ? kShuffle : kStandardOrder) ? 0x8000 : 0);
	}

	_menu = choices;
	_menuFlags = flags;
	_menuFrom = 0;
	_menuUnder = menuRowAt(_system->getEventManager()->getMousePos(), true);

	debugMenuIfChanged();
}

/**
 * A press on the mixer panel.  A ladder's column sets its own value; anywhere
 * else on the picture is let be, and a press outside it puts the panel away.
 */
void DuneEngine::pressMixer(const Common::Point &at) {
	// The list in the panel is the mixer's own: the ways the music may be
	// played, and the way out of the game.
	if (!_menu.empty()) {
		const int row = menuRowAt(at);

		if (row >= 0) {
			chooseFromMenu(row);
			return;
		}
	}

	// The ladders stand thirty-eight apart, so half of that less a little
	// keeps a column clear of the words down the right hand side.
	enum { kHalfWide = 14 };

	if (at.x < kMixerLeft || at.x >= kMixerLeft + 210 ||
		at.y < kMixerTop || at.y >= kMixerTop + 142) {
		_state[kRetainedSaidHow] = _state[kSaidHow]; // A541, on leaving the mixer.
		_onMixer = false;
		redraw();
		if (_inDesert)
			showDesertMenu();
		else
			showRoomMenu();
		return;
	}

	const int across = at.x - kMixerLeft;
	const int down = CLIP<int>(at.y - kMixerTop, kMixerLadderTop, kMixerLadderFoot);

	// VOICE TEST says a line so that the voices can be heard at the volume
	// they have been set to.
	if (across >= kMixerTestLeft && across < kMixerTestRight &&
		at.y - kMixerTop >= kMixerTestTop && at.y - kMixerTop < kMixerTestFoot) {
		debugC(1, kDebugScript, "the mixer is trying a voice out");
		playSpeech(1, 0);
		return;
	}

	// One of the three ways what is said may be shown.
	if (across >= kMixerSaidLeft && across < kMixerSaidRight &&
		at.y - kMixerTop >= kMixerSaidTop &&
		at.y - kMixerTop < kMixerSaidTop + kMixerSaidRows * kMixerSaidRowHeight) {
		const int row = (at.y - kMixerTop - kMixerSaidTop) / kMixerSaidRowHeight;

		_state[kSaidHow] = (byte)row;
		debugC(1, kDebugScript, "what is said is shown as %d", row);

		showMixer();
		return;
	}

	for (int i = 0; i != kMixerSliders; i++) {
		if (ABS(across - kMixerLadder[i]) > kHalfWide)
			continue;

		const int run = kMixerLadderFoot - kMixerLadderTop;
		const int loud = (kMixerLadderFoot - down) * 255 / run;

		static const char *const kKeys[kMixerSliders] = {
			"speech_volume", "music_volume", "sfx_volume"
		};

		ConfMan.setInt(kKeys[i], loud);
		syncSoundSettings();

		debugC(1, kDebugScript, "the mixer's %s is set to %d", kKeys[i], loud);

		showMixer();
		return;
	}
}

/**
 * The planet in the panel's corner is a way into the game's own menu, which is
 * what COMMAND1.HSQ's words 177 to 187 spell out - and the last of them,
 * `EXIT GLOBE`, is what the menu is called by.  Pressing it is how a game is
 * saved or given up.
 */
/*
 * The fresco the planet in the panel opens on, which is FRESK.HSQ: its third
 * sprite is the picture itself, 138 across and 119 down, and 0xB84A puts it at
 * 91,20 over a picture filled with the colour 0xF0.  The first two sprites are
 * the doors in front of it, 106 across and the whole height of the picture,
 * and 0xB87E draws one at the far left and the other at 214 less the same
 * amount, so that as the counter at DS:0xDD0F runs down from nought to -106
 * they slide apart and off the sides.
 */
enum {
	kFrescoLeft = 91, kFrescoTop = 20, kFrescoGround = 0xf0,
	kDoorsApart = 0xd6, kDoorsStop = -106, kDoorsOpen = -112, kDoorStep = 16
};

/**
 * How far round the world the ball in the planet's view stands, and how far
 * it is tilted.  It is not the clock that turns it: 0xB9E0 adds a step to the
 * word at DS:0x494C and holds it inside the three hundred and ninety-eight
 * the world is round, and 0xBA15 does the same for the tilt at DS:0x2460
 * within the ninety-eight either side of the equator - the two of them called
 * with eight or thirty-two one way or the other as the player turns the ball.
 * 0xBA75 sets both outright, and 0xBA9E glides them to a mark, taking at most
 * thirty-two round and twenty-four down at a time and drawing the ball again
 * between one step and the next.
 *
 * 0xBA9E targets the player's position. The longitude is a whole turn in
 * sixteen bits and scales to the world's three hundred and ninety-eight
 * columns.
 */
Common::Point DuneEngine::playerGlobePosition() const {
	if (_state.size() <= 7)
		return Common::Point(-1, 0);

	// 407E tests the latitude sentinel, not the retained current-place word.
	// During travel DS:114E can still name the departure place, while DS:4/6
	// already hold Paul's live position.
	uint16 longitude;
	int latitude;
	if (_state[6] == 0x80) {
		if (_state.size() <= 0x114f)
			return Common::Point(-1, 0);
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here < 0x100 || (uint)here + 6 > _state.size())
			return Common::Point(-1, 0);
		longitude = READ_LE_UINT16(_state.begin() + here + 2);
		latitude = (int16)READ_LE_UINT16(_state.begin() + here + 4);
	} else {
		longitude = READ_LE_UINT16(_state.begin() + 4);
		latitude = (int8)_state[6];
	}

	return Common::Point((uint32)longitude * Map::kEquatorWidth / 65536u,
			CLIP<int>(latitude, 1 - (int)Map::kLatitudes, (int)Map::kLatitudes - 1));
}

/** B8A7/BA75: use the map centre and start at least 32 units from the equator. */
void DuneEngine::centreGlobeOnMap() {
	_globeSpin = (uint32)(uint16)_worldLongitude * Map::kEquatorWidth / 65536u;
	const int latitude = CLIP<int>(_worldLatitude, 1 - (int)Map::kLatitudes, Map::kLatitudes - 1);
	_globeTilt = latitude < 0 ? MIN(-32, latitude) : MAX(32, latitude);
}

/** BA9E: glide through every intermediate globe frame before BC81 opens the map. */
void DuneEngine::glideToPlayerOnGlobe() {
	const Common::Point target = playerGlobePosition();
	if (target.x < 0)
		return;

	bool anotherFrame;
	do {
		// B683 chooses the shorter arc. At a half-turn tie it takes the
		// positive direction, including when the raw difference was negative.
		int round = target.x - _globeSpin;
		if (round > (int)Map::kEquatorWidth / 2)
			round -= Map::kEquatorWidth;
		else if (round <= -(int)Map::kEquatorWidth / 2)
			round += Map::kEquatorWidth;
		const int down = target.y - _globeTilt;
		anotherFrame = round >= 32 || round <= -32 || down >= 24 || down <= -24;
		turnGlobe(CLIP(round, -32, 32), CLIP(down, -24, 24));
	} while (anotherFrame && !shouldQuit());
}

/**
 * Turns the ball, which is what 0xB9E0 and 0xBA15 do between them: the
 * longitude runs round the three hundred and ninety-eight the world is, and
 * the tilt is held within the ninety-eight either side of the equator.
 */
void DuneEngine::turnGlobe(int round, int down) {
	_globeSpin = (int)(((_globeSpin + round) % (int)Map::kEquatorWidth
						+ (int)Map::kEquatorWidth) % (int)Map::kEquatorWidth);
	_globeTilt = CLIP<int>(_globeTilt + down, 1 - (int)Map::kLatitudes, (int)Map::kLatitudes - 1);

	drawFresco();

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
	_system->updateScreen();
	// B98B/B98E presents this phase before B9A8/B9E0 advances the next one.
	_globeSpin = (_globeSpin + 1) % (int)Map::kEquatorWidth;
}

/** 1DC6's B9B9/B9CC/B9C0/B9D3 use the opposite signs to map panning. */
void DuneEngine::workGlobeCompass(int way) {
	static const int kRound[] = { 0, -32, 0, 32 };
	static const int kDown[] = { 8, 0, -8, 0 };
	if (way >= 0 && way < 4)
		turnGlobe(kRound[way], kDown[way]);
}

/** B84A/B84D: the clean ground, before the doors or Paul's display marker. */
void DuneEngine::drawFrescoGround(bool results) {
	_graphics.setFrameBuffer(_framebuffer1);
	// fillRect uses exclusive right/bottom bounds. With the doors open the
	// outermost column and scene row are exposed, so they must not retain
	// pixels from the room or map that preceded the globe.
	_graphics.fillRect(0, 0, kScreenWidth, kSceneHeight, kFrescoGround);

	openSpriteSheet("FRESK.HSQ");
	// The fresco does not carry the terrain ramp used by MAP.HSQ.  Opening it
	// repeatedly while the doors slide can therefore leave entries 16 onward
	// black, making the world disappear precisely when SEE RESULTS is chosen.
	// Restore ONMAP's low palette blocks explicitly on every globe redraw.
	applyMapColours(80);
	drawSprite(2, kFrescoLeft, kFrescoTop);

	// The world itself, inside the oval the third sprite leaves for it, which
	// is the rectangle DS:0x2448 keeps.
	enum { kGlobeLeft = 96, kGlobeTop = 25, kGlobeWide = 129, kGlobeHigh = 110 };

	debugC(3, kDebugGraphics, "the world stands %d of %d round, tilted %d",
		   _globeSpin, Map::kEquatorWidth, _globeTilt);

	_map.drawGlobe(_framebuffer1, kScreenWidth, kGlobeLeft, kGlobeTop,
				   kGlobeWide, kGlobeHigh, _globeSpin, _globeTilt, Map::kMapRamp,
				   results);
}

/** B87E: paint only the doors, retaining the caller's source/display page. */
void DuneEngine::drawFrescoDoors() {
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet("FRESK.HSQ", false);
	// B8F3 paints before subtracting16. Its terminal cursor is -112,
	// but the retained door artwork is the preceding -96 frame. B98B
	// refreshes only the globe, leaving these narrow side borders intact.
	const int paintedDoors = _frescoDoors <= kDoorsStop ? kDoorsOpen + kDoorStep : _frescoDoors;
	drawSprite(0, paintedDoors, 0);
	drawSprite(1, kDoorsApart - paintedDoors, 0);
}

void DuneEngine::drawFresco() {
	drawFrescoGround(_frescoDoors != 0);
	drawFrescoDoors();

	// With the frame open, what the two sides have between them is written
	// over the picture either side of the ball.
	if (_frescoDoors != 0) {
		if (_frescoDoors == kDoorsOpen && !_resultsPage.empty()) {
			restoreResultsPage();
		} else {
			drawResults();
			if (_frescoDoors == kDoorsOpen) {
				_resultsPage.resize(kScreenWidth * kScreenHeight);
				memcpy(_resultsPage.begin(), _framebuffer1, _resultsPage.size());
			}
		}
	}

	setCompass(kCompassGlobe);
	drawPanel();
	drawMenu();
	// B977 paints the source page; BC0C adds Paul only to the displayed
	// copy. BD00 later samples that clean source, not the marker's pixels.
	_globeBackground.resize(kScreenWidth * kScreenHeight);
	memcpy(_globeBackground.begin(), _framebuffer1, _globeBackground.size());
	_globePresented = _globeBackground;
	_globePresentedSpin = _globeSpin;
	_globeDrawing = GlobeRenderState(); // 145h leaves the next 148h run at its start.
	drawGlobePlayerMarker();
}

void DuneEngine::drawGlobePlayerMarker() {
	drawGlobePlayerMarker(_globeSpin);
}

void DuneEngine::drawGlobePlayerMarker(int spin) {
	// BAF2 uses 407E's exact longitude/latitude; BC0C draws ICONES 54 at
	// that projected x and y minus the sprite height, over the displayed UI.
	uint16 longitude = 0;
	int latitude = 0;
	bool valid = _state.size() > 6;
	if (valid && _state[6] == 0x80) {
		const uint32 place = _state.size() > 0x114f
			? READ_LE_UINT16(_state.begin() + 0x114e) : 0;
		if (place < 0x100 || place + 6 > _state.size())
			valid = false;
		else {
			longitude = READ_LE_UINT16(_state.begin() + place + 2);
			latitude = (int16)READ_LE_UINT16(_state.begin() + place + 4);
		}
	} else if (valid) {
		longitude = READ_LE_UINT16(_state.begin() + 4);
		latitude = (int8)_state[6];
	}
	// BAF2 suppresses the marker while 227D's modal drawing latch is active.
	if (valid && _state.size() > 0x227d && _state[0x227d] == 0) {
		const Common::Point marker = _map.globeMarkerPosition(longitude, latitude, spin, _globeTilt);
		if (marker.x >= 0) {
			openSpriteSheet("ICONES.HSQ", false);
			int width = 0, height = 0;
			if (currentSpriteSheet->size(54, &width, &height))
				drawSprite(54, marker.x, marker.y - height);
		}
	}
}

/** BC81: both Exit Globe controls glide back to Paul before the map opens. */
void DuneEngine::returnToPlayerMapFromGlobe() {
	glideToPlayerOnGlobe();
	lookAtPlayer();
	showMapFromGlobe(Common::Point(160, 79));
}

/** BC8D/BC99/5A3D: pixelating boxes, then effect 2 reveals the new map. */
void DuneEngine::showMapFromGlobe(const Common::Point &anchor) {
	if (_globeBackground.size() != kScreenWidth * kScreenHeight)
		drawFresco();
	Common::Array<byte> clean(_globeBackground);
	const Common::Rect limits(READ_LE_UINT16(_state.begin() + 0x2440),
		READ_LE_UINT16(_state.begin() + 0x2442), READ_LE_UINT16(_state.begin() + 0x2444),
		READ_LE_UINT16(_state.begin() + 0x2446));
	const bool cursorShown = CursorMan.showMouse(false);
	const bool fast = ConfMan.getBool("dune_nodelay");
	const bool previousTransition = _noticeTransitionActive;
	_noticeTransitionActive = true;
	uint32 deadline = _system->getMillis();
	uint elapsed = 0;
	auto present = [&](bool wait) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		updateGameMusic();
		if (wait) {
			deadline += 5;
			elapsed += 5;
			const uint32 now = _system->getMillis();
			if (!fast && now < deadline)
				_system->delayMillis(deadline - now);
		}
	};
	for (uint phase = 0; phase < 8 && !shouldQuit(); phase++) {
		const Common::Rect bounds = Graphics::globeMapZoomBounds(anchor, limits, phase);
		WRITE_LE_UINT16(_state.begin() + 0xdd06, bounds.left);
		WRITE_LE_UINT16(_state.begin() + 0xdd08, bounds.top);
		WRITE_LE_UINT16(_state.begin() + 0xdd0a, bounds.right);
		WRITE_LE_UINT16(_state.begin() + 0xdd0c, bounds.bottom);
		_state[0xdd0e] = 7;
		const uint16 started = (uint16)(_system->getMillis() / 5);
		uint sample = 0;
		do {
			if (!Graphics::pixelateGlobeMapBox(clean.begin(), _framebuffer1, bounds, sample))
				break;
			present(fast);
			sample++;
			// 3581/35BC cycles samples until ten unsigned timer ticks pass,
			// not for a fixed number of paints. Fast mode supplies one tick
			// per sample, matching the controlled original-driver oracle.
		} while (!shouldQuit() && (fast ? sample < 10 :
			(uint16)((uint16)(_system->getMillis() / 5) - started) < 10));
	}
	Common::Array<byte> old(kScreenWidth * kScreenHeight);
	memcpy(old.begin(), _framebuffer1, old.size());
	_frescoDoors = 0;
	_onGlobe = false;
	_onWorld = true;
	_state[0xfb] = 0xff; // 5A3D selects strategic map speech.
	_state[0x46eb] = 0x80;
	showMapMenu(false);
	showWorld(false);
	Common::Array<byte> target(kScreenWidth * kScreenHeight);
	memcpy(target.begin(), _framebuffer1, target.size());
	memcpy(_framebuffer1, old.begin(), old.size());
	if (!fast)
		deadline = _system->getMillis();
	for (uint phase = 0; phase < 74 && !shouldQuit(); phase++) {
		Graphics::revealGlobeMapFrame(target.begin(), _framebuffer1, phase);
		present(true);
	}
	// C108/C4CD copies the entire composed map after the last spiral chunk.
	memcpy(_framebuffer1, target.begin(), target.size());
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
	if (fast)
		advanceClockElapsed(elapsed, false, false);
	_noticeTransitionActive = previousTransition;
	CursorMan.showMouse(cursorShown);
}

/** DOS BED7: snapshot totals/trends and scale six byte-sized targets. */
void DuneEngine::prepareResultsGaugeTargets() {
	if (_state.size() <= 0xdd1c)
		return;
	WRITE_LE_UINT16(_state.begin() + 0x115c, hour() & 0xfff0);
	updateResultsTotals();
	updateResultsTrends();
	static const uint kResultWords[6] = { 0xa4, 0xa2, 0xa8, 0xa6, 0xac, 0xaa };
	for (uint i = 0; i != ARRAYSIZE(kResultWords); i++) {
		const uint16 value = READ_LE_UINT16(_state.begin() + kResultWords[i]);
		const uint16 target = i < 2 ? (value >> 1) + 1
			: i < 4 ? (value >> 4) + 1 : (value >> 8) + 1;
		_state[0xdd11 + i] = (byte)target; // BEF0 stores AL, not AX.
	}
}

/** DOS BE29: opening results starts all six rising gauges from zero. */
void DuneEngine::initialiseResultsGauges() {
	if (_state.size() <= 0xdd1c)
		return;
	prepareResultsGaugeTargets();
	_resultsPage.clear();
	memset(_state.begin() + 0xdd17, 0, 6);
	// BE54 calls DA25 then falls directly into BE57, painting its first
	// step immediately. Registration is not a wait for another twelve ticks.
	stepResultsGauges();
	_resultsGaugeRemainder = 0;
	_resultsGaugeTime = _system->getMillis() / 5;
}

/** DOS 1C39/BDBB: retarget open results after work, retaining callback time. */
void DuneEngine::refreshResultsGauges() {
	if (!_onGlobe || _onMixer || _inCabin || _frescoDoors == 0 || _state.size() <= 0xdd1c)
		return;
	prepareResultsGaugeTargets();
	// BDE0's CMPSB/LOOPNE visits every matching counter. Keep unfinished
	// columns where they stand; a completed one takes one more BE57 step
	// so its direction glyph is painted again (the numbers are written now).
	for (uint i = 0; i < 6; i++) {
		if (_state[0xdd17 + i] == _state[0xdd11 + i])
			_state[0xdd17 + i]--;
	}
	if (_resultsPage.empty()) {
		_resultsPage.resize(kScreenWidth * kScreenHeight);
		memcpy(_resultsPage.begin(), _framebuffer1, _resultsPage.size());
	}
	// BDBB writes both text lists now, even while a gauge is unfinished.
	// It neither erases the existing columns nor runs BE57 prematurely.
	byte *visible = _framebuffer1;
	_framebuffer1 = _resultsPage.begin();
	_graphics.setFrameBuffer(_framebuffer1);
	drawResults(false);
	_framebuffer1 = visible;
	_graphics.setFrameBuffer(_framebuffer1);
	presentResultsPage();
}

/** Keep BE57's accumulated artwork outside B98B's rotating globe inset. */
void DuneEngine::restoreResultsPage() {
	restoreResultsPage(_framebuffer1);
}

void DuneEngine::restoreResultsPage(byte *page) {
	static const Common::Rect boxes[] = {
		Common::Rect(8, 6, 209, 15), Common::Rect(216, 6, 317, 15),
		// FRESK's Harkonnen crest at y=136 is fourteen rows high. Retain its
		// last two rows too; B98B leaves both crests intact outside the inset.
		Common::Rect(8, 20, 96, 90), Common::Rect(11, 124, 96, 150),
		Common::Rect(236, 8, 317, 81), Common::Rect(236, 80, 317, 150)
	};
	for (uint i = 0; i < ARRAYSIZE(boxes); i++) {
		const Common::Rect &box = boxes[i];
		for (int y = box.top; y < box.bottom; y++)
			memcpy(page + y * kScreenWidth + box.left,
					_resultsPage.begin() + y * kScreenWidth + box.left, box.width());
	}
}

void DuneEngine::presentResultsPage() {
	restoreResultsPage();
	if (_globeBackground.size() == kScreenWidth * kScreenHeight)
		restoreResultsPage(_globeBackground.begin());
	if (_globePresented.size() == kScreenWidth * kScreenHeight)
		restoreResultsPage(_globePresented.begin());
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** BE57 paints one strip/cap per changed column, without clearing old ink. */
void DuneEngine::paintResultsGaugeSteps(const byte *previous) {
	if (_resultsPage.empty())
		return;
	static const int x[6] = { 26, 54, 252, 280, 252, 280 };
	static const int foot[6] = { 62, 62, 54, 54, 125, 125 };
	byte *visible = _framebuffer1;
	_framebuffer1 = _resultsPage.begin();
	_graphics.setFrameBuffer(_framebuffer1);
	openSpriteSheet("ICONES.HSQ", false);
	_font.setFace(Font::kFaceAlternate);
	for (uint i = 0; i < 6; i++) {
		const byte current = _state[0xdd17 + i];
		if (current == previous[i])
			continue;
		const int rise = MIN<int>(current, 30);
		drawSprite(i & 1 ? 56 : 55, x[i], foot[i] - rise);
		drawSprite(57, x[i], foot[i] - rise - 10);
		if (current == _state[0xdd11 + i] && _state[0x116a + i] <= 3)
			_font.drawChar(_framebuffer1, x[i] + 4, foot[i] - rise - 10,
					_state[0x116a + i], i & 1 ? 0x25 : 0x3f);
	}
	_framebuffer1 = visible;
	_graphics.setFrameBuffer(_framebuffer1);
}

/** DOS BE57: each registered callback moves every unfinished column one step. */
bool DuneEngine::stepResultsGauges() {
	if (_state.size() <= 0xdd1c)
		return false;
	bool changed = false;
	for (uint i = 0; i != 6; i++) {
		byte &current = _state[0xdd17 + i];
		const byte target = _state[0xdd11 + i];
		if (current != target) {
			current += current < target ? 1 : -1;
			changed = true;
		}
	}
	return changed;
}

/** D9D2 runs at most one BE57 callback per poll, retaining modulo 12 ticks. */
bool DuneEngine::pollResultsGauges(uint16 ticks) {
	if (!_onGlobe || _frescoDoors == 0 || _state.size() <= 0xdd1c)
		return false;
	const uint16 elapsed = ticks + _resultsGaugeRemainder;
	_resultsGaugeRemainder = elapsed % 12;
	byte previous[6];
	memcpy(previous, _state.begin() + 0xdd17, 6);
	const bool changed = elapsed >= 12 && stepResultsGauges();
	if (changed)
		paintResultsGaugeSteps(previous);
	return changed;
}

void DuneEngine::updateResultsGauges() {
	if (!_onGlobe || _frescoDoors == 0)
		return;
	// BE29/loading already initialised the anchor; zero is a valid epoch.
	const uint32 now = animationClockTicks();
	const bool changed = pollResultsGauges(animationElapsedTicks(_resultsGaugeTime, now));
	_resultsGaugeTime = now;
	if (!changed)
		return;
	presentResultsPage();
}

/**
 * Slides the two halves of the frame off the sides of the screen and back
 * again, which 0xB8F3 and 0xB915 do sixteen pixels at a time.
 */
void DuneEngine::slideFrescoDoors(bool open) {
	// B8F3 paints0,-16,..,-96 then retains-112 without painting it.
	// B915 increments first, painting-96,..,0 before its final closed copy.
	if (_frescoDoors == kDoorsStop) // Older native checkpoints used a clamped cursor.
		_frescoDoors = kDoorsOpen;
	Common::Array<byte> clean;
	if (open) {
		// B8F3 builds B84A/BE1D once, before changing DD02. C474 keeps
		// that clean scene; C43E restores it before each outgoing door paint.
		// There is no BC0C marker until the final B98B presentation.
		drawFrescoGround(_state[0xdd02] != 0);
		openSpriteSheet("FRESK.HSQ", false);
		drawResults();
		clean.resize(kScreenWidth * kSceneHeight);
		memcpy(clean.begin(), _framebuffer1, clean.size());
		_resultsPage.resize(kScreenWidth * kScreenHeight);
		memcpy(_resultsPage.begin(), _framebuffer1, _resultsPage.size());
	} else {
		// B915 removes BE57, then paints directly over the retained display.
		// Its globe, old marker and statistics must not be redrawn/advanced.
		_resultsGaugeTime = 0;
	}
	while (open ? _frescoDoors > kDoorsStop : _frescoDoors != 0) {
		if (!open)
			_frescoDoors = MIN(0, _frescoDoors + kDoorStep);
		WRITE_LE_UINT16(_state.begin() + 0xdd0f, (uint16)_frescoDoors);
		// B8F3/B915 paint synchronously. C4DD hides the cursor, not a
		// D9D2 poll: leave the registered gauge clock/remainder untouched
		// until the next ordinary input poll after the film returns.

		if (open)
			memcpy(_framebuffer1, clean.begin(), clean.size());
		drawFrescoDoors();
		if (open)
			drawFace(); // C4DD/C4F0's 1797 notch belongs to the presented page.

		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
								  kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (!ConfMan.getBool("dune_nodelay"))
			_system->delayMillis(30);
		if (open)
			_frescoDoors -= kDoorStep;
	}
	WRITE_LE_UINT16(_state.begin() + 0xdd0f, (uint16)_frescoDoors);
	_state[0xdd02] = open ? 0xff : 0; // B96E/B964: results versus standard vision.
	if (open) {
		// Retain exactly the final displayed gauge/crest artwork and narrow
		// door borders, not counters repainted over the moving door film.
		memcpy(_resultsPage.begin(), _framebuffer1, _resultsPage.size());
	} else {
		// C07C/B87E presents the final closed page once more.
		drawFrescoDoors();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** D2BD/5ADF: close map children before releasing their actors and callbacks. */
void DuneEngine::teardownStrategicMap(bool animateContact) {
	// B8C6/186B first run D2BD's child callbacks while the chief and ring
	// are still registered. Clearing MAP2 first bypasses 82B7's report
	// restart, including its dialogue actions/one-shot marks.
	_state[0xdce6] = 0x80; // D2BD suppresses rolling each popped command list.
	if (_menuKind == kMenuMoveTroop) {
		restoreTroopRouteReport();
		const uint record = READ_LE_UINT16(_state.begin() + 0x46ef);
		if (record && record + 3 < _state.size() && (_state[record + 3] & 0x40))
			showTroopMenu(false, false);
		else
			showSettledTroopMenu(false, false);
		drawMenu(); // D2FD restores the parent before 8751's ring/report close.
	} else if (_menuKind == kMenuEquipment && _state[0x46f5]) {
		finishEquipment(true, false); // 7D68 acknowledgement, without Done's extra flush.
	}
	closeSurveyWindow();
	closeMapPlaceCard();
	finishTroopContact(animateContact);
	closeMapTroopCard();
	_state[0x46d8] = 1; // 7B36 suppresses subsequent map-window effects.
	_state[0x46eb] = 0;
	_state[0x46f3] = 0;
	_state[0x46f4] = 0;
	_state[0xdce6] = 0; // 5ADF/7B36 finishes with no pending command roll.
	_panelMenuOldPage.clear();
	WRITE_LE_UINT16(_state.begin() + 0x46f8, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1954, 0);
	WRITE_LE_UINT16(_state.begin() + 0xa5c0, 0);
	WRITE_LE_UINT16(_state.begin() + 0x3cbe, 0);
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0);
	WRITE_LE_UINT16(_state.begin() + 0xdbe2, 0);
	_pressedPlace = _pointedAtPlace = -1;
	_mapWindowGeometryValid = _troopWindowGeometryValid = false;
	clearMapUnits();
	_mapAnimationActive = false; // DA5F removes the 6B34 callback.
}

enum {
	kSeeResults = 176, kStandardVision = 177, kSaveGame = 178, kLoadGame = 179,
	kSeeMapOfThisArea = 181, kExitGlobe = 182, kExitGame = 186
};

void DuneEngine::setGlobeMenu(bool pickingArea, bool present) {
	Common::Array<Common::String> choices;
	_menuParts.clear();
	if (pickingArea) {
		// BC1F installs DS:2062, with a preselected row and a no-op callback.
		_menuParts.push_back(kSeeMapOfThisArea);
		choices.push_back(command(kSeeMapOfThisArea));
		const Common::Array<uint16> flags(1, 0x8000);
		showMenu(choices, &flags, present);
		return;
	}

	// B941 switches the second row of DS:204A according to the doors.
	const uint16 rows[] = {
		kExitGlobe, (uint16)(_frescoDoors != 0 ? kStandardVision : kSeeResults),
		kLoadGame, kSaveGame, kExitGame
	};
	for (uint16 row : rows) {
		_menuParts.push_back(row);
		choices.push_back(command(row));
	}
	showMenu(choices, nullptr, present);
}

void DuneEngine::showGlobe() {
	const bool returningFromConfirmation = _onGlobe && _menuKind == kMenuLeaving;

	// B8C6 first unwinds the menu through D2BD, then 5ADF releases the
	// strategic map. Do this before replacing an equipment/contact menu:
	// its close callback commits transfers and consumes the active report.
	// B941's later results/menu refresh must not repeat that teardown.
	if (!_onGlobe) {
		teardownStrategicMap(true);
		_frescoDoors = 0;
		_state[0xdd02] = 0; // B827 opens a fresh globe in Standard Vision.
		WRITE_LE_UINT16(_state.begin() + 0xdd0f, 0);
		centreGlobeOnMap();
		_globeAnimationTime = _system->getMillis() / 5;
	}

	_onGlobe = true;
	_menuKind = kMenuGlobe;
	setCompass(kCompassGlobe);

	// The fresco has a song of its own, which 0xAA96 answers while the planet
	// is open; leaving the room's song on played ARRAKIS behind the globe.
	playTheRightSong();

	// D2E2 restores the parent menu on the current globe frame.
	if (!returningFromConfirmation)
		_globePresented.clear(); // B972/B98B requests a full frame.
	setGlobeMenu(false, true);
}

/**
 * Whether the player really means to give the game up, which the game asks
 * outright: its words 184 and 185 are the two answers.
 */
void DuneEngine::showLeaving() {
	enum { kYes = 183, kNo = 184 };

	Common::Array<Common::String> choices;
	_menuParts.clear();

	choices.push_back(command(kYes));
	_menuParts.push_back(kYes);
	choices.push_back(command(kNo));
	_menuParts.push_back(kNo);

	_menuKind = kMenuLeaving;
	showMenu(choices);
}

/**
 * What the two sides have between them, which is what SEE RESULTS opens the
 * frame of the world onto.  None of it belongs in the panel: the recording
 * lays it over the picture either side of the ball, and the panel keeps the
 * five rows the globe screen has all along - only its second reads STANDARD
 * VISION while the frame stands open.
 *
 * The words are the command file's own, written over as the game runs: the
 * hundred and ninety-third holds the day, the ninety-fourth the charisma, the
 * ninety-fifth and sixth the two shares of the world, the ninety-eighth and
 * ninety-ninth what each side makes of the spice, and the two hundred and
 * first and second how many men each has.  DS:2482/2494 in the original
 * engine give their exact text origins and colours.
 *
 * 0xBFE3 counts the ownership bits in MAP.HSQ, 0xC02E and 0xC04E prepare the
 * production and army totals, and the six little gauges at 0xBE57 scale those
 * values into thirty vertical steps.  Keeping that arithmetic matters: troop
 * sizes are tens of men and the two houses are deliberately shown blue for
 * the Harkonnens and red for the Atreides.
 */
void DuneEngine::drawResults() {
	drawResults(true);
}

void DuneEngine::drawResults(bool gauges) {
	enum {
		kControlled = 197, kProduction = 200, kMen = 203
	};
	// DS:2482/2494 give the DOS text origins and inks. The numeric entries in
	// COMMAND1 retain their leading spaces: E2E3 writes three right-aligned
	// share digits and E31C writes five unit digits before BF61's final zero.
	enum {
		kDayLeft = 16, kTopRow = 6, kCharismaLeft = 216,
		kHarkColumn = 20, kOurColumn = 48, kSharesRow = 69,
		kControlledLeft = 8, kControlledRow = 80,
		kSpiceHark = 240, kSpiceOurs = 272, kSpiceRow = 60,
		kProductionLeft = 236, kProductionRow = 71,
		kMenHark = 240, kMenOurs = 272, kMenRow = 131,
		kMenLeft = 236, kMenLabelRow = 142,
		kCrestNameLeft = 35, kOurNameRow = 125, kHarkNameRow = 139
	};

	// Which of COMMAND1's lines each side is named by.
	enum { kOurName = 174, kHarkName = 175 };

	// These are the exact colours 0xBEBD selects after each gauge reaches its
	// target: even (Harkonnen) columns use 0x3F, odd (Atreides) ones 0x25.
	enum { kTopInk = 0xfd, kInk = 0xfb, kHarkInk = 0x3f, kOurInk = 0x25 };

	const uint harkMenUnits = READ_LE_UINT16(_state.begin() + 0xac);
	const uint ourMenUnits = READ_LE_UINT16(_state.begin() + 0xaa);
	const uint harkSpiceUnits = _state.size() > 0xa9
		? READ_LE_UINT16(_state.begin() + 0xa8) : 0;
	const uint ourSpiceUnits = _state.size() > 0xa7
		? READ_LE_UINT16(_state.begin() + 0xa6) : 0;
	const uint harkShare = READ_LE_UINT16(_state.begin() + 0xa4);
	const uint ourShare = READ_LE_UINT16(_state.begin() + 0xa2);

	const uint day = ((hour() + 3) >> 4) % 365 + 1;
	const uint charisma = _state.size() > 0x29 ? _state[0x29] >> 1 : 0;

	// FRESK's third and fourth sprites are the two crests and their names.
	// They must be taken before switching to ICONES for the six gauges.
	if (gauges) {
		drawSprite(3, 11, 124);
		drawSprite(4, 11, 136);
	}

	static const int kGaugeX[6] = { 26, 54, 252, 280, 252, 280 };
	static const int kGaugeFoot[6] = { 62, 62, 54, 54, 125, 125 };
	const uint gauge[6] = {
		harkShare / 2 + 1, ourShare / 2 + 1,
		harkSpiceUnits / 16 + 1, ourSpiceUnits / 16 + 1,
		(harkMenUnits >> 8) + 1, (ourMenUnits >> 8) + 1
	};
	// DD11..DD16 are BEF0's targets, DD17..DD1C BE57's live counters.
	// Legacy fresh views that never ran the opening film have zero targets;
	// keep their complete static presentation when drawn directly.
	bool animated = false;
	if (_state.size() > 0xdd1c) {
		for (uint i = 0; i != 6; i++)
			animated |= _state[0xdd11 + i] != 0;
	}
	auto complete = [&](uint i) {
		return !animated || (_state[0xdd11 + i] != 0 &&
			_state[0xdd11 + i] == _state[0xdd17 + i]);
	};

	if (gauges)
		openSpriteSheet("ICONES.HSQ", false);
	_font.setFace(Font::kFaceAlternate);
	for (uint i = 0; gauges && i != ARRAYSIZE(gauge); i++) {
		const int rise = MIN<uint>(animated ? _state[0xdd17 + i] : gauge[i], 30);
		// 0xBE57 moves the four-pixel block up one pixel at a time without
		// erasing its previous positions, building a solid vertical gauge.
		for (int step = 1; step <= rise; step++)
			drawSprite((i & 1) ? 56 : 55, kGaugeX[i], kGaugeFoot[i] - step);
		if (rise)
			drawSprite(57, kGaugeX[i], kGaugeFoot[i] - rise - 10);
		// BEBD/D12F retain BEA0's cap origin (ten pixels above the bar).
		// Unfinished columns have no direction glyph.
		const byte trend = _state[0x116a + i];
		if (complete(i) && trend <= 3)
			_font.drawChar(_framebuffer1, kGaugeX[i] + 4, kGaugeFoot[i] - rise - 10,
					trend, (i & 1) ? kOurInk : kHarkInk);
	}

	// BFA7 uses the final digit, except when the preceding digit is 1.
	const uint endingDigit = day % 10;
	const char *const kEnding = day % 100 / 10 == 1 ? "th"
							  : endingDigit == 1 ? "st"
							  : endingDigit == 2 ? "nd"
							  : endingDigit == 3 ? "rd" : "th";

	// BDFA selects D068's nine-row face; D075 selects seven for the sides.
	// D1A6 supplies F0 as the background ink, not a transparent sentinel.
	_font.setFace(Font::kFaceNormal);
	_font.drawString(_framebuffer1, kDayLeft, kTopRow,
					 Common::String::format("%3u%s day on DUNE", day, kEnding), kTopInk, 0xf0);
	_font.drawString(_framebuffer1, kCharismaLeft, kTopRow,
					 Common::String::format("CHARISMA = %3u", charisma), kTopInk, 0xf0);
	_font.setFace(Font::kFaceAlternate);

	_font.drawString(_framebuffer1, kHarkColumn, kSharesRow,
					 Common::String::format("%3u%%", harkShare), kHarkInk, 0xf0);
	_font.drawString(_framebuffer1, kOurColumn, kSharesRow,
					 Common::String::format("%3u%%", ourShare), kOurInk, 0xf0);
	_font.drawString(_framebuffer1, kControlledLeft, kControlledRow,
					 command(kControlled), kInk, 0xf0);

	_font.drawString(_framebuffer1, kSpiceHark, kSpiceRow,
					 Common::String::format("%5u0 ", harkSpiceUnits), kHarkInk, 0xf0);
	_font.drawString(_framebuffer1, kSpiceOurs, kSpiceRow,
					 Common::String::format("%5u0 ", ourSpiceUnits), kOurInk, 0xf0);
	_font.drawString(_framebuffer1, kProductionLeft, kProductionRow,
					 command(kProduction), kInk, 0xf0);

	_font.drawString(_framebuffer1, kMenHark, kMenRow,
					 Common::String::format("%5u0 ", harkMenUnits), kHarkInk, 0xf0);
	_font.drawString(_framebuffer1, kMenOurs, kMenRow,
					 Common::String::format("%5u0 ", ourMenUnits), kOurInk, 0xf0);
	_font.drawString(_framebuffer1, kMenLeft, kMenLabelRow, command(kMen), kInk, 0xf0);

	// The two crests are named beside themselves.  The list of writing at
	// DS:0x2494 ends with COMMAND1's ATREIDES at 35,125 in the Atreides ink
	// and its HARKONNENS at 35,139 in the Harkonnen one; leaving them out left
	// the two badges standing on their own with nothing to say whose they are.
	_font.drawString(_framebuffer1, kCrestNameLeft, kOurNameRow,
					 command(kOurName), kOurInk, 0xf0);
	_font.drawString(_framebuffer1, kCrestNameLeft, kHarkNameRow,
					 command(kHarkName), kHarkInk, 0xf0);

	debugC(2, kDebugGraphics,
		   "day %u, charisma %u, territory %u%%/%u%%, spice %u/%u, men %u/%u",
		   day, charisma, harkShare, ourShare, harkSpiceUnits * 10,
		   ourSpiceUnits * 10, harkMenUnits * 10, ourMenUnits * 10);
}

/**
 * The globe screen with the frame open.  The list stays the globe's own -
 * 0xB941 only writes its second row - and what the two sides have between
 * them is drawn over the picture by drawResults.
 */
void DuneEngine::showResults() {
	showGlobe();
}

/**
 * What there is to do with the world up.  The four are COMMAND1.HSQ's own:
 * leaving the maps, giving a troop its orders, the spice density - which the
 * game greys out until the player has been shown it - and taking an
 * ornithopter to fly somewhere.
 */
void DuneEngine::showMapMenu(bool present, bool restoreParent) {
	closeMapPlaceCard();
	_mapWindowGeometryValid = false;
	if (_menuKind == kMenuDensity || _menuKind == kMenuMoveTroop)
		closeSurveyWindow(); // 58FA/5933 dismiss the inset survey.
	if (READ_LE_UINT16(_state.begin() + 0x46ef)) {
		finishTroopContact(); // 8770/D338 unwinds the contact before the map menu.
		_state[0x46f3] = 0;
		_troopOrdersFromMap = true;
	}
	_state[0x46f4] = 0;
	_pressedPlace = -1;
	_pointedAtPlace = -1;
	_state[0x1954] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x46ef, 0);
	playTheRightSong();
	enum {
		kExitMaps = 98,
		kContactTroops = 97,
		kGiveOrders = 146,
		kSeeSpiceDensity = 99,
		kTakeOrnithopter = 166
	};

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();

	// 878C/7F27 refreshes equipment only. The troop counts/strengths may
	// still describe 82DA's destination, not Paul's current place; replacing
	// that context here changes the state retained across an accepted move.
	if (!restoreParent && _state[8] != 0xff && (_state[8] < 0x20 || _state[0xb] < 3))
		countThingsHere(false);

	choices.push_back(command(kExitMaps));
	_menuParts.push_back(kMapExit);
	flags.push_back(0);

	// Before 6715 has registered this viewport's actors, the recruited count
	// provides the provisional remote-contact state. showWorld replaces it
	// with the actual pool count before painting the menu, as DOS 878C does.
	const byte rallied = _state.size() > 0x28 ? _state[0x28] : 0;

	// Once Paul can speak to troops at a distance, the original row changes
	// from GIVE ORDERS TO TROOP to CONTACT FREMEN TROOPS.  Both labels dispatch
	// to 0x86CC, which selects an actor on the map rather than opening a list.
	// 878C enables the pre-telepathy row only if the current place's troop
	// chain has a known head; a recruited troop elsewhere cannot be reached.
	const bool distantContact = READ_LE_UINT16(_state.begin() + 0x1176) >= 2;
	bool reachableTroop = rallied != 0;
	if (!distantContact) {
		reachableTroop = false;
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here >= 0x100 && (uint)here + 9 < _state.size()) {
			const byte id = _state[here + 9];
			if (id != 0) {
				const uint32 record = 0x8aa + 27 * (uint32)(id - 1);
				reachableTroop = record + 3 < _state.size() &&
					_state[record + 3] < 0x80;
			}
		}
	}
	choices.push_back(command(distantContact ? kContactTroops : kGiveOrders));
	_menuParts.push_back(kMapOrders);
	flags.push_back(reachableTroop ? 0 : 0x4000);

	// 87C6/87CF enables the density row at stage five, before the chief's
	// first-assignment briefing need have run. The native briefing cache is
	// independent: using it here also hid FIND PROSPECTORS on that route.
	const bool surveyAvailable = storyStage() >= 5;
	choices.push_back(command(kSeeSpiceDensity));
	_menuParts.push_back(kMapDensity);
	flags.push_back(surveyAvailable ? 0 : 0x4000);

	// 878C starts this row disabled in a desert or an inner palace/fort room.
	// Otherwise 7F27's local machine count decides whether it is selectable.
	// 5FC3 applies the same room gate when opening a place's travel submenu.
	// 8792/87C3 always writes the fourth row, even when Paul is walking
	// in the desert. Removing it incorrectly moves FIND PROSPECTORS up.
	choices.push_back(command(kTakeOrnithopter));
	_menuParts.push_back(kMapOrnithopter);
	const bool canTakeOff = _state[8] != 0xff &&
		(_state[8] < 0x20 || _state[0xb] < 3) &&
		equipmentSpare(READ_LE_UINT16(_state.begin() + 0x114e), 1) != 0;
	flags.push_back(canTakeOff ? 0 : 0x4000);

	// 87DA adds FIND PROSPECTORS at the same stage-five boundary. It is the
	// quick route back to the unique specialist troop for further surveys.
	if (surveyAvailable) {
		choices.push_back(command(102));
		_menuParts.push_back(kMapProspectors);
		flags.push_back(0);
	}

	_menuKind = kMenuMap;
	showMenu(choices, &flags, present);
}

/** DOS 0x86CC selects a map actor, using 0x1955 as a preferred id first. */
void DuneEngine::contactMapTroop() {
	dismissMapTitle(); // 86CC/5BEB restores the legend before selection gates.
	if (_state[0x28] == 0)
		return;
	if (READ_LE_UINT16(_state.begin() + 0x1176) < 2) {
		// 0x86B9 selects the first troop linked at Paul's place, not the first
		// member of a list of all recruited units.
		const uint16 place = READ_LE_UINT16(_state.begin() + 0x114e);
		if (place < 0x100 || place + 9 >= _state.size())
			return;
		const byte id = _state[place + 9];
		_state[0x1954] = id;
		lookAtPlayer();
		rebuildStrategicMap(); // 86C6 calls 5A9A, even at an unchanged centre.
		if (id != 0)
			showTroopContact((uint)id - 1, true);
		return;
	}

	// 0x86DD first tries the high-byte preference from the last accepted
	// contact/card or FIND PROSPECTORS. 0x6917 requires an actor on the map;
	// otherwise 0x86FA cycles from zero through the visible known actors.
	if (_state[0x1954] == 0 && _state[0x1955] != 0) {
		const byte id = _state[0x1955];
		_state[0x1954] = id;
		for (uint i = 0; i < _mapUnitOrder.size(); i++) {
			if (_mapUnitOrder[i] == (uint)id - 1 &&
					(uint)id - 1 < _map.troops().size()) {
				// 0x6917 checks the actor's presence, not its recruitment
				// state. 0x8685 applies the known-state gate afterwards.
				showTroopContact((uint)id - 1, true);
				return;
			}
		}
		_state[0x1954] = 0;
	}
	const int next = nextMapTroop();
	if (next >= 0 && canSelectMapTroop((uint)next))
		showTroopContact((uint)next, true);
}

/** Keep the ordinary map controls while MAP2's density survey is over it. */
void DuneEngine::showDensityMap() {
	_state[0x4722] = 0;
	// 53F1 toggles an existing MAP2 off through 58FA. Closing must not
	// rerun 541F/5584 or consume a capture queued while the survey was open.
	if (_state[0x46eb] & 0x40) {
		closeSurveyWindow();
		_menuKind = kMenuMap;
		return;
	}
	// 5416/5BEB, 5F79 and 79DE remove the old legend/place/compact report
	// before 541F opens MAP2 over the retained map and command panel.
	dismissMapTitle();
	if (_pressedPlace >= 0 || mapTroopCard() >= 0)
		dismissMapInformation();
	_surveyLongitude = _worldLongitude;
	_surveyLatitude = clampSurveyLatitude(_worldLatitude, kRouteMapHigh);
	_speech.clear();
	_menuKind = kMenuDensity;
	refreshSurveyWindow();
}

/** 541F/5584: release a captured region only after its last enemy fort falls. */
void DuneEngine::updateSurveyTerritory() {
	const uint16 pending = READ_LE_UINT16(_state.begin() + 0x115a);
	WRITE_LE_UINT16(_state.begin() + 0x115a, 0);
	if (!pending)
		return;
	bool released[256] = {};
	for (uint region = 1; region <= 12; region++) {
		if (!(pending & (1 << (region - 1))))
			continue;
		bool enemyFort = false;
		for (uint place = 0x100; place + 28 <= _state.size() && _state[place] != 0xff; place += 28) {
			// 5D36 accepts villages/sietches and captured forts, including
			// undiscovered ones. Neither discovery nor battle state is a gate.
			if (_state[place] == region && _state[place + 8] >= 0x28 && !(_state[place + 10] & 8)) {
				enemyFort = true;
				break;
			}
		}
		if (enemyFort)
			continue;
		for (uint place = 0x100; place + 28 <= _state.size() && _state[place] != 0xff; place += 28) {
			if (_state[place] == region && _state[place + 0x10])
				released[_state[place + 0x10]] = true;
		}
		if (region == 7)
			released[0x42] = true; // 55D5's deposit without a required place record.
	}
	_map.liberateSpiceDeposits(_spiceMap, released);
}

/** The route editor used by MOVE TROOP, over the density survey. */
void DuneEngine::showMoveTroopMenu(bool resetView) {
	syncPendingTroopRoute();
	if (resetView) {
		// 80DF presents the instruction under the old command list before
		// 541F opens MAP2. Only then do 807F/80A9 arm the new menu's roll.
		_menuKind = kMenuMoveTroop;
		_state[0x46eb] &= ~0x40;
		const uint instruction = _orderingTroop == 2 ? 85 : 84;
		_speechParts.clear();
		_speechPart = 0;
		_speech = command(instruction - 1);
		updateSpeechWordCount(); // 80DF/8E16 lays out the instruction, not the old report.
		// 88AF retains the command request and clears the old contextual
		// voice variant even when 4774 suppresses 8117's recording.
		WRITE_LE_UINT16(_state.begin() + 0x4780, instruction);
		_state[0x47e0] = 0;
		// 8110 adds 10Ah and D810 (the shared Fremen voice base) to
		// the one-based command before 9EFD. These are PO15E/15F,
		// not ordinary report lines. Scripted 4774 waits suppress speech.
		if (!_state[0x4774])
			playSpeech(_dialogue.speakerBase(14) + 0x10a + instruction, 15, false, false);
		openTroopSurvey();
	}

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();
	if (_orderingTroop == 2) {
		// DS:2136: the specialist can retain up to three stops before Done.
		const uint commands[] = { kAddRouteDestination, 91, 160, 162 };
		for (uint i = 0; i < ARRAYSIZE(commands); i++) {
			choices.push_back(command(commands[i]));
			_menuParts.push_back(commands[i]);
			flags.push_back(i == 0 && (_moveDestinations.empty() || _moveDestinations.size() >= 3)
				? 0x4000 : 0);
		}
	} else {
		choices.push_back(command(162));
		_menuParts.push_back(162);
		flags.push_back(0);
	}
	_menuKind = kMenuMoveTroop;
	_state[0x46eb] = 0xc0; // 541F/5564: the route editor is a map survey.
	// 807F/80A9 enter through D323/D32F; 828C refreshes via D33A only.
	showMenu(choices, &flags, false, resetView);
	if (!resetView) {
		refreshSurveyWindow(); // 8286/542F presents the updated pending stops first.
	}
	// The instruction/inset were already painted. D410 changes the command
	// rows without rebuilding their underlying chief, terrain or actor pool.
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** 82B7 restores the report before either cancelling or confirming a route. */
void DuneEngine::restoreTroopRouteReport() {
	if (_state[0x46eb] & 0x40) {
		closeSurveyWindow(); // 58FA releases MAP2, retaining the large map.
		const MenuKind previousMenu = _menuKind;
		_menuKind = kMenuTroopContact;
		_troopInfoPage = 0;
		_speechParts.clear();
		_speechPart = 0;
		_speech.clear();
		WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
		sayTroopInformation(false, false); // 7BE0 restarts, not a continuation.
		// Keep the old child list while the full report replaces its route
		// instruction. Do not flush the armed roll until the parent is built.
		_graphics.setFrameBuffer(_framebuffer1);
		drawTroopStatus(_orderingTroop); // C551/7BE0 repaint the retained chief.
		// 7C02 and 82B7 both present this report before D2FD pops the list.
		for (uint page = 0; page < 2; page++) {
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
		_menuKind = previousMenu;
	}
}

/** D2E2/824D: restore the report before popping the route editor. */
void DuneEngine::cancelTroopRouteEditor() {
	armPanelMenuTransition(); // D316 precedes the child's close callback.
	// 824D closes the editor but does not run 80C8: retain DOS's pending
	// queue, independently of the native editor cache and committed 11D3.
	if (_state[0x46eb] & 0x40)
		syncPendingTroopRoute();
	_moveDestinations.clear();
	_pointedAtPlace = -1;
	restoreTroopRouteReport();
	const uint record = 0x8aa + 27 * _orderingTroop;
	if (_state[record + 3] & 0x40)
		showTroopMenu(false, false);
	else
		showSettledTroopMenu(false, false); // D2FD restores retained 210A rows.
	drawMenu();
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** The same inset bounds serve rendering, destination picking and panning. */
Common::Rect DuneEngine::moveMapRect() const {
	const int top = _troopContactTop >= 50 ? 21 : 37;
	return Common::Rect(kRouteMapLeft, top, kRouteMapLeft + kRouteMapWide, top + kRouteMapHigh);
}

bool DuneEngine::hasSurveyMap() const {
	return (_state[0x46eb] & 0x40) && (_menuKind == kMenuMoveTroop ||
		((_menuKind == kMenuProspectorBriefing || isStageEventMenu()) && _troopInfoPage == 1) || _menuKind == kMenuDensity);
}

Common::Rect DuneEngine::surveyMapRect() const {
	if (_menuKind == kMenuMoveTroop)
		return moveMapRect();
	Common::Point origin(READ_LE_UINT16(_state.begin() + 0x11c1), READ_LE_UINT16(_state.begin() + 0x11c3));
	// 13A0/1399 retains the chief's 79EE frame, independently of the
	// standalone density position which 5406 reads from 11C1/11C3.
	if ((_menuKind == kMenuProspectorBriefing || isStageEventMenu()) && _troopInfoPage == 1)
		origin = Common::Point(READ_LE_UINT16(_state.begin() + 0x4710), READ_LE_UINT16(_state.begin() + 0x4712));
	// 79EE's upper-chief handover starts at (92,30). 541F does not apply
	// 59D1's drag clamp; the first motion brings it within top <=29.
	if (origin != Common::Point(92, 30))
		origin = clampSurveyWindowPosition(origin);
	return Common::Rect(origin.x + 5, origin.y + 7, origin.x + 165, origin.y + 96);
}

Common::Rect DuneEngine::surveyFrameRect() const {
	const Common::Rect map = surveyMapRect();
	return Common::Rect(map.left - 5, map.top - 7, map.right + 5, map.bottom + 12);
}

bool DuneEngine::canDragSurvey() const {
	// 5978/82A0: both MOVE TROOP menus lock their route card in place.
	// D89B bypasses this callback entirely while event bytecode is active.
	return _onWorld && !_onGlobe && !_inCabin && !_onMixer &&
		!_state[0x4774] && hasSurveyMap() && _menuKind != kMenuMoveTroop;
}

bool DuneEngine::beginSurveyDrag(const Common::Point &at) {
	if (!canDragSurvey())
		return false;
	const Common::Rect frame = surveyFrameRect();
	if (!Common::Rect(frame.left + 10, frame.top + 1, frame.right, frame.top + 7).contains(at))
		return false;
	_surveyDragging = true;
	_surveyDragPosition = Common::Point(frame.left, frame.top);
	_surveyDragMouse = at;
	toggleSurveyDragOutline(); // 5978/5982: XOR the retained page, not a map redraw.
	return true;
}

/** 5982: toggle only the moving outline, retaining the map and its actors. */
void DuneEngine::toggleSurveyDragOutline() {
	drawSurveyDragOutline(_framebuffer1, kScreenWidth, _surveyDragPosition);
	_system->copyRectToScreen(_framebuffer1 + _surveyDragPosition.y * kScreenWidth + _surveyDragPosition.x,
		kScreenWidth, _surveyDragPosition.x, _surveyDragPosition.y, 170, 108);
	_system->updateScreen();
}

void DuneEngine::finishSurveyDrag(bool commit) {
	if (!_surveyDragging)
		return;
	toggleSurveyDragOutline();
	_surveyDragging = false;
	const Common::Point previous(READ_LE_UINT16(_state.begin() + 0x11c1),
		READ_LE_UINT16(_state.begin() + 0x11c3));
	if (commit && canDragSurvey() && _surveyDragPosition != previous) {
		WRITE_LE_UINT16(_state.begin() + 0x11c1, _surveyDragPosition.x);
		WRITE_LE_UINT16(_state.begin() + 0x11c3, _surveyDragPosition.y);
		refreshStrategicMap(); // 59B8's 5B10, also used by FIND PROSPECTORS.
	}
}

/** 5B10/8850: rebuild the strategic map and reopen an existing MAP2. */
void DuneEngine::refreshStrategicMap() {
	// A refused FIND request can leave the opening legend displayed. 5B10
	// does not call 5BEB; a later 86CC or 5416 owns that release.
	_state[0x46d8] = 1;
	_state[0xdce6] = 0x80; // 7B36 suppresses menu-return effects during cleanup.
	if (READ_LE_UINT16(_state.begin() + 0x46ef)) {
		finishTroopContact(false, false); // 7B36 unwinds without a closing outline.
		_state[0x46f3] = 0;
		_troopOrdersFromMap = true;
	}
	// 5F79/79DE release the old windows through C6AD while their actors
	// still exist. Only the subsequent 5A9A replaces the retained pool.
	if (_pressedPlace >= 0 || READ_LE_UINT16(_state.begin() + 0x46f8)) {
		const byte *box = _state.begin() + 0x1668;
		const Common::Rect bounds(READ_LE_INT16(box), READ_LE_INT16(box + 2),
				READ_LE_INT16(box + 4), READ_LE_INT16(box + 6));
		closeMapPlaceCard();
		restoreMapInformationBackground(bounds, true);
	}
	if (mapTroopCard() >= 0) {
		const Common::Rect bounds = mapTroopCardBounds();
		closeMapTroopCard();
		restoreMapInformationBackground(bounds, true);
	}
	_state[0xdce6] = 0;
	_state[0x46f4] = 0;
	rebuildStrategicMap();
}

/** 5A9A: recreate map actors/menu and reopen MAP2 without 7B36's cleanup. */
void DuneEngine::rebuildStrategicMap() {
	const bool survey = _state[0x46eb] & 0x40;
	const byte selected = _state[0x1954]; // 86B9 selects its local head before 5A9A.
	// Avoid showMapMenu's density-close branch: 5AC8 reopens MAP2 rather
	// than discarding it, and 5B10 restores its cursor frame afterwards.
	_menuKind = kMenuMap;
	showMapMenu(false);
	_state[0x1954] = selected;
	if (survey)
		_menuKind = kMenuDensity;
	WRITE_LE_UINT16(_state.begin() + 0x47ba, 0); // 878C, even without a contact.
	rebuildMapUnits(); // 5A9A recreates actors even at an unchanged centre.
	if (READ_LE_UINT16(_state.begin() + 0x1176) >= 2 && _menuFlags.size() > 1)
		_menuFlags[1] = _mapUnitOrder.empty() ? 0x4000 : 0; // 878C uses the new pool.
	if (survey) {
		_surveyLongitude = _worldLongitude;
		_surveyLatitude = clampSurveyLatitude(_worldLatitude, kRouteMapHigh);
		updateSurveyTerritory();
		resetSurveyHover();
		WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
	}
	redraw();
}

bool DuneEngine::handleSurveyDrag(const Common::Event &event) {
	if (_surveyDragging && !canDragSurvey())
		_surveyDragging = false;
	if (!_surveyDragging)
		return event.type == Common::EVENT_LBUTTONDOWN && beginSurveyDrag(event.mouse);
	if (event.type == Common::EVENT_LBUTTONUP) {
		finishSurveyDrag(true);
	} else if (event.type == Common::EVENT_MOUSEMOVE) {
		// 59C8 releases at the panel without applying this event's delta.
		if (event.mouse.y >= 152) {
			finishSurveyDrag(true);
		} else {
			// Incremental deltas avoid a dead zone after overshooting a clamp.
			toggleSurveyDragOutline(); // 59CE removes the old XOR before applying the delta.
			_surveyDragPosition = clampSurveyWindowPosition(Common::Point(
				_surveyDragPosition.x + event.mouse.x - _surveyDragMouse.x,
				_surveyDragPosition.y + event.mouse.y - _surveyDragMouse.y));
			_surveyDragMouse = event.mouse;
			toggleSurveyDragOutline();
		}
	} else if (event.type == Common::EVENT_FOCUS_LOST ||
			(event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
		finishSurveyDrag(false);
		return false; // Still handle focus changes or open ScummVM's menu.
	}
	return true;
}

/** 5944/5955: the close control occupies the left of the inset's top bar. */
bool DuneEngine::moveMapCloseAt(const Common::Point &at) const {
	const Common::Rect map = surveyMapRect();
	return Common::Rect(map.left - 4, map.top - 6, map.left + 5, map.top).contains(at);
}

/** 596D: clicking the bottom strip alternates density and troop distribution. */
bool DuneEngine::surveyLegendAt(const Common::Point &at) const {
	const Common::Rect map = surveyMapRect();
	return Common::Rect(map.left - 4, map.bottom, map.right + 5, map.bottom + 12).contains(at);
}

/** Which edge of the currently displayed route inset is being hovered. */
int DuneEngine::moveMapScrollDirection(const Common::Point &at) const {
	if (_surveyDragging || !_onWorld || _onGlobe || _inCabin || _onMixer || !hasSurveyMap())
		return -1;
	return MAX(-1, (int)surveyMapControlAt(surveyFrameRect(), at));
}

/**
 * Which known place is under a point of the route editor's small density
 * map.  This is a different projection and scale from the large map behind
 * it; using placeAt() here made the hand and tooltip name an unrelated place
 * even though the subsequent click was decoded against the card.
 *
 * Ordinary troops can select any known site. The destination-specific
 * dialogue, not the hotspot filter, rejects palaces and villages. Only the
 * prospectors' multi-stop editor filters unsuitable kinds here (8256).
 */
int DuneEngine::moveDestinationAt(const Common::Point &at) const {
	if (_orderingTroop >= _map.troops().size())
		return -1;
	const int place = surveyPlaceAt(at);
	if (place >= 0 && _orderingTroop == 2 && _map.places()[place].kind >= 0x20 &&
			!(_map.places()[place].flags & 8))
		return -1;
	return place;
}

int DuneEngine::surveyPlaceAt(const Common::Point &at) const {
	// 5944 handles the header and legend before 5C03/5E6D. Within the
	// remaining rows, clicks use D6FE's outer frame just like hover: its
	// side padding can still be within eight pixels of a visible sietch.
	const Common::Rect map = surveyMapRect();
	if (at.y < map.top || at.y >= map.bottom)
		return -1;
	return surveyPlaceAt(at, true);
}

int DuneEngine::surveyPlaceAt(const Common::Point &at, bool hover) const {
	const Common::Rect map = surveyMapRect();
	const Common::Rect frame = surveyFrameRect();
	const bool inside = hover ?
		at.x > frame.left && at.x < frame.right && at.y > frame.top && at.y < frame.bottom :
		map.contains(at);
	if (_surveyDragging || !inside)
		return -1;

	return nearestMapPlace(at, 0x40, 0xff, 9); // 5C03/81F3 temporarily select the inset's tag.
}

/** Give the troop the known sietch pressed inside the survey card. */
void DuneEngine::addMoveDestination(const Common::Point &at) {
	const int place = moveDestinationAt(at);
	if (place < 0)
		return;

	if (_orderingTroop != 2 || _moveDestinations.size() >= 3)
		_moveDestinations.clear();
	_moveDestinations.push_back((uint)place);
	_pointedAtPlace = place;
	debugC(1, kDebugScript, "troop %u destination is place %d", _orderingTroop,
		   place);
	if (_orderingTroop == 2) {
		if (_moveDestinations.size() < 3) {
			showMoveTroopMenu(false);
			return;
		}
		if (!waitForRoutePreview())
			return;
	}
	commitTroopRoute();
}

/** 8286..829B: paint the third stop, then wait 50 ticks before accepting. */
bool DuneEngine::waitForRoutePreview() {
	_routePreviewActive = true;
	showMoveTroopMenu(false);
	const uint32 started = _system->getMillis();
	const bool noDelay = ConfMan.getBool("dune_nodelay");
	do {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			updatePointerClickKeys(event);
			if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER)
				quitGame();
		}
		// E3A0 services the scheduler, not the room/map command handlers or
		// D831's hourly dispatch. A queued press cannot edit a fourth stop or
		// activate the menu revealed when the route is accepted.
		// EF7A's timer interrupt continues during this wait. Account it before
		// returning to 8214/84A6, so confirmation queries see the current hour
		// without waiting for the next ordinary poll to advance the clock.
		if (noDelay && !shouldQuit())
			advanceClockElapsed(250, false, false);
		else
			tick(false);
		updateGameMusic();
		{
			const AnimationClock::Poll poll(_animationClock, _system->getMillis());
			updateMapAnimation();
			updateMapSpeech(); // D9D2 retains A7C2 while the instruction is playing.
			updateSkyFade();
		}
		if (shouldQuit() || noDelay)
			break;
		const uint32 now = _system->getMillis();
		if (now - started >= 250)
			break;
		_system->delayMillis(MIN<uint32>(5, 250 - (now - started)));
	} while (!shouldQuit());
	_routePreviewActive = false;
	return !shouldQuit();
}

/** Mirror the specialist editor's cache into DOS 4718/4738 pending state. */
void DuneEngine::syncPendingTroopRoute() {
	// Ordinary moves never write the specialist's 4718 queue or 4738 count.
	if (_orderingTroop != 2 || _state.size() <= 0x4738)
		return;
	const uint count = MIN<uint>(_moveDestinations.size(), 3);
	for (uint i = 0; i < 3; i++)
		WRITE_LE_UINT16(_state.begin() + 0x4718 + 2 * i,
				i < count && _moveDestinations[i] < _map.places().size()
				? 0x100 + 28 * _moveDestinations[i] : 0);
	_state[0x4738] = (byte)count;
}

/** Store and begin the route assembled by the map editor. */
void DuneEngine::commitTroopRoute() {
	if (_orderingTroop >= 67 || _troopRoutes.size() < 67 * 3
		|| (_moveDestinations.empty() && _orderingTroop != 2))
		return;
	const bool travellingParent = _state[0x8aa + 27 * _orderingTroop + 3] & 0x40;
	// 8214 restores and restarts the chief's report before 8220 copies the
	// prospectors' pending queue or 82DA assesses the proposed destination.
	syncPendingTroopRoute();
	restoreTroopRouteReport();
	if (_orderingTroop == 2) {
		for (uint i = 0; i < 3; i++)
			WRITE_LE_UINT16(_state.begin() + 0x11d3 + 2 * i,
				i < _moveDestinations.size() ? 0x100 + _moveDestinations[i] * 28 : 0);
		syncProspectorRoute();
	}
	const uint16 destination = _moveDestinations.empty() ? 0 : 0x100 + _moveDestinations[0] * 28;
	_moveDestinations.clear();
	_pointedAtPlace = -1;
	if (_orderingTroop == 2 && !READ_LE_UINT16(_state.begin() + 0x11d3)) {
		cancelTroopRouteEditor(); // MAP2 is already closed: do not replay the report.
		return;
	}
	// 8220 commits the specialist queue BEFORE asking: a busy surveyor
	// retains it but waits for completion. Other refused moves retain their
	// old destination and native route, including an ongoing collection trip.
	if (!requestTroopMove(destination, false)) {
		cancelTroopRouteEditor(); // Preserve the refusal under the returning parent list.
		return;
	}
	_menuKind = kMenuTroopContact;
	// 823A starts the route immediately after 82DA's acknowledgement page.
	// 878C/D338 then closes contact and stops the voice through 7B58/A7A5.
	// Unlike an occupation change, this accepted editor has no menu roll.
	if (_orderingTroop != 2) {
		_troopRoutes[_orderingTroop * 3] = (destination - 0x100) / 28 + 1;
		_troopRoutes[_orderingTroop * 3 + 1] = _troopRoutes[_orderingTroop * 3 + 2] = 0;
	}
	beginTroopRoute(_orderingTroop);
	_map.readTroops(_state.begin(), _state.size());
	// Choosing the destination completes this order and exposes the unobscured
	// large terrain map.  The small MAP2 route survey is gone in the recording,
	// while the ordinary EXIT MAPS rows and the travelling troop remain.
	// D2FD restores the registered parent before 878C unwinds the chief.
	// A newly departing troop must not substitute a travelling parent here.
	if (travellingParent)
		showTroopMenu(false, false);
	else
		showSettledTroopMenu(false, false);
	drawMenu();
	finishTroopContact(true);
	// 8240 closes the accepted route's contact through 8250, then jumps
	// directly to 878C. It bypasses 877A's explicit map-contact teardown,
	// so a route ordered from a room retains its 46F3 origin when the map
	// menu returns. Clearing it here loses the room-origin viewport context.
	showMapMenu(false); // 878C changes only the rows; it does not call 5D82.
	drawMenu();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * What pressing a place on the maps offers, which the pair of entries at
 * DS:0x20DC holds: going there flying an ornithopter, and Cancel under it.
 * DS:0x20E8 is the same pair for a worm.  The place is named over the map
 * while the two rows stand.
 *
 * Nothing at all used to happen where the maps were pressed unless a flight
 * was already waiting for somewhere to be picked, which left the maps with
 * nothing to do but be looked at.
 */
void DuneEngine::showPlacePressed(uint index) {
	enum { kGoThereFlying = 88, kGoThereRiding = 89 };

	if (index >= _map.places().size())
		return;

	const bool selectedTroop = _state[0x1954] != 0;
	const bool restoreParent = selectedTroop || _menuKind == kMenuPlace;
	if (hasTroopContact())
		finishTroopContact(); // 5FB3/7B36 release the previously selected troop.
	// 7B36/8770 clears 46F3 only when it releases a selected troop.
	// An accepted room-started route has already closed that contact and
	// retained its origin; merely opening a place report must keep it.
	if (selectedTroop)
		_state[0x46f3] = 0;
	const Common::Rect troopCard = mapTroopCard() >= 0 ? mapTroopCardBounds() : Common::Rect();
	const byte *placeBox = _state.begin() + 0x1668;
	const Common::Rect placeCard = _pressedPlace >= 0 ?
			Common::Rect(READ_LE_INT16(placeBox), READ_LE_INT16(placeBox + 2),
					READ_LE_INT16(placeBox + 4), READ_LE_INT16(placeBox + 6)) : Common::Rect();
	closeMapTroopCard();
	closeMapPlaceCard();
	paintMapActorChange(troopCard); // 7B36 restores only released windows before 600E.
	paintMapActorChange(placeCard);
	_mapWindowGeometryValid = false;
	_troopOrdersFromMap = !_state[0x46f3];
	_pointedAtPlace = -1;
	if (restoreParent) {
		// 7B36 restores the root list through 8770 or 5F79 before
		// 600E opens the new site card and D323 rolls its travel rows in.
		showMapMenu(false);
		drawMenu();
	}
	// 5FB6..6000 skips only the travel submenu at Paul's own place, in
	// inner village/fort/palace rooms, or in desert before worm training.
	// Desert travel uses DS:20E6's worm rows, not the aircraft at the last site.
	const bool desert = _state[8] == 0xff;
	if (READ_LE_UINT16(_state.begin() + 0x114e) == 0x100 + 28 * index ||
			(!desert && _state[0xb] > 2 && _state[8] >= 0x20) ||
			(desert && !(_state[0xa] & 0x40))) {
		_speech.clear();
		if (!restoreParent)
			showMapMenu(false);
		openMapPlaceCard(index);
		return;
	}

	_pressedPlace = -1;

	_speech = "";
	_speaker = "";

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	Common::Array<uint> parts;

	// 5FDD refreshes 7F27's spare inventory before testing 46FF. A cached
	// aircraft count can be stale after changing equipment or troop work.
	if (!desert) {
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		for (uint item = 0; item < 7; item++)
			_state[0x46fe + item] = equipmentSpare(here, item);
	}
	choices.push_back(command(desert ? kGoThereRiding : kGoThereFlying));
	parts.push_back(desert ? kPlaceWorm : kPlaceFly);
	flags.push_back(desert || ornithoptersHere() != 0 ? 0 : 0x4000);

	choices.push_back(command(kCancel));
	parts.push_back(kPlaceCancel);
	flags.push_back(0);

	// 6004 opens and presents the card with the parent list still installed.
	// Only then does D323 arm, install and roll in the travel submenu.
	openMapPlaceCard(index);
	armPanelMenuTransition();
	_menuKind = kMenuPlace;
	_menuParts = parts;
	showMenu(choices, &flags);
}

/**
 * What there is to do out in the open, which the panel shows while the player
 * is between one place and the next.  0x2FAA builds two quite different lists
 * out of COMMAND1.HSQ's own words, and which of them by the two bits of
 * DS:0x11C9 that say whether the player is being carried.
 *
 * On foot: the maps, calling a worm - which 0x2FBA greys out until the story
 * has reached seventy-nine - and waiting, for the evening while it is still
 * morning and for the morning after that, with the mixer under them.
 *
 * Being carried: skipping to where the flight is bound, or, once a flight has
 * been broken off and DS:0x11CB is no longer nought, going back to where it
 * started and - past the fiftieth of the story - towards the nearest place.
 * Changing where it is bound is the last row either way.
 *
 * Only the second list was ever built, and only its broken-off form, so a
 * flight could not be skipped and there was nothing at all to do on foot.
 */
void DuneEngine::showDesertMenu(bool present, bool restorePage) {
	if (!restorePage)
		_state[0x1f0f] = 0; // 2EFB builds the ordinary parent on its first page.
	resetTroopNoticeIdle();
	// 2EFB rebuilds commands without changing the retained view-mode byte.
	// As with showRoomMenu, map entry/exit owns its sign transition.
	playTheRightSong();
	// 2ED3/3090 rebuilds presence only when no vehicle carries Paul. Its
	// 3127 pass removes temporary village/troop residents before 3093 scans
	// the new coordinates. Otherwise walking away leaves the merchant here.
	Common::Array<byte> here;
	if (_state[0x11c9] == 0)
		here = peopleHere();
	enum {
		kSeeDuneMap = 151, kMixerPanel = 157, kWaitForEvening = 164,
		kWaitForMorning = 165, kCallAWorm = 167, kSkipToDestination = 168,
		kTowardsNearest = 169, kBackToStart = 171, kChangeDestination = 87,
		kWormIsCalled = 0x4f, kNearestIsKnown = 0x32, kEvening = 11
	};

	Common::Array<Common::String> choices;
	Common::Array<uint16> flags;
	_menuParts.clear();

	const byte story = _state.size() > 0x2a ? _state[0x2a] : 0;
	// 2FAA tests the actual low transport bits. Named worm departure still
	// has raw8 while186B restores this scene;4709 folds it to10 afterward.
	// Predicting that fold here shows the carried commands too early.
	const bool carried = _state.size() > 0x11c9 && (_state[0x11c9] & 3) != 0;

	debugC(2, kDebugScript, "the desert list: carried %02x, flying to %d, to a point %d",
		   _state.size() > 0x11c9 ? _state[0x11c9] : 0, _flyingTo, _flightToPoint ? 1 : 0);

	if (!carried) {
		choices.push_back(command(kSeeDuneMap));
		_menuParts.push_back(kDesertMap);
		flags.push_back(0);

		choices.push_back(command(kCallAWorm));
		_menuParts.push_back(kDesertWorm);
		flags.push_back(story >= kWormIsCalled ? 0 : 0x4000);

		// 0x1AE0 takes the hour out of the low four bits of the clock, so a
		// day is sixteen hours long; 0x2FCC offers the evening until the
		// eleventh of them and the morning after that.
		const bool untilEvening = (hour() & 0xf) < kEvening;

		choices.push_back(command(untilEvening ? kWaitForEvening : kWaitForMorning));
		_menuParts.push_back(untilEvening ? kDesertEvening : kDesertMorning);
		flags.push_back(0);

		choices.push_back(command(kMixerPanel));
		_menuParts.push_back(kDesertMixer);
		flags.push_back(0);

		// Followers remain selectable in the open desert. The original appends
		// them after WAIT and the mixer just as a room appends its occupants;
		// omitting them made Gurney's harvester conversation unreachable on the
		// exact clue route, even though his portrait still stood in the panel.
		for (uint i = 0; i < here.size(); i++) {
			const byte person = here[i];
			choices.push_back(command(kFirstPerson + person));
			_menuParts.push_back(person);
			flags.push_back(0);
		}
	} else {
		const byte brokenOff = _state.size() > 0x11cb ? _state[0x11cb] : 0;

		if (brokenOff == 0) {
			choices.push_back(command(kSkipToDestination));
			_menuParts.push_back(kDesertSkip);
			// 2FF0 copies the complete 21FC command word, including 4182's
			// hazard effect. IGNORE WARNING resumes visible travel; it must
			// not re-enable fast-forward while hostile terrain still blocks it.
			flags.push_back((_state[0x21fd] & 0x40) ? 0x4000 : 0);
		} else {
			choices.push_back(command(kBackToStart));
			_menuParts.push_back(kDesertBack);
			flags.push_back(0);

			if (story >= kNearestIsKnown) {
				choices.push_back(command(kTowardsNearest));
				_menuParts.push_back(kDesertNearest);
				flags.push_back(0);
			}
		}

		choices.push_back(command(kChangeDestination));
		_menuParts.push_back(kDesertChange);
		flags.push_back(0);
	}

	_menuKind = kMenuDesert;
	showMenu(choices, &flags, present);
}

/**
 * The compact map put over the desert after CALL A WORM. Like the
 * ornithopter selector, it accepts open sand as well as known places:
 * the training ride in the original is made by choosing the caption
 * "DESERT southwards" immediately below the palace.
 */
void DuneEngine::showWormMap(bool departing) {
	enum { kCancel = 162 };
	if (departing) {
		// 4285 installs the unstarted worm before 4305 rolls Cancel onto
		// the retained scene. 497A instead enters directly at 430B: keep
		// the active vehicle, position and destination during a redirect.
		_state[0x11c9] = 8;
		_state[0x473e] = 0;
		WRITE_LE_UINT16(_state.begin() + 0x487e, 1);
	} else {
		_state[0x4728] = 1; // 497A, so 4457 restores the moving landscape.
	}
	_onWorld = _onGlobe = _inCabin = false;

	Common::Array<Common::String> choices;
	choices.push_back(command(kCancel));

	_menuParts.clear();
	_menuParts.push_back(kPlaceCancel);
	_menuKind = kMenuWormMap;
	showMenu(choices, nullptr, false, true);
	drawMenu();
	finishPanelMenuTransition();
	// 4285/4305 shares the compact destination selector with the orni,
	// but retains the landscape instead of installing cockpit instruments.
	_state[0x11ca] = 1;
	_state[0x46eb] = 1;
	_speech.clear();
	_speakerPerson = -1;
	beginCabinInstruction();
	_wormPointerPoint = -1;
	lookAtPlayer();

	drawWormMap();
}

void DuneEngine::closeWormMap() {
	_wormPointerPoint = -1;
	_state[0x46eb] = 0;
	if (!READ_LE_UINT16(_state.begin() + 0x11c5))
		_state[0x11c9] = 0; // 443B/4442: an unstarted call is not transport.
	_state[0x11ca] = 0;
	if (_inDesert) {
		// 4457/49D4 rebuilds the carried inset after the selector has
		// reused its map scratch page. The next movement still recenters.
		const byte recenter = _state[0x4728];
		if ((int8)recenter > 0 && _flyingTo >= 0) {
			resetFlightInset();
			drawMapInset();
			_state[0x4728] = recenter;
		}
		showDesertMenu();
	} else {
		drawScene();
		showRoomMenu();
	}
}

/** Translate the worm card's map pixels back to TABLAT's world. */
bool DuneEngine::wormMapPointsAt(const Common::Point &at, int *across,
								  int *down) const {
	return cabinWorldPointAt(at, across, down); // B5F9, no equator round trip.
}

/** Draw the selector without replacing the desert that remains behind it. */
void DuneEngine::drawWormMap() {
	// 4285/4305 overlays the retained page, without 37DF/4D06. Rebuilding
	// the desert here restarted its harvester's frame and callback phase on
	// every selector redraw. Fresh loads rebuild the backing before redraw().
	_graphics.setFrameBuffer(_framebuffer1);
	byte backdropPalette[(256 - 80) * 3];
	_system->getPaletteManager()->grabPalette(backdropPalette, 80, 256 - 80);
	// 4311 installs all four world-pan buttons, including for a worm.
	setCompass(kCompassWorld);
	drawPanel();
	drawMenu();

	openSpriteSheet("PERS.HSQ");
	openSpriteSheet("ONMAP.HSQ");
	openSpriteSheet("ORNYPAN.HSQ");
	// 439F/5B69: four nested one-pixel outlines around DS:149C.
	for (int rim = 4; rim > 0; rim--)
		_graphics.fillRect(81 - rim, 45 - rim, 241 + rim, 134 + rim, 254 - 2 * rim);
	_graphics.fillRect(77, 33, 245, 41, 0xf5); // 43BC, DS:14A4.
	drawCabinMap(false);
	drawCabinLabel();
	drawFace(); // 1797 remains above the frame's bottom row at y=137.

	// PERS and ONMAP supply the card's low map/marker colours. They also carry
	// higher partial ranges which must not recolour the retained backdrop.
	// 4305 does not reload the sky: retain its in-progress fade, including
	// the panel ramp. ICONES may still install the face colours at224..239.
	// A local battlefield instead retains its complete ATTACK palette.
	if (_inDesert) {
		_system->getPaletteManager()->setPalette(backdropPalette, 80, 224 - 80);
		_system->getPaletteManager()->setPalette(backdropPalette + (240 - 80) * 3, 240, 16);
	} else {
		_system->getPaletteManager()->setPalette(backdropPalette, 80, 256 - 80);
	}

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * The sandworm rising after its destination has been selected. VER.HNM is a
 * one-shot 320 by 152 scene movie, separate from MNT1's travelling dunes.
 * The original keeps the game panel and its Cancel row in front of it.
 */
void DuneEngine::animateWormCall() {
	Resource movie = _archive.openMember("VER.HNM");
	if (!movie)
		return;

	// 47AD/1797 prepares the lowered face before C108 presents VER.
	// The film covers the top152 rows, including the crest's upper pixels.
	// Redrawing the panel after every decode stamped those pixels over it.
	_graphics.setFrameBuffer(_framebuffer1);
	setCompass(kCompassWorld);
	for (int i = 1; i <= 4; i++)
		_panel[kFirstCompassElement + i].flags |= 0x20;
	drawPanel();
	drawMenu();

	_video->setReader(movie);
	_video->setInterlace(false);
	_video->start();
	memcpy(_video->_frameBuffer, _framebuffer1,
		   kScreenWidth * kSceneHeight);
	// 47BD..47C5 protects the retained panel while VER's own palette is
	// visible. Loading the normal sky or interface ramps over it recolours
	// the movie; any active3916 fade must work on that movie palette instead.
	const byte previousOverlay = _state[0x227d];
	_state[0x227d] = 1;
	MovieInputStream inputStream(_video->_frameOffsets, _video->_headerSize);

	const uint32 frameTime = 1000 / 12;
	uint32 nextFrameTime = _system->getMillis() + frameTime;
	uint frame = 0;
	while (!_video->done() && !shouldQuit()) {
		// Presenting the preceding frame leaves the general blitter
		// aimed at the game page. HNM deltas must instead accumulate on the
		// movie's retained page before that page is copied out.
		_graphics.setFrameBuffer(_video->_frameBuffer);
		_video->decodeAVFrame();
		memcpy(_framebuffer1, _video->_frameBuffer,
			   kScreenWidth * kSceneHeight);
		_graphics.setFrameBuffer(_framebuffer1);

		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(Common::String::format("%s-worm%02u.ppm",
										 ConfMan.get("dune_dump").c_str(), frame));
		frame++;

		// C108 presents frame1 before491C starts its C9E8/DD63 loop.
		// CA60 then advances before each query, including the final query
		// after the stream closes. VER's53 polls belong to frames2..54,
		// not frames1..53; dropping the last poll loses its live callbacks.
		if (frame == 1)
			continue;
		const uint16 previousButtons = inputStream.present(frame);
		const PlaybackResult asked = pollSkip();
		if (asked != kPlaybackFinished)
			break;
		pollWormMovieTimers(previousButtons, _system->getEventManager()->getButtonState() & 7);
		if (_video->done())
			break;
		if (!ConfMan.getBool("dune_nodelay")) {
			const uint32 now = _system->getMillis();
			if (now < nextFrameTime)
				_system->delayMillis(nextFrameTime - now);
		}
		nextFrameTime += frameTime;
	}

	_video->stopAudio();
	_state[0x227d] = previousOverlay;
	_flightMovie = -1;
	_flightMovieFallback = false;
}

/** C9E8/DD63: the hardware clock runs even when DD7E skips E3CC/D9D2. */
bool DuneEngine::pollWormMovieTimers(uint16 previousButtons, uint16 buttons) {
	tick(false);
	if (!movieTimerEligible(previousButtons, buttons))
		return false;
	refreshInputRandom(); // DD9E/DDA3 refreshes before the registered callbacks.
	updateGameMusic();
	updateSkyFade();
	return true;
}

/** Begin the worm journey to the arbitrary point selected on its card. */
void DuneEngine::takeWormTo(int across, int down, int place) {
	if (_state.size() <= 0x487e)
		return;
	// From a battlefield, 407E uses the fort as the course origin.
	// 40D5 and 40DD still need the room coordinates for the boarding walk.
	if (!_inDesert && _state[6] == 0x80) {
		const uint place = READ_LE_UINT16(_state.begin() + 0x114e);
		if (place < 0x100 || (place - 0x100) % 28 || (place - 0x100) / 28 >= _map.places().size())
			return;
		_cameFromPlace = (place - 0x100) / 28;
	}
	if (_cameFromPlace >= _map.places().size())
		return;

	_flightToPoint = place < 0;
	_flightTargetLongitude = (uint16)across;
	_flightTargetLatitude = (int16)down;
	_guidedFlightPlace = -1;
	_flightDiscoveryPrompt = false;
	// 0x4709 folds the worm's raw value eight into low transport value two.
	// Keep both marks because the high bit identifies how it was called and
	// the low bits are what the desert/menu code tests while it carries Paul.
	_state[0x11c9] = 8 | 2;
	_state[0x487e] = 1;
	if (_state.size() > 0x473e)
		_state[0x473e] = 0;
	if (_state.size() > 0x11cb)
		_state[0x11cb] = _flightToPoint ? 0xff : 0;

	// 4944/496A selects the course before 4709 starts departure. Prepare it
	// without enabling the travelling renderer: VER's deltas need the still
	// desert, not the worm's live MNT1 film, underneath them.
	const uint destination = place >= 0 ? place : _cameFromPlace;
	_flightSteered = false;
	WRITE_LE_UINT16(_state.begin() + 0x11c5, 0x100 + 28 * destination);
	prepareFlightCourse(destination);
	if (_state[0x2b]) {
		_state[0x2b] = 0; // 4718/B21 leaves the battle before request seven.
		stopBattleAnimation();
	}
	WRITE_LE_UINT16(_state.begin() + 0x472b, 0); // 4732, before the intro.
	clearFlightHazard();
	// 474B/40D5 runs request seven for a worm as well as an orni.
	// Keep the selected course while any resident's answer is displayed;
	// the common completion also leaves unwilling followers behind.
	_pendingBoardingPlace = destination;
	_pendingBoardingGuided = false;
	_onWorld = _onGlobe = _inCabin = false;
	_state[0x23] = 7;
	releaseSilentLocalSpeaker();
	_state[0x47a7] = 0;
	walkLocalRoomRequests();
	finishVehicleBoarding();
}

void DuneEngine::completeWormDeparture(uint destination) {
	// 4748 repaints the departure desert before 4795 advances the story.
	// This also seeds VER's retained page when the movie is available.
	_inDesert = true;
	drawDesert(false);
	_state[0x11ca] = 0; // 40D5/4AC4 releases the selector pause.
	if (storyStage() == 79)
		advanceStory(80);
	_state[0xe8] = 0; // 47AD lowers the face before the worm rises.

	// The target card gives way to the worm's own approach movie while its
	// visible Cancel row and map button remain in the panel.
	animateWormCall();
	_flyingTo = destination;
	_state[8] = 0xff; // 475B: the room words remain through the first position step.
	advanceCarriedPosition(false); // 4760, before the first live callback.
	WRITE_LE_UINT16(_state.begin() + 0x114e, 0);
	WRITE_LE_UINT16(_state.begin() + 0x4729, 0);
	_state[0x46eb] = 0;
	_state[0x11ca] = 0;
	_flightTrail.clear();
	_state[0x4728] = 0; // 49EA also clears the old inset hide/refresh latch.

	_flightFrame = 0;
	_flightInsetSet = false; // 4988 prepares the worm's first inset after its departure step.
	// 4769 clears the last-step word for worms as well as ornithopters.
	// The first live callback is immediately due unless the timer has just
	// started or wrapped within its first 0x300 ticks.
	const uint32 nowMs = _system->getMillis();
	_nextFlightStep = FlightStepClock::first(nowMs);
	stopFlightMovie();
	_speech.clear();
	_speakerPerson = -1;
	_wormPointerPoint = -1;
	_flightSkyRestartPending = true;

	debugC(1, kDebugScript, "riding a worm from place %u to desert point %d,%d",
		   _cameFromPlace, _flightTargetLongitude, _flightTargetLatitude);
	showDesertMenu();
	_state[0x4727] = 0xff; // 4779/4AB8 enables ordinary movement after the first scene.
	dispatchCarriedRoomRequests(); // 476F/2DBF/35AD scans the first carried scene.
}

/**
 * 407E supplies the player's coordinates (or those of DS:114E when latitude
 * is the 0x80 room sentinel).  5344 scores each known place by the larger of
 * its absolute latitude difference and the high byte of its wrapped longitude
 * difference.  Ties retain the first place in the table.
 */
int DuneEngine::nearestPlace() const {
	if (_state.size() <= 7)
		return -1;

	uint16 across = READ_LE_UINT16(_state.begin() + 4);
	int16 down = (int8)_state[6];
	if (_state[6] == 0x80) {
		if (_state.size() <= 0x114f)
			return -1;
		const uint16 at = READ_LE_UINT16(_state.begin() + 0x114e);
		if (at < 0x100 || (uint32)at + 6 > _state.size())
			return -1;
		across = READ_LE_UINT16(_state.begin() + at + 2);
		down = (int16)READ_LE_UINT16(_state.begin() + at + 4);
	}

	int found = -1;
	uint16 closest = 0xffff;

	for (uint i = 0; i != _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		if (!place.isKnown())
			continue;

		const uint16 delta = (uint16)(place.longitude - across);
		const uint16 dx = MIN(delta, (uint16)(0 - delta)) >> 8;
		const uint16 dy = ABS((int)place.latitude - (int)down);
		const uint16 off = MAX(dx, dy);

		if (found < 0 || off < closest) {
			found = (int)i;
			closest = off;
		}
	}

	return found;
}

/**
 * Waiting where the player stands until the evening or until the morning, as
 * 0x0F48 and 0x0F67 do it: the one puts the clock at the twelfth hour of the
 * day it already stands in, the other at the first hour of the next.  Neither
 * moves it backwards.
 */
void DuneEngine::waitUntil(bool evening) {
	enum { kHoursToADay = 16 };

	const uint16 now = hour();
	const uint at = now & 0xf;
	uint16 target;

	if (evening) {
		if (at >= 12)
			return;

		target = (uint16)((now & ~0xf) | 12);
	} else {
		if (at < 11)
			return;

		target = (uint16)((now & ~0xf) + kHoursToADay);
	}

	const uint32 idleTime = _noticeIdleTime;
	const uint16 idleWord = READ_LE_UINT16(_state.begin() + 0xdc5a);
	const bool armVision = storyStage() == 20;
	// 0F54/0F73 prepares only the nearby dawn/dusk boundary, using now+2,
	// not the final wait target. Other waits retain the displayed colours
	// while FD9's hour work registers the gradual transition to the target.
	if ((evening && at < 2) || (!evening && at < 13))
		prepareWaitSkyPalette((uint16)(now + 2));
	finishSkyFade(); // 0F96/390A precedes the real FD9 hour work.
	stopDesertHarvester(); // 0F99/4D00 removes the old exterior actor.
	advanceWaitHours((int16)(target - now));
	// 0FAA/189A restores through 2DB1/2DD3. Ordinary rooms clear their
	// contact history at 2E07, so companions can be contacted anew.
	if (!_state[0x2b])
		WRITE_LE_UINT16(_state.begin() + 0x14, 0);
	// 2DB1/2E0D rebuilds the exterior page rather than retaining the
	// silent companion portrait left by STOP TALKING. A no-op wait above
	// keeps that portrait; only an actual wait restores the bare scenery.
	_speakerPerson = -1;
	_speakerFrame = 0;
	redraw();
	showDesertMenu();
	if (_inDesert && !_state[0x11c9] && !_state[0x2b]) {
		// 0F95 restores through 2DB1/35AD, whose one-render inhibit
		// consumes the old answer index without selecting another reply.
		_state[0x47a6] = 0xff;
		showLocalRoomRequests();
	}
	raisePanelFace(); // 189A/18B7 raises it after the restored scene is shown.
	// 0F95 refreshes scenery, not the input-idle clock. The native menu
	// rebuild resets it, so restore that phase before 0F8E's subtraction.
	// This only arms the next ordinary poll; it must still check Paul is
	// alone outside before calling 1071/B17A.
	_noticeIdleTime = idleTime - (armVision ? 1000 * 5 : 0);
	WRITE_LE_UINT16(_state.begin() + 0xdc5a, idleWord - (armVision ? 1000 : 0));
}

/** DOS 2BA1/1071: the first desert vision and its Emperor demand. */
bool DuneEngine::showFirstDesertVision() {
	if (!_inDesert || _state.size() <= 0x1194 || storyStage() != 20 ||
			READ_LE_UINT16(_state.begin() + 0x10) != 0 || _state[8] != 0xff ||
			_dialogue.count() <= 135 || _dialogue.entry(135).empty())
		return false;

	// 1071 changes the story and character records before its first demand.
	_state[0xff] = 0;
	_state[0x2a] = 21;
	_state[0xfdb] = 1;
	WRITE_LE_UINT16(_state.begin() + 0x1018, 0x200b); // Gurney to room eleven.
	WRITE_LE_UINT16(_state.begin() + 0x101a, 0x0180);
	_state[0xfe8] = 10; // Jessica in the palace's tenth room.
	_state[0xd5] = 0xff;
	WRITE_LE_UINT16(_state.begin() + 0x118d, hour() >> 4); // 2090.
	queueEmperorDemand();
	_state[0xa] |= 1;
	queueTroopNotice(1, 0); // 1071's final 29EE call.

	// 2BC1's B17A selects silently. The visible message belongs to queued
	// event one (part 132), not this part-135 bookkeeping record.
	markAutomaticStoryRecord();
	return true;
}

/** B17A/96B5: mark the next automatic story record without presenting it. */
void DuneEngine::markAutomaticStoryRecord() {
	if (_dialogue.count() <= 135)
		return;
	Common::Array<Dialogue::Record> &records = _dialogue.entry(135);
	for (uint i = 0; i < records.size(); i++) {
		Dialogue::Record &record = records[i];
		if (!dialogueRecordAvailable(record, 0x80) || !conditionHolds(record.condition))
			continue;
		// B17A sets C6's high bit for the duration of 96B5. With these
		// unexpanded records, A034 skips 88AF; speaker sixteen also skips
		// A0D3's voice path. A042 still tests the PREVIOUS paragraph's
		// continuation pointer: A092/A097 must not mark the record (or add
		// its book page) while that paragraph has an unread page.
		if (!READ_LE_UINT16(_state.begin() + 0x47b6)) {
			rememberBookRecord(record, 16);
			record.flags |= 0x80;
			_state[0x19] = 0xff;
			_state[0x47a8] = 0; // A09F consumes the selected line's continuation latch.
		}
		if ((int8)_state[0xfb] >= 0)
			_state[0x28e7] = _state[0x28e8]; // A0E2's retained-dialogue mode.
		break;
	}
}

/*
 * The four ways out of a place, as the table at DS:0x1454 holds them: a step
 * across the world in the low byte and a step down it in the high one, read
 * at twice the way's own number once that has been made positive.  So going
 * north takes a step up, east one to the right, and so on.
 */
/** DOS B5CF: one raw longitude/fractional-latitude step, with polar clamps. */
static void advanceDesertWalk(uint16 &longitude, uint16 &latitude, const byte *step) {
	const int8 across = step[0];
	const int8 down = step[1];
	longitude = (uint16)(longitude + across);
	if (!down)
		return;
	int degree = (int8)(latitude & 0xff);
	int fraction = latitude >> 8;
	if (down > 0) {
		if (++fraction == 256) {
			// B5CF tests the incremented signed degree with JL 98.
			if (degree < 97) {
				degree++;
				fraction = 0;
			} else {
				fraction = 255;
			}
		}
	} else if (--fraction < 0) {
		// The negative branch similarly requires JG -98.
		if (degree > -97) {
			degree--;
			fraction = 255;
		} else {
			fraction = 0;
		}
	}
	latitude = (uint16)((fraction << 8) | (degree & 0xff));
}

/**
 * Out of a building and into the open.  0x3FDF empties the place the player is
 * at and keeps where they were standing in the words at DS:0x4 and DS:0x6,
 * which is why the low byte of DS:0x6 is no longer 0x80 out here.
 */
void DuneEngine::enterDesert(int way) {
	if (_state.size() <= 0x114f)
		return;

	const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
	_cameFromPlace = here >= 0x100 ? (here - 0x100) / 28 : 0;

	if (_cameFromPlace >= _map.places().size())
		return;

	const Place &from = _map.places()[_cameFromPlace];

	_state[0xe7] = 0; // 3FD2 clears the exhaustion-return dialogue flag at the door.
	WRITE_LE_UINT16(_state.begin() + 0x114e, 0);

	// 3FF5 runs the same B5CF step used on every following desert frame.
	// Retain its latitude fraction for terrain selection and the return step.
	uint16 longitude = from.longitude;
	uint16 latitude = (uint8)from.latitude;
	advanceDesertWalk(longitude, latitude, _staticState.begin() + 0x1456 + 2 * way);

	moveMatchingFollowers(longitude, latitude);
	WRITE_LE_UINT16(_state.begin() + 4, longitude);
	WRITE_LE_UINT16(_state.begin() + 6, latitude);
	_state[0xd] = _state[0xb];
	_state[0xb] = (byte)longitude;

	// 0x3FEB puts 0xFF in both of the bytes that say which place and which
	// kind of place the player is in, and 0x37C7 reads the first of them to
	// know that they are out of doors.  Leaving the kind standing left the
	// room table still answering out here, so an edge of the screen walked
	// into a room that is nowhere near.
	_state[8] = 0xff;
	_state[9] = 0xff;

	_inDesert = true;
	_desertEntryWay = way;
	_onWorld = false;
	_speech = "";
	_speakerPerson = -1;

	// 0x443B puts the byte that says what is carrying Paul back to nought
	// whenever DS:0x11C5 - the machine he set out in - is nought, which is
	// what it is once a flight has set down and the ornithopter is standing
	// at the place behind him.  Walking out of a door is walking out on
	// foot.  The flight's own 4 was left in that byte, and 0x4709 folds it
	// down over the two low bits before anything is tested, so the desert
	// went on offering SKIP TO DESTINATION and CHANGE DESTINATION however
	// far Paul walked and never WAIT FOR EVENING - which is the one row the
	// night Chani asks for can be reached through.
	if (_state.size() > 0x11c9)
		_state[0x11c9] = 0;
	// 4057 returns through 2DBF/2E07 on this path too. Contact history
	// belongs to the room just left, unless battle context retains it.
	if (!_state[0x2b])
		WRITE_LE_UINT16(_state.begin() + 0x14, 0);

	// Crossing the door first moves only into the fractional strip: the
	// place's terrain remains behind Paul and an unattended harvester beside
	// it is visible. Held directions continue at one B5CF tick per frame.
	drawDesert();
	// 380C marks an on-foot landscape; 2E29 retains that transition kind
	// for the next redraw and 1771's saved event context.
	_state[0x46e0] = _state[0x46df];
	showDesertMenu();
	if (!_state[0x2b])
		showLocalRoomRequests(); // 4057/2DBF/35AD also runs for uncarried desert.
}

/** 2170: settle the nine named residents before an outside arrival. */
void DuneEngine::settleArrivalResidents() {
	if (_state.size() < 0xd82a)
		return;
	for (uint who = 0; who < 9; who++) {
		byte *person = _state.begin() + 0xfd8 + 16 * who;
		if (person[15] & 0x40)
			continue;
		uint16 latitude = READ_LE_UINT16(person + 2);
		if (who == 3 && latitude == 0x0180)
			person[0] = 4; // 2184: Duncan's communication errand settles at home.
		if ((byte)latitude != 0x80) {
			// 221D/5344 uses the raw person words, not Paul's 407E projection.
			// Wrapped absolute differences and the low-byte comparison matter
			// for these fractional/outdoor records; ties keep the first place.
			const uint16 longitude = READ_LE_UINT16(person);
			uint found = 0, closest = 0xffff;
			for (uint place = 0x100; place + 28 <= _state.size() &&
					READ_LE_UINT16(_state.begin() + place) != 0xffff; place += 28) {
				if (_state[place + 0xa] & 0x80)
					continue;
				const uint16 across = READ_LE_UINT16(_state.begin() + place + 2) - longitude;
				const uint16 down = READ_LE_UINT16(_state.begin() + place + 4) - latitude;
				const uint16 dx = MIN(across, (uint16)-across) >> 8;
				const uint16 dy = MIN(down, (uint16)-down);
				const uint16 distance = (byte)dx >= (byte)dy ? dx : dy;
				if (distance < closest) {
					found = place;
					closest = distance;
				}
			}
			if (!found)
				continue;
			const byte which = (found - 0x100) / 28 + 1;
			WRITE_LE_UINT16(person, (uint16)((_state[found + 8] << 8) | 1));
			WRITE_LE_UINT16(person + 2, (uint16)((which << 8) | 0x80));
			if (which == _state[9])
				person[15] |= 4;
		} else if (person[0] == 1) {
			if (person[1] >= 0x21 || (latitude >> 8) == _state[9])
				continue;
			const uint place = 0xe4 + 28 * (latitude >> 8);
			if (place + 0xa >= _state.size() || (_state[place + 0xa] & 2))
				continue;
			person[0]++;
			if ((latitude >> 8) == 1) {
				person[0] = _state[0x144d + person[14]];
				person[15] &= (byte)~4;
				if (person[0] == 6 && storyStage() < 36)
					person[0] = 10;
			}
		} else if (who >= 5 && (latitude >> 8) == 1) {
			uint16 seed = READ_LE_UINT16(_state.begin() + 0xd828), value;
			do {
				do {
					value = combatRandom(seed, 0xcbd1) & 15; // E3DF's inclusive ten.
				} while (value > 10);
				value = (byte)(value + 2);
			} while (storyStage() < 84 && (value == 3 ||
				(storyStage() < 36 && (value == 11 || value == 6))));
			WRITE_LE_UINT16(_state.begin() + 0xd828, seed);
			person[0] = (byte)value;
		}
	}
}

/** 40C3/36EE: move flagged records at Paul's old coordinates only. */
void DuneEngine::moveMatchingFollowers(uint16 longitude, uint16 latitude) {
	if (_state.size() < 0xfd8 + 16 * 16)
		return;
	const uint16 oldLongitude = READ_LE_UINT16(_state.begin() + 4);
	const uint16 oldLatitude = READ_LE_UINT16(_state.begin() + 6);
	for (uint who = 0; who < 16; who++) {
		byte *person = _state.begin() + 0xfd8 + 16 * who;
		if (!(person[15] & 0x40) || READ_LE_UINT16(person) != oldLongitude ||
				READ_LE_UINT16(person + 2) != oldLatitude)
			continue;
		WRITE_LE_UINT16(person, longitude);
		WRITE_LE_UINT16(person + 2, latitude);
	}
}

/** 4B3B: one carried position callback, optionally recording a map trail. */
void DuneEngine::advanceCarriedPosition(bool recordTrail) {
	if (_flyingTo < 0 || (uint)_flyingTo >= _map.places().size() ||
			_state.size() <= 0x472c)
		return;
	uint16 longitude;
	int16 latitude;
	if (!playerWorldPosition(longitude, latitude))
		return;
	// Every sixteenth callback calls 0FD9 with CX=1 before moving.
	const uint16 callbacks = READ_LE_UINT16(_state.begin() + 0x472b) + 1;
	WRITE_LE_UINT16(_state.begin() + 0x472b, callbacks);
	if (!(callbacks & 15))
		advanceWaitHours(1);

	const bool manual = _flightSteered || _flightToPoint;
	if (!manual) {
		const Place &to = _map.places()[_flyingTo];
		flightBearing(longitude, latitude, to.longitude, to.latitude, _flightHeading);
	}
	byte fraction = _state[0x11cc];
	advanceFlightPosition(longitude, latitude, _flightHeading, fraction,
			_map.step(ABS((int)latitude)), manual);
	_state[0x11c7] = _flightHeading;
	_state[0x11cc] = fraction;
	moveMatchingFollowers(longitude, (uint16)latitude);
	WRITE_LE_UINT16(_state.begin() + 4, longitude);
	// 4B3B stores the whole signed BX latitude after 40C3, not only BL.
	WRITE_LE_UINT16(_state.begin() + 6, (uint16)latitude);
	if (recordTrail && (_state[0x11c9] & 3)) {
		eraseFlightInsetHead(); // 4A1A precedes 4A00 and any arrival/report return.
		rememberFlightTrailPoint();
	}
}

/** 4FFB: fast-forward the hidden route; false means 4182 interrupted it. */
bool DuneEngine::advanceSkippedFlight() {
	if (_flyingTo < 0 || (uint)_flyingTo >= _map.places().size())
		return false;
	_state[0x11c8] = 0;
	_flightSteered = false;
	_flightToPoint = false;
	const Place &target = _map.places()[_flyingTo];
	const int32 targetCell = _map.groundOffset(target.longitude, target.latitude);
	for (uint i = 0; i != 200 && !shouldQuit(); i++) {
		advanceCarriedPosition(false);
		const uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
		const int16 latitude = (int8)_state[6];
		if (_map.groundOffset(longitude, latitude) == targetCell)
			return true;

		_state[0x23] = 0; // 501F: a prior report is not a fresh SKIP hazard.
		checkFlightHazards();
		if (_state[0x23]) {
			// 4F2F undoes 4182's decrement before returning to 2E52.
			_state[0x4726] += 0x20;
			return false;
		}
	}
	return !shouldQuit();
}

/**
 * One step over the world. B5CF moves a single raw longitude unit east/west
 * or one 1/256-degree latitude fraction north/south. Each mouse press
 * dispatches one step; the native arrow-key shortcut can repeat it.
 */
void DuneEngine::stepThroughDesert(int way) {
	if (way < 0 || way > 3 || _state.size() <= 7)
		return;
	if (_flyingTo < 0) {
		finishSoundEffectLoop(); // 3F27/AC30, before an on-foot step.
		stopSpeech(); // 3F2E/A7A5 stops the previous room's recording.
	}
	if (_flyingTo >= 0 && (_flightDiscoveryPrompt || _state[0x11ca]))
		return; // 978E/4ACA: even direct compass input cannot move a report.

	uint16 longitude;
	uint16 latitudeWord;
	int16 latitude;
	if (_flyingTo >= 0) {
		advanceCarriedPosition(true);
		longitude = READ_LE_UINT16(_state.begin() + 4);
		latitudeWord = READ_LE_UINT16(_state.begin() + 6);
		latitude = (int8)latitudeWord;
	} else {
		// 3F49: the low seven bits count outdoor moves; the high bit requests
		// 3723's post-redraw exhaustion check. INC's signed saturation is 127.
		_state[0x4735] = MIN<uint>(_state[0x4735] & 0x7f, 126) + 1;
		_state[0x4735] |= 0x80;
		// 3F59 counts only outdoor on-foot steps, saturating at twenty.
		// The initial room-to-desert step enters at 3FF5 and skips this;
		// carried callbacks have their own path and do not add fatigue.
		if (_state[0xf4] < 20)
			_state[0xf4]++;
		longitude = READ_LE_UINT16(_state.begin() + 4);
		latitudeWord = READ_LE_UINT16(_state.begin() + 6);
		advanceDesertWalk(longitude, latitudeWord, _staticState.begin() + 0x1456 + 2 * way);
		latitude = (int8)latitudeWord;
	}

	int arrived = _map.placeAt(longitude, latitude);
	// 3FFE/4015 tests BH==0 and the exact place longitude before entering;
	// sharing a MAP.HSQ cell alone is not sufficient on foot.
	if (_flyingTo < 0 && (latitudeWord >> 8 || arrived < 0 ||
			_map.places()[arrived].longitude != longitude))
		arrived = -1;
	if (_flyingTo < 0 && arrived < 0) {
		// 4057 moves records before writing Paul. If the step enters a place,
		// arriveAt performs that single move straight to the room coordinates,
		// not via the intermediate world point (which could pick up strangers).
		moveMatchingFollowers(longitude, latitudeWord);
		WRITE_LE_UINT16(_state.begin() + 4, longitude);
		WRITE_LE_UINT16(_state.begin() + 6, latitudeWord);
		_state[0xd] = _state[0xb];
		_state[0xb] = (byte)longitude;
	}
	// 4F49 checks the destination before 4F52/35E9 scans the surrounding
	// terrain. A completed ordinary course lands without revealing a nearby
	// off-route place. Broken-off/point courses still pass through discovery.
	if (_flyingTo >= 0 && !_flightToPoint && !_flightSteered && wayToFly() < 0) {
		const int bound = _flyingTo;
		_inDesert = false;
		flyTo((uint)bound, true);
		if (_speakerPerson < 0)
			showRoomMenu();
		return;
	}
	if (_flyingTo >= 0 && dispatchCarriedRoomRequests())
		return;

	// 409A/4015 is the on-foot entrance, including the exact longitude and
	// zero latitude fraction checked above. Carried arrival is exclusively
	// 4F49's destination-cell test before discovery, not a generic map marker.
	if (arrived >= 0 && _flyingTo < 0) {
		debugC(1, kDebugScript, "out of the open and into place %u", arrived);

		_inDesert = false;
		_desertEntryWay = -1;
		flyTo((uint)arrived, true);
		if (_speakerPerson < 0)
			showRoomMenu();
		return;
	}

	// The terrain movie has its own fifteen-frame clock while a flight's map
	// position changes on its own travel clock.  Redrawing here decoded
	// extra movie frames at every position step (and showDesertMenu decoded a
	// second one), producing visible jumps in an otherwise smooth animation.
	if (_flyingTo >= 0) {
		updateFlightInset();
		// 4FAD tail-calls 4E8E after movement/cache painting. This may
		// change the next bearing, but must not move Paul or decode a frame.
		selectFlightTerrain();
		return;
	}

	// 4057 enters 2DBF/2E07 on each on-foot redraw. Physical presence at
	// 0012 is independent of this room's contact mask and request guard.
	if (!_state[0x2b]) {
		WRITE_LE_UINT16(_state.begin() + 0x14, 0);
		_state[0x46df] = 0;
	}
	drawDesert();
	_state[0x46e0] = _state[0x46df]; // 2E29, as at the first outdoor step.
	showDesertMenu();
	if (!_state[0x2b])
		showLocalRoomRequests(); // 4057/2DBF/35AD before the next on-foot poll.
	pollDesertExhaustion();
}

/** E387: an interrupt-only wait, with no input or animation callback polls. */
void DuneEngine::waitDesertExhaustion(uint16 ticks) {
	const uint32 milliseconds = ticks * 5u;
	if (ConfMan.getBool("dune_nodelay")) {
		advanceClockElapsed(milliseconds, false, false);
		return;
	}
	const uint32 began = _system->getMillis();
	while (!shouldQuit() && _system->getMillis() - began < milliseconds) {
		// E387 enables timer interrupts, not D9D2/ACE6's music handoff.
		updateMusicFade();
		_system->delayMillis(5);
	}
	// Interrupt hours accrue during E387, before FD9 drains them. Do not
	// postpone this elapsed time until after the five forced return hours.
	tick(false);
}

/** 2E62/3723: prolonged walking shows SUN, then returns to the last place. */
bool DuneEngine::pollDesertExhaustion() {
	if (!_inDesert || _flyingTo >= 0 || _onWorld || _onGlobe || _onMixer ||
			_inCabin || _speakerPerson >= 0 || _state[0x47a7] || _state[0x46d9] ||
			_menuKind != kMenuDesert || !(_state[0x4735] & 0x80))
		return false;
	const byte pending = _state[0x4735];
	_state[0x4735] &= 0x7f;
	if (pending < 0x94 || ((pending - 0x94) & 15))
		return false;

	const bool cursorShown = CursorMan.showMouse(false);
	showPicture(34); // 37AD/C2F2: SUN.HSQ, not a synthesized fatigue overlay.
	waitDesertExhaustion(MIN<uint>((pending - 0x94) / 16 + 1, 10) << 8);
	if (_state[0x4735] < 0x37) {
		drawDesert();
		showDesertMenu();
	} else {
		// E77 plays five frames of DOS movie eleven (DEAD3), not a defeat
		// menu or the complete DEAD film. Keep the current panel beneath it.
		// AD50 stops the current track and explicitly requests song three.
		stopTheSong();
		if (!musicIsOff()) {
			_song = _wantedSong = kSongs[3];
			if (_music)
				_music->play(_song);
		}
		Resource movie = _archive.openMember("DEAD3.HNM");
		if (movie) {
			_video->setReader(movie);
			_video->setInterlace(false);
			_video->start();
			memcpy(_video->_frameBuffer, _framebuffer1, kScreenWidth * kScreenHeight);
			for (uint frame = 0; frame < 5 && !_video->done() && !shouldQuit(); frame++) {
				_graphics.setFrameBuffer(_video->_frameBuffer);
				_video->decodeAVFrame();
				drawFace(); // C108's 1797 callback restores the panel's raised face.
				uint16 sequence = 1;
				// E94/C108 effect 3C uses the same feedback walk as effect 8,
				// but only eighty pixel pairs before each 200-Hz timer tick.
				while (!shouldQuit()) {
					const bool done = Graphics::revealScene(_video->_frameBuffer, _framebuffer1, sequence, 80);
					_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
					_system->updateScreen();
					if (done)
						break;
					waitDesertExhaustion(1);
				}
			}
			_video->stopAudio();
			_graphics.setFrameBuffer(_framebuffer1);
		}
		returnFromDesertExhaustion();
	}
	CursorMan.showMouse(cursorShown);
	return true;
}

/** 375A..37A7: real five-hour work, 401F arrival, then the interior move. */
void DuneEngine::returnFromDesertExhaustion() {
	advanceWaitHours(5);
	const uint16 place = READ_LE_UINT16(_state.begin() + 0x1150);
	if (place < 0x100 || place + 28 > _state.size() || (place - 0x100) % 28)
		return;
	// 3760/C108 dissolves to C0AD's black page before the arrival callbacks.
	Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	byte oldPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	memset(_framebuffer1, 0, oldPage.size());
	showMapEntryTransition(oldPage.begin(), oldPalette);
	drawPanel(); // 3768/86 and 1A0F restore the panel and current time.
	_state[0x4735] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x114e, place);
	updateBattleContext(); // 402E runs before the room coordinate is stored.
	WRITE_LE_UINT16(_state.begin() + 0x98, 0);
	WRITE_LE_UINT16(_state.begin() + 0x9a, 0);
	revealPlace(place);
	const byte kind = _state[place + 8];
	const uint16 whose = (((place - 0x100) / 28 + 1) << 8) | 0x80;
	_state[8] = kind;
	_state[9] = whose >> 8;
	if (kind >= 0x20)
		_state[place + 10] |= 0x10;
	settleArrivalResidents();
	uint16 where = (kind << 8) | 1;
	auto move = [&](uint16 room) {
		moveMatchingFollowers(room, whose);
		WRITE_LE_UINT16(_state.begin() + 4, room);
		_state[0xd] = _state[0xb];
		_state[0xb] = (byte)room;
		WRITE_LE_UINT16(_state.begin() + 6, whose);
	};
	move(where);
	// 3788: sietches use room two, Atreides palace uses ten; villages and
	// forts retain room one. Move matching companions at both original edges.
	if (kind < 0x21)
		where = (kind << 8) | (kind == 0x20 ? 10 : 2);
	move(where);
	_state[0xe7]++;
	_state[0x47a6] = 0; // 35AD consumes the synchronous-transition latch.
	_state[0x47a7] = 0;
	_state[0x1a] = 0;
	_inDesert = false;
	_desertEntryWay = -1;
	_speech.clear();
	_speakerPerson = -1;
	roomFor((place - 0x100) / 28);
	peopleHere(); // 4002/2E07 prepares the new village account exactly once.
	// 3799/C108 uses effect 34 again for 4057's final room. The shared
	// driver dissolve retains its original midpoint palette handover.
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	drawScene(false);
	showRoomMenu(false);
	showMapEntryTransition(oldPage.begin(), oldPalette);
}

/**
 * Which picture the desert is seen as.  0x384A picks between two sets: the
 * nineteen of open sand, DN20.HSQ to DN38.HSQ, and the ten with rock in them,
 * VG01.HSQ to VG10.HSQ.  Rock is either the place the player last stood at
 * saying so in bit 0 of its byte at +0xA, or one of the four points of the
 * world about this one carrying 0x10 in the two bits 0x30 of its ground.
 *
 * Which of the set is drawn comes out of where the player is - 0x3878 puts the
 * latitude over the longitude, adds one and takes the remainder - so a point
 * of the world always looks the same.
 */
const char *DuneEngine::desertPicture() const {
	static const char *const kSand[] = {
		"DN20.HSQ", "DN21.HSQ", "DN22.HSQ", "DN23.HSQ", "DN24.HSQ", "DN25.HSQ",
		"DN26.HSQ", "DN27.HSQ", "DN28.HSQ", "DN29.HSQ", "DN30.HSQ", "DN31.HSQ",
		"DN32.HSQ", "DN33.HSQ", "DN34.HSQ", "DN35.HSQ", "DN36.HSQ", "DN37.HSQ",
		"DN38.HSQ"
	};
	static const char *const kRock[] = {
		"VG01.HSQ", "VG02.HSQ", "VG03.HSQ", "VG04.HSQ", "VG05.HSQ",
		"VG06.HSQ", "VG07.HSQ", "VG08.HSQ", "VG09.HSQ", "VG10.HSQ"
	};

	if (_state.size() <= 0x1151)
		return kSand[0];

	// 3814 uses DS:1150, not the native door/flight bookkeeping. It compares
	// only the longitude with that last place: a north/south step may still use
	// one of the place's nearby views even after the latitude has changed.
	const uint16 lastPlace = READ_LE_UINT16(_state.begin() + 0x1150);
	const bool inRoom = _state[6] == 0x80;
	if (lastPlace >= 0x100 && (uint)lastPlace + 8 < _state.size() &&
			(inRoom || READ_LE_UINT16(_state.begin() + 4) ==
				READ_LE_UINT16(_state.begin() + lastPlace + 2))) {
		const char *nearPlace = outsidePicture(_state[lastPlace + 8], inRoom ? 0 : _state[7],
				_state[lastPlace]);
		if (nearPlace)
			return nearPlace;
	}

	const uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	const uint16 which = (uint16)((((uint16)_state[6] << 8) | _state[7]) ^ longitude) + 1;

	return groundIsRock() ? kRock[which % ARRAYSIZE(kRock)]
						  : kSand[which % ARRAYSIZE(kSand)];
}

/**
 * Whether there is rock where the player is standing, which is what 0x384A
 * asks before it picks the desert's picture: the place last stood at sets bit
 * 0 of its byte at +0xA, or one of the four points of the world about this one
 * carries 0x10 in the two bits 0x30 of its ground.
 */
bool DuneEngine::groundIsRock() const {
	if (_state.size() <= 0x1151)
		return false;

	uint16 longitude;
	int16 latitude;
	if (!playerWorldPosition(longitude, latitude))
		return false;

	// DS:0x1150 keeps the place the player was last at even out here, where
	// DS:0x114E has been emptied.  It is a pointer, so anything below the
	// first of the places is nobody.
	const uint16 nearest = READ_LE_UINT16(_state.begin() + 0x1150);

	if (nearest >= 0x100 && nearest + 0xa < _state.size()
		&& (_state[nearest + 0xa] & 1) != 0)
		return true;

	for (int along = -1; along != 3; along++) {
		if ((_map.groundAt(longitude, latitude, along) & 0x30) == 0x10)
			return true;
	}

	return false;
}

/**
 * Low-memory bank for the active landscape film. DOS DC00/DC02 hold active
 * and requested movie-table indices, whose reserved first slot means that
 * 1 is DFL2 and 2..5 are MNT1..4. CBC2 uses the active index, not the terrain
 * callback's pending request, until the loop boundary.
 */
Common::String DuneEngine::flightLoop() const {
	// CBC2 uses 97 + the DOS movie index for its low-memory loop bank.
	// DFL2's worm fallback is not one of the four MNT loop resources.
	const uint movie = READ_LE_UINT16(_state.begin() + 0xdc00);
	return movie >= 2 && movie <= 5
			? Common::String(getResourceFilenameByIndex(INDEX_MNT1_LOP + movie - 2)) : Common::String();
}

/**
 * One frame of a looping movie, laid down over the whole of the view.
 *
 * A .LOP opens with a word saying how long its own head is - twenty-four - and
 * a word of 0xFFFF, and then five long words: where each of its four blocks
 * begins, counted from the end of the head, and where the last of them ends.
 * A block is six bytes of its own - how long it is, and the two words the
 * frame drifts by - and then an HSQ, which unpacks to the 320 by 152 of the
 * view, eight bits to a pixel.  There is no palette in any of it, so the
 * colours are the hour's sky, the same as the desert's own pictures.
 */
bool DuneEngine::drawFlightFrame(const Common::String &loop, uint frame) {
	enum { kHeadSize = 24, kFrames = 4, kBlockHead = 6, kWide = 320, kHigh = 152 };
	if (loop.empty())
		return false;

	Resource file = _archive.openMember(loop);
	if (!file)
		return false;

	const uint32 size = file->size();
	if (size < kHeadSize)
		return false;

	Common::Array<byte> raw;
	raw.resize(size);
	file->read(raw.begin(), size);

	if (READ_LE_UINT16(raw.begin()) != kHeadSize)
		return false;

	const uint which = frame % kFrames;
	const uint32 from = kHeadSize + READ_LE_UINT32(raw.begin() + 4 + 4 * which);
	const uint32 to = kHeadSize + READ_LE_UINT32(raw.begin() + 4 + 4 * (which + 1));

	if (from + kBlockHead >= to || to > size)
		return false;

	Common::MemoryReadStream block(raw.begin() + from + kBlockHead, to - from - kBlockHead);

	const int unpacked = block.readUint16LE();
	block.readByte();
	const int packed = block.readUint16LE();
	block.readByte();

	if (unpacked != kWide * kHigh || packed <= 0 || (uint32)packed > to - from - kBlockHead)
		return false;

	Common::Array<byte> picture;
	picture.resize(unpacked);
	decompressHSQ(&block, packed, picture.begin(), unpacked);

	memcpy(_framebuffer1, picture.begin(), kWide * kHigh);

	debugC(2, kDebugGraphics, "the ground goes by: %s, frame %u of four",
		   loop.c_str(), which);

	return true;
}

/**
 * Advances one frame of the full CD flight movie. MNT1 is the ornithopter's
 * open landscape and DFL2 the worm's; a destination film replaces it during
 * the final approach. The four-frame .LOP files remain the
 * low-memory fallback, but using them on the CD release made a conspicuous
 * four-frame repeat in place of the much longer animation seen in the
 * recording.
 */
bool DuneEngine::drawFlightMovieFrame(bool advance, bool restart) {
	const bool forcedChange = _approachMovie >= 0 && _flightMovie != _approachMovie;
	if (_flightMovie >= 0 && !forcedChange && !advance && !restart) {
		// A menu/inset/portrait redraw retains the displayed film, even at
		// its final frame. Only a ready CA60 poll may decode or loop it.
		if (_flightMovieFallback)
			return false;
		memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * kSceneHeight);
		_graphics.setFrameBuffer(_framebuffer1);
		return true;
	}
	const uint32 tick = _system->getMillis() / 5;
	const bool boundary = _flightMovieFallback
			? _flightFrame % 4 == 0 && _flightFrame != _flightLoopLastFrame : _video->done();
	int wanted = restart && _flightMovie >= 0 ? _flightMovie : _approachMovie;
	if (wanted < 0) {
		if (_flightMovie < 0) {
			// 37F4/CA1B starts the base film before its terrain callback.
			const uint base = READ_LE_UINT16(_state.begin() + 0x487e);
			wanted = base >= 1 && base <= 5 ? base - 1 : HNM_MNT1;
		} else if (boundary) {
			// CB61 changes the requested film only at a loop boundary,
			// not whenever the next terrain sample changes.
			const uint next = READ_LE_UINT16(_state.begin() + 0xdc02);
			wanted = next >= 1 && next <= 5 ? next - 1 : _flightMovie;
		} else {
			wanted = _flightMovie;
		}
	}
	const char *filename = getResourceFilenameByIndex(wanted + INDEX_FIRST_HNM);

	const bool starting = restart || _flightMovie != wanted || (!_flightMovieFallback && _video->done());
	if (starting) {
		WRITE_LE_UINT16(_state.begin() + 0xdc00, wanted + 1);
		WRITE_LE_UINT16(_state.begin() + 0xdc02, wanted + 1);
		Resource movie = _archive.openMember(filename);
		if (!movie) {
			// Retain the active index for CBC2's four-frame substitute.
			// A redraw must not restart the base film or use a pending bank.
			_flightMovie = _approachMovie >= 0 ? -1 : wanted;
			_flightMovieFallback = _approachMovie < 0;
		} else {
			_video->setReader(movie);
			_video->setInterlace(false);
			_video->start();
			_flightMovie = wanted;
			_flightMovieFallback = false;
		}
	}
	_flightLoopLastFrame = _flightFrame;
	_flightFilmTime = tick;
	_flightFilmClockSet = _flightMovie >= 0;

	if (_flightMovieFallback || _flightMovie < 0 || _video->done())
		return false;

	// drawDesert() paints the panel after every decoded frame and therefore
	// leaves the graphics blitter on the game page. Point it back at the HNM
	// page so delta frames build on one another; otherwise frame zero was copied
	// forever and both orni and worm travel appeared completely still.
	_graphics.setFrameBuffer(_video->_frameBuffer);
	_video->decodeAVFrame();
	if (starting)
		_flightFilmTime = _system->getMillis() / 5; // CA1B's initial CA59 follows preparation.
	memcpy(_framebuffer1, _video->_frameBuffer, kScreenWidth * kSceneHeight);

	// Starting a movie points the graphics routines at its private page.  The
	// panel and any words over the flight belong on the game's page.
	_graphics.setFrameBuffer(_framebuffer1);
	return true;
}

void DuneEngine::stopFlightMovie() {
	if (_flightMovie >= 0)
		_video->stopAudio();

	_flightMovie = -1;
	_flightMovieFallback = false;
	_flightSkyRestartPending = false;
	_flightLoopLastFrame = 0;
	_approachMovie = -1;
	_flightFilmClockSet = false;
	WRITE_LE_UINT16(_state.begin() + 0x35a6, 0);
}

/** DOS 488A: the three film arrivals follow their distinct class branches. */
void DuneEngine::animateOrnithopterApproach(byte kindOfPlace) {
	const bool noDelay = ConfMan.hasKey("dune_nodelay") && ConfMan.getBool("dune_nodelay");
	if (kindOfPlace <= 0x20 && !_flightMovieFallback) {
		// 48A2 polls CA60 before 4EC6 with AX=0. A rocky film returns
		// through MNT4, then MNT1; the final film starts only at MNT1's
		// handover frame. No position or terrain-forecast callback runs here.
		_state[0x4728] = 0x80;
		const uint handover = kindOfPlace < 0x20 ? 60 : 22;
		while (!shouldQuit()) {
			if (noDelay || flightFrameDue(_system->getMillis() / 5)) {
				if (_flightFilmClockSet)
					_flightFrame++;
				drawFlightMovieFrame(true);
			}
			drawDesert();
			// Missing HNM assets retain the existing four-pose substitute;
			// it has no full-film handover frame to wait for.
			if (_flightMovieFallback || _flightMovie < 0)
				break;
			uint16 base = READ_LE_UINT16(_state.begin() + 0x487e);
			const uint16 current = READ_LE_UINT16(_state.begin() + 0xdc00);
			const uint16 wanted = flightTerrainMovie(0, 0, base, current);
			WRITE_LE_UINT16(_state.begin() + 0x487e, base);
			WRITE_LE_UINT16(_state.begin() + 0xdc02, wanted);
			if (_flightMovie == HNM_MNT1 && (uint)_video->_currentFrame == handover)
				break;
			if (hold(flightFramePeriod() * 5) == kPlaybackAborted)
				break;
		}
	}
	// 5E4F yields class six for a sietch and seven for the palace; their
	// 48A2 branch switches to SIET/PALACE. Class nine instead enters 48DD with
	// AX=9: C8FB starts movie-table entry nine, FORT.HNM, and waits for it.
	// Classes eight and ten take the reverse-ORNYTK branch with no extra film.
	_approachMovie = kindOfPlace < 0x20 ? HNM_SIET :
		kindOfPlace == 0x20 ? HNM_PALACE : HNM_FORT;
	while (!shouldQuit()) {
		if (_flightMovie == _approachMovie && (noDelay || flightFrameDue(_system->getMillis() / 5)))
			drawFlightMovieFrame(true);
		drawDesert();
		if (_flightMovie != _approachMovie || _video->done())
			break;
		if (ConfMan.hasKey("dune_dump") && _video->_currentFrame % 15 == 0)
			dumpScreenPPM(Common::String::format("%s-approach%03d.ppm",
										 ConfMan.get("dune_dump").c_str(), _video->_currentFrame));
		if (hold(flightFramePeriod() * 5) == kPlaybackAborted)
			break;
	}
	_approachMovie = -1;
}

/**
 * The open desert: the hour's own sky, and one whole picture of the desert
 * drawn over it.  0x380C asks for the wide sky - the byte at DS:0x22E3 tells
 * 0x3971 to take SKYDN.HSQ rather than SKY.HSQ - and 0xC2F2 then loads the
 * picture and lays its first sprite down at the corner, palette and all.
 */
void DuneEngine::drawDesert(bool showInset, bool present, bool sceneOnly) {
	const bool restoreHarvester = _restoreHarvesterPage && _harvesterActive;
	// 37DF marks the landscape page; 380C/388D selects the on-foot
	// transition kind even for a scripted pose's redraw, not only movement.
	_state[0x47a4] |= 1;
	if (!(_state[0x11c9] & 3))
		_state[0x46df] = 1;
	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	const bool carried = _state.size() > 0x11c9 && (_state[0x11c9] & 3) != 0;
	// 37F4/3805 retains the low palette inherited from the cabin or a
	// companion's discovery portrait. Reloading ORNYPAN on every frame
	// overwrote Gurney's colours32..55, including after the next landing.
	// Save loading reconstructs missing legacy colours once, not here.
	// Neither the CD journey movie nor its four-frame low-memory fallback has
	// a palette of its own; both are indexed through the current hour's wide
	// sky colours.  This must happen even when the movie decoder supplies the
	// pixels.  Applying it only on the fallback path made a loaded/in-flight
	// scene entirely black and looked as though no travel animation existed.
	if (_flightSkyRestartPending) {
		_flightSkyRestartPending = false;
		restartFlightSkyPalette(); // 37F4/3805 retains the inherited low palette.
	} else {
		applySkyPalette(true);
	}
	drawSky();

	// Under a flight the ground goes by rather than standing still: one of
	// the six looping movies is laid over the sky a frame at a time, and it
	// is only afoot that the one whole picture of the desert is drawn.
	bool moving = false;
	if (_flyingTo >= 0)
		moving = drawFlightMovieFrame();

	if (!moving && _flyingTo >= 0)
		moving = drawFlightFrame(flightLoop(), _flightFrame);

	if (!moving) {
		openSpriteSheet(desertPicture());
		drawSprite(0, 0, 0);
	}

	// 4AEB/4D6C composites the animated back from ALL four VER sprites.
	// VER.BIN supplies each film frame's pose and origins; a single large
	// sprite is only one ridge, not the complete worm.
	const bool ridingWorm = _flyingTo >= 0 && (_state[0x11c9] & 3) == 2;
	if (ridingWorm) {
		Resource animation = _archive.openMember("VER.BIN");
		if (animation) {
			Common::Array<byte> data;
			data.resize(animation->size());
			animation->read(data.begin(), data.size());
			Common::Array<WormBackStamp> stamps;
			if (wormBackFrame(data.begin(), data.size(), _flightFrame, stamps)) {
				openSpriteSheet("VER.HSQ", false);
				Common::Rect clip(READ_LE_INT16(data.begin()), READ_LE_INT16(data.begin() + 2),
						READ_LE_INT16(data.begin() + 4), READ_LE_INT16(data.begin() + 6));
				clip.clip(Common::Rect(kScreenWidth, kSceneHeight));
				for (uint i = 0; i < stamps.size(); i++)
					currentSpriteSheet->drawMapSprite(stamps[i].sprite, stamps[i].x, stamps[i].y, clip);
			}
		}
	}

	refreshDesertEquipment(); // 37EB/4E12 refreshes the nearby site before 4D06.
	if (restoreHarvester) {
		memcpy(_framebuffer1, _harvesterPage.begin(), _harvesterPage.size());
		_restoreHarvesterPage = false;
		playSoundEffect(5); // Rebuild transient digital playback after load.
	} else {
		prepareDesertHarvester();
	}

	if (carried && showInset) {
		// The landscape movie palette is applied while its frame is decoded.
		// Restore the hour's wide ramp afterwards for the inset's indices 80..95;
		// doing it before the decoder left that small map purple and unreadable.
		applySkyPalette(true);
		paintFlightInset();
	}

	// The three journey buttons belong to a machine or worm carrying Paul;
	// on foot the original leaves the ordinary four-way compass up.  Showing
	// the journey controls while walking both hid west entirely and made the
	// other directions look like three unrelated upward arrows.
	if (carried) {
		setCompass(kCompassFlight);

		// During an ordinary uninterrupted flight the recording shows only
		// the framed empty box.  The three journey buttons appear after travel
		// has been broken off, alongside BACK TO STARTING POINT, TOWARDS THE
		// NEAREST PLACE and CHANGE DESTINATION.
		const bool brokenOff = _state.size() > 0x11cb && _state[0x11cb] != 0;
		if (!brokenOff) {
			for (int i = 1; i <= 3; i++)
				_panel[kFirstCompassElement + i].sprite = 0xffff;
		}
	} else {
		enum { kNoWayOutBox = 35, kHidden = 0x20 };

		setCompass(kCompassRoom);
		_panel[kFirstCompassElement].sprite = kNoWayOutBox;
		_panel[kFirstCompassElement + 5].flags |= kHidden;
	}
	if (sceneOnly)
		return;
	drawPanel();
	drawMenu();

	if (!_speech.empty())
		drawSpeech(_speech, kSpeechColour);
	if (_harvesterActive) {
		// The panel's face/bar reaches above line 152. A checkpoint retains
		// exactly the page last presented, including any callback overlap;
		// redraw must not quietly substitute a newly painted panel there.
		if (restoreHarvester)
			memcpy(_framebuffer1, _harvesterPage.begin(), _harvesterPage.size());
		else
			memcpy(_harvesterPage.begin(), _framebuffer1, _harvesterPage.size());
	}

	if (present) {
		finishPanelMenuTransition();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** 4E12/4DED/7F27: refresh the equipment at the terrain point under Paul. */
void DuneEngine::refreshDesertEquipment() {
	if (_state.size() <= 0x4705 || (_state[0x11c9] & 3) || _state[6] == 0x80)
		return;
	const uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	const int16 latitude = (int8)_state[6];
	const int32 point = _map.groundOffset(longitude, latitude);
	if (point < 0 || !(_map.groundAt(longitude, latitude) & 0x40))
		return;
	// 409A finds the place by its terrain offset, not DS:1150's last site
	// or a remote chief's retained equipment context. 4E39's word arithmetic
	// admits three longitude steps before its centre and four after it.
	for (uint place = 0x100; place + 28 <= _state.size() && _state[place] != 0xff; place += 28) {
		if (READ_LE_UINT16(_state.begin() + place + 6) != point)
			continue;
		if ((uint16)(READ_LE_UINT16(_state.begin() + place + 2) - longitude + 4) < 8) {
			for (uint item = 0; item < 7; item++)
				_state[0x46fe + item] = equipmentSpare(place, item);
		}
		return;
	}
}

bool DuneEngine::loadHarvesterBank() {
	if (_harvesterBank.empty()) {
		Resource bank = _archive.openMember("MOIS.HSQ");
		if (!bank)
			return false;
		_harvesterBank.resize(bank->size());
		bank->read(_harvesterBank.begin(), _harvesterBank.size());
	}
	return _harvesterBank.size() > 0x2aa6;
}

void DuneEngine::stopDesertHarvester() {
	_harvesterActive = false;
	_restoreHarvesterPage = false;
	_harvesterRemainder = 0;
	_harvesterBackdrop.clear();
	_harvesterPage.clear();
}

/** 4D57/4C92: the last MOIS entry is a composite, not a drawable sprite. */
void DuneEngine::selectHarvesterSequence(uint sequence) {
	const byte *bank = _harvesterBank.begin();
	const uint index = READ_LE_UINT16(bank);
	const uint entries = READ_LE_UINT16(bank + index) / 2;
	const uint record = index + READ_LE_UINT16(bank + index + 2 * (entries - 1)) + 4;
	const uint table = record + 8 + READ_LE_UINT16(bank + record + 8);
	const uint start = table + READ_LE_UINT16(bank + table + 2 * sequence);
	const uint origin = (byte)(5 + (_state[7] << 2)); // Both shifts and the add are bytes.
	const uint16 words[] = {49, (uint16)origin, 41, (uint16)MIN<uint>(origin + 173, 320), 152,
		(uint16)start, (uint16)start, (uint16)(record + 10), (uint16)origin, 41,
		0, 0, 0, 0, 173, 127};
	for (uint i = 0; i < ARRAYSIZE(words); i++)
		WRITE_LE_UINT16(_state.begin() + 0x485e + 2 * i, words[i]);
}

/** 4D06: show parked equipment first, then register the working sequence. */
void DuneEngine::prepareDesertHarvester() {
	stopDesertHarvester();
	_state[0xf6] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x4733, 0);
	if (_flyingTo >= 0 || (_state[0x11c9] & 3))
		return;
	const uint place = READ_LE_UINT16(_state.begin() + 0x1150);
	if (place < 0x100 || place + 28 > _state.size() ||
			READ_LE_UINT16(_state.begin() + place + 2) != READ_LE_UINT16(_state.begin() + 4) ||
			_state[place + 4] != _state[6])
		return;
	byte working = 0;
	const Common::Array<uint> standing = _map.troopsAt(place, _state.begin(), _state.size());
	for (uint i = 0; i < standing.size(); i++) {
		const uint troop = 0x8aa + 27 * standing[i];
		// 4E04 counts only an actual mining order, not a waiting/travelling
		// miner or a different occupation carrying unused machinery.
		if (_state[troop + 3] == 0 && (_state[troop + 0x19] & 0x80))
			working++;
	}
	const byte total = _state[place + 0x14];
	WRITE_LE_UINT16(_state.begin() + 0x4733, working | (total << 8));
	if (!total || _state[7] > total || !loadHarvesterBank())
		return;
	_state[0xf6] = 1;
	selectHarvesterSequence(1);
	paintDesertHarvester();
	if (_state[7] > working)
		return;
	// C412 saves the already-drawn parked machine, not an empty desert.
	_harvesterBackdrop.resize(kScreenWidth * kSceneHeight);
	_harvesterPage.resize(kScreenWidth * kSceneHeight);
	memcpy(_harvesterBackdrop.begin(), _framebuffer1, _harvesterBackdrop.size());
	memcpy(_harvesterPage.begin(), _framebuffer1, _harvesterPage.size());
	selectHarvesterSequence(0);
	_harvesterActive = true;
	_harvesterTime = _system->getMillis() / 5;
	playSoundEffect(5); // 4D43/AB15 requests the bank's infinite SN5.VOC loop.
}

/** 4BDF/4C45: one zero-terminated frame, with the asset's own part lists. */
void DuneEngine::paintDesertHarvester() {
	const byte *bank = _harvesterBank.begin();
	const uint size = _harvesterBank.size();
	uint at = READ_LE_UINT16(_state.begin() + 0x486a);
	if (at >= size)
		return;
	if (bank[at] == 0xff)
		at = READ_LE_UINT16(_state.begin() + 0x4868);
	const uint parts = READ_LE_UINT16(_state.begin() + 0x486c);
	const int x = READ_LE_INT16(_state.begin() + 0x486e);
	const int y = READ_LE_INT16(_state.begin() + 0x4870);
	const Common::Rect clip(x, y, MIN<int>(x + 173, kScreenWidth), MIN<int>(y + 127, kSceneHeight));
	openSpriteSheet("MOIS.HSQ", false);
	while (at < size && bank[at] != 0) {
		uint part = bank[at++];
		if (part == 1 && at < size)
			part = 0x100 + bank[at++];
		if (part < 2 || parts + 2 * (part - 2) + 2 > size)
			return;
		uint p = parts + READ_LE_UINT16(bank + parts + 2 * (part - 2));
		for (; p + 2 < size && bank[p]; p += 3)
			currentSpriteSheet->drawMapSprite(bank[p] - 1, x + bank[p + 1], y + bank[p + 2], clip);
	}
	WRITE_LE_UINT16(_state.begin() + 0x486a, at + 1);
}

/** DA25/D9D2/4BB9: one callback per due poll, even after a long stall. */
bool DuneEngine::pollDesertHarvester(uint16 ticks) {
	if (!_harvesterActive)
		return false;
	const uint16 elapsed = ticks + _harvesterRemainder; // D9D2's addition wraps in AX.
	_harvesterRemainder = elapsed % 16;
	if (elapsed < 16 || !_inDesert || _flyingTo >= 0 || _onWorld || _onGlobe || _onMixer ||
			_inCabin || _menuKind != kMenuDesert || READ_LE_UINT16(_state.begin() + 0x47c6))
		return false;
	const Common::Rect clip(READ_LE_UINT16(_state.begin() + 0x4860),
		READ_LE_UINT16(_state.begin() + 0x4862), READ_LE_UINT16(_state.begin() + 0x4864),
		READ_LE_UINT16(_state.begin() + 0x4866));
	_graphics.setFrameBuffer(_framebuffer2);
	for (int y = clip.top; y < clip.bottom; y++)
		memcpy(_framebuffer2 + y * kScreenWidth + clip.left,
			_harvesterBackdrop.begin() + y * kScreenWidth + clip.left, clip.width());
	paintDesertHarvester();
	// 4BB9 presents through C4F0/1797: the panel crest overlaps the
	// landscape and must be repainted AFTER the working machine. Keep it
	// out of the parked backing, and copy only the callback's rectangle;
	// drawing the complete face on the display would also touch the menu.
	if (!_state[0x227d] && clip.bottom >= 137 && clip.right >= 126 && clip.left < 194)
		drawFace();
	for (int y = clip.top; y < clip.bottom; y++) {
		memcpy(_framebuffer1 + y * kScreenWidth + clip.left,
			_framebuffer2 + y * kScreenWidth + clip.left, clip.width());
		memcpy(_harvesterPage.begin() + y * kScreenWidth + clip.left,
			_framebuffer2 + y * kScreenWidth + clip.left, clip.width());
	}
	_graphics.setFrameBuffer(_framebuffer1);
	return true;
}

void DuneEngine::updateDesertHarvester() {
	const uint32 now = animationClockTicks();
	const uint16 elapsed = animationElapsedTicks(_harvesterTime, now);
	_harvesterTime = now;
	if (pollDesertHarvester(elapsed)) {
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kSceneHeight);
		_system->updateScreen();
	}
}

bool DuneEngine::validDesertHarvesterState() {
	if (!loadHarvesterBank() || _harvesterRemainder >= 16 || !_inDesert || _flyingTo >= 0 ||
			_onWorld || _inCabin || _onMixer || _harvesterBackdrop.size() != kScreenWidth * kSceneHeight ||
			_harvesterPage.size() != kScreenWidth * kSceneHeight)
		return false;
	const uint x = (byte)(5 + (_state[7] << 2));
	const uint16 words[] = {49, (uint16)x, 41, (uint16)MIN<uint>(x + 173, 320), 152,
		0x2a35, 0, 0x29e3, (uint16)x, 41, 0, 0, 0, 0, 173, 127};
	for (uint i = 0; i < ARRAYSIZE(words); i++)
		if (i != 6 && READ_LE_UINT16(_state.begin() + 0x485e + 2 * i) != words[i])
			return false;
	const uint cursor = READ_LE_UINT16(_state.begin() + 0x486a);
	// 36 three-byte frames, plus the final FF loop marker.
	return cursor >= 0x2a35 && cursor <= 0x2aa1 && (cursor - 0x2a35) % 3 == 0;
}

/** A person speaking while Paul is on foot in the desert. */
void DuneEngine::drawDesertDialogue(bool present) {
	const Common::String speech = _speech;
	_speech.clear();
	drawDesert(true, false);
	_speech = speech;

	// Unlike the 8bpp flight movie, the on-foot picture and character banks
	// were designed to share palette ranges.  Draw the character directly over
	// the restored desert so their own partial palette supplies the exact face
	// colours while leaving the sky and terrain intact.  The former drawScene()
	// path cleared the page as an empty room, which was the real cause of the
	// solid black background.
	_graphics.setFrameBuffer(_framebuffer1);
	// 978E/91A0 opens only the close-up bank over the retained landscape.
	// Keep its inherited shared colours: reloading PERS also overwrites
	// SIET1's 224..239 ramp during Chani's night conversation. Modern saves
	// restore the displayed palette before rebuilding this scene.
	// Speaker sixteen is the game's narrator. Resource eighteen happens to be
	// FRM3, but that Fremen bank is not a narrator portrait.
	if (_speakerPerson >= 0 && _speakerPerson < 16)
		drawCurrentSpeakerFigure();

	// Characters are laid down before the panel in an ordinary room.  Repaint
	// it in the same order here: ICONES restores the interface/font palette and
	// the follower portrait which the speaker bank otherwise leaves black.
	drawPanel();
	drawMenu();
	if (!_speech.empty())
		drawSpeech(_speech, kSpeechColour);
	finishLocalSpeakerScene();

	if (present) {
		finishPanelMenuTransition();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/**
 * The ornithopters standing outside a place, which 0x3A24 draws once the place
 * itself is up and only in its first room - the one it is seen from outside.
 * How many there are is the byte at DS:0x46FF, which is the second of the
 * seven a place keeps for what stands at it.
 *
 * Each is three of ORNY.HSQ's sprites, which 0x3AA9 lays out: the body at the
 * corner it is given, the next six across and thirty down from that, and the
 * last four across and fifty down.  The last is one of six, picked by the
 * counter at DS:0x4731 as the machine takes off; at rest that counter is
 * nought and the first of the six is drawn.
 *
 * 0x3A95 puts the first of them at 149 across and 57 down outside a sietch and
 * at 202 across and 73 down outside anywhere else, and 0x3A73 steps each after
 * it seventy across and ten down.
 */
void DuneEngine::drawOrnithoptersHere(byte kindOfPlace) {
	const uint here = ornithoptersHere();
	const uint first = _takingOff ? 1 : 0;
	if (here <= first)
		return;

	int x = kindOfPlace < 0x20 ? 149 : 202;
	int y = kindOfPlace < 0x20 ? 57 : 73;

	for (uint i = 0; i != first; i++) {
		x += 70;
		y += 10;
	}

	for (uint i = first; i != here; i++) {
		drawOrnithopter(x, y, 0);

		x += 70;
		y += 10;
	}

}

/**
 * One machine, out of the four sprites 0x3AA9 lays down for it: the body where
 * it is asked for, the belly six across and thirty down from that, the legs
 * four across and fifty down, and the wing eighty-one back and three up.  The
 * last two are the ones that move: the counter at DS:0x4731 runs the wing
 * through ORNYTK.HSQ's ninth to its twenty-third, and the legs fold away over
 * the six from its third once the counter is past fifteen.
 *
 * The wing was never drawn at all, so a machine standing outside a place was
 * a body on legs with nothing to fly by.
 */
void DuneEngine::drawOrnithopter(int x, int y, uint flap) {
	enum { kBody = 0, kBelly = 1, kFirstLeg = 2, kLegs = 6, kFirstWing = 8, kWings = 15 };

	// 0x3A51 opens ORNY (resource 0x33) for parked machines. Takeoff switches
	// to ORNYTK (resource 0x2A) at 0x4825 and draws all four parts from it.
	// Only ORNYTK has the fifteen unfolding wings at indices 8..22.
	openSpriteSheet(_takingOff ? "ORNYTK.HSQ" : "ORNY.HSQ", false);
	// C305/C30D uses DS:D834: parked and unfolding parts stop at row152.
	const Common::Rect clip(0, 0, kScreenWidth, kSceneHeight);
	currentSpriteSheet->drawMapSprite(kBody, x, y, clip);
	currentSpriteSheet->drawMapSprite(kBelly, x + 6, y + 30, clip);
	const uint phase = _takingOff ? flap : 0;
	const uint leg = phase < kWings ? 0 : MIN<uint>(phase - kWings, kLegs - 1);
	currentSpriteSheet->drawMapSprite(kFirstLeg + leg, x + 4, y + 50, clip);
	currentSpriteSheet->drawMapSprite(kFirstWing + MIN<uint>(phase, kWings - 1), x - 81, y - 3, clip);
}

/**
	* Unfolds the selected ornithopter, folds its legs and lifts it out of the
	* outside view before the travelling desert begins.  The DOS routine runs
	* frames 0..32: frames 0..14 unfold the wing, then the complete composite
	* moves up and left while the legs fold.
 */
void DuneEngine::animateOrnithopterTakeoff(byte kindOfPlace) {
	if (ornithoptersHere() == 0)
		return;

	_takingOff = true;
	const int parkedX = kindOfPlace < 0x20 ? 149 : 202;
	const int parkedY = kindOfPlace < 0x20 ? 57 : 73;
	playSoundEffect(6); // 47F4: unfolding/takeoff uses SN6's silence and loops.

	for (uint frame = 0; frame != 33 && !shouldQuit(); frame++) {
		// Drawing the room with _takingOff set removes only the selected machine
		// and anybody who has boarded it; other parked ornis remain behind.
		// It is not shown yet: the machine goes over it first, and showing the
		// room without it and then again with it made the machine blink once a
		// frame all the way up.
		drawScene(false);

		// This is the DOS arithmetic: movement starts after frame 14.  Each
		// frame draws all four parts at the new origin; drawSprite clips each
		// sprite against the screen, so no part is removed as a whole.
		const uint lift = frame > 14 ? frame - 14 : 0;
		const int x = parkedX - 5 * (int)lift;
		const int y = parkedY - (int)(lift * lift) / 2;
		if (frame == 13 || frame == 26)
			finishSoundEffectLoop(); // 483C/4813 release the current loop, not all loops.
		drawOrnithopter(x, y, frame);

		setCompass(kCompassFlight);
		for (int i = 1; i <= 3; i++)
			_panel[kFirstCompassElement + i].sprite = 0xffff;
		drawPanel();
		drawMenu();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
								  kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(Common::String::format("%s-takeoff%02u.ppm",
											 ConfMan.get("dune_dump").c_str(), frame));

		// 487E/4885 calls3916 once after each frame copy, independently
		// of D9D2's sixteen-tick phase. Do not drain it all at once.
		if (_state[0x46d7]) {
			if (_state[0x46df])
				stepSkyFade();
			else
				stopSkyFade();
		}
		if (hold(100) == kPlaybackAborted)
			break;
	}

	_takingOff = false;
}

/** DOS 488A/48E9: village and Harkonnen-site arrivals run ORNYTK backwards. */
void DuneEngine::animateOrnithopterLanding(byte kindOfPlace) {
	// 5E4F classifies a village as eight and a Harkonnen site as ten. The
	// palace (seven), sietches (six) and forts (nine) take other paths.
	if (!((kindOfPlace >= 0x21 && kindOfPlace < 0x28) || kindOfPlace >= 0x30))
		return;

	_takingOff = true; // Hide the newly parked machine behind the moving one.
	playSoundEffect(7); // 48FD/AB15: landing is the compressed SN7.VOC program.
	const int parkedX = 202;
	const int parkedY = 73;
	// 48FD stores 31 and 47FB calls 4821 before decrementing the phase.
	for (int frame = 31; frame >= 0 && !shouldQuit(); frame--) {
		drawScene(false);
		const int lift = frame > 14 ? frame - 14 : 0;
		if (frame == 13) // 4813's phase-26 release is skipped for negative CL.
			finishSoundEffectLoop();
		drawOrnithopter(parkedX + 5 * lift,
					  parkedY - lift * lift / 2, frame);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
								  kScreenWidth, kScreenHeight);
		_system->updateScreen();
		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(Common::String::format("%s-landing%02d.ppm",
										 ConfMan.get("dune_dump").c_str(), frame));
		// Landing uses the same4821 frame body and direct3916 call.
		if (_state[0x46d7]) {
			if (_state[0x46df])
				stepSkyFade();
			else
				stopSkyFade();
		}
		if (hold(100) == kPlaybackAborted)
			break;
	}
	_takingOff = false;
}

/** 4988/5B5D: a newly prepared inset centers once, not at every movie frame. */
void DuneEngine::resetFlightInset() {
	// 4988/5B5D replaces the same centre used by the strategic selector,
	// not a private flight-only camera. Keep its native cache in step with
	// DS:197C/197E so leaving the selector cannot retain its old pan position.
	_worldLongitude = READ_LE_UINT16(_state.begin() + 4);
	_worldLatitude = clampSurveyLatitude((int8)_state[6], 56);
	WRITE_LE_UINT16(_state.begin() + 0x197c, _worldLongitude);
	WRITE_LE_UINT16(_state.begin() + 0x197e, (uint16)_worldLatitude);
	_state[0x4728] &= 0x80;
	_flightInsetSet = true;
}

/** 4B2B/C46F: the movie receives the retained inset, including its frame. */
void DuneEngine::paintFlightInset() {
	if (_state[0x4728] & 0x80)
		return;
	if (!_flightInsetSet) {
		resetFlightInset();
		drawMapInset();
		return;
	}
	for (int y = 0; y < 64; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + 200,
			_state.begin() + kMapScratchOrigin + y * kScreenWidth + 200, 120);
}

/** 4A1A: even a report/arrival return has already turned the old head gray. */
void DuneEngine::eraseFlightInsetHead() {
	if (!_flightInsetSet || (_state[0x4728] & 0x80) || _flightTrail.empty())
		return;
	const Common::Rect view(204, 4, 316, 60);
	const Common::Point &point = _flightTrail.back();
	const Common::Point old = _map.surveyPosition(READ_LE_UINT16(_state.begin() + 0x197c),
		(int16)READ_LE_UINT16(_state.begin() + 0x197e), (uint16)point.x, point.y, view);
	if (view.contains(old)) {
		_graphics.setFrameBuffer(_state.begin() + kMapScratchOrigin);
		openSpriteSheet("ICONES.HSQ", false);
		drawSprite(47, old.x - 1, old.y - 1);
		_graphics.setFrameBuffer(_framebuffer1);
	}
}

/** 4F5C: update the cache AFTER request dispatch, before the next movie copy. */
void DuneEngine::updateFlightInset() {
	if (!_flightInsetSet || (_state[0x4728] & 0x80) || _state[0x11ca])
		return;
	const Common::Rect view(204, 4, 316, 60);
	auto position = [&](const Common::Point &point) {
		return _map.surveyPosition(READ_LE_UINT16(_state.begin() + 0x197c),
			(int16)READ_LE_UINT16(_state.begin() + 0x197e), (uint16)point.x, point.y, view);
	};
	byte *cache = _state.begin() + kMapScratchOrigin;
	_graphics.setFrameBuffer(cache);
	openSpriteSheet("ICONES.HSQ", false);
	if (_state[0x4728]) {
		// 49D9 rebuilds the shared map page, not the already displayed
		// movie. Preserve that working-page rectangle across the painter.
		Common::Array<byte> displayed(120 * 64);
		for (int y = 0; y < 64; y++)
			memcpy(displayed.begin() + y * 120, _framebuffer1 + y * kScreenWidth + 200, 120);
		resetFlightInset();
		_graphics.setFrameBuffer(_framebuffer1);
		drawMapInset();
		for (int y = 0; y < 64; y++)
			memcpy(_framebuffer1 + y * kScreenWidth + 200, displayed.begin() + y * 120, 120);
		_graphics.setFrameBuffer(cache);
	}
	const Common::Point here = position(Common::Point((int16)READ_LE_UINT16(_state.begin() + 4), (int8)_state[6]));
	if (view.contains(here)) {
		if (here.x < 214 || here.x >= 306 || here.y < 10 || here.y >= 54)
			_state[0x4728] = 1; // Recenter on the NEXT movement, not this frame.
		else
			drawSprite(48, here.x - 1, here.y - 1);
	}
	_graphics.setFrameBuffer(_framebuffer1);
}

/** DOS 49A0/4A5A: rebuild the retained viewport, frame, sites and gray route. */
void DuneEngine::drawMapInset() {
	enum {
		kLeft = 204, kTop = 4, kRight = 316, kBottom = 60,
		kFirstPlacePicture = 58, kDestinationPicture = 46, kTrailPicture = 47
	};
	const Common::Rect view(kLeft, kTop, kRight, kBottom);
	if (!_flightInsetSet)
		resetFlightInset();
	const uint16 longitude = READ_LE_UINT16(_state.begin() + 0x197c);
	const int16 latitude = (int16)READ_LE_UINT16(_state.begin() + 0x197e);
	// 49A0 installs DS:148A and paints through B6C3; 5B69 draws four
	// concentric outlines around it, from FC down to F6.
	for (int edge = 0; edge != 4; edge++) {
		const int left = kLeft - 1 - edge, top = kTop - 1 - edge;
		const int right = kRight + edge, bottom = kBottom + edge;
		const byte ink = 252 - 2 * edge;
		_graphics.fillRect(left, top, right + 1, top + 1, ink);
		_graphics.fillRect(left, bottom, right + 1, bottom + 1, ink);
		_graphics.fillRect(left, top, left + 1, bottom + 1, ink);
		_graphics.fillRect(right, top, right + 1, bottom + 1, ink);
	}
	_map.drawTerrainInset(_framebuffer1, kScreenWidth, longitude, latitude, view,
			_state.begin() + kMapScratchOrigin, kMapScratchBytes);
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	openSpriteSheet("ICONES.HSQ", false);
	for (uint i = 0; i != _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		if (!place.isKnown())
			continue;
		const Common::Point at = _map.surveyPosition(longitude, latitude,
				place.longitude, place.latitude, view);
		if (!view.contains(at))
			continue;
		uint sprite = kFirstPlacePicture + placeClass(place.kind);
		if (place.kind < 0x20 && distanceFromPaul(place.longitude, place.latitude) >
				READ_LE_UINT16(_state.begin() + 0x1176))
			sprite += 5;
		drawSpriteCentred(sprite, at.x, at.y);
	}
	// DOS 49D4/49D9 calls 49A0 and then 4A5A for either vehicle. 49A0
	// projects the record at DS:11C5, not the independently steered world
	// point; an open-ground course still retains its bookkeeping place here.
	const uint16 target = READ_LE_UINT16(_state.begin() + 0x11c5);
	if (target && (uint)target + 6 <= _state.size()) {
		const Common::Point at = _map.surveyPosition(longitude, latitude,
				READ_LE_UINT16(_state.begin() + target + 2),
				(int16)READ_LE_UINT16(_state.begin() + target + 4), view);
		if (view.contains(at))
			drawSprite(kDestinationPicture, at.x - 1, at.y - 1);
	}
	// 4A5A walks the same route ring after both vehicle redraws. Its ICONES
	// 47 marks are three pixels square, with an extra top-left bounds test.
	for (uint i = 0; i < _flightTrail.size(); i++) {
		const Common::Point at = _map.surveyPosition(longitude, latitude,
				(uint16)_flightTrail[i].x, _flightTrail[i].y, view);
		if (at.x >= kLeft + 1 && at.x < kRight - 1 &&
				at.y >= kTop + 1 && at.y < kBottom - 1)
			drawSprite(kTrailPicture, at.x - 1, at.y - 1);
	}
	for (int y = view.top; y < view.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + view.left,
				_framebuffer2 + y * kScreenWidth + view.left, view.width());
	// 49A0 switches to C085's DS:4C60 page before drawing, then C07C
	// restores the working page. Retain the inset AND its four frame rings
	// in that shared page: MAP2's last boundary row later reads this backing.
	for (int y = kTop - 4; y < kBottom + 4; y++)
		memcpy(_state.begin() + kMapScratchOrigin + y * kScreenWidth + kLeft - 4,
				_framebuffer1 + y * kScreenWidth + kLeft - 4, kRight - kLeft + 8);
	_graphics.setFrameBuffer(_framebuffer1);
}

/**
 * What the map screen offers once an ornithopter has been taken: the player is
 * to press the place they mean to fly to, or think better of it.  The two are
 * COMMAND1.HSQ's own.
 */
void DuneEngine::showFlightMenu() {
	enum { kSelectDestination = 86 };

	Common::Array<Common::String> choices;
	_menuParts.clear();

	choices.push_back(command(kSelectDestination));
	choices.push_back(command(kCancel));

	for (uint i = 0; i != choices.size(); i++)
		_menuParts.push_back(i);

	// The first row says what to do rather than offering anything.
	Common::Array<uint16> flags;
	flags.resize(choices.size(), 0);
	flags[0] = 0x4000;

	_menuKind = kMenuFlight;
	showMenu(choices, &flags);
}

/**
 * What Paul may say to whoever he is speaking to, which is what the game puts
 * up as it begins. DS:1F7E uses COMMAND1.HSQ word159 for idle TALK TO ME,
 * then word145/146 for COME WITH ME/STAY HERE,149 for WHAT and148 for STOP.
 * D617/D61D swap the first label between words144 and159 during speech.
 */
void DuneEngine::showTalkMenu() {
	showTalkMenu(true);
}

void DuneEngine::showTalkMenu(bool present) {
	if (_menuKind == kMenuRoom || _menuKind == kMenuDesert) {
		_conversationParentMenu = _menu;
		_conversationParentParts = _menuParts;
		_conversationParentFlags = _menuFlags;
		_conversationParentKind = _menuKind;
		_conversationParentFrom = _menuFrom;
	}
	enum {
		kHeading = 158,
		kComeWithMe = 144,
		kStayHere = 145,
		kGiveOrders = 146,
		kStopTalking = 147,
		kWhat = 148,
		kWorkForMe = 149,
		kRallyToMe = 150,
		kOverpower = 155
	};
	enum { kFremen = 14, kChief = 15 };

	// Somebody who is already walking with Paul is asked to stay rather than
	// to come.
	const bool along = _speakerPerson >= 0 && isFollowing((uint)_speakerPerson);

	// 0x90BD picks the second row by who is being spoken to, and it is not
	// the same offer to everybody. A Fremen who is nobody in particular is
	// asked to work for Paul, or to rally to him while DS:0xA bit 0x10 selects
	// that later request; either one asks for his whole troop rather than the
	// man. A chief whose troop is Paul's already is given its orders instead.
	// Anybody else is asked along, or asked to stay where they walk with him
	// already, unless their record marks them as unable to follow.
	//
	// Offering to take a Fremen or a chief along is what put a number with no
	// picture behind it into the two boxes in the panel's foot, and the panel
	// then asked ICONES.HSQ for a sprite past its end every time it drew
	// them: the nine pictures it holds, from its sixty-fifth, are the nine
	// people the game names, Duke Leto to Harah, and nobody else has one.
	//
	// 0x90BD builds the list out of a person's own record, which is not always
	// the one being spoken to: a Fremen whose troop has just been sworn is
	// handed the chief's.
	const int listAs = _listAs >= 0 ? _listAs : _speakerPerson;

	uint second;
	bool secondDisabled = false;

	// 0x90CC replaces every ordinary second row except the Captain's own
	// while he is present and has not yet been handcuffed.
	if (listAs != 12 && _state.size() > 0x10a7 &&
			(READ_LE_UINT16(_state.begin() + 0x12) & 0x1000) != 0 &&
			(_state[0x10a7] & 0x10) == 0)
		second = kOverpower;
	else if (listAs == kChief)
		second = kGiveOrders;
	else if (listAs == kFremen)
		second = _state.size() > 0xa && (_state[0xa] & 0x10) != 0
			? kRallyToMe : kWorkForMe;
	else {
		// 90F7 starts with 4091: record flag 80 leaves COME WITH ME
		// disabled, even when flag 40 is also set. The chief, Fremen and
		// captive-helper overrides above take precedence over this gate.
		const uint flagsAt = 0xfd8 + 16 * (uint)listAs + 15;
		secondDisabled = listAs >= 0 && listAs < 16 && flagsAt < _state.size()
			&& (_state[flagsAt] & 0x80) != 0;
		second = !secondDisabled && along ? kStayHere : kComeWithMe;
	}

	Common::Array<Common::String> choices;
	choices.push_back(command(kHeading));
	choices.push_back(command(second));
	choices.push_back(command(kWhat));
	choices.push_back(command(kStopTalking));

	_menuKind = kMenuTalk;
	_menuParts.clear();
	// The second row keeps which of the offers it is, because what choosing
	// it does is not the same for all of them; the rest are their own number.
	for (uint i = 0; i != choices.size(); i++)
		_menuParts.push_back(i == 1 ? second : i);

	// 1F7E's TALK TO ME row calls 9472 to read on; WHAT calls 9ED5 to
	// repeat the current recording. Both remain enabled command rows.
	Common::Array<uint16> flags(choices.size(), 0);
	flags[1] = secondDisabled ? 0x4000 : 0;
	showMenu(choices, &flags, present, true); // 911A arms before dialogue's D280.
}

/**
 * Writes the list into the panel's middle box.  0xD48A does one row: it puts
 * the pen at 93 and a pixel down from the row's own top, writes the words,
 * and then fills what is left of the row out to 227 with the colour behind
 * them, so the box is made of the rows rather than cleared first.
 */
/**
 * Which entry of the list a row of the panel shows.  The panel holds five
 * rows, and a list with more than that in it keeps its last row for
 * COMMAND1.HSQ's `  Others...` - the game's own way of turning a page, its
 * hundred and sixtieth word - so four are shown at a time.  Without it
 * everything past the fifth was written into the list and could never be
 * reached.
 */
bool DuneEngine::menuTurnsPages() const {
	return _menu.size() > kMenuRows;
}

int DuneEngine::menuEntryOnRow(uint row) const {
	if (!menuTurnsPages())
		return row < _menu.size() ? (int)row : -1;

	// D3CD reserves row4 while more entries remain; D3F6 puts Others
	// immediately after the final page's last entry, leaving later rows empty.
	const uint remaining = _menuFrom < _menu.size() ? _menu.size() - _menuFrom : 0;
	const uint count = MIN<uint>(remaining, kMenuRows - 1);
	if (row == count)
		return kTurnThePage;

	const uint at = _menuFrom + row;
	return row < count ? (int)at : -1;
}

/** D9D2/176B toggles the event selection once per due poll, not per period. */
bool DuneEngine::pollEventMenu(uint16 ticks) {
	if (_state.size() <= 0x4775 || !_state[0x4774])
		return false;
	const uint16 elapsed = ticks + _eventMenuRemainder;
	if (elapsed < 100) {
		_eventMenuRemainder = elapsed;
		return false;
	}
	_eventMenuRemainder = elapsed % 100;
	_state[0x4775] ^= 0xff;
	return true;
}

void DuneEngine::updateEventMenu() {
	const uint32 now = animationClockTicks();
	pollEventMenu(animationElapsedTicks(_eventMenuTime, now));
	_eventMenuTime = now;
	// 176B changes only the phase. The ordinary D50F selection paint after
	// input applies it, without an extra repaint inside the timer callback.
}

bool DuneEngine::talkMenuShowsSpeaking() const {
	return _menuKind == kMenuTalk && _mixer->isSoundHandleActive(_speechHandle);
}

void DuneEngine::drawMenu() {
	enum { kSpeakingHeading = 143, kOthers = 159 };
	_paintedTalkMenuSpeaking = talkMenuShowsSpeaking();
	int routeSelection = -1;
	// 80AC preselects Add a Destination with one or two stops, and
	// Give New Destinations with zero or three. The editor can also
	// repaint the previous troop menu while its survey is opening.
	if (_menuKind == kMenuMoveTroop && !_menuParts.empty() && _menuParts[0] == kAddRouteDestination)
		routeSelection = !_moveDestinations.empty() && _moveDestinations.size() < 3 ? 0 : 1;

	// Every one of the five rows is drawn, whether there is anything on it or
	// not: an empty one is filled with the colour behind and that is what
	// makes the box.
	for (uint row = 0; row != kMenuRows; row++) {
		const int entry = menuEntryOnRow(row);
		const uint16 what = entry >= 0 && (uint)entry < _menuFlags.size()
							? _menuFlags[entry] : 0;

		// A row that only names something is written in its own colour and is
		// never picked out; any other under the pointer is.
		const bool named = entry != kTurnThePage && (what & 0x4000) != 0;
		// D50F bypasses pointer hover while an event is active: 4775
		// alternates row zero and no row, including the silent tableaux.
		const bool event = _state.size() > 0x4775 && _state[0x4774];
		const bool under = !named && ((int)row == routeSelection || (event ? row == _state[0x4775] :
			((int)row == _menuUnder || (what & 0x8000) != 0)));

		const byte ink = under ? (byte)kMenuBackground
						 : named ? (byte)kMenuHeadingColour : (byte)kMenuColour;
		const byte behind = under ? (byte)kMenuColour : (byte)kMenuBackground;

		const int y = kMenuTop + row * kMenuRowHeight + 1;

		// 0xD48A picks the small face before it writes a row.
		_font.setFace(Font::kFaceAlternate);

		Common::String line = entry == 0 && _paintedTalkMenuSpeaking ? command(kSpeakingHeading)
							  : entry == kTurnThePage ? command(kOthers)
							  : entry >= 0 ? _menu[entry] : Common::String();
		// D48A writes one small-font space through D12F before the command.
		// That advances the glyph origin four pixels beyond the fill edge.
		const int glyphLeft = kMenuTextLeft + _font.charWidth(' ');
		while (!line.empty() && _font.stringWidth(line) > kMenuFillRight - glyphLeft)
			line.deleteLastChar();

		// The original's letters are laid on the colour behind them - each
		// cell of the text is filled with it as the glyph is cut out - so the
		// row is filled first and the words written over it, rather than the
		// fill starting where the words end.  Without it a row written dark
		// on light lost the light behind its words.
		_graphics.fillRect(kMenuTextLeft, y, kMenuFillRight, y + 7, behind);

		_font.drawString(_framebuffer1, glyphLeft, y, line, ink);
	}
}

/** D911/5C76: strategic picking after the active controls have been tested. */
void DuneEngine::pressStrategicMap(const Common::Point &at) {
	// 5C76 removes the opening legend before testing this same click
	// against the troops and places underneath it.
	dismissMapTitle(); // 5BEB/C6AD, not a new terrain/actor pass.
	// 5C7C/D6FE consumes clicks inside the active report before looking
	// for actors or places underneath it. Equipment/survey cards above
	// have their own handlers; this is the remaining contact window.
	if (mapContactContains(at) || mapInformationContains(at))
		return;
	// A rallied troop is contacted by pressing its standing Fremen figure on
	// the map.  This is the route the original interface uses immediately
	// after the stillsuit-maker's troop joins Paul.
	if (_menuKind == kMenuMap || _menuKind == kMenuPlace ||
			_menuKind == kMenuDensity || hasTroopContact()) {
		const int troop = troopAt(at);
		if (troop >= 0) {
			// 872C does not fall through to a place when a troop cannot
			// yet be contacted. Before telepathy, only Paul's site works.
			if (canSelectMapTroop(troop)) {
				// 872C's reach gate precedes 8685/58FA: an unreachable
				// known actor leaves MAP2 open; an eligible click closes it
				// before contact. 6946/troopAt rejects unmet actors earlier.
				// Do not select a site below a known but unreachable troop.
				if (_menuKind == kMenuDensity) {
					showMapMenu(false);
					_state[0x1954] = _map.troops()[troop].id; // 8747, even if 6906 refuses.
					_state[0x46d8] = 1; // 8685's immediate old-window teardown.
				}
				if (hasTroopContact() && _orderingTroop == (uint)troop) {
					// 874D goes directly to 7C02, without 780A/D323.
					// Refresh the report under the current occupation/menu
					// list instead of discarding that child for troop orders.
					sayTroopInformation(false);
				} else {
					if (hasTroopContact())
						finishTroopContact();
					showTroopContact((uint)troop, true);
				}
			}
			return;
		}
	}

	const int place = placeAt(at);

	// A place is flown to at once where an ornithopter has already been
	// taken, which is what SELECT DESTINATION ON MAP is asking for.  Open
	// ground is a destination of its own, the same as it is through the
	// cockpit window: a course that had set itself down over chosen sand
	// could otherwise never be given a new one, and there is no place at
	// all within the window over most of the world.
	if (_menuKind == kMenuFlight) {
		if (place >= 0) {
			_onWorld = false;
			_speech = "";
			takeOffFor(place);
			return;
		}

		int across = 0, down = 0;

		if (worldPointsAt(at, &across, &down)) {
			const uint16 here = _state.size() > 0x114f
				? READ_LE_UINT16(_state.begin() + 0x114e) : 0;
			const uint from = here >= 0x100 ? (here - 0x100) / 28
											: _cameFromPlace;

			if (from < _map.places().size()) {
				_onWorld = false;
				_speech = "";
				takeOffFor(from, false, (uint32)across * 65536u / Map::kEquatorWidth,
						down - Map::kEquator);
			}
		}

		return;
	}

	// Otherwise the place is named and the two rows that go with it are
	// put up, which is 0x941D's way of reaching a place from the maps.
	if (_menuKind == kMenuDensity)
		showMapMenu(false); // 5CCA closes MAP2 after checking the old mode.
	if (place >= 0 && place != _pressedPlace)
		showPlacePressed((uint)place);
	else {
		// 5CCA closes place/compact reports on empty ground or a second
		// press on the selected place. The first click is not forwarded.
		if (hasTroopContact()) {
			armPanelMenuTransition(); // 5CDA precedes 8763's report close.
			dismissTroopContact(_troopOrdersFromMap);
			return;
		}
		if (_pressedPlace >= 0 || mapTroopCard() >= 0)
			dismissMapInformation();
		// With no window held, 5CCA's close calls are no-ops. Do not
		// expose unrequested terrain or replace C412's actor background.
	}
}

/**
 * A press of the pointer at a point of the screen, wherever the player is.
 * Kept apart from the loop that reads the events so that a way through the
 * game can be walked without a hand on the mouse.
 */
void DuneEngine::pressAt(const Common::Point &at) {
	PanelMenuAction menuAction(*this);
	if (_menuKind == kMenuBook) {
		pressBook(at);
		return;
	}
	if (showingDefeat()) {
		const int row = at.y >= kMenuTop ? menuRowAt(at) : -1;
		if (row >= 0)
			chooseFromMenu(row);
		return;
	}
	// The mixer panel stands over everything while it is up, and takes every
	// press until it is put away.
	if (_onMixer) {
		pressMixer(at);
		return;
	}
	if (_state[0x4774] && (_menuKind == kMenuProspectorBriefing ||
			_menuKind == kMenuPalaceGathering)) {
		// D89B routes completed clicks to 1707, not the map handlers:
		// any point advances the script except strictly inside 1B56's WHAT
		// rectangle when 1FAE offers replay. Silent tableaux use 1FBA, so
		// even that point advances them. Do not reach map/room controls
		// behind the active script's continuation panel.
		const byte *box = _state.begin() + 0x1b56;
		const bool repeat = _menu.size() > 1 &&
			at.x > READ_LE_UINT16(box) && at.y > READ_LE_UINT16(box + 2) &&
			at.x < READ_LE_UINT16(box + 4) && at.y < READ_LE_UINT16(box + 6);
		chooseFromMenu(repeat ? 1 : 0);
		return;
	}
	if (_localRoomRequestPending) {
		// 96F1 is still inside its automatic paragraph, not a fresh
		// conversation. WHAT may replay it; other clicks advance its pages.
		if (at.y >= kMenuTop && menuRowAt(at) == 2)
			repeatSpeech();
		else
			sayRestOfLine();
		return;
	}
	if (_pendingBoardingPlace >= 0) {
		stopSpeech();
		finishVehicleBoarding();
		return;
	}
	if (_pendingRoomExit >= 0) {
		// ABD5/A7C2 owns the completed paragraph's speech wait. A press
		// skips that recording, not the requested exit or its cancellation.
		stopSpeech();
		finishRoomDeparture();
		return;
	}
	if (_pendingRoomCompass >= 0) {
		finishDeferredRoomCompass();
		return;
	}
	// The paragraph/voice wait owns every click until it returns. Opening
	// the book first discards its pages while leaving the pending action or
	// departure active, and can move the room underneath the book.
	if (!_onWorld && !_onGlobe && !_inCabin && !_takingOff &&
			(_menuKind == kMenuRoom || _menuKind == kMenuDesert || _menuKind == kMenuTalk) &&
			at.x > 24 && at.x < 69 && at.y > 155 && at.y <= 176 && !_state[0x11c9]) {
		openBook();
		return;
	}

	// A distant operational report is modal like the original transmission:
	// any acknowledgement returns to the room it interrupted.
	if (_menuKind == kMenuTroopNotice) {
		dismissTroopNotice();
		return;
	}
	if (_menuKind == kMenuMessageView && at.y < kSceneHeight) {
		// The active transmission's image acknowledges the first command
		// (D443/2993: Viewed), not ordinary dialogue continuation. 2997 is
		// the viewer's menu-stack cleanup; after Viewed clears the caller,
		// its early return preserves request six.
		closeMessage(6);
		return;
	}

	const PanelElement &mapCentre = _panel[kFirstCompassElement];
	if (!_onGlobe && !_surveyDragging &&
			(_onWorld || _inCabin || _menuKind == kMenuWormMap) &&
			mapCentre.handler == 0x5b05 && (mapCentre.flags & 0xa0) == 0x80 &&
			panelControlContains(at, mapCentre.x, mapCentre.y, mapCentre.x2, mapCentre.y2)) {
		if (_menuKind == kMenuMoveTroop && hasSurveyMap()) {
			// 5B05/541F centres the route inset on the retained strategic camera.
			_surveyLongitude = _worldLongitude;
			_surveyLatitude = clampSurveyLatitude(_worldLatitude, kRouteMapHigh);
			refreshSurveyWindow();
		} else {
			// 5B0D/8850 centres on Paul and invokes the current map redraw.
			lookAtPlayer();
			if (_menuKind == kMenuWormMap)
				drawWormMap();
			else if (_inCabin)
				showCabin();
			else
				refreshStrategicMap();
		}
		return;
	}

	// The compact selector is modal over the desert. Its pan controls and
	// Cancel are active, but no click walks through the landscape beneath it.
	if (_menuKind == kMenuWormMap) {
		const int pan = mapPanDirection(at);
		if (pan >= 0) {
			scrollWorld(pan);
			return;
		}
		int across = 0, down = 0;
		if (wormMapPointsAt(at, &across, &down)) {
			const int place = cabinPlaceAt(at);
			if (!confirmCabinDestination(cabinDestinationAt(at)))
				return;
			if (_flyingTo >= 0) {
				// 4944 updates an existing course, not 4748/VER's
				// initial departure. Retain the live position and vehicle.
				takeOffFor(place >= 0 ? (uint)place : _cameFromPlace, false,
						place >= 0 ? -1 : across, place >= 0 ? -1 : down);
			} else {
				takeWormTo(across, down, place);
			}
			return;
		}
		const int row = menuRowAt(at);
		if (row >= 0)
			chooseFromMenu((uint)row);
		return;
	}

	// Globe virtual edges/compass belong to its own callbacks, never to
	// the retained strategic/room handlers beneath it.
	if (_onGlobe) {
		const int button = globeControlAt(at);
		if (button == 0) {
			returnToPlayerMapFromGlobe();
		} else if (button >= 1 && button <= 4) {
			workGlobeCompass(button - 1);
		} else if (button == 5) {
			glideToPlayerOnGlobe(); // BA9E's centre stays on the globe.
		} else {
			uint16 longitude;
			int latitude;
			if (_menuKind == kMenuGlobe && _map.globeWorldPosition(at,
					_globeSpin, _globeTilt, longitude, latitude)) {
				// BC64 chooses this point, not Paul, and paints once before
				// 5A3D restores the strategic map and registers its actors.
				lookAtWorld(longitude, latitude);
				turnGlobe(0, 0);
				showMapFromGlobe(at);
				return;
			}
			const int row = menuRowAt(at);
			if (row >= 0)
				chooseFromMenu(row);
		}
		return;
	}

	// D866/D6B7 resolves a viewport-edge arrow before map destination or
	// actor/site picking, including the south strip up to y=154.
	const int mapPan = mapPanDirection(at);
	if (mapPan >= 0) {
		scrollWorld(mapPan);
		return;
	}

	// Pressing the world itself, which is only the picture and not the panel;
	// a press in the panel belongs to the list.
	// A press inside the cabin's own window is a press on the world, and
	// anywhere else on the instruments is let be.
	if (_inCabin && at.y < kSceneHeight) {
		int longitude = 0, latitude = 0;

		if (!cabinWorldPointAt(at, &longitude, &latitude))
			return;

		int place = cabinPlaceAt(at);
		bool toPoint = false;
		// Anywhere else on the map is flown to as a point of the world: the
		// machine is bound for the ground the pointer names rather than for
		// anything standing on it.  The place the flight is booked against is
		// only bookkeeping then, so wherever the player set out from will do.
		if (place < 0) {
			place = (int)_cameFromPlace;

			const uint16 here = _state.size() > 0x114f
								? READ_LE_UINT16(_state.begin() + 0x114e) : 0;

			if (here >= 0x100)
				place = (int)((here - 0x100) / 28);

			toPoint = place >= 0 && (uint)place < _map.places().size();

			if (!toPoint)
				place = -1;
		}
		debugC(1, kDebugScript, "cabin press at %d,%d is world %d,%d, place %d",
			   at.x, at.y, longitude, latitude, place);

		if (place >= 0) {
			if (!confirmCabinDestination(cabinDestinationAt(at)))
				return;
			_onWorld = false;
			_inCabin = false;
			_speech = "";
			takeOffFor((uint)place, false, toPoint ? longitude : -1, latitude);
		}

		return;
	}

	if (_surveyDragging || beginSurveyDrag(at))
		return;

	if (_onWorld && hasSurveyMap() && at.y < kSceneHeight && surveyLegendAt(at)) {
		_state[0x4722] ^= 0xff;
		refreshSurveyWindow(); // 596D/5975: retain the strategic map and actors.
		return;
	}

	// MOVE TROOP uses the framed density survey rather than the large map
	// underneath it.  A press on one of that card's sietch marks adds a stop
	// instead of opening the ordinary place/flight menu.
	if (_onWorld && _menuKind == kMenuMoveTroop && at.y < kSceneHeight) {
		const Common::Rect map = moveMapRect();
		// 81F3/D6FE cancel outside the frame; 5955 cancels on its close box.
		// Do not let the same press reach the large map behind the editor.
		const Common::Rect insideFrame(map.left - 4, map.top - 6, map.right + 5, map.bottom + 12);
		if (moveMapCloseAt(at) || !insideFrame.contains(at))
			chooseFromMenu(_menu.size() - 1);
		else
			addMoveDestination(at);
		return;
	}

	if (_onWorld && hasSurveyMap() && at.y < kSceneHeight) {
		// 5C7C/D6FE sends only points strictly inside standalone MAP2 to
		// 5923. Outside points still reach the strategic actor/site handlers;
		// the prospectors' scripted handover remains a separate modal view.
		if (_menuKind != kMenuDensity)
			return;
		const Common::Rect frame = surveyFrameRect();
		if (frame.contains(at) && at.x > frame.left && at.y > frame.top) {
			// An inset site recentres the large map, not a flight submenu for
			// the strategic site under it. Covered empty ground is also inert.
			const int place = surveyPlaceAt(at);
			if (place >= 0) {
				// 5933 releases MAP2 without painting its old rectangle.
				// 8850 rebuilds the strategic actors even at the same centre.
				_state[0x46eb] &= ~0x40;
				WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0);
				lookAtWorld(_map.places()[place].longitude, _map.places()[place].latitude);
				refreshStrategicMap();
			} else if (moveMapCloseAt(at)) {
				closeSurveyWindow(); // 5955 uses 58FA and keeps the parent map.
				_menuKind = kMenuMap;
			}
			return;
		}
	}

	// The equipment report is a picture-based picker: its icons sit above the
	// ordinary panel and do not belong to the map underneath them.
	if (_onWorld && _menuKind == kMenuEquipment && at.y < kSceneHeight) {
		const int item = equipmentAt(_orderingTroop, at);
		if (item >= 0)
			changeEquipment(_orderingTroop, (byte)item);
		else {
			const Common::Rect spare = equipmentCardRect(true);
			const bool unused = at.x > spare.left && at.x < spare.right &&
				at.y > spare.top && at.y < spare.bottom;
			// 5CA5/D2E2 invokes the equipment menu's 7D68 close callback.
			// Clicking outside both cards applies transfers, but does not
			// forward this click to the terrain or dismiss the troop contact.
			if (!mapContactContains(at) && !unused)
				finishEquipment();
		}
		return;
	}

	if (_onWorld && !_onGlobe && at.y < kSceneHeight) {
		pressStrategicMap(at);
		return;
	}

	// The panel's right hand box carries the controls, and they were drawn
	// but never worked: the four ways out of a room, and while somebody is
	// being carried the three buttons of the journey - turn one way, go on,
	// turn the other.
	{
		const int button = compassButtonAt(at);

		if (button >= 0) {
			const bool carried = _state.size() > 0x11c9
								 && (_state[0x11c9] & 0xf) != 0;

			// The cabin uses D72D's active world template, whose four arrow
			// rectangles differ from the room compass. Handle them before the
			// room-exit branch can turn a pan into a walk through a doorway.
			if (_inCabin && button >= 1 && button <= 4) {
				scrollWorld(button - 1);
				return;
			}

			if (_inDesert && carried && _flyingTo >= 0 && !_inCabin &&
					!_onGlobe && _menuKind == kMenuDesert && !_flightDiscoveryPrompt) {
				enum { kTurn = 4, kLeftButton = 1, kOnButton = 2, kRightButton = 3 };

				if (button == kLeftButton)
					turnFlight(-kTurn);
				else if (button == kRightButton)
					turnFlight(kTurn);
				else if (button == kOnButton) {
					_state[0x11ca] = 0; // 4F09/4AC4 resumes without resetting 4729.
					pollFlightMovement(); // Same input word and timer as this press.
				}

				return;
			}

			if (!_onWorld && !_inDesert && button >= 1 && button <= 4) {
				static const Cursor kWays[4] = {
					Cursor::North, Cursor::East, Cursor::South, Cursor::West
				};

				goThroughEdge(kWays[button - 1]);
				return;
			}
		}
	}

	const bool strategicClick = _onWorld && !_onGlobe && !_inCabin &&
		(_menuKind == kMenuMap || _menuKind == kMenuPlace || _menuKind == kMenuDensity ||
		 ((_menuKind == kMenuTroopContact || _menuKind == kMenuOrders) && hasTroopContact()));
	if (!_menu.empty()) {
		const int row = menuRowAt(at);

		if (row >= 0) {
			const int entry = menuEntryOnRow(row);
			const bool disabled = entry >= 0 && (uint)entry < _menuFlags.size() &&
					(_menuFlags[entry] & 0x4000);
			if (disabled && strategicClick) {
				// D48A clears the row's active bit, so D6B7 misses it
				// and D911 sends the click to the strategic map.
				pressStrategicMap(at);
				return;
			}
			if (!disabled || (_menuKind != kMenuDensity && _menuKind != kMenuMoveTroop)) {
				chooseFromMenu(row);
				return;
			}
		}
	}

	const bool retainedPortrait = _speakerPerson >= 0 && _speech.empty() &&
		!_onWorld && !_inCabin && !_takingOff &&
		(_menuKind == kMenuTalk ||
		 ((_menuKind == kMenuRoom || _menuKind == kMenuDesert) && _state[0x11c9] == 0));

	// 9215/92C9 gives the two companion portraits their own dispatch, before
	// 9468's catch-all continuation click. A normal room/desert header permits
	// this only on foot; the ordinary talk header also permits switching to
	// another companion. Other modals and maps do not use that handler.
	if (!_onWorld && !_inCabin && !_takingOff &&
			(_menuKind == kMenuTalk ||
			 ((_menuKind == kMenuRoom || _menuKind == kMenuDesert) && _state[0x11c9] == 0))) {
		const int person = companionAt(at);
		if (person >= 0 &&
				(person != _speakerPerson || _speech.empty())) {
			// 945B reopens the same caller only with no displayed paragraph.
			// DOS tracks its text-control pointer in 479E; native room text uses
			// _speech instead, so the saved DOS pointer is not authoritative here.
			// 925C closes the previous talk menu before taking up a different
			// caller. Settle pending COME/STAY answers, but do not draw the floor
			// between speakers: an active captive's attempt mask must survive.
			if (_speakerPerson >= 0 && person != _speakerPerson)
				letGoOfPerson();
			takeUpPerson((uint)person);
			recordPersonConversation((uint)person);
			enterDialogue((uint)person, kWhatHeSays);
			showTalkMenu(false);
			sayNextLine();
			return;
		}
	}

	if (retainedPortrait) {
		Common::Array<SpeakerSprite> sprites;
		Common::Rect picture;
		collectSpeakerSprites(_speakerPerson, _speakerFrame, _speakerAnimation,
			-1, sprites, nullptr, false, nullptr, &picture);
		// D6FE uses strict edges. 945B goes through 93AA when 479E is zero:
		// reopen this caller's ordinary news, rather than read its old cursor.
		if (at.x > picture.left && at.x < picture.right &&
				at.y > picture.top && at.y < picture.bottom) {
			const uint person = _speakerPerson;
			takeUpPerson(person);
			recordPersonConversation(person);
			enterDialogue(person, kWhatHeSays);
			showTalkMenu(false);
			sayNextLine();
			return;
		}
	}

	// The planet in the corner of the panel is the way into the game's own
	// menu.  It stands in the box the first entry of the table at DS:0x1AE6
	// keeps, and it is only there while the maps are up: in a room that box
	// holds the book instead.
	if (_onWorld && _menuKind != kMenuGlobe && _menuKind != kMenuLeaving) {
		const PanelElement box = readPanelElement(_staticState.begin() + 0x1ae6);

		if (panelControlContains(at, box.x, box.y, box.x2, box.y2)) {
			showGlobe();
			return;
		}
	}

	// While the maps are up the same four boxes carry the arrows that move the
	// window over the world, just like the viewport-edge arrows handled above.
	if (_onWorld) {
		const int way = compassWayAt(at);

		if (way >= 0) {
			if (_menuKind == kMenuMoveTroop)
				scrollMoveMap(way);
			else
				scrollWorld(way);
			return;
		}
	}

	// D911 passes unused panel space to the active map handler. The
	// strategic picker can still hit actors and sites across the border.
	if (_onWorld && !_onGlobe && !_inCabin && at.y >= kSceneHeight) {
		if (strategicClick) {
			pressStrategicMap(at);
			return;
		}
		if (_menuKind == kMenuEquipment) {
			finishEquipment(); // 5CA5/D2E2 commits pending transfers.
			return;
		}
		if (_menuKind == kMenuMoveTroop) {
			cancelTroopRouteEditor(); // 81F3/8246 takes the normal Cancel path.
			return;
		}
	}

	// The four boxes between the arrows of the compass, which is how the player
	// walks about a place: the handlers they name - 0x3F15, 0x3F1A, 0x3F1F and
	// 0x3F24 - are the same four the edges of the picture go through.  A box
	// whose arrow is not drawn is not pressed either, which is what the flag
	// 0x20 that 0x3066 puts there says.  These were worked out and never
	// asked, so the panel's compass did nothing at all and a room could only
	// be left by pressing the edge of the picture.
	if (!_onWorld && !_inCabin) {
		// The sixth room-compass entry is the small centre mark.  At the palace
		// it is a real button (0x18EE), opening PALPLAN rather than taking one of
		// the four exits.  It is hidden outside the player's home place and in
		// the outside room, exactly where updateCompass hides the sprite as well.
		const PanelElement &centre = _panel[kFirstCompassElement + 5];
		const uint16 where = _state.size() > 5
			? READ_LE_UINT16(_state.begin() + 4) : 0;
		if ((centre.flags & 0x20) == 0 && (where >> 8) == 0x20 &&
			(where & 0xff) != 1 && at.x >= centre.x && at.x < centre.x2 &&
			at.y >= centre.y && at.y < centre.y2) {
			showPalacePlan();
			return;
		}

		const int way = compassWayAt(at);

		if (way >= 0 && (_panel[kFirstCompassElement + 1 + way].flags & 0x20) == 0) {
			if (_inDesert)
				stepThroughDesert(way);
			else
				goThroughEdge(way == 0 ? Cursor::North : way == 1 ? Cursor::East
							  : way == 2 ? Cursor::South : Cursor::West);

			return;
		}
	}

	const Cursor under = cursorForPosition(at);

	// Older native saves may contain the erroneously presented part-135
	// card. Allow it to be dismissed without consuming the real palace notice.
	if (_inDesert && _dialoguePart == 135 && _speakerPerson == 16) {
		stopSpeech();
		_speakerPerson = -1;
		_speakerFrame = 0;
		_speakerAnimation = 0;
		_speech = "";
		_speechParts.clear();
		showDesertMenu();
		return;
	}

	// 9468/D41B returns immediately for the modal answer header 1FFE.
	// A scene click cannot advance even a continuation page while Paul must
	// choose ARGUE, ACCEPT or REFUSE; only the command rows answer the offer.
	if (_menuKind == kMenuShipmentResponse)
		return;

	// While a line is on the screen a press reads on, which is what 0x9468 does
	// with the box the line is written in; the room behind it is not pressed at
	// all.  A press used to go looking for whoever was standing where it fell,
	// which with a person already up meant that a press anywhere started
	// talking to whoever stood on the first mark.
	if (_speakerPerson >= 0 && !retainedPortrait) {
		// D6B7 dispatches the registered picture's continuation handler,
		// not inactive panel controls or empty panel space. Menu rows and
		// active compass/companion controls have already been handled above.
		// Falling through from a hidden arrow incorrectly consumed the news
		// after an automatic doorway refusal and dismissed its paragraph.
		if (at.y >= kSceneHeight)
			return;
		// 0x9468 turns away only the one region at 0x1FFE and reads on for
		// every other press: the box the line is written in covers the whole
		// picture, whatever the pointer would otherwise be over.  Asking for
		// the plain pointer here meant a line could never be read on from the
		// open desert, where the middle of the screen is a walking arrow -
		// and a fresh person selection starts the part again, so
		// Chani's night, which is the twelfth record of her part, could not
		// be reached at all and the story stopped at sixty-nine.
		if (_menuKind == kMenuTalk)
			continueRoomConversation();
		else if (!sayRestOfLine())
			sayNextLine();

		return;
	}

	// Out in the open there is nobody to speak to and no door to go through:
	// an edge is another step over the world.  Each press is one step, and
	// each step shows another stretch of the desert; the heading a press asked
	// for used to be kept and nothing ever acted on it, so walking out there
	// with the mouse did nothing at all.
	if (_inDesert) {
		if (under == Cursor::North)
			stepThroughDesert(0);
		else if (under == Cursor::East)
			stepThroughDesert(1);
		else if (under == Cursor::South)
			stepThroughDesert(2);
		else if (under == Cursor::West)
			stepThroughDesert(3);

		return;
	}

	const int standing = standingAt(at);

	if (standing >= 0) {
		// The same as choosing them out of the room's menu: they leave the mark
		// they stood on, are drawn large, and say whatever news they have.
		const Standing &person = _sceneMarkers[standing];
		const uint who = person.who;

		if (who == 15 && person.chief != 0xff && _state.size() > 0x476c)
			_state[0x476c] = person.chief;

		startRoomConversation(who);
	} else if (under != Cursor::Pointer) {
		goThroughEdge(under);
	}
}

/** Log changed panel rows once, including redraw-heavy cabin and map views. */
void DuneEngine::debugMenuIfChanged() {
	bool changed = _menu.size() != _lastDebugMenu.size();
	if (!changed) {
		for (uint i = 0; i != _menu.size(); i++) {
			if (_menu[i] != _lastDebugMenu[i]) {
				changed = true;
				break;
			}
		}
	}
	if (!changed)
		return;
	for (uint i = 0; i != _menu.size(); i++)
		debugC(1, kDebugScript, "  row %u: %s", i, _menu[i].c_str());
	_lastDebugMenu = _menu;
}

void DuneEngine::showMenu(const Common::Array<Common::String> &choices,
						  const Common::Array<uint16> *flags, bool present) {
	showMenu(choices, flags, present, false);
}

void DuneEngine::showMenu(const Common::Array<Common::String> &choices,
						  const Common::Array<uint16> *flags, bool present, bool armTransition) {
	if (armTransition)
		armPanelMenuTransition();
	if (_menuKind != kMenuMap)
		closeMapTitle();
	closeMapTroopCard();
	_menu = choices;

	_menuFlags.clear();
	if (flags)
		_menuFlags = *flags;
	else
		_menuFlags.resize(choices.size(), 0);
	_menuFrom = 0;
	if ((_menuKind == kMenuRoom || _menuKind == kMenuDesert) && choices.size() > kMenuRows) {
		const uint from = _state[0x1f0f] / 4;
		if (from < choices.size() && from % (kMenuRows - 1) == 0)
			_menuFrom = from;
	}
	_menuUnder = menuRowAt(_system->getEventManager()->getMousePos(), true);

	debugMenuIfChanged();

	// A transition prepares its target menu without putting that page on
	// screen yet. The caller then draws the target scene without presenting.
	if (!present)
		return;

	// Whatever the player is looking at, the room or the world, with the list
	// drawn into the panel over it.
	redraw();
	finishPanelMenuTransition();

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

int DuneEngine::menuRowAt(const Common::Point &position, bool hover) const {
	// D6B7 uses strict horizontal/top edges and an inclusive bottom edge
	// (it decrements BX before comparing the bottom). Subtract one before
	// division: 160..167 is row0, and 192..199 is row4, not an empty row5.
	// D50F includes the left edge when highlighting a row.
	const int left = kMenuLeft + (hover ? 0 : 1);
	if (_menu.empty() || position.x < left || position.x >= kMenuRight ||
			position.y <= kMenuTop)
		return -1;

	const int row = (position.y - kMenuTop - 1) / kMenuRowHeight;
	// D3D9/D3F6 counts displayed entries, including Others but not the
	// empty rows below it. Those rows must not dispatch/reset an action.
	const uint remaining = _menuFrom < _menu.size() ? _menu.size() - _menuFrom : 0;
	const uint rows = menuTurnsPages() ? MIN<uint>(remaining, kMenuRows - 1) + 1 : (uint)_menu.size();

	return (row >= 0 && row < (int)MIN<uint>(rows, kMenuRows)) ? row : -1;
}

/** D881/D886: hover runs on an idle poll, never on a button edge. */
void DuneEngine::updateMenuHover(const Common::Point &position, bool buttonEdge) {
	if (buttonEdge)
		return;
	bool menuChanged = false;
	if (_menuKind == kMenuGlobe) {
		const bool pickingArea = kGlobeMapArea.contains(position);
		menuChanged = pickingArea != (_menuParts.size() == 1 && _menuParts[0] == kSeeMapOfThisArea);
		if (menuChanged)
			setGlobeMenu(pickingArea, false);
	}
	const int over = _state[0x4774] ? (_state[0x4775] == 0 ? 0 : -1) : menuRowAt(position, true);
	if (menuChanged || over != _menuUnder || _paintedTalkMenuSpeaking != talkMenuShowsSpeaking()) {
		_menuUnder = over;
		drawMenu();
		_system->copyRectToScreen(_framebuffer1 + 320 * kMenuTop, kScreenWidth,
				0, kMenuTop, kScreenWidth, kMenuRows * kMenuRowHeight);
	}
}

void DuneEngine::chooseFromMenu(uint panelRow) {
	PanelMenuAction menuAction(*this);
	resetTroopNoticeIdle();
	if (_localRoomRequestPending) {
		if (panelRow == 2)
			repeatSpeech();
		else
			sayRestOfLine();
		return;
	}
	if (_pendingBoardingPlace >= 0) {
		stopSpeech();
		finishVehicleBoarding();
		return;
	}
	if (_pendingRoomExit >= 0) {
		stopSpeech();
		finishRoomDeparture();
		return;
	}
	if (_pendingRoomCompass >= 0 && !_state[0x4774]) {
		finishDeferredRoomCompass();
		return;
	}
	if (_menuKind == kMenuTroopNotice) {
		dismissTroopNotice();
		return;
	}

	const int entry = menuEntryOnRow(panelRow);

	// The last row of a list too long for the panel turns the page.
	if (entry == kTurnThePage) {
		_menuFrom += kMenuRows - 1;
		if (_menuFrom >= _menu.size())
			_menuFrom = 0;
		if (_menuKind == kMenuRoom || _menuKind == kMenuDesert)
			_state[0x1f0f] = 4 * _menuFrom; // D423/D429 retain this table's page offset.

		redraw();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		return;
	}

	if (entry < 0 || (uint)entry >= _menuParts.size())
		return;

	const uint row = (uint)entry;
	if (_menuKind == kMenuBook) {
		if (_menuParts[row] == 4)
			closeBook();
		else
			showBookTopic(_menuParts[row]);
		return;
	}
	if (_menuKind == kMenuDefeat || _menuKind == kMenuDefeatExit) {
		const uint action = _menuParts[row];
		if (_menuKind == kMenuDefeatExit) {
			if (action == 183)
				quitGame();
			else
				showDefeatMenu();
		} else if (action == 185) {
			restartAfterDefeat();
		} else if (action == 179) {
			loadGameDialog();
			resetPointerButtons();
			redraw();
		} else if (action == 186) {
			showDefeatMenu(true);
		} else if (action == 148) {
			playDefeatSpeech();
		}
		return;
	}

	// A row that only names something is not a choice.
	if (row < _menuFlags.size() && (_menuFlags[row] & 0x4000))
		return;

	if (_menuKind == kMenuPalacePlan && _menuParts[row] == kRoomPlanDone) {
		showRoomMenu();
		return;
	}

	if (_menuKind == kMenuMirror) {
		enum {
			kLookAway = 156, kSaveGame = 178, kLoadGame = 179,
			kRestartGame = 185, kExitGame = 186
		};

		switch (_menuParts[row]) {
		case kSaveGame:
			saveGameDialog();
			resetPointerButtons();
			showMirror();
			break;
		case kLoadGame:
			loadGameDialog();
			resetPointerButtons();
			redraw();
			break;
		case kExitGame:
			showLeaving();
			break;
		case kRestartGame:
			// The original begins again immediately.  Reusing the engine's
			// initial-data path is safe here; the next event-loop turn will draw
			// the opening palace state from the newly restored memory.
			if (loadStaticData()) {
				_state[0x115]++;
				_state[kHowMusicGoes] = 0;
				_state[kMusicIsOff] &= (byte)~kMusicOffBit;
				const uint16 whose = READ_LE_UINT16(_state.begin() + 6);
				const uint place = (whose >> 8) ? (whose >> 8) - 1 : 0;
				const byte room = (byte)(READ_LE_UINT16(_state.begin() + 4) & 0xff);
				introScene(place, room, READ_LE_UINT16(_state.begin() + 0x12), -1);
				showRoomMenu();
			}
			break;
		default: // Look away
			showRoomMenu();
			break;
		}
		return;
	}

	// The row that takes a list down puts back what the panel showed before
	// it, which is the maps where the troops were reached from.
	if (_menuParts[row] == kGoBack) {
		// 29D4's Cancel restores the room with D316/D280, not a hard cut.
		if (_menuKind == kMenuNewMessages || _menuKind == kMenuSeenMessages) {
			armPanelMenuTransition();
			_speech.clear();
			// 29DA/2DBF rebuilds the room, including its inhibited 35AD
			// resident pass. Restoring only the commands leaves the previous
			// answer counter live after cancelling either message list.
			_state[0x47a6] = 0xff;
			_state[0xdce6] = _state[0x47a4] = _state[0x46df] = 0;
			WRITE_LE_UINT16(_state.begin() + 0x47aa, 0);
			if (!_state[0x2b])
				WRITE_LE_UINT16(_state.begin() + 0x14, 0);
			placeTheWandering();
			drawScene(false);
			showRoomMenu();
			showLocalRoomRequests();
			return;
		}
		// The occupation templates at 215A..21A6 use D2E2: pop this
		// list, retaining the underlying contact and its selected traveller.
		if (_menuKind == kMenuOrders) {
			finishTroopOccupationMenu();
			return;
		}
		_speech = "";

		if (_onWorld)
			showMapMenu();
		else if (_inDesert)
			showDesertMenu();
		else
			showRoomMenu();

		return;
	}

	if (_menuKind == kMenuPalaceGathering) {
		if (row == 1) {
			// DS:1FAE binds WHAT to 9ED5, including a retained recording
			// whose current raw caller found no new paragraph at 9761.
			repeatSpeech();
			return;
		}
		continuePalaceGathering(_menuParts[row]);
		return;
	}

	if (_menuKind == kMenuNewMessages || _menuKind == kMenuSeenMessages) {
		showMessage(_menuParts[row], _menuKind == kMenuSeenMessages);
		return;
	}

	if (_menuKind == kMenuMessageView) {
		if (_menuParts[row] == 1) {
			// DS:1FF2 binds WHAT to 9ED5; retain the remote image.
			repeatSpeech();
		} else {
			// 2993 returns to the room; 29C1 arms its restored command list.
			closeMessage(6);
		}
		return;
	}

	if (_menuKind == kMenuShipmentResponse) {
		// 0xA244's modal list writes one for ACCEPT, two for REFUSE and three
		// for ARGUE.  The dialogue conditions then choose Duncan's next offer or
		// the smuggler's next price.
		// DS:1FFE binds WHAT to 9ED5: replay the current voice without
		// selecting another line or rerunning its action. 9ED5 first finishes
		// the current portrait block through 9985.
		if (_menuParts[row] == 3) {
			repeatSpeech();
			return;
		}

		if (_speakerPerson == 13 && _state.size() > 0x11f2) {
			enum { kAccept = 1, kRefuse = 2, kArgue = 3 };
			byte response = _menuParts[row] == 0 ? kArgue
				: (byte)_menuParts[row];
			const uint16 account = READ_LE_UINT16(_state.begin() + 0x10b4);

			if (account >= 0x10d8 && account + 17 <= _state.size()) {
				if (response == kAccept && _state[0x9d] != 0) {
					// 0x23E6 transfers the selected item to the village's unused
					// equipment and adds its price to this smuggler's unpaid bill.
					_state[account + 2] &= 0x9f;
					const uint price = _state[0x9d];
					_state[0x9d] = 0;
					WRITE_LE_UINT16(_state.begin() + 0x20,
							(uint16)(READ_LE_UINT16(_state.begin() + 0x20) + price));
					const uint oldBill = READ_LE_UINT16(_state.begin() + account + 0x0e);
					WRITE_LE_UINT16(_state.begin() + account + 0x0e,
							(uint16)(oldBill + price));
					if (oldBill == 0)
						_state[0x22]++;
					_state[account + 0x10] = (byte)(hour() >> 4);

					const uint16 itemWord = READ_LE_UINT16(_state.begin() + 0x11f1);
					const uint item = itemWord >= 0xe8 ? itemWord - 0xe8 : 0xffff;
					const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
					if (item < _state[0x1141] && account + 4 + item < _state.size() &&
							here >= 0x100 && here + 0x14 + item < _state.size()) {
						if (_state[account + 4 + item] != 0)
							_state[account + 4 + item]--;
						_state[here + 0x14 + item]++;
						_map.readPlaces(_state.begin(), _state.size());
					}
				} else if (response == kRefuse) {
					// DOS 243E uses E3B7's saved secondary stream. A zero roll
					// falls through to 246B: it becomes an ARGUE response, retaining
					// the offer and setting the bargaining flag for the next line.
					if ((troopRandom(true) & 7) != 0)
						_state[0x9d] = 0;
					else {
						_state[0x9e] |= 0x10;
						response = kArgue;
					}
				} else if (response == kArgue && _state[0x9d] != 0) {
					const byte chance = (byte)(troopRandom(true) & 3);
					if (chance == 0) {
						_state[0x9e] |= 0x10;
					} else {
						_state[0x9e] = (byte)((_state[0x9e] + 1) & 3);
						if ((uint)(chance & 1) + _state[0x1a] >= _state[account + 1])
							_state[0x9d] = 0;
						else
							_state[0x9d] -= _state[0x9d] >> 3;
					}
				}
			}

			_state[0x9f] = response;
			_state[0x1a]++;
			if (response != kArgue)
				showTalkMenu(false);
			sayNextLine();
			return;
		}

		_state[0x9f] = _menuParts[row] == 0 ? 3 : (byte)_menuParts[row];
		_state[0x1a]++; // 2496/2499: every actual answer, never opening or WHAT.
		if (_menuParts[row] != 0)
			showTalkMenu(false);
		sayNextLine();
		return;
	}

	if (_menuKind == kMenuGlobe) {
		switch (_menuParts[row]) {
		case kSeeMapOfThisArea:
			break;
		case kExitGlobe:
			returnToPlayerMapFromGlobe();
			break;
		case kSeeResults:
			// 0xB96B slides the frame off the sides and shows what is behind
			// it; 0xB961 puts it back.
			initialiseResultsGauges();
			slideFrescoDoors(true);
			showResults();
			break;
		case kStandardVision:
			slideFrescoDoors(false);
			showGlobe();
			break;
		case kSaveGame:
			saveGameDialog();
			resetPointerButtons();
			// B35A/D2E2 restores the parent, even after a successful save.
			// Repaint its retained frame; do not run another B98B draw.
			redraw();
			break;
		case kLoadGame: {
			_loadingGlobeMenu = true;
			const bool loaded = loadGameDialog();
			_loadingGlobeMenu = false;
			resetPointerButtons();
			// Successful decode already runs B400 before presenting any saved
			// UI. Cancel only unwinds D2E2, preserving the retained frame.
			if (!loaded)
				redraw();
			break;
		}
		case kExitGame:
			showLeaving();
			break;
		default:
			break;
		}

		return;
	}

	if (_menuKind == kMenuLeaving) {
		enum { kYes = 183 };

		if (_menuParts[row] == kYes) {
			quitGame();
			return;
		}

		// EXIT GAME is offered by three different screens.  The original puts
		// back the one which asked the question when NO is chosen; forcing the
		// globe here made cancelling from the mixer or mirror jump across the
		// game to an unrelated interface.
		if (_onMixer)
			showMixer();
		else if (_onGlobe)
			showGlobe();
		else
			showMirror();
		return;
	}

	if (_menuKind == kMenuMixer) {
		enum {
			kAsACd = 265, kByThePlace = 266, kShuffle = 267,
			kStandardOrder = 268, kMusicOff = 269,
			kDone = 160, kCancel = 162, kExitGame = 186
		};

		const uint what = _menuParts[row];

		switch (what) {
		case kExitGame:
			showLeaving();
			return;

		case kDone:
			// The list is taken down and the mixer with it.
			_state[kRetainedSaidHow] = _state[kSaidHow];
			_mixerOrdering = false;
			_onMixer = false;
			redraw();
			if (_inDesert)
				showDesertMenu();
			else
				showRoomMenu();
			return;

		case kMusicOff:
			// 0xAEAF: the music is off from here on, and 0xAEB7 takes the
			// song that was playing off with it.
			if (_state.size() > kMusicIsOff)
				_state[kMusicIsOff] |= kMusicOffBit;

			stopTheSong();
			break;

		case kByThePlace:
			// 0xAC6E: the place chooses the song again, and 0xAD5E puts the
			// one belonging here on at once.
			if (_state.size() > kMusicIsOff)
				_state[kMusicIsOff] &= (byte)~kMusicOffBit;
			if (_state.size() > kHowMusicGoes)
				_state[kHowMusicGoes] = 0;

			stopTheSong();
			playTheRightSong();
			break;

		case kAsACd:
			// 0xAC7E does not set anything: it asks first whether the nine
			// are to be taken in order or shuffled.
			_mixerOrdering = true;
			showMixer();
			return;

		case kShuffle:
		case kStandardOrder:
			// 0xAC90 and 0xAC97 set the two bits between them, and 0xACAE
			// then starts the nine again from the first of them.
			if (_state.size() > kHowMusicGoes) {
				if (what == kShuffle) {
					_state[kHowMusicGoes] |= (byte)(kCdStyle | kShuffled);
				} else {
					_state[kHowMusicGoes] |= (byte)kCdStyle;
					_state[kHowMusicGoes] &= (byte)~kShuffled;
					orderThePlaylist();
				}
			}

			if (_state.size() > kMusicIsOff)
				_state[kMusicIsOff] &= (byte)~kMusicOffBit;

			stopTheSong();
			playNextOfPlaylist(true);
			break;

		case kCancel:
			_mixerOrdering = false;
			showMixer(); // D2E2 closes only the CD-order submenu.
			return;
		default:
			return;
		}

		// AC6E/AEAF return to the mixer's parent after a music choice;
		// ACAE closes both the CD-order submenu and the mixer itself.
		_state[kRetainedSaidHow] = _state[kSaidHow];
		_mixerOrdering = false;
		_onMixer = false;
		redraw();
		if (_inDesert)
			showDesertMenu();
		else
			showRoomMenu();
		return;
	}

	if (_menuKind == kMenuTroops) {
		showTroopContact(_menuParts[row], true);
		return;
	}

	if (_menuKind == kMenuDiscovery) {
		if (_menuParts[row] == kDiscoveryChange) {
			// 497A/98E6 removes the report before opening the selector.
			// Both command tables have priority F8, so D338 replaces the
			// warning rather than nesting a menu that Cancel could reopen.
			releaseCarriedReportForSelector();
			if ((_state[0x11c9] & 3) == 2) {
				showWormMap(false);
				return;
			}
			_inCabin = true;
			_cabinFromMap = false;
			_cabinTyped = 0;
			_cabinPointerPoint = -1;
			enterCabin();
		} else if (_menuParts[row] == kDiscoveryGo) {
			// 4944 already set the course during the sighting. 97CF gives
			// back the controls without resetting its heading or fraction.
			stopSpeech();
			if (_speakerPerson >= 0 && _speakerPerson < 16) {
				// 97DD/97E1 marks the selected physical person; the carried
				// branch does not settle grounded companion/floor records.
				_state[0xfe7 + 16 * _speakerPerson] |= 0x20;
				_state[0xfe7 + 16 * _speakerPerson] &= (byte)~4;
				WRITE_LE_UINT16(_state.begin() + 0x47e1, 0);
				// 97F2/8C8A also releases the paragraph. A7A5 alone only
				// stops audio; a retained owner would keep B45 actors paused
				// after this warning is gone, including on landing at a fort.
				WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
				// 8C8A/9BAC and the 98B2 presentation callback also release
				// the silent portrait's pose and animation registration.
				releaseSilentLocalSpeaker();
			}
			_state[0x11ca] = 0; // 4ABE, including the caption-only fallback.
			stopFlightMovie(); // 4ABE/37F4 restarts the base landscape film.
			_state[0x4728] = 0; // 37F4/4988 rebuilds the inset at the live position.
			_flightInsetSet = false;
			_flightSkyRestartPending = true;
			_speakerIdleActive = false;
			_speakerIdlePerson = -1;
			_flightDiscoveryPrompt = false;
			_guidedFlightPlace = -1;
			_speakerPerson = -1;
			_speakerFrame = 0;
			_speech.clear();
			showDesertMenu();
		} else {
			// 9ED5 repeats the retained report in the same caller's voice,
			// without selecting another follower or traversing part 132 again.
			repeatSpeech();
			if (_speakerPerson < 0)
				showFlightWarning();
			else
				showFlightDiscovery();
		}
		return;
	}

	if (_menuKind == kMenuOrders) {
		if ((_menuParts[row] & 3) == 3)
			findTroopEquipment(_orderingTroop);
		else if (_menuParts[row] == 5)
			startTroopEspionage(_orderingTroop);
		else
			giveOrder(_orderingTroop, (byte)_menuParts[row]);
		return;
	}

	if (_menuKind == kMenuEquipment) {
		finishEquipment();
		return;
	}

	if (_menuKind == kMenuProspectorBriefing) {
		if (_menuParts[row] == 1) {
			// DS:1FAE's WHAT callback is 9ED5: repeat the current voice,
			// without running another event opcode or changing its text.
			repeatSpeech();
			return;
		}
		if (_troopInfoPage == 0) {
			// 12F8: opcode 0E (13A0) opens the special density window.
			WRITE_LE_UINT16(_state.begin() + 0x477a, 0x12f9);
			WRITE_LE_UINT16(_state.begin() + 0x2220, 0x1fae);
			const uint16 line = prepareProspectorBriefingLine();
			// Briefing progress is independent of the stage-five map-menu gate.
			// Keep it out of DS:0xA: the original uses its 0x10 bit as live story
			// scratch (including the independent WORK/RALLY request wording),
			// and Jessica's next special-powers dialogue requires it clear.
			_hasSpiceDensityMap = true;
			_troopInfoPage = 1;
			// 13A0/88AF calls 80DF, retaining 79EE's inset placement, not
			// the standalone density window's previously dragged origin.
			openTroopSurvey();
			playSpeech(line, 15); // 13BF follows the scripted, silent opening.
			showProspectorBriefing();
		} else if (_troopInfoPage == 1) {
			// 12F9: opcode 10 (13AA/58FA) closes the window before
			// the chief explains how to update the map.
			WRITE_LE_UINT16(_state.begin() + 0x477a, 0x12fa);
			closeSurveyWindow();
			const uint16 line = prepareProspectorBriefingLine();
			playSpeech(line, 15);
			_troopInfoPage = 2;
			_graphics.setFrameBuffer(_framebuffer1);
			drawTroopStatus(_orderingTroop); // 13AA's frame and new paragraph.
			showProspectorBriefing();
		} else {
			// 12FA: FF restores the room/mode saved by 1771 and releases
			// the active event. DOS leaves the cursor on the FF byte.
			const uint16 savedRoom = READ_LE_UINT16(_state.begin() + 0x4776);
			_state[4] = (byte)savedRoom;
			_state[0x46e0] = (byte)(savedRoom >> 8);
			releaseEventScript();
			_troopInfoPage = 0;
			showTroopMenu(false, true); // 1768 -> 780A/D323; retain line 437.
			drawMenu();
			finishPanelMenuTransition();
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
					kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
		return;
	}

	if (_menuKind == kMenuMoveTroop) {
		if (_orderingTroop == 2) {
			switch (_menuParts[row]) {
			case kAddRouteDestination: return; // 80C7 is RET: choose the next place on the map.
			case 91:
				_moveDestinations.clear();
				showMoveTroopMenu(false);
				return;
			case 160:
				commitTroopRoute();
				return;
			default: break;
			}
		}
		cancelTroopRouteEditor();
		return;
	}

	if (_menuKind == kMenuTroopContact) {
		enum {
			kAskForMore = 76, kModifyEquipment = 77, kChangeOccupation = 78,
			kMoveTroop = 79, kNextTroop = 80, kNoMoreOrders = 81,
			kCancel = 162
		};

		switch (_menuParts[row]) {
		case kAskForMore:
			sayTroopInformation();
			break;
		case kChangeOccupation:
			showOrders(_orderingTroop);
			break;
		case kModifyEquipment:
			showEquipment(_orderingTroop);
			break;
		case kMoveTroop:
			_moveDestinations.clear();
			if (_orderingTroop == 2) {
				for (uint i = 0; i < 3; i++) {
					const uint place = READ_LE_UINT16(_state.begin() + 0x11d3 + 2 * i);
					if (!place)
						break;
					_moveDestinations.push_back((place - 0x100) / 28);
				}
			}
			_pointedAtPlace = -1;
			showMoveTroopMenu();
			break;
		case kNextTroop: {
			const int next = nextMapTroop();
			// 872C tests reach only after 86FA has picked the next actor. An
			// unreachable pick leaves this contact in place; it does not skip on.
			if (next >= 0 && canSelectMapTroop((uint)next))
				showTroopContact((uint)next, _troopOrdersFromMap);
			break;
		}
		case kNoMoreOrders:
			// 2122 follows Next Troop with 8770, which stays on the map.
			// The ordinary orders list uses 8763 to check the room origin.
			dismissTroopContact(_troopOrdersFromMap || _menuParts[0] == kNextTroop);
			break;
		case kCancel:
			// 214A's D2E2 pops to the map without going through 8763/8770.
			armPanelMenuTransition();
			dismissTroopContact(true, true);
			break;
		}
		return;
	}

	if (_menuKind == kMenuWaiting) {
		takeUpWaiting(_menuParts[row]);
		return;
	}

	if (_menuKind == kMenuMap || _menuKind == kMenuDensity) {
		if (_menuParts[row] == kMapExit) {
			_onWorld = false;
			teardownStrategicMap(false);
			_state[0x28e7] = _state[0x28e8]; // 187D, not 5ADF's map-only cleanup.
			_state[0xfb] = groundViewMode(_state[0xfb]);
			if (!_inDesert)
				peopleHere(); // 2EB2/3093 restores the physical room context.
			showMapReturnView(_inDesert);

			// Stage sixteen begins with Gurney's immediate summary when the
			// player leaves the map after distributing the second harvester.  It
			// is dialogue record 328, not a synthesized notification, so retain
			// its voice, one-shot mark and normal TALK TO choices.
			if (!_inDesert && storyStage() == 16 && isFollowing(4)) {
				for (uint part = 4 * 8; part != 4 * 8 + 4; part++) {
					if (part >= _dialogue.count())
						break;
					Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
					for (uint line = 0; line != records.size(); line++) {
						Dialogue::Record &notice = records[line];
						if (notice.line != 328 || (notice.flags & 0x80) != 0 ||
							!conditionHolds(notice.condition))
							continue;

						// This follower is represented in the panel rather than on the
						// floor.  The event nevertheless uses SIET scene three's central
						// speaking mark (128,62) for its enlarged backdrop.
						rememberWhereTheyStood(4, 128, 62);
						takeUpPerson(4);
						_dialogueMask = 0x80;
						_dialoguePart = part;
						_dialogueLine = line;
						showTalkMenu(false);
						sayChosenLine();
						return;
					}
				}
			}
		} else if (_menuParts[row] == kMapOrders) {
			// The original menu table at DS:20F4 sends command 98 to 0x86CC.
			// The former text list let Paul reach every recruited troop long
			// before he could contact it remotely.
			contactMapTroop();
		} else if (_menuParts[row] == kMapOrnithopter) {
			// 42D9/4057 returns Paul and carried followers to room one,
			// then 186B closes the maps before the ordinary 42E9 boarding.
			// Cancel therefore returns to the exterior, not this map.
			const uint16 outside = (READ_LE_UINT16(_state.begin() + 4) & 0xff00) | 1;
			moveMatchingFollowers(outside, READ_LE_UINT16(_state.begin() + 6));
			WRITE_LE_UINT16(_state.begin() + 4, outside);
			_state[0xd] = _state[0xb];
			_state[0xb] = 1;
			const byte *room = roomRecord(_state[8], 1);
			if (room && room[0])
				_place = room[0] - 1;
			teardownStrategicMap(false);
			_state[0x28e7] = _state[0x28e8]; // 4057 returns through 186B before boarding.
			_state[0xfb] = groundViewMode(_state[0xfb]);
			_onWorld = false;
			showMapReturnView(false);
			boardOrnithopter();
		} else if (_menuParts[row] == kMapDensity) {
			showDensityMap();
		} else if (_menuParts[row] == kMapProspectors) {
			// 0x5B1E centres on troop three's actual coordinates and primes
			// 0x1955 before asking 0x86CC to contact it. Reach one allows
			// this only when the prospectors are stationary at Paul's place.
			const uint32 record = 0x8e0;
			_state[0x1955] = 3;
			lookAtWorld(READ_LE_UINT16(_state.begin() + record + 6),
					(int16)READ_LE_UINT16(_state.begin() + record + 8));
			refreshStrategicMap(); // 5B30/5B10, not a passive terrain redraw.
			if (READ_LE_UINT16(_state.begin() + 0x1176) <= 1 &&
					((_state[record + 3] & 0x40) != 0 ||
					 READ_LE_UINT16(_state.begin() + record + 4) !=
					 READ_LE_UINT16(_state.begin() + 0x114e)))
				return;
			if (_state[0x46eb] & 0x40)
				return; // 5B4A preserves the reopened survey instead of contacting.
			contactMapTroop();
		}
		return;
	}

	if (_menuKind == kMenuFlight) {
		enum { kCancelRow = 1 };

		// The cabin's list is the one entry of DS:0x212E, a way out; the
		// older list of things to fly to keeps its own second row.
		if (_inCabin && _menuParts[row] == kMapExit) {
			_inCabin = false;
			_state[0x11ca] = 0; // 445A/4AC4 releases the selector's pause, even grounded.
			_state[0x46eb] = _cabinFromMap ? 0x80 : 0; // 4417 then 5A1A if maps return.
			// 443B abandons a merely boarded vehicle when no carried
			// destination exists. An airborne Cancel retains its vehicle.
			if (!READ_LE_UINT16(_state.begin() + 0x11c5))
				_state[0x11c9] = 0;

			// Older native saves can retain the replaced warning while in
			// the cabin. Its real 4415/445A return resumes flight, too.
			if (_inDesert && _flyingTo >= 0 && _flightDiscoveryPrompt)
				releaseCarriedReportForSelector();

			if (_cabinFromMap) {
				showWorld();
				showMapMenu();
			} else if (_inDesert && _flyingTo >= 0) {
				// 440F/4ABE/37F4 rebuilds the inset and restarts the
				// landscape after the cockpit reused the map page.
				_state[0x4728] = 0;
				_flightInsetSet = false;
				stopFlightMovie();
				_flightSkyRestartPending = true;
				_onWorld = false;
				showDesertMenu();
			} else {
				_onWorld = false;
				drawScene();
				showRoomMenu();
			}
			return;
		}

		if (row == kCancelRow)
			showMapMenu();

		return;
	}

	if (_menuKind == kMenuPlace) {
		if ((_menuParts[row] == kPlaceFly || _menuParts[row] == kPlaceWorm) && _pressedPlace >= 0) {
			const uint to = (uint)_pressedPlace;
			const bool worm = _menuParts[row] == kPlaceWorm;
			WRITE_LE_UINT16(_state.begin() + 0x487e, worm ? 1 : 2);
			_state[0x473e] = worm ? 0 : 1;
			if (worm) {
				// 50EA/42AA prepares a new named worm ride. The signed map
				// mode bypasses the call movie; 4709 folds raw eight below.
				_state[0x11c9] = 8;
			}
			// 50FB/4057 moves the party before 186B paints the return.
			// Aircraft use room one; worms keep the live desert coordinates.
			const uint16 returning = worm ? READ_LE_UINT16(_state.begin() + 4) :
					(READ_LE_UINT16(_state.begin() + 4) & 0xff00) | 1;
			moveMatchingFollowers(returning, READ_LE_UINT16(_state.begin() + 6));
			WRITE_LE_UINT16(_state.begin() + 4, returning);
			_state[0xd] = _state[0xb];
			_state[0xb] = (byte)returning;
			if (!worm) {
				const byte *room = roomRecord(_state[8], 1);
				if (room && room[0])
					_place = room[0] - 1;
			}

			teardownStrategicMap(false); // 50EA/186B/5ADF release the map before carried departure.
			_state[0x28e7] = _state[0x28e8];
			_state[0xfb] = groundViewMode(_state[0xfb]);
			_onWorld = false;
			_speech = "";
			if (!_inDesert)
				peopleHere();
			showMapReturnView(_inDesert); // 50EA completes 186B before departure.
			if (!worm)
				_state[0x11c9] = 4; // 5112 boards the aircraft after the room return.
			takeOffFor(to);
		} else {
			dismissMapInformation();
		}

		return;
	}

	if (_menuKind == kMenuDesert) {
		switch (_menuParts[row]) {
		case kDesertMap:
			openWorldMap();
			break;

		case kDesertChange:
			if ((_state[0x11c9] & 3) == 2) {
				// DS:21F8 uses 497A for both vehicles. 50C4 is the
				// separate TOWARDS NEAREST PLACE command, not a selector.
				showWormMap(false);
				break;
			}
			// 497A enters the same cockpit selector as boarding, centred
			// on the live airborne position. It does not open the large map.
			_onWorld = true;
			_inCabin = true;
			_cabinFromMap = false;
			_cabinTyped = 0;
			_cabinPointerPoint = -1;
			_state[0x4728] |= 1;
			_speech = "";
			lookAtPlayer();
			enterCabin();
			break;

		case kDesertSkip:
			// 4FFB silently moves up to 200 position callbacks before landing.
			// Terrain frames are skipped, not the flight's hours or route.
			if (_flyingTo >= 0) {
				const int bound = _flyingTo;

				// The DOS menu dispatcher keeps the selected action pointer here;
				// 41CC uses it to flag an interrupted fast flight.
				WRITE_LE_UINT16(_state.begin() + 0x1f12, 0x4ffb);
				if (!advanceSkippedFlight()) {
					// 4F2F hands the interrupted route to 2E52. Its carried
					// dispatcher clears event four, rechecks this same cell with
					// 4182, then installs the airborne encounter control at
					// 1F9E (callback 4ABE). The first hazard decrement was
					// undone by 4F2F; this second one must be retained.
					if (prepareCarriedRoomRequests()) {
						checkFlightHazards();
						if (_state[0x23] && beginCarriedReport())
							break;
					}
					showDesertMenu();
					break;
				}
				_inDesert = false;
				flyTo((uint)bound);
				if (_speakerPerson < 0)
					showRoomMenu();
			}
			break;

		case kDesertBack:
			// 50A2/50A5 restarts the CURRENT film through CA1B before
			// changing course. Its CC96/4AEB repaint advances VER.BIN once,
			// without resetting that independent packet cursor. Retaining the
			// old landscape/pose here left the worm one pose behind on return.
			if (_flightMovie >= 0) {
				_flightFrame++;
				drawFlightMovieFrame(true, true);
			}
			// Round and back to where the flight began.  0x50B8 and 0x50D5
			// both end on 0x50BE, which puts the broken-off byte back to
			// zero: a course given again is a course to a place, and until
			// that byte is cleared the machine goes on flying the point it
			// was steered to and the list goes on offering the way back.
			_flyingTo = (int)_cameFromPlace;
			_flightSteered = false;
			_flightToPoint = false;
			setFlightCourse();
			if (_state.size() > 0x11cb)
				_state[0x11cb] = 0;
			showDesertMenu();
			break;

		case kDesertNearest:
			_flyingTo = nearestPlace();
			_flightSteered = false;
			_flightToPoint = false;
			setFlightCourse();
			if (_state.size() > 0x11cb)
				_state[0x11cb] = 0;
			showDesertMenu();
			break;

		case kDesertEvening:
			waitUntil(true);
			break;

		case kDesertMorning:
			waitUntil(false);
			break;

		case kDesertMixer:
			_speech = "";
			showMixer();
			break;

		case kDesertWorm:
			_speech = "";
			showWormMap();
			break;

		default:
			if (_menuParts[row] < 16) {
				const uint person = _menuParts[row];
				takeUpPerson(person);
				recordPersonConversation(person);
				enterDialogue(person, kWhatHeSays);
				showTalkMenu(false);
				sayNextLine();
			}
			break;
		}

		return;
	}

	if (_menuKind == kMenuWormMap) {
		closeWormMap();
		return;
	}

	if (_menuKind == kMenuRoom) {
		enum { kViewNewMessages = 214, kMessagesAlreadySeen = 215 };

		if (_menuParts[row] == kViewNewMessages) {
			showMessageList(false);
		} else if (_menuParts[row] == kMessagesAlreadySeen) {
			showMessageList(true);
		} else if (_menuParts[row] == kRoomMap) {
			openWorldMap();
		} else if (_menuParts[row] == kRoomMassiveAttack || _menuParts[row] == kRoomFightDay) {
			fightHere(_menuParts[row] == kRoomMassiveAttack);
		} else if (_menuParts[row] == kRoomBattleWorm) {
			_speech.clear();
			showWormMap();
		} else if (_menuParts[row] == kRoomOrnithopter) {
			boardOrnithopter();
		} else if (_menuParts[row] == kRoomMixer) {
			_speech = "";
			_speakerPerson = -1;
			showMixer();
		} else if (_menuParts[row] == kRoomMirror) {
			showMirror();
		} else if (_menuParts[row] == kRoomMirrorAway) {
			drawScene();
			showRoomMenu();
		} else {
			uint person = _menuParts[row];
			debugC(1, kDebugScript, "the room's row %u takes up %u", row, person);
			if (person >= kRoomChiefBase && person < kRoomChiefBase + 8) {
				if (_state.size() > 0x476c)
					_state[0x476c] = (byte)(person - kRoomChiefBase);
				person = 15;
			}
			startRoomConversation(person);
		}
		return;
	}

	if (_menuKind == kMenuTalk) {
		enum { kHeadingRow = 0, kComeWithMeRow = 1, kWhatRow = 2, kStopRow = 3 };

		if (row == kWhatRow) {
			repeatSpeech(); // 1F8A -> 9ED5: no text, selection or action.
		} else if (row == kHeadingRow) {
			continueRoomConversation();
		} else if (row == kStopRow) {
			// Done with them: they go back to standing on a mark.
			finishRoomConversation();
		} else if (row == kComeWithMeRow && _speakerPerson >= 0) {
			enum {
				kGiveOrders = 146, kWorkForMe = 149, kRallyToMe = 150,
				kOverpower = 155, kPrisonerReply = 133,
				kFremen = 14, kChief = 15,
				kFremensTroop = 0x4756
			};

			const uint who = (uint)_speakerPerson;
			const uint offered = _menuParts[row];
			if (offered == kOverpower) {
				// 0x9584 grants each person one attempt. The first costs the local
				// troop 41 morale and subtracts 41 from ED, wrapping as a byte.
				// A negative result subdues the Captain and marks the troop.
				const uint16 troop = READ_LE_UINT16(_state.begin() + 0x4768);
				const uint16 tried = READ_LE_UINT16(_state.begin() + 0xee);
				const uint16 bit = who < 16 ? (uint16)(1u << who) : 0;
				if (bit != 0 && (tried & bit) == 0 &&
						troop >= 0x8aa && troop + 27 <= _state.size()) {
					WRITE_LE_UINT16(_state.begin() + 0xee, tried | bit);
					_state[troop + 0x15] -= 0x29;
					_state[0xed] -= 0x29;
					if ((_state[0xed] & 0x80) != 0) {
						_state[troop + 3] |= 0x10;
						_state[0x10a7] |= 0x10;
					}
				}

				// 0x970B chooses shared part 133, but its speaker remains the
				// person Paul asked, not that part's synthetic group sixteen.
				_dialoguePart = kPrisonerReply;
				_dialogueMask = 0x20;
				if (_dialoguePart < _dialogue.count()) {
					Common::Array<Dialogue::Record> &replies = _dialogue.entry(_dialoguePart);
					for (uint line = 0; line < replies.size(); line++) {
						if (!dialogueRecordAvailable(replies[line], _dialogueMask) ||
								!conditionHolds(replies[line].condition))
							continue;
						_dialogueLine = line;
						sayChosenLine();
						break;
					}
				}
				showTalkMenu();
				return;
			}

			// DOS 5A03 enters the strategic map before 8685 opens the troop
			// report. The room-origin flag suppresses the ordinary map legend and
			// later returns NO MORE ORDERS to the room. Presenting only the final
			// card skipped the visible map/menu handoff in the Tuono-Tabr recording.
			if (offered == kGiveOrders) {
				// 5A03/68EB uses the selected chief even while caller14 is
				// retained after an oath.4756 can already name the next recruit.
				const uint16 record = READ_LE_UINT16(_state.begin() + 0x4758
									  + 2 * (_state[0x476c] & 7));

				if (record >= 0x8aa && record + 27 <= _state.size()) {
					const uint troop = (record - 0x8aa) / 27;
					Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
					memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
					byte oldPalette[256 * 3];
					_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
					letGoOfPerson();
					_state[0xfb] = roomOrderViewMode(_state[0xfb]); // 5A10, before 5A1A/18BA.
					_state[0x46f3] = 1;
					_troopOrdersFromMap = false;
					_onWorld = true;
					_onGlobe = false;
					_inCabin = false;
					_state[0x28e7] = 1; // 5A1A marks the room-origin map view too.
					_state[0x46eb] = 0x80; // 5A03 enters through 5A1A/5A7E too.
					lookAtPlayer();
					closeMapTitle();
					showMapMenu(false);
					showWorld(false);
					showMapEntryTransition(oldPage.begin(), oldPalette);
					showTroopContact(troop, false);
				}
				return;
			}

			const bool along = isFollowing(who);
			// 95C1 prepares every COME WITH ME / WORK FOR ME request, not
			// just a generic Fremen. DS:0xAC is the population
			// threshold, charisma lowers the demand, and DS:0x36 is the local
			// troop's motivation prepared by enterDialogue.  The low-motivation
			// Timin troop therefore refuses for now, while the first and third
			// troops accept just as they do in the reference playthrough.
			if (!along && _state.size() > 0xad) {
				_state[0x23] = 0;
				if (READ_LE_UINT16(_state.begin() + 0xac) >= 1000 &&
					_state[0x29] <= 100) {
					const byte wanted = (byte)((100 - _state[0x29]) >> 2);
					if (wanted > _state[0x36])
						_state[0x23] = 2;
				}
				debugC(2, kDebugScript,
					   "Fremen recruitment compares %u hostile, charisma %u, motivation %u: answer %u",
					   READ_LE_UINT16(_state.begin() + 0xac), _state[0x29],
					   _state[0x36], _state[0x23]);
			}

			// What they answer to being asked along is the sixth part of their
			// group and what they answer to being left the seventh: Duke
			// Leto's is "No, I have to stay here!" and then "Now Paul! THAT'S
			// ENOUGH! I said NO!" for asking again.  The line's own action
			// says whether they agreed - 0xA1D0 sets the byte at DS:0x47A5 to
			// 0xFF where they did and 0xA1D6 clears it where they did not - so
			// it is set before the line is read and looked at after.
			if (_state.size() > 0x47a5)
				_state[0x47a5] = 0xff;

			if (_dialoguePart / 8 == who && (_dialoguePart & 7) < 4) {
				_personNewsPart = _dialoguePart;
				_personNewsLine = _dialogueLine;
				_personNewsMask = _dialogueMask;
				_personNewsEnded = READ_LE_UINT16(_state.begin() + 0x47ba) == 0xffff;
			}
			enterDialogue(who, along ? kStayingBehind : kComingAlong);
			sayNextLine();
			// 953F/95EE counts consecutive requests after their answer.
			// CONDIT tests its parity for the "already told you" reply;
			// ordinary TALK resets it at 9481. 95F2 clears COME's context.
			_state[0x1b]++;
			if (!along)
				_state[0x23] = 0;

			if (_state.size() > 0x47a5 && _state[0x47a5] == 0xff) {
				if (offered == kWorkForMe || offered == kRallyToMe) {
					// The Fremen brings his troop rather than himself.  He
					// goes on standing there and speaking - 0x9652 hands
					// 0x90BD the chief's own record only to build the list
					// out of it, and never touches who is being spoken to -
					// so all that changes is the second row, which reads
					// GIVE ORDERS TO TROOP from here on.  Taking the chief up
					// instead put another man's face over another corner of
					// the room in the middle of the talk.
					enlistTroop(READ_LE_UINT16(_state.begin() + kFremensTroop));

					_listAs = kChief;
					showTalkMenu();
					return;
				}

				// 9608/9556 change the person's travel flag when they answer;
				// 9673/9655 update the portrait slots only on leaving this talk.
				if (along && chaniCurePlace(READ_LE_UINT16(_state.begin() + 0x47a2)))
					_state[0xf9] += 0x10; // 954C/9551: two hours of cure credit.
				markPersonFollowing(who, !along);
			}

			// 95EE..961A returns to the retained talk table. The answer was
			// already presented; resampling the pointer here changes its highlight.
		}
		// The heading is not a choice.
		return;
	}

	_dialoguePart = _menuParts[row] / 1000;
	_dialogueMask = 0x80;

	const uint line = _menuParts[row] % 1000;
	_dialogueLine = line == 0 ? 0xffff : line - 1;

	_menu.clear();
	sayNextLine();
}

/**
 * What a line does once it has been said.  The low nibble of a record's flags
 * picks one of the routines the game keeps in a table at 0xA107, and the ones
 * written out here are the short ones that only touch the game's own memory.
 * A record with 0x80 in its flags is passed over by several of them.
 */
void DuneEngine::runLineAction(const Dialogue::Record &record) {
	const byte action = record.flags & 0xf;
	const bool marked = (record.flags & 0x80) != 0;

	// A049 skips action dispatch for zero. In particular, the chief's work
	// reports must retain 5274's actual hidden-sietch distance/bearing and FB
	// report mode; their later location clues use those existing predicates.
	if (action == 0 || _state.size() < 0x10000)
		return;

	switch (action) {
	case 3:  // 0xA1F7 - stage-selected event bytecode, for several speakers
		beginEventScript(stageEventScript(storyStage()));
		break;

	case 1:  // 0xA1D0
		_state[0x47a5] = 0xff;
		break;
	case 2:  // 0xA1D6
		_state[0x47a5] = 0;
		break;
	case 6:  // 0xA1E8
		_state[0x47a8]++;
		break;
	case 7:  // 0xA1DC
		_state[0x47a5] = 0x80;
		break;
	case 5:  // 0xA248 - the same answer panel, for a smuggler invoice
	case 4:  // 0xA244 - put up ARGUE / ACCEPT / REFUSE / WHAT
		// This action is shared by every question with a modal answer, including
		// Stilgar's Water-of-Life offer.  Duncan and the smugglers additionally
		// prepare report cards, but limiting the menu itself to those two speakers
		// left Stilgar's question under the ordinary talk controls and made it
		// impossible to answer.
		// A244 enters with zero and A248 with one.  Duncan's action-nine
		// handler uses that distinction to pay a village account instead of
		// preparing an Emperor shipment.
		_state[0x476d] = action == 5 ? 1 : 0;
		_state[0x9f] = 0;
		showShipmentResponse(); // A258 returns before A25B prepares the report.
		if (_speakerPerson == 3) {
			// A27F increments AX while deriving the gesture selector, not
			// DS:1A. 2499 advances the answer counter only on a response.
			// The bill is the exceptional last entry of the report-pose table at
			// DS:0x197C.  Its first report word is 0x38, so 0xA274 changes the
			// animation-table offset from 0x0A to 0x10.  99BE first finishes
			// the current speech block, then plays the twenty-frame green-book
			// animation (index eight). Do not expose its final still now.
			if (action != 5)
				_speakerAnimation = 6;
			_speakerReport = action == 5 ? 6 : 4;
		} else if (_speakerPerson == 13) {
			// A25B selects 6 + twice the regional variant, after the
			// current eight-frame speech block. The following odd-numbered
			// table is the reverse motion, not the price presentation.
			_speakerReport = 5;
		}
		// A244/A248 both fall through A25B. A shipment proposal needs its
		// descriptor just like the invoice and the ordinary production reports.
		if (_speakerPerson == 3 || _speakerPerson == 13)
			selectLocalSpeakerReport(record.line);
		break;
	case 8:  // 0xA125 - speaker-specific report preparation
		if (_speakerPerson == 12) {
			// A146 clears only this fort's hidden bit. Unlike ordinary 425B
			// discovery, it does not clear the unlock stage, count a sietch or
			// advance the story. 932E selected this intelligence before talking.
			const uint16 place = _state.size() > 0x11cf
				? READ_LE_UINT16(_state.begin() + 0x11ce) : 0;

			if (place >= 0x100 && (uint)place + 0xa < _state.size()) {
				_state[place + 0xa] &= 0x7f;
				_map.readPlaces(_state.begin(), _state.size());
			}
		} else if (_speakerPerson == 1) {
			// Jessica's first "your powers have increased" record runs the
			// speaker-one branch at 0xA186.  It advances the prescience/contact
			// range kept at 0x1176 and derives the next-dialogue latch at D5.
			// From its initial value one this becomes thirty and 0x7B, making
			// the wider-range alternative false and selecting line 81 instead.
			uint16 power = READ_LE_UINT16(_state.begin() + 0x1176);
			if (_state[0x0a] & 2) {
				addCharisma(40); // A18D/6F78 also raises sworn troops' motivation.
				power = (uint16)-50;
			} else if (power == 1) {
				addCharisma(10); // A1A2: only the first ordinary range increase.
				power = 10;
			}
			power = (uint16)(power + 20);
			WRITE_LE_UINT16(_state.begin() + 0x1176, power);
			_state[0xd5] = power < 100 ? (byte)(0x80 - power / 6) : 0;
		} else if (_speakerPerson == 5) {
			// A125 installs2CCF after the reply is presented. Its fade,
			// three-hour wait and room requests belong to finishDialogueLine,
			// not the paragraph action before9EFD starts Stilgar's voice.
		} else if (_speakerPerson == 3) {
			prepareShipmentOptions();
		} else if (_speakerPerson == 13) {
			// 0x2388 advances the fish-village chapter and selects the next
			// stocked equipment type from this village's seventeen-byte account.
			advanceStory(60);
			// 238D/E3B7 uses the saved secondary stream, also consumed by
			// REFUSE and ARGUE. Keeping it in DS:D824 makes a restored bargain
			// take the same branch as the original DOS engine.
			_state[0x9e] = (byte)(troopRandom(true) & 3);
			const uint16 account = READ_LE_UINT16(_state.begin() + 0x10b4);
			if (account >= 0x10d8 && account + 17 <= _state.size()) {
				_state[account + 3] = (byte)(hour() >> 4);
				_state[0x1a] = 0;
				const byte kinds = _state[0x1141];
				uint16 item = (uint16)(READ_LE_UINT16(_state.begin() + 0x11f1) - 0xe8);
				// 23AB gives the stock walk two boundary subtractions, not
				// one attempt per equipment kind. 11F1 is shared with troop
				// reports, so its previous value need not be a valid item.
				uint wraps = 2;
				while (wraps) {
					item = (uint16)(item + 1); // INC AX, including carry into AH.
					while ((byte)item >= kinds) {
						// CMP/SUB AL preserve AH for the stock/price address.
						item = (item & 0xff00) | (byte)((byte)item - kinds);
						if (--wraps == 0)
							break;
					}
					if (!wraps)
						break;
					const uint16 stock = (uint16)(account + 4 + item);
					const uint16 price = (uint16)(account + 9 + item);
					if (stock >= _state.size() || price >= _state.size())
						break;
					if (_state[stock] == 0)
						continue;
					WRITE_LE_UINT16(_state.begin() + 0x11f1, (uint16)(0xe8 + item));
					_state[0x9d] = (byte)((_state[price] & 0x7f) << 1);
					break;
				}
			}
		}
		break;
	case 9:  // 0xA157 - commit an Emperor answer or smuggler-bill answer
		// 0xA162 gives Stilgar his own branch: 0x2D2C puts one on the war at
		// DS:0xC2 and then 0x2D35 walks the three fortresses whose records
		// begin at 0x138, gathering every troop standing there which is set
		// to the fourth order and carries the third piece of equipment, and
		// 0x2D4A sends each of them against the Harkonnen palace at 0x11C.
		//
		// That is the march on the palace the whole war has been building
		// towards, and nothing was doing any of it: Stilgar could say the
		// word and no Fremen would move.
		if (_speakerPerson == 5) {
			enum { kWar = 0xc2, kFirstFortress = 0x138, kFortresses = 3,
				   kTroops = 0x8aa, kTroopSize = 27, kAtArms = 4,
				   kEquipped = 4, kPalace = 1, kMarching = 7 };

			_state[kWar]++; // 2D2C is an eight-bit INC, including wraparound.

			// 1258/661D visits each fortress's standing chain in link order.
			// 2D35 collects its first eligible records before 2D4A starts any
			// routes: those departures would otherwise change the chains during
			// selection. A travelling troop may still name this fort at +4, but
			// it is not on the standing chain and must not march from it.
			Common::Array<uint> marchers;
			for (uint i = 0; i != kFortresses && marchers.size() < kMarching; i++) {
				const uint32 fortress = kFirstFortress + i * 28;
				if (fortress + 28 > _state.size())
					break;
				byte id = _state[fortress + 9];
				for (uint visited = 0; id && visited < 67 && marchers.size() < kMarching; visited++) {
					if (id > _map.troops().size())
						break;
					const uint32 troopRecord = kTroops + (id - 1) * kTroopSize;
					if (troopRecord + kTroopSize > _state.size())
						break;
					const byte next = _state[troopRecord + 1];
					// 6906 excludes hidden records; 2D62 compares the complete
					// task byte, not just its occupation nibble.
					if (_state[troopRecord + 3] == kAtArms &&
							(_state[troopRecord + 0x19] & kEquipped))
						marchers.push_back(id - 1);
					id = next;
				}
			}
			for (uint i = 0; i < marchers.size(); i++) {
				const uint t = marchers[i];
				if (t >= 67 || _troopRoutes.size() < 67 * 3)
					continue;
				_troopRoutes[t * 3] = (byte)(kPalace + 1);
				_troopRoutes[t * 3 + 1] = 0;
				_troopRoutes[t * 3 + 2] = 0;
				beginTroopRoute(t);
				// 2D51 calls 8308 after 84A6's seven initial substeps.
				// Give this troop its four/eight-step travel update now,
				// before the next marcher's route consumes the shared phase.
				advanceTroopTravel(t);
			}

			_map.readTroops(_state.begin(), _state.size());
			debugC(1, kDebugScript,
				   "Stilgar sends %u troop(s) against the palace; the war stands at %u",
				   (uint)marchers.size(), _state[kWar]);
			break;
		}

		if (_speakerPerson != 3)
			break;
		if (_state[0x9f] < 2) {
			if (_state[0x476d] == 0) {
				const uint option = (_state[0x1a] - 1) & 3;
				WRITE_LE_UINT16(_state.begin() + 0xc0,
					READ_LE_UINT16(_state.begin() + 0xb4 + 2 * option));
				WRITE_LE_UINT16(_state.begin() + 0x1158, 0xffff);
			} else {
				// 0x2517 pays the account chosen by prepareShipmentOptions().
				// 251D exchanges the invoice with zero; 2520 unconditionally
				// decrements the unpaid byte, including zero-value bills and wrap.
				const uint16 account = READ_LE_UINT16(_state.begin() + 0x10b4);
				if (account >= 0x10d8 && account + 17 <= _state.size()) {
					const uint16 amount = READ_LE_UINT16(_state.begin() + account + 0x0e);
					WRITE_LE_UINT16(_state.begin() + account + 0x0e, 0);
					_state[0x22]--;
					WRITE_LE_UINT16(_state.begin() + 0xa0,
						(uint16)(READ_LE_UINT16(_state.begin() + 0xa0) - amount));
					WRITE_LE_UINT16(_state.begin() + 0x1172,
						(uint16)(READ_LE_UINT16(_state.begin() + 0x1172) + amount));
				}
			}
		} else if (_state[0x476d] != 0) {
			// Arguing postpones this bill; refusing marks it for disposal.  Those
			// two bits are what the next Duncan conversation's conditions read.
			const uint16 account = READ_LE_UINT16(_state.begin() + 0x10b4);
			if (account >= 0x10d8 && account + 17 <= _state.size()) {
				_state[account + 2] &= 0x9f;
				_state[account + 2] |= _state[0x9f] == 2 ? 0x40 : 0x20;
			}
		}
		break;
	case 11: // 0xA219 - the story moves on by one
		// This one counts the stage up itself rather than going through the
		// story engine, so none of the stage setups run: only the stages that
		// are a multiple of four ever have one, and those are reached by the
		// action below.
		if (!marked) {
			_state[0x2a]++;
			_state[0xff] = 0;
			// 0xA22A calls 0x100B the first time the story advances.  That
			// single byte is the high half of Duncan's palace location in his
			// person record; without it he was drawn from a stale presence bit
			// but omitted from the room menu and could not be spoken to.
			if (_state[0x2a] == 1)
				_state[0x100b] = 1;
			// A219 calls B17A's silent selection, not a narrator card. Keep
			// the chief/person's text, continuation and live voice intact.
			markAutomaticStoryRecord();
			debugC(1, kDebugScript, "the story counts on to stage %d", _state[0x2a]);
		}
		break;
	case 12: // 0xA235 - on to the next multiple of four
		if (!marked) {
			advanceStory((_state[0x2a] & 0xfc) + 4);
			// Once Leto has formally sent Paul and Jessica to find Gurney, the
			// preceding return-event latch is spent.  Otherwise Jessica's old
			// desert-collapse making-off record wins every searched room.
			if (_speakerPerson == 0 && record.line == 15)
				_state[0xe7] = 0;
		}
		break;
	case 14: // 0xA1ED
		if (!marked)
			_state[0xc2]++;
		break;
	case 15: // 0xA172 - speaker-specific early story bookkeeping
		if (_speakerPerson == 1) {
			_state[0xf5]++;
		} else if (_speakerPerson == 3) {
			// Duncan's first explanation opens the valuable-spice continuation
			// in his person record.  Without this bit condition 206 fails and
			// the conversation falls through to his literal ellipsis instead.
			if (storyStage() < 16) {
				_state[0xff7] |= 0x10;
			} else {
				// 24A3 calls A1E8 even when a shipment reply is queued.
				// A09F consumes this latch so the next TALK closes rather than
				// continuing into Duncan's stock/proposal news.
				_state[0x47a8]++;
				// After line 211, 0x24A3 turns the satisfaction byte made by
				// sendSpiceShipment() into one of the Emperor's editions 7..12.
				// An exactly fulfilled request (0x80) selects edition nine, the
				// response which appears next in the reference message list.
				if (READ_LE_UINT16(_state.begin() + 0xc0) != 0 &&
					_state[0xb] == 8 &&
					READ_LE_UINT16(_state.begin() + 0x1158) == 0) {
					byte edition = 7;
					static const uint16 thresholds[] = { 1, 0x40, 0x80, 0x90, 0xff };
					for (uint i = 0; i != ARRAYSIZE(thresholds); i++) {
						if (_state[0xbe] < thresholds[i])
							edition++;
					}
					WRITE_LE_UINT16(_state.begin() + 0xc0, 0);
					_state[0xbf] |= 1;
					queueEvent((uint16)((edition << 8) | 11));
					if (edition == 12)
						_state[0x11bb]++;
				} else {
					WRITE_LE_UINT16(_state.begin() + 0xc0, 0);
					_state[0xbf] |= 1;
				}
			}
		}
		break;
	case 10: // 0xA25B - select the line's gesture/report animation
		// For an ordinary named speaker DS:0x47D0 is zero, so the selector
		// starts at 0x0A.  The animation reader adds two bytes into its offset
		// table, which names animation six.  Banks with fewer animations fall
		// safely back to their ordinary talking pose in drawSpeakerFigure().
		// Emperor's quota and Duncan's spoken stock/production and shortfall
		// reports install 47E1's deferred gesture selector,
		// but 99BE first finishes the current eight-frame speech block. Keep
		// their idle pose until that callback enters animation six (index five).
		if (_speakerPerson != 11 && (_speakerPerson != 3 ||
			(record.line != 232 && record.line != 234 && record.line != 235 &&
			 record.line != 240)))
			_speakerAnimation = 6;
		if (_speakerPerson == 11) {
			_speakerReport = 4;
			selectLocalSpeakerReport(record.line);
		}
		if (_speakerPerson == 3) {
			if (record.line == 232 || record.line == 240)
				_speakerReport = 1;
			else if (record.line == 234)
				_speakerReport = 2;
			else if (record.line == 235)
				_speakerReport = 3;
			if (_speakerReport >= 1 && _speakerReport <= 3)
				selectLocalSpeakerReport(record.line);
		}
		break;
	case 13: // 0xA28E/A2A0 - a named speaker's remote report exposes its map
		if (_state[0x28e7] != 1 &&
			READ_LE_UINT16(_state.begin() + 0x47e6) !=
			READ_LE_UINT16(_state.begin() + 0x114e))
			_state[0x47a4] = 0;
		break;
	default:
		debugC(2, kDebugScript, "line action %d is not written yet", action);
		break;
	}
}

/** 8E16/8E9E: count laid-out words for 9908's fourfold idle duration. */
void DuneEngine::updateSpeechWordCount() {
	const SpeechLayout layout = layoutSpeech(_font, _speech, kSpeechWidth);
	byte count = 0;
	for (uint i = 0; i < layout.rows.size(); i++)
		count += layout.rows[i].count;
	_state[0x478c] = count;
}

/**
 * Puts the first part of a line up. A line may be more than can be read at
 * once, and 0xFE in it says that the rest waits for a press, which is why the
 * original shows a long line as two screenfuls one after another.
 */
bool DuneEngine::forcedSpeechCaption() const {
	return _speechDisplayForced && _speechPart < _speechParts.size() &&
		_speech == _speechParts[_speechPart];
}

/** 89C1/89C8: retain the next text page independently of its presentation. */
void DuneEngine::updateSpeechContinuation() {
	uint next = 0;
	if (_speechPart + 1 < _speechParts.size()) {
		for (uint i = 0; i <= _speechPart; i++)
			next += _speechParts[i].size() + 1;
	}
	// Native strings replace DOS's resident phrase segment. Keep a nonzero
	// logical byte offset, not a dangling host/resource pointer, in 47B6.
	// STOP may discard the display strings but must retain this story gate;
	// the next decoded paragraph or 940C's new-person news resets it.
	WRITE_LE_UINT16(_state.begin() + 0x47b6, MIN<uint>(next, 0xffff));
}

void DuneEngine::sayLineInParts(const Common::String &line) {
	sayLineInParts(line, false);
}

void DuneEngine::sayLineInParts(const Common::String &line, bool forceDisplay) {
	// 8B11 first runs 8C8A: remove the old caption and repaint 47C8's
	// retained body pose before the next recording starts. A room rebuild
	// must not put the previous voice's cached mouth back over that pose.
	_localSpeechMouth = -1;
	_localSpeechVisible.clear();
	_dialogueActionPending = false;
	if (_state.size() > 0x47e0)
		_state[0x47e0] = 0; // 88AF: a new phrase starts without the previous variant.
	_speechParts.clear();
	_speechPart = 0;
	_speechDisplayForced = forceDisplay;
	_speechCaptionText.clear();
	_speechCaptionPixels.clear();
	_speechCaptionMode = 0;
	WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
	_speechCaptionShadow = 0;
	_localSpeakerBalloonText.clear();

	uint from = 0;
	for (uint at = 0; at <= line.size(); at++) {
		if (at != line.size() && line[at] != '\f')
			continue;

		_speechParts.push_back(Common::String(line.c_str() + from, at - from));
		from = at + 1;
	}

	_speech = _speechParts.empty() ? Common::String() : _speechParts[0];
	updateSpeechContinuation();
	// 88DA/8888 skips caption layout, not 8944's FE-page decoding or
	// continuation pointer. The prior478C remains available to9908. Defeat
	// messages and A0F1's forced reports still perform actual layout.
	if (_state[0x28e7] >= 2 && !_state[0x46d9] && !forceDisplay) {
		WRITE_LE_UINT16(_state.begin() + 0x479e, 1); // 888F: hidden speech still owns the scene/B45 pause.
	} else {
		if (!_state[0x28e7] && !_state[0x46eb] && !_state[0xc6] &&
				!_state[0x227d] && !_state[0x46d9] && READ_LE_UINT16(_state.begin() + 0x47c4) < 16) {
			WRITE_LE_UINT16(_state.begin() + 0x4784, 16);
			WRITE_LE_UINT16(_state.begin() + 0x4786, 16);
			WRITE_LE_UINT16(_state.begin() + 0x4788, 1);
			WRITE_LE_UINT16(_state.begin() + 0x478a, 0);
			WRITE_LE_UINT16(_state.begin() + 0x479e, 0x223c); // 8F28's ordinary-caption descriptor.
		}
		updateSpeechWordCount();
	}
}

/** 88AF/8944: choose a contextual recording once, at paragraph entry. */
void DuneEngine::prepareDialogueLine(const Dialogue::Record &record, uint person, uint16 reportTroop) {
	// 88AF retains the selected paragraph's request before its text and
	// 978E/9908 portrait preparation, not only when 9EFD starts the voice.
	// Idle selection and WHAT must not still describe the previous line.
	if (_state.size() > 0x4781)
		WRITE_LE_UINT16(_state.begin() + 0x4780, record.line | 0x800);
	const Common::String raw = phraseOf(person, record.line);
	const bool forceDisplay = _state[0x28e7] == 2 && record.forceBalloon && !_state[0x46eb] &&
		READ_LE_UINT16(_state.begin() + 0x47c4) < 16;
	if (forceDisplay)
		_state[0x28e7] = 1; // A0F1 selects balloons before paragraph layout.
	sayLineInParts(fillInLine(raw, reportTroop), forceDisplay);
	if (_state.size() <= 0x47e0)
		return;
	_state[0x47de] = record.flags;
	_state[0x47df] = (byte)(record.condition + 1);
	resolveDialogueVoiceVariant(raw);
	if (_state[0x47de] & 0x10) {
		// 89D3's fallback uses E3B7 with mask three, not modulo three.
		_state[0x47e0] = troopRandom(true) & 3;
		_state[0x47de] &= ~0x10;
	}
	if (_state[0x28e7] < 2)
		prepareLocalBalloonCaption(); // 88DA suppresses hidden initial pages;94DD bypasses that gate.
}

/** 8A3B/8ACC consume the first eligible word/number, including nested words. */
void DuneEngine::resolveDialogueVoiceVariant(const Common::String &raw, uint depth) {
	if (depth >= 12 || !(_state[0x47de] & 0x10))
		return;
	const byte action = _state[0x47de] & 0xf;
	for (uint i = 0; i < raw.size() && (_state[0x47de] & 0x10); i++) {
		const byte c = (byte)raw[i];
		if (c >= 0xf0)
			return; // FE is a page boundary too; don't inspect the next page.
		if (c >= 0xd0) {
			i += c == 0xd0 ? 2 : c == 0xd1 ? 4 : 1;
			continue;
		}
		bool selected = false;
		uint16 value = 0;
		uint16 which = 0;
		if (c >= 0x80 && c < 0x90) {
			if (c == 0x80) {
				if (i + 2 >= raw.size())
					return;
				which = ((byte)raw[i + 1] << 8) | (byte)raw[i + 2];
				i += 2;
			} else {
				which = READ_LE_UINT16(_state.begin() + 0x11eb + 2 * (c & 0xf));
			}
			value = which;
			switch (c) {
			case 0x8b:
				value -= 264;
				selected = true;
				break;
			case 0x84:
				value -= 72;
				selected = value == 0;
				if (!selected) {
					--value;
					selected = (byte)value < 3;
					if (!selected) {
						value -= 0xffcf;
						selected = value < 12;
					}
				}
				break;
			case 0x83:
			case 0x8c:
				value -= 232;
				selected = (byte)value < 7;
				break;
			case 0x86:
			case 0x87:
			case 0x88:
				value -= 209;
				selected = action == 1 && (byte)value < 7;
				break;
			case 0x85:
				value = which == 116 ? 1 : 0;
				selected = action == 1;
				break;
			case 0x89:
				value -= 218;
				selected = (byte)value < 8;
				break;
			default:
				break;
			}
		} else if (c >= 0x90 && c < 0xa0) {
			if (++i >= raw.size())
				return;
			const byte at = (byte)raw[i];
			value = c == 0x92 ? READ_LE_UINT16(_state.begin() + at) : _state[at];
			if (at == 0xcf) {
				value = MIN<uint>((byte)value, 8);
				selected = true;
			} else if (at == 0x55 || at == 0x61) {
				value = MIN<uint>((byte)(value - 1), 4);
				selected = true;
			} else if (at == 0x44 && action == 1) {
				value = MIN<uint>((byte)(value - 1), 8);
				selected = true;
			}
		}
		if (selected) {
			_state[0x47e0] = (byte)value;
			_state[0x47de] &= ~0x10;
		} else if (which != 0) {
			resolveDialogueVoiceVariant(command(which - 1), depth + 1);
		}
	}
}

bool DuneEngine::sayRestOfLine() {
	if (_speechPart + 1 >= _speechParts.size())
		return false;

	// 94DD paints the next paragraph and 94E8 advances its voice request.
	// A042 skips the action on intermediate pages, but both branches still
	// enter A0AA/978E to prepare this page's portrait before 9EFD.
	_localSpeechMouth = -1;
	_localSpeechVisible.clear();
	_speech = _speechParts[++_speechPart];
	updateSpeechContinuation();
	updateSpeechWordCount();
	prepareLocalBalloonCaption();
	debugC(1, kDebugScript, "  ... %s", _speech.c_str());
	if (_state.size() > 0x47dd &&
			(READ_LE_UINT16(_state.begin() + 0x4780) & 0x800) &&
			READ_LE_UINT16(_state.begin() + 0x47c4) <= 16) {
		// 94E8 advances the actual request, not the dialogue cursor. Direct
		// announcements and shared report banks must keep their caller/mode.
		WRITE_LE_UINT16(_state.begin() + 0x4780,
			(uint16)(READ_LE_UINT16(_state.begin() + 0x4780) + 0x1000));
	}
	const bool completesLine = _dialogueActionPending && _speechPart + 1 == _speechParts.size();
	if (completesLine) {
		completeDialogueLine();
	} else {
		if (!_state[0x46eb] && _speakerPerson >= 0 && _speakerPerson < 16)
			prepareLocalSpeakerParagraph();
		if (_inDesert)
			drawDesertDialogue(false);
		else
			drawScene(false);
	}
	// A03F calls 9EFD directly after preparing the new page. Only WHAT's
	// 9ED5 completes the old pose block through 9985; doing that here would
	// skip the first frames of the newly prepared paragraph's portrait.
	const uint16 request = READ_LE_UINT16(_state.begin() + 0x4780);
	const uint16 caller = READ_LE_UINT16(_state.begin() + 0x47c4);
	if (request && caller <= 16)
		playSpeech(request & ~0x0c00, caller, _state[0x47dd] != 0, (request & 0x800) != 0);
	presentDialogueLine();
	if (completesLine)
		finishDialogueLine();
	finishRoomDeparture();
	finishVehicleBoarding();

	return true;
}

/** 9468 and 1F82 both enter 9472, resuming news after a COME/STAY answer. */
void DuneEngine::continueRoomConversation() {
	if (sayRestOfLine())
		return;
	// COME WITH ME and STAY HERE are independent answer parts. Once
	// that answer has been given, TALK returns to the person's ordinary
	// news rather than walking into a second refusal from the same
	// request part. This is how Gurney follows his agreement to remain
	// at Ergsun-Clam with "I'm teaching the Fremen...".
	const uint localPart = _dialoguePart & 7;
	if (_speakerPerson >= 0 &&
			(localPart == kComingAlong || localPart == kStayingBehind)) {
		if (_personNewsPart != 0xffff && _personNewsPart / 8 == (uint)_speakerPerson) {
			// 95E5/9533 use 9F31/9F8B's independent request selector.
			// TALK resumes the ordinary cursor, not a fresh conversation.
			_dialoguePart = _personNewsPart;
			_dialogueLine = _personNewsLine;
			_dialogueMask = _personNewsMask;
			WRITE_LE_UINT16(_state.begin() + 0x47be, _dialoguePart);
			WRITE_LE_UINT16(_state.begin() + 0x47ba, _personNewsEnded ? 0xffff : 0);
			_state[0x47c2] = _dialogueMask;
			_personNewsPart = 0xffff;
		} else {
			// Legacy saves cannot reconstruct the interrupted cursor.
			// Retain their existing contextual Chani fallback.
			enum { kAroundHere = 2, kChani = 7, kOxtynTabr = 26 };
			const uint16 here = _state.size() > 0x114f
				? READ_LE_UINT16(_state.begin() + 0x114e) : 0;
			if (_speakerPerson == kChani && localPart == kComingAlong &&
					storyStage() == 69 && here == 0x100 + kOxtynTabr * 28)
				enterDialogue(kChani, kAroundHere);
			else
				enterDialogue((uint)_speakerPerson, kWhatHeSays);
		}
	}
	sayNextLine();
}

void DuneEngine::sayNextLine() {
	// The remote-message reaction is a request, not Thufir's news cursor.
	// DOS retains ordinary part16/mask80 beneath it. Both TALK and scene
	// continuation must restore that reader before the close/selection pass;
	// resource conditions select the chapter's news without a record shortcut.
	if (_menuKind == kMenuTalk && _speakerPerson == 2 && _dialoguePart == 20 &&
			((_dialogueLine == 3 && _state[0xe9] == 10) ||
			 (_dialogueLine == 4 && _state[0xe9] == 11)))
		beginPersonNews(2);

	if (_menuKind == kMenuTalk && (_dialoguePart & 7) < 4 &&
			(_dialogueMask == 0x80 || (_speakerPerson == 13 && _dialogueMask == 0x20)))
		_state[0x1b] = 0; // 9472/9481, before continuation or fresh selection.
	// Which part is being read is enterDialogue's to settle: it stays inside
	// the person's own group of eight.  Walking on from an empty one, which is
	// what used to happen here, ran out of the group and into the next
	// person's.
	if (_dialoguePart >= _dialogue.count())
		return;
	if (_menuKind == kMenuTalk &&
			(_dialogueMask == 0x80 || (_speakerPerson == 13 && _dialogueMask == 0x20)) &&
			_speakerPerson >= 0 && (_dialoguePart & 7) < 4 &&
			READ_LE_UINT16(_state.begin() + 0x47ba) == 0xffff) {
		// A09F consumes an action's end-report latch and A0A7 leaves
		// SI=FFFF. 94B9 closes, except for 13's 94CC request-mask restart.
		if (_speakerPerson == 13) {
			_dialoguePart &= ~7u;
			_dialogueLine = 0xffff;
			_dialogueMask = 0x20;
			WRITE_LE_UINT16(_state.begin() + 0x47be, _dialoguePart);
			WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
			_state[0x47c2] = 0x20;
		} else {
			finishRoomConversation();
			return;
		}
	}
	// 949F/9B49 belongs to ordinary news, after 94B9's terminal/restart
	// branch and before 9F9E expands context. Request parts use 9F8B instead;
	// multipart pages use 94DD and must not lower the card between pages.
	if ((_dialoguePart & 7) < 4 && !_dialogueActionPending &&
			_speechPart + 1 >= _speechParts.size() &&
			(_speakerPerson == 13 || READ_LE_UINT16(_state.begin() + 0x47ba) != 0xffff))
		closeLocalSpeakerReport();
	if (_speakerPerson >= 0)
		preparePersonDialogueContext((uint)_speakerPerson);

	// A restored/previously accepted load must be dispatched before Duncan's
	// news reader treats C0 as a completed shipment and queues its reaction.
	if (_speakerPerson == 3 && _menuKind == kMenuTalk && checkSpiceShipment())
		return;

	// Jessica's "Wait! I can feel something..." is selected from her
	// making-off part because Paul pressed the compass without ending their
	// corridor conversation. Once the interruption has been read, TALK goes
	// back to her ordinary room observations and reveals the left-hand door.
	if (_speakerPerson == 1 && _dialoguePart == 12 && _dialogueLine == 3 &&
		storyStage() == 6 && _state[0xb] == 2) {
		_dialoguePart = 9;
		_dialogueLine = 4; // the record before PHRASE11 line 59
	}

	// The automatic stillsuit-store observation lives in Gurney's event part,
	// while his following TALK continues in the ordinary room-observation
	// part.  0x96F1 returns to that news part after the event record; keeping
	// the event part selected made TALK inert or repeat an unrelated line.
	if (_speakerPerson == 4 && _dialoguePart == 36 && _dialogueLine == 2 &&
		storyStage() == 5) {
		_dialoguePart = 35;
		_dialogueLine = 5; // the record before PHRASE11 line 351
		_state[0x19] = 0;
	}

	// Duncan's automatic room-eight shipment confirmation is the last record
	// of his making-off part. The next TALK returns to his ordinary news and
	// first says that the requested amount was sent; leaving the request part
	// selected instead repeated unrelated COME WITH ME/STAY HERE replies.
	if (_speakerPerson == 3 && _dialoguePart == 28 && _dialogueLine == 2 &&
		_state.size() > 0xc1 && READ_LE_UINT16(_state.begin() + 0xc0) != 0 &&
		_state[0xb] == 8) {
		_dialoguePart = 25;
		_dialogueLine = 1; // the record before PHRASE11 line 208
		_state[0x19] = 0;
	}

	// Ordinary news follows 9486/9B49's forward-only cursor. In particular,
	// Gurney's sietch proposal must not jump back to his harvester lesson:
	// a real equipment visit already selected that earlier record on entry.

	// At the hidden Tuono sietch, Gurney follows the general harvester lesson
	// with the specific instruction to send the two nearest troops to collect
	// the machines.  That instruction is in the next news part; scanning all of
	// the current part first selected a generic sunset/night observation and
	// postponed the story clue indefinitely.
	if (_speakerPerson == 4 && _dialoguePart == 33 && _dialogueLine == 0 &&
		storyStage() == 13 && _state.size() > 0x114f &&
		READ_LE_UINT16(_state.begin() + 0x114e) == 0x100 + 16 * 28 &&
		_dialogue.count() > 34 && _dialogue.entry(34).size() > 9) {
		_dialoguePart = 34;
		_dialogueLine = 9; // PHRASE11 line 327
		sayChosenLine();
		return;
	}

	// After reporting that a sietch has no troop, Stilgar immediately inspects
	// its unattended equipment.  The following record reads the expanded place
	// bytes at 0x55..0x5b; map/contact code may have left those describing the
	// last remote troop, so expand the place Paul is actually standing in before
	// looking for that continuation.
	if (_speakerPerson == 5 && _dialoguePart == 42 && _dialogueLine == 6 &&
			_state.size() > 0x114f) {
		const uint16 place = READ_LE_UINT16(_state.begin() + 0x114e);
		if (place >= 0x100 && place + 0x1a < _state.size()) {
			WRITE_LE_UINT16(_state.begin() + 0x11ce, place);
			nearestPlaces(place);
			_state[0x4d] = _state[place + 8];
			WRITE_LE_UINT16(_state.begin() + 0x4e,
							(uint16)((_state[place] << 8) | _state[place + 1]));
			_state[0x51] = _state[place + 0xa];
			_state[0x52] = _state[place + 0x12];
			for (uint i = 0; i != 7; i++)
				_state[0x55 + i] = _state[place + 0x14 + i];
		}
	}

	Common::Array<Dialogue::Record> &part = _dialogue.entry(_dialoguePart);
	const bool ordinaryNews = _menuKind == kMenuTalk &&
		(_dialogueMask == 0x80 || (_speakerPerson == 13 && _dialogueMask == 0x20)) &&
		_speakerPerson >= 0 && (_dialoguePart & 7) < 4;
	if (part.empty() && !ordinaryNews)
		return;

	// 9486 resumes after the completed record; 94AB walks forward through
	// the remaining news parts. It never wraps ordinary news to earlier
	// repeatable records. Those can be heard again in a fresh conversation.
	bool found = false;
	const uint first = _dialogueLine == 0xffff ? 0 : _dialogueLine + 1;
	for (uint line = first; line < part.size(); line++) {
		if (dialogueRecordAvailable(part[line], _dialogueMask) &&
			conditionHolds(part[line].condition)) {
			_dialogueLine = line;
			found = true;
			break;
		}
	}

	if (!found) {
		// The first four parts in a person's group carry their news and reports;
		// the last four are separate requests, including making off, COME WITH
		// ME and STAY HERE. TALK may continue into the other news parts (Duncan's
		// production cards are in part two), but must never fall into a request
		// Paul did not make.  The old eight-part walk did exactly that after
		// Gurney's stillsuit clue, answering with a line about staying behind.
		const uint group = _dialoguePart & ~7u;
		const uint local = _dialoguePart & 7u;

		// An answer to a request Paul did make carries on into the eighth
		// part, which nothing else ever reaches: Thufir's word that he has
		// gathered the others for the attack on the palace lives there, and
		// with it the action which moves the war on.  Reading on from a
		// request is not the same as falling into one Paul never made.
		if (local == kComingAlong || local == kStayingBehind) {
			enum { kEighth = 7 };
			const uint eighth = group + kEighth;

			if (eighth < _dialogue.count())
				for (uint line = 0; line != _dialogue.entry(eighth).size(); line++) {
					if (!dialogueRecordAvailable(_dialogue.entry(eighth)[line], _dialogueMask) ||
						!conditionHolds(_dialogue.entry(eighth)[line].condition))
						continue;

					_dialoguePart = eighth;
					_dialogueLine = line == 0 ? 0xffff : line - 1;
					sayNextLine();
					return;
				}
		}

		if ((_dialogueMask == 0x80 || ordinaryNews) && local < 4) {
			for (uint offset = 1; offset < 4 && !found; offset++) {
				if (ordinaryNews && local + offset >= 4)
					break;
				const uint next = group + ((local + offset) & 3);
				if (next >= _dialogue.count())
					continue;

				Common::Array<Dialogue::Record> &lines = _dialogue.entry(next);
				for (uint line = 0; line < lines.size(); line++) {
					if (!dialogueRecordAvailable(lines[line], _dialogueMask) ||
						!conditionHolds(lines[line].condition))
						continue;

					_dialoguePart = next;
					_dialogueLine = line == 0 ? 0xffff : line - 1;
					sayNextLine();
					return;
				}
			}
		}

		if (ordinaryNews) {
			// 94C0 jumps to D2E2 on news exhaustion, invoking the same
			// 97CF cleanup as STOP TALKING. Smuggler 13 instead has its
			// 94CC restart: first news part, with mask 20 instead of 80.
			if (_speakerPerson == 13) {
				_dialoguePart = group;
				_dialogueLine = 0xffff;
				WRITE_LE_UINT16(_state.begin() + 0x47be, group);
				WRITE_LE_UINT16(_state.begin() + 0x47ba, 0);
				_state[0x47c2] = 0x20;
				_dialogueMask = 0x20;
				// 94CC also restarts when the previous pass already used 20.
				// Search once here rather than recursively retry an invalid
				// group with no eligible records forever.
				for (uint next = group; next < group + 4 && next < _dialogue.count(); next++) {
					const Common::Array<Dialogue::Record> &lines = _dialogue.entry(next);
					for (uint line = 0; line < lines.size(); line++) {
						if (!dialogueRecordAvailable(lines[line], 0x20) ||
								!conditionHolds(lines[line].condition))
							continue;
						_dialoguePart = next;
						_dialogueLine = line;
						WRITE_LE_UINT16(_state.begin() + 0x47be, next);
						sayChosenLine();
						return;
					}
				}
				return;
			}
			finishRoomConversation();
			return;
		}

		// If no later news part applies, allow repeatable instructions before
		// the record just spoken to wrap within the selected part.
		const uint until = MIN(first, part.size());
		for (uint line = 0; line < until && !found; line++) {
			if (!dialogueRecordAvailable(part[line], _dialogueMask) ||
				!conditionHolds(part[line].condition))
				continue;

			_dialogueLine = line;
			found = true;
			break;
		}

		if (!found) {
			debugC(1, kDebugScript, "part %u has no line whose condition holds", _dialoguePart);
			return;
		}
	}

	sayChosenLine();
}

/** Speaks the resource record already selected by the dialogue traversal. */
void DuneEngine::sayChosenLine() {
	if (_dialoguePart >= _dialogue.count() ||
		_dialogueLine >= _dialogue.entry(_dialoguePart).size())
		return;

	Common::Array<Dialogue::Record> &part = _dialogue.entry(_dialoguePart);
	Dialogue::Record &record = part[_dialogueLine];
	// 9FD8 excludes map/modal display and the narrator. Requests retain their
	// selected local caller even when their record belongs to shared part133.
	if (!_state[0x46eb] && _speakerPerson >= 0 && _speakerPerson < 16)
		lowerPanelFace();

	debugC(2, kDebugScript, "line %d, condition %d: %s", record.line,
		   record.condition, _condit.describe(record.condition).c_str());

	prepareDialogueLine(record, _dialoguePart / 8);

	const uint speaker = _dialoguePart == 133 && _speakerPerson >= 0
		? (uint)_speakerPerson : _dialoguePart / 8;
	debugC(1, kDebugScript, "%s says: %s",
		   command(kFirstPerson + speaker).c_str(), _speech.c_str());
	_speakerPerson = (int)speaker;
	_speakerReport = 0;

	// One recording to a line, named after the line's own number.  Not every
	// line has one; the ones that do are said aloud.
	// Shared prisoner replies use the person Paul asked; ordinary parts use
	// their eight-part speaker group. 95B7 enables the automatic-event base
	// for part 133 without changing the caller's bank (A6F1), so Gurney's
	// replies are PE3FA/PE3FB, not offsets in his ordinary dialogue set.
	const uint16 voiceLine = record.line;
	const bool reportVoice = _dialoguePart == 133;
	_dialogueActionPending = true;
	const bool singlePage = _speechParts.size() <= 1;
	if (singlePage) {
		completeDialogueLine();
	} else {
		if (!_state[0x46eb] && _speakerPerson >= 0 && _speakerPerson < 16)
			prepareLocalSpeakerParagraph();
		if (_inDesert)
			drawDesertDialogue(false);
		else
			drawScene(false);
	}
	// A042 runs before A0D3/9EFD even on the first (only) text page.
	// Room-request cleanup and accepted departures follow the started voice.
	playSpeech(voiceLine, (int)speaker, reportVoice);
	presentDialogueLine();
	if (singlePage)
		finishDialogueLine();
}

/** A042 defers the action, read mark and cursor until the last text page. */
void DuneEngine::completeDialogueLine() {
	if (!_dialogueActionPending)
		return;
	_dialogueActionPending = false;
	if (_dialoguePart >= _dialogue.count() ||
			_dialogueLine >= _dialogue.entry(_dialoguePart).size())
		return;
	Dialogue::Record &record = _dialogue.entry(_dialoguePart)[_dialogueLine];

	runLineAction(record);

	// 0xA092 records that this pass produced a line.  Duncan's valuable-spice
	// continuation (condition 206) tests this byte; leaving it at the zero set
	// when he was taken up selected his ellipsis (condition 25) instead.
	_state[0x19] = 0xff;

	// 0xA097 marks every record after its action has run.  In particular this
	// keeps introductions and other stage-changing lines from firing twice.
	rememberBookRecord(record, _speakerPerson >= 0 ? _speakerPerson : _dialoguePart / 8);
	record.flags |= 0x80;
	// A09F consumes the action latch once, retaining FFFF as the next news
	// cursor for early termination. Keep the selected record for WHAT and
	// finish any remaining text pages before the next TALK closes the menu.
	WRITE_LE_UINT16(_state.begin() + 0x47ba, _state[0x47a8] ? 0xffff : 0);
	_state[0x47a8] = 0;

	// Opening the right-hand communication-room door begins a short, ordered
	// gathering rather than returning straight to the ordinary TALK TO panel.
	// Keep Jessica's completed description above the event's first Continue row.
	if (_speakerPerson == 1 && record.line == 70 && storyStage() == 12)
		preparePalaceGathering(0);
	// An ordinary automatic room-entry request can select Jessica's return
	// greeting too. Its action has already started 134F; expose Continue now
	// rather than leaving TALK active behind 4774's event gate. Otherwise the
	// greeting repeats and the script never reaches its first spoken beat.
	if (_speakerPerson == 1 && record.line == 104 && storyStage() >= 20 && storyStage() < 24)
		preparePalaceGathering(101);
	// Leto's automatic greeting starts 1370 through the same request path.
	// Retain his completed line, then let the script select Jessica; TALK
	// cannot continue this interleaved audience while the event gate is active.
	if (_speakerPerson == 0 && record.line == 33 && _state[0x4774] &&
			READ_LE_UINT16(_state.begin() + 0x477a) == 0x1370)
		preparePalaceGathering(201);
	// Kynes' first greeting advances stage 85 to 88.  The following replies
	// are an ordered introduction shared by Chani, Kynes and Stilgar, shown
	// under the event's Continue/WHAT panel rather than the ordinary talk list.
	if (_speakerPerson == 6 && record.line == 14 && storyStage() == 88) {
		// The line's stage-88 action has already called 1196/1771 to start
		// CS:12FB. Starting it here again increments DS:4774 twice.
		preparePalaceGathering(400);
	}
	// Chani's night confession launches the 0x1313 event script.  Keep her
	// completed line on screen, but replace ordinary TALK TO with the sequence's
	// Continue/WHAT panel so the next press begins the event rather than skipping
	// ahead to her later sietch clue.
	if (_speakerPerson == 7 && record.line == 40 && storyStage() == 72)
		preparePalaceGathering(310);
	// Gurney's "Hey! here we are" starts CS:12DB: nine resource-backed
	// adviser replies around the sietch's room-two redraw. Ordinary TALK
	// traversal cannot join five different speakers in this scripted order.
	if (_speakerPerson == 4 && record.line == 361 && storyStage() >= 48)
		preparePalaceGathering(600);
	// Known campaign presentations above retain their already paired paths.
	// Other callers of A1F7 still need an actual conditional event reader.
	if ((record.flags & 0xf) == 3 && _state[0x4774] && _menuKind != kMenuPalaceGathering)
		showStageEventMenu(true);

	// A0AA/978E follows the final page's action and read mark. Earlier
	// pages enter it without running that action, through the callers above.
	if (!_state[0x46eb] && _speakerPerson >= 0 && _speakerPerson < 16)
		prepareLocalSpeakerParagraph();
	if (_inDesert)
		drawDesertDialogue(false);
	else
		drawScene(false);
}

/** 94DA/D280 presents the prepared page and rolls the panel after 9EFD. */
void DuneEngine::presentDialogueLine() {
	finishPanelMenuTransition();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
			kScreenWidth, kScreenHeight);
	_system->updateScreen();
	// A0E2 restores the user's mode after presenting each local paragraph.
	// The already-prepared forced caption remains owned by its retained page.
	if ((int8)_state[0xfb] >= 0)
		_state[0x28e7] = _state[0x28e8];
}

/** Caller cleanup follows 9EFD, not the paragraph's A042 action. */
void DuneEngine::finishDialogueLine() {
	if (_dialogueActionPending || _dialoguePart >= _dialogue.count() ||
			_dialogueLine >= _dialogue.entry(_dialoguePart).size())
		return;
	const Dialogue::Record &record = _dialogue.entry(_dialoguePart)[_dialogueLine];
	const bool waterOfLife = _speakerPerson == 5 && (record.flags & 0xf) == 8;
	// Acceptance can happen in the communication room itself. There is no
	// subsequent room-entry event in that case (35AD runs after dialogue too).
	if (_speakerPerson == 3 && (record.flags & 0xf) == 9 &&
			_state[0x476d] == 0 && _state[0x9f] == 1)
		checkSpiceShipment();
	finishLocalRoomRequest();
	if (waterOfLife)
		finishWaterOfLife();
}

/** DOS2CCF: complete Stilgar's answer, drink, then poll the room after FD9. */
void DuneEngine::finishWaterOfLife() {
	_state[0x0a] |= 0x08;
	if (_state[0x9f] != 1)
		return;

	_waterOfLifeActive = true; // The synchronous continuation cannot be saved midway.
	const bool fast = ConfMan.getBool("dune_nodelay");
	const bool cursorShown = CursorMan.showMouse(false); // DBB2
	auto wait = [&](uint32 milliseconds, bool speaking, bool reading) {
		if (fast) {
			advanceClockElapsed(milliseconds, false, false);
			return;
		}
		const uint32 started = _system->getMillis();
		while (!shouldQuit() && (speaking ? _mixer->isSoundHandleActive(_speechHandle) :
				_system->getMillis() - started < milliseconds)) {
			const PlaybackResult asked = pollSkip();
			if (asked == kPlaybackAborted || (reading && asked != kPlaybackFinished))
				break;
			_system->delayMillis(5);
			tick(false); // DDB0/E387/ABD5 leave hourly work for FD9.
			updateGameMusic();
			if (speaking)
				updateLocalSpeaker();
		}
	};
	const bool speaking = _mixer->isSoundHandleActive(_speechHandle);
	wait(speaking ? 0 : 3000, speaking, !speaking); // DDB0's600-tick reading window.
	if (!shouldQuit() && _state[0x29] < 100) {
		_state[0x46d9] = 3; // 2D26: the unsuccessful trial kills Paul.
	} else if (!shouldQuit()) {
		_state[0x0a] |= 0x02;
		_state[0xd5] = 0xff;
		byte palette[768];
		_system->getPaletteManager()->grabPalette(palette, 0, 256);
		keepScreen();
		applyWipe(0x38);
		wait(5000, false, false); // E387/1000, with the faded page retained.
		if (!shouldQuit()) {
			// 8C8A removes the caption before36 brings the room back.
			stopSpeech();
			_speech.clear();
			_speechParts.clear();
			_speechPart = 0;
			updateSpeechContinuation();
			drawScene(false);
			keepScreen();
			_system->getPaletteManager()->setPalette(palette, 0, 256);
			applyWipe(0x36);
			advanceWaitHours(3);
			_state[0x23] = 0x11;
			showLocalRoomRequests(); // 2D23/35AD, including Chani's reaction.
		}
	}
	CursorMan.showMouse(cursorShown);
	_waterOfLifeActive = false;
}

/**
 * Steps through one of the four ways out of a room into the room beyond it.
 * Every place keeps its rooms in the table at DS:0x13C4: five bytes to a
 * room, the first saying what it looks like and the four after it which room
 * each way out leads to.  What used to happen here was that an edge walked
 * the scenes of the file one after another, which went through rooms that do
 * not join and missed the ones that do.
 */
void DuneEngine::goThroughEdge(Cursor cursor) {
	if (_localRoomRequestPending)
		return; // An automatic paragraph owns input until its last page.
	if (_pendingBoardingPlace >= 0) {
		stopSpeech();
		finishVehicleBoarding();
		return;
	}
	if (_pendingRoomExit >= 0) {
		stopSpeech();
		finishRoomDeparture();
		return;
	}
	if (_pendingRoomCompass >= 0) {
		finishDeferredRoomCompass();
		return;
	}
	if (!_inDesert && battleHere())
		return; // 2FFB: local battle has no room exits.
	// Which of the four ways out was taken.  The handlers the compass names -
	// 0x3F15, 0x3F1A, 0x3F1F and 0x3F24 - count them north, east, south and
	// west, and 0x3F6A reads the room's record at that many bytes in.
	int way = -1;
	if (cursor == Cursor::North)
		way = 0;
	else if (cursor == Cursor::East)
		way = 1;
	else if (cursor == Cursor::South)
		way = 2;
	else if (cursor == Cursor::West)
		way = 3;

	if (way < 0)
		return;

	if (!_inDesert) {
		if (_menuKind == kMenuMessageView) {
			// 3F27's D2BD unwinds1FF2 through2997, with request zero.
			// Clear the remote edition and finish the console before leaving.
			const byte previousRoll = _state[0xdce6];
			_state[0xdce6] = 0x80;
			closeMessage(0);
			_state[0xdce6] = previousRoll;
			if (_localRoomRequestPending || _state[0x4774]) {
				_pendingRoomCompass = way;
				return; // The callback's automatic paragraph owns input first.
			}
		}
		// 3F27 calls D2BD before AC30/A7A5. Its ordinary talk-header
		// callback gives the floor back and raises the panel face before
		// trying the exit. A resident can then veto that attempt and lower
		// the face again; keeping the old talk open skipped both phases.
		if (_menuKind == kMenuTalk)
			finishRoomConversation();
		finishSoundEffectLoop(); // 3F2B/AC30, even for a closed door.
		stopSpeech(); // 3F2E/A7A5, BEFORE selecting a new departure reply.
	}

	// 3F32/3F37 clear the previous move's request/first-visit latches on
	// every compass attempt, even if the door is closed or already visited.
	// 3F9A sets the visit latch again only for a newly entered place.
	_state[0x47a9] = 0;
	_state[0x26] = 0;

	// Out of doors an edge of the screen is a step over the world and not a
	// door: 0x3F44 only reads the room table where the low byte of DS:0x6 is
	// 0x80, which it is not out here.
	if (_inDesert) {
		stepThroughDesert(way);
		return;
	}
	const byte kindOfPlace = _state.size() > 8 ? _state[8] : 0;
	const byte room = _state.size() > 0xb ? _state[0xb] : 0;
	const byte *record = roomRecord(kindOfPlace, room);

	if (!record)
		return;

	const byte leads = record[1 + way];

	if (leads == 0)
		return;

	// A room transition is authoritative about what is now on screen.  This is
	// normally already false, but it also makes a resumed/scripted compass move
	// out of a saved map or cabin discard that overlay instead of serialising a
	// room picture with the old map flag and restoring the wrong interface.
	_onWorld = false;
	_onGlobe = false;
	_inCabin = false;

	// A way out that names a room the story has still to open is shut, which
	// 0x3066 tells apart by comparing the byte as a signed number against -5:
	// what is left of the high half, 0xFB to 0xFF, leaves the building
	// altogether, and 0x80 to 0xFA is the same room the low half would name
	// with a door across it.  Sending those out into the desert as well put
	// the player half way across the world through a door that is shut.
	if (leads >= 0x80 && (int8)leads < -5) {
		debugC(1, kDebugScript, "the way out of the %s of room %d is shut",
			   way == 0 ? "north" : way == 1 ? "east" : way == 2 ? "south" : "west",
			   room);
		return;
	}

	// 0x3FD2 makes the way's own number positive and reads the step at twice
	// that in the table at DS:0x1454, so 0xFF is north, 0xFE east, 0xFD south
	// and 0xFC west - the same way round as the slot the number stands in.
	if (leads >= 0x80) {
		debugC(1, kDebugScript, "out of the %s of room %d and into the desert",
			   way == 0 ? "north" : way == 1 ? "east" : way == 2 ? "south" : "west", room);
		enterDesert(way);
		return;
	}

	prepareRoomEntryCheckpoints(); // 3F72..3FA7, before a resident can veto entry.

	// 3FAA/36D3 tries every matching resident in the OLD room. The selected
	// person and the companion panel do not determine request priority.
	_state[0x0c] = leads;
	_state[0x23] = 1;
	releaseSilentLocalSpeaker(); // 36D3/98B2, before walking the old room's replies.
	_state[0x47a5] = 0xff; // A1C4: an absent reply permits departure.
	_state[0x47a7] = 0;
	_pendingRoomExit = way;
	_pendingRoomLead = leads;
	if (walkLocalRoomRequests()) {
		// Ordinary requests complete through finishLocalRoomRequest. The
		// queued-report fallback has already completed its own cleanup.
		if (!_localRoomRequestPending && _pendingRoomExit >= 0)
			finishRoomDeparture();
		return;
	}
	finishRoomDeparture();
}

/** 3F72/3F9F: automatic entry saves bracket the first-visit state writes. */
void DuneEngine::prepareRoomEntryCheckpoints() {
	if (_state[0xb] == 1)
		saveRoomEntryCheckpoint(kLastPlaceSaveSlot);

	// The second checkpoint includes the new visit flag/count, but still
	// stands in the old room. Even a later resident refusal retains it.
	const byte kindOfPlace = _state[8];
	if (_state.size() > 0x114f) {
		const uint32 place = READ_LE_UINT16(_state.begin() + 0x114e);

		if (place >= 0x100 && place + 0xa < _state.size() &&
			!(_state[place + 0xa] & 0x10)) {
			_state[place + 0xa] |= 0x10;
			if (kindOfPlace < 0x20)
				_state[0x25]++;
			_state[0x26] = 0xff;
			saveRoomEntryCheckpoint(kNewSietchSaveSlot);
		}
	}
}

/** Resume3F27 only after its message-close callback has returned. */
void DuneEngine::finishDeferredRoomCompass() {
	if (_pendingRoomCompass < 0 || _localRoomRequestPending || _state[0x4774] ||
			_pendingRoomExit >= 0 || _pendingBoardingPlace >= 0)
		return;
	const int way = _pendingRoomCompass;
	_pendingRoomCompass = -1;
	static const Cursor ways[] = {Cursor::North, Cursor::East, Cursor::South, Cursor::West};
	goThroughEdge(ways[way]);
}

/** 36E8/3FBD/A1E2/ABD5: clear the request, veto, then wait before moving. */
void DuneEngine::finishRoomDeparture() {
	if (_pendingRoomExit < 0 || _localRoomRequestPending)
		return;
	_state[0x23] = 0;
	if (_state[0x47a5] != 0xff) {
		_pendingRoomExit = -1;
		_pendingRoomLead = 0;
		return;
	}
	// 3FC5 waits for the recording after the gate, without reselecting or
	// moving followers. The ordinary event loop services mouths and input.
	if (_mixer->isSoundHandleActive(_speechHandle))
		return;
	const int way = _pendingRoomExit;
	const byte leads = _pendingRoomLead;
	_pendingRoomExit = -1;
	_pendingRoomLead = 0;
	completeRoomMove(leads, way);
}

/** 3FCA/4057: an accepted interior move, after the old-room reply. */
void DuneEngine::completeRoomMove(byte leads, int way) {
	const byte kindOfPlace = _state[8];
	const byte room = _state[0x0b];
	moveMatchingFollowers((uint16)((kindOfPlace << 8) | leads),
		READ_LE_UINT16(_state.begin() + 6));
	_state[0xd] = _state[0xb];
	_state[0xb] = leads;
	_state[0xc] = leads;
	WRITE_LE_UINT16(_state.begin() + 4, (uint16)((kindOfPlace << 8) | leads));
	_state[0x23] = 5; // 3FCA: successful interior move, not person identity.
	WRITE_LE_UINT16(_state.begin() + 0x14, 0); // 2E07: contact bits for the new room.

	// Stepping into the room the story ends in ends it, which is what 0x4072
	// asks of these same two words.
	const bool endingBegins = !storyIsOver();
	endStoryOnReaching((uint16)((kindOfPlace << 8) | leads));

	const byte *arrived = roomRecord(kindOfPlace, leads);
	if (arrived && arrived[0])
		_place = (byte)(arrived[0] - 1);

	debugC(1, kDebugScript, "through the %s of room %d into room %d, which is 0x%02x",
		   way == 0 ? "north" : way == 1 ? "east" : way == 2 ? "south" : "west",
		   room, leads, _place);

	// Whoever was being spoken to is left behind in the room that was, and so
	// is whatever was written over it - the words of a place the player has
	// just come to, among them, which used to follow them from room to room.
	letGoOfPerson();
	stopSpeech();
	_speech = "";

	if (endingBegins && storyIsOver()) {
		// CS:128F draws the crowded throne room and stops at 12A2 before
		// Gurney's first line. The event menu makes its explicit roster live.
		WRITE_LE_UINT16(_state.begin() + 0x4778, 0x1291);
		WRITE_LE_UINT16(_state.begin() + 0x477a, 0x12a2);
		preparePalaceGathering(500);
		drawScene();
		return;
	}

	peopleHere(); // 3FCA/2E07 prepares account context before room painting.
	drawScene();
	showRoomMenu();
	bool roomEntryNotice = showLocalRoomRequests();

	// After the Water-of-Life decision, entering the palace's first inner room
	// begins Jessica's news of Leto's death.  Its opening sentence is automatic
	// PHRASE11 line 111 rather than a DIALOGUE.HSQ record; the following WHAT
	// continues at Jessica's line 56 and then line 57.  Leaving this as an
	// ordinary room menu let her generic comments win and stranded the campaign
	// before the Thufir briefing.
	if (!roomEntryNotice && kindOfPlace == 0x20 && leads == 2 && storyStage() == 76 &&
			(_state[0x0a] & 0x08) != 0 && _dialogue.count() > 9 &&
			_dialogue.entry(9).size() > 2) {
		takeUpPerson(1);
		_dialogueMask = 0x80;
		_dialoguePart = 9;
		_dialogueLine = 1; // the record immediately before PHRASE11 line 56
		showTalkMenu(false);
		sayLineInParts(fillInLine(phraseOf(1, 111)));
		_speakerPerson = 1;
		_speakerFrame = 0;
		_speakerAnimation = 0;
		_speakerReport = 0;
		playSpeech(111, 1);
		debugC(1, kDebugScript, "%s says: %s",
			   command(kFirstPerson + 1).c_str(), _speech.c_str());
		drawScene();
		roomEntryNotice = true;
	}

	// Stilgar's introduction is Leto's actual room-entry request (line 33,
	// action 3). showLocalRoomRequests() starts 1370 where Leto resides;
	// completeDialogueLine() exposes its Continue menu. Do not relocate the
	// parents or force that audience in room four before Paul reaches them.

	if (!roomEntryNotice)
		roomEntryNotice = checkSpiceShipment();

	// DOS 3FCA starts gatherings through the resident request walk above.
	// DS:25 counts visited sietches; eleven visits and a viewed shipment reply
	// do not relocate Leto, Jessica or Harah into room four. Jessica's return
	// greeting (line 104, condition 81) starts 134F in her actual room through
	// showLocalRoomRequests()/sayChosenLine(), not a separate palace fallback.

	// Jessica's automatic sensing requests were already selected by the
	// resource-backed making-off walk above. Ordinary part-nine news is
	// selected only by TALK; a room-dependent condition alone does not make
	// it an arrival event. Searching it again injected unwanted paragraphs
	// into every doorway and started the communications gathering too soon.
	// 2E5B/2E6C/2E7A raises Paul's panel face only when this room render
	// produced no automatic paragraph and no event still owns the scene.
	if (!roomEntryNotice && !_state[0x4774])
		raisePanelFace();
}

/**
 * DOS 0xB647 projects a place relative to the exact map centre. Longitude
 * uses that latitude's stored width, not a fixed equatorial pixel grid.
 */
Common::Point DuneEngine::placeOnScreen(const Place &place) const {
	return _map.strategicPosition(_worldLongitude, _worldLatitude, place.longitude, place.latitude);
}

Common::Point DuneEngine::playerMapPosition() const {
	if (_state.size() <= 0x114f)
		return Common::Point(-32768, -32768);
	if (_state[6] == 0x80) {
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here < 0x100 || (here - 0x100) / 28 >= _map.places().size())
			return Common::Point(-32768, -32768);
		return placeOnScreen(_map.places()[(here - 0x100) / 28]);
	}
	return _map.strategicPosition(_worldLongitude, _worldLatitude,
			READ_LE_UINT16(_state.begin() + 4), (int8)_state[6]);
}

/**
 * Where Paul is standing, in the rows and columns the flat map is laid out
 * in, or -1,-1 where that is not known.  0x407E hands his own longitude and
 * latitude out: inside a place they are the place's, which the word at
 * DS:0x114E names, and out in the sand the word at DS:4 and the signed byte
 * at DS:6 carry them directly.
 */
Common::Point DuneEngine::whereWeStand() const {
	uint16 longitude = 0;
	int16 latitude = 0;

	if (_state.size() > 6 && _state[6] == 0x80) {
		const uint32 at = _state.size() > 0x114f
			? READ_LE_UINT16(_state.begin() + 0x114e) : 0;

		if (at < 0x100 || at + 6 > _state.size())
			return Common::Point(-1, -1);

		longitude = READ_LE_UINT16(_state.begin() + at + 2);
		latitude = (int16)READ_LE_UINT16(_state.begin() + at + 4);
	} else if (_state.size() > 6) {
		longitude = READ_LE_UINT16(_state.begin() + 4);
		latitude = (int8)_state[6];
	} else {
		return Common::Point(-1, -1);
	}

	return Common::Point((int)((uint32)longitude * Map::kEquatorWidth / 65536u),
						 (int)Map::kEquator + latitude);
}

/**
 * World-grid proximity for the worm selector. Cabin site selection instead
 * uses DOS 5E6D's projected screen-space descriptors below.
 */
int DuneEngine::placeNearest(int across, int down) const {
	enum { kReach = 5 };

	int nearest = -1, best = kReach + 1;

	for (uint i = 0; i != _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		if (!place.isKnown())
			continue;

		const int x = (int)((uint32)place.longitude * Map::kEquatorWidth / 65536u);
		const int y = (int)Map::kEquator + place.latitude;
		const int off = ABS(x - across) + ABS(y - down);

		if (off < best) {
			best = off;
			nearest = (int)i;
		}
	}

	return nearest;
}

/** 4586/5E6D: the cabin picks the nearest visible descriptor in screen pixels. */
int DuneEngine::cabinPlaceAt(const Common::Point &position) const {
	const Common::Rect view(81, 45, 241, 134); // DS:149C
	if (!view.contains(position))
		return -1;
	int selected = -1, nearest = 9;
	for (uint i = 0; i < _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		if (!place.isKnown() || place.kind == 0xff)
			continue;
		const Common::Point at = _map.surveyPosition(_worldLongitude,
				_worldLatitude, place.longitude, place.latitude, view);
		if (!view.contains(at))
			continue;
		const int distance = ABS(position.x - at.x) + ABS(position.y - at.y);
		if (distance < nearest) {
			nearest = distance;
			selected = i;
		}
	}
	return selected;
}


/**
 * Which point of the world a press on the flat map falls on, or false where it
 * falls outside the map at all. Invert the latitude-dependent strategic
 * projection, then convert to the world-point coordinates the flight uses.
 */
bool DuneEngine::worldPointsAt(const Common::Point &at, int *across, int *down) const {
	if (!across || !down || at.y >= kSceneHeight || at.y < 0 ||
			at.x < 0 || at.x >= kScreenWidth)
		return false;

	const int latitude = _worldLatitude + ((at.y - 76) >> 2);
	if (latitude < -98 || latitude > 98)
		return false;
	const int width = _map.width(ABS(latitude));
	if (!width)
		return false;
	const uint16 longitude = (uint16)(_worldLongitude + (at.x - 160) * 65536 / (4 * width));
	*across = (uint32)longitude * Map::kEquatorWidth / 65536;
	*down = Map::kEquator + latitude;

	return *down >= 0 && *down < (int)Map::kMapRows;
}

int DuneEngine::placeAt(const Common::Point &position) const {
	// 5CB7 checks the current tag. With MAP2 open (C0), the retained
	// strategic entries (80) cannot open a site report on the same click.
	return nearestMapPlace(position, _state[0x46eb], 0x31, 20);
}

/** 5E6D: pick from the painted list, retaining the first descriptor on ties. */
int DuneEngine::nearestMapPlace(const Common::Point &position, byte mode, byte kindLimit, int nearest) const {
	int selected = -1;
	const byte *data = _state.begin();
	for (uint descriptor = 0xa5c0; descriptor + 6 <= 0xaa70; descriptor += 6) {
		const uint record = READ_LE_UINT16(data + descriptor);
		if (!record)
			break;
		if (data[descriptor + 5] != mode || record < 0x100 || (record - 0x100) % 28)
			continue;
		const uint candidate = (record - 0x100) / 28;
		if (candidate >= _map.places().size() || data[record + 8] >= kindLimit)
			continue;
		const Common::Point at(READ_LE_INT16(data + descriptor + 2), data[descriptor + 4]);
		const int distance = ABS(at.x - position.x) + ABS(at.y - position.y);
		if (distance < nearest) {
			nearest = distance;
			selected = (int)candidate;
		}
	}
	return selected;
}

/**
 * Which of ONMAP.HSQ's little figures stands for a troop on the flat map.
 *
 * 0x6770 does not keep a picture in the troop's own record: it looks one up in
 * a table of lists, and a list is a run of three bytes at a time - which
 * sprite, and a signed movement delta - ending at a nought. Unlike the
 * close-up part lists, these are successive frames. 0xC5CF takes the first and,
 * where the list holds more than one, enters it into the drawing table at
 * DS:0x3CBE so that the frames go round.
 *
 * Which list is a troop's rests on the low nibble of its own byte at +3, which
 * is what it has been set to do: the words at DS:0x179C name a list for each
 * of the sixteen, and the words at DS:0x16B6 do the same for a troop whose
 * byte carries 0x10 or 0x20.  Work of the first kind, for example, is the four
 * figures nineteen to twenty-two, and military training is the twelve from
 * forty-three.
 *
 * A troop on its way keeps no place but a longitude and a latitude, and 0x6827
 * works its heading out of where it stands against where it is going: the
 * larger of the two distances says whether it is going across or down, its
 * sign says which way, and the words at DS:0x18BF hold four lists to a heading
 * for each of the four kinds of journey.
 */
uint DuneEngine::troopSpriteList(const Troop &troop, uint index) const {
	enum {
		kSettledLists = 0x179c, kBusyLists = 0x16b6, kTravellingLists = 0x18bf
	};

	// A troop whose own +0x10 carries 0x10 is not on the map at all.
	if (troop.flags & 0x0010)
		return 0;
	// 677D: an unmet Fremen troop is represented by the static marker only
	// at a revealed troop site. Non-Fremen actors bypass this contact gate.
	if (!(troop.flags & 0x0080) && (troop.state & 0x80))
		return (uint)troop.place + 0xa < _state.size() && (_state[troop.place + 0xa] & 0x10)
			? 0x181f : 0;

	uint32 table = kSettledLists;
	uint entry = troop.state & 0xf;

	if (troop.state & 0x40) {
		// 684A reads the live destination, not a later queued survey stop.
		int way = 0;
		const uint destination = (troop.place - 0x100) / 28;
		if (destination < _map.places().size()) {
			const Place &place = _map.places()[destination];

			const int across = (int)(int16)(place.longitude - troop.longitude) >> 8;
			const int down = (int)(place.latitude - troop.latitude);

			// 0x684A counts a heading north, east, south and west, taking
			// whichever of the two distances is the longer as the way it is
			// going and the sign of that one as which way round.
			way = ABS(across) >= ABS(down) ? (across < 0 ? 3 : 1)
											: (down < 0 ? 0 : 2);
		}

		table = kTravellingLists;
		entry = (uint)way + 4 * ((troop.state & 0xf) >> 2);
	} else if (troop.state & 0x30) {
		table = kBusyLists;
	}

	const uint32 at = table + 2 * entry;
	if (at + 2 > _state.size())
		return 0;

	uint32 list = READ_LE_UINT16(_state.begin() + at);
	if ((troop.state & 0x40) == 0) {
		const uint record = 0x8aa + index * 27;
		const byte equipment = record + 0x19 < _state.size() ? _state[record + 0x19] & 0xc0 : 0;
		if (troop.order() == 0 && equipment) {
			// 0x67AC/0x680A distinguish a harvester, an ornithopter and both.
			const uint kind = equipment == 0x80 ? 0 : equipment == 0x40 ? 1 : 2;
			static const uint16 working[] = { 0x17bc, 0x17c9, 0x1806 };
			static const uint16 idle[] = { 0x1813, 0x1817, 0x181b };
			list = (troop.state & 0x30) ? idle[kind] : working[kind];
		} else if ((troop.state & 0x30) == 0 && (list == 0x1774 || list == 0x1732)) {
			if (list == 0x1732 && (uint)troop.place + 0xa < _state.size() &&
					(_state[troop.place + 0xa] & 2)) {
				list = 0x16aa;
			} else {
				// Units at the back of the group use the corresponding facing.
				const uint slot = (troop.slot - 1) & 7;
				if (slot >= 3)
					list += 10 * MIN<uint>(slot - 2, 3);
			}
		}
	}
	if (list == 0 || list >= _state.size())
		return 0;

	return list;
}

bool DuneEngine::hasTroopContact() const {
	return _menuKind == kMenuTroopContact || _menuKind == kMenuOrders ||
		_menuKind == kMenuEquipment || _menuKind == kMenuProspectorBriefing ||
		_menuKind == kMenuMoveTroop ||
		// A1F7 can temporarily replace a troop console with another story
		// event. Preserve its actual owner through saves and FF, without
		// inventing a contact for ordinary room-owned gatherings.
		(isStageEventMenu() && _onWorld && _orderingTroop < _map.troops().size() &&
		 READ_LE_UINT16(_state.begin() + 0x46ef) == 0x8aa + 27 * _orderingTroop);
}

void DuneEngine::updateMapSpeech() {
	if (pollMapSpeechMouth()) {
		const int top = (_troopContactTop ? _troopContactTop : 5) + 4;
		_system->copyRectToScreen(_framebuffer1 + top * kScreenWidth + 10,
				kScreenWidth, 10, top, 59, 59);
		_system->updateScreen();
	}
}

/** A7C2/A82E/9E75: repaint only the retained contact face during a wait. */
bool DuneEngine::pollMapSpeechMouth() {
	if (!_onWorld || _onGlobe || _inCabin || _onMixer || !hasTroopContact() ||
			_orderingTroop >= _map.troops().size() || _troopPortraitVisible.size() != 59 * 59)
		return false;
	const int mouth = speechMouth();
	if (mouth < 0 || mouth == _troopContactMouth)
		return false; // A7A5 stops without replacing the last painted mouth.
	const int bank = _troopPortraitKey / 256;
	if (bank != 12 && (bank < 14 || bank > 16))
		return false;
	const uint crop = 0x22b9 + (bank == 12 ? 12 : 4 * (bank - 14));
	const int left = READ_LE_UINT16(_state.begin() + crop);
	const int top = READ_LE_UINT16(_state.begin() + crop + 2);
	if (left + 59 > kScreenWidth || top + 59 > kScreenHeight)
		return false;
	// Keep 9E75's clean/visible mouth history. A full showWorld() would erase
	// the in-progress command roll and could re-register map actors.
	drawTroopPortrait(bank == 12 ? 12 : 15, left, top, _troopPortraitKey & 255, mouth);
	_graphics.setFrameBuffer(_framebuffer1);
	const int faceTop = (_troopContactTop ? _troopContactTop : 5) + 4;
	for (int y = 0; y < 59; y++)
		memcpy(_framebuffer1 + (faceTop + y) * kScreenWidth + 10,
				_troopPortraitVisible.begin() + y * 59, 59);
	return true;
}

/** Where a troop's little standing figure falls on the flat map. */
enum { kStandingPoints = 0x1672 };

Common::Point DuneEngine::troopOnScreen(const Troop &troop, uint index) const {
	// A troop normally names the place it occupies.  While 0x40 is set its
	// record instead carries a live longitude and latitude at +6 and +8, which
	// is how the original keeps the marker moving between two places.
	if ((troop.state & 0x40) == 0 && troop.place >= 0x100) {
		const uint which = (troop.place - 0x100) / 28;
		if (which < _map.places().size()) {
			const Place &place = _map.places()[which];
			Common::Point at = placeOnScreen(place);

			// A place has sixteen points to stand a troop on and the troop
			// carries which of them is its own at +2, counted from one.  0x687B
			// reads the pair of signed bytes the point keeps at DS:0x1672 and
			// adds them to where the place itself falls, so that the troops of
			// one sietch stand about it rather than on top of one another; a
			// place whose byte at +0xA carries 2 has its points turned over,
			// which puts the group on the other side of it.
			byte point = (byte)(troop.slot - 1);
			if (place.flags & 0x02)
				point ^= 0x08;
			point &= 0x0f;

			const uint32 where = kStandingPoints + 2 * point;
			if (where + 2 <= _state.size()) {
				at.x += (int8)_state[where];
				at.y += (int8)_state[where + 1];
			}

			return at;
		}
	}

	return _map.strategicPosition(_worldLongitude, _worldLatitude, troop.longitude, troop.latitude);
}

/** 6946 reads the actor pool even when a clipped move caused no repaint. */
int DuneEngine::troopAt(const Common::Point &position, Common::Rect *bounds) const {
	const byte *actor = mapSpriteActorAt(_state.begin(), position);
	if (!actor)
		return -1;
	const uint owner = READ_LE_UINT16(actor + 10);
	if (owner < 0x8aa || (owner - 0x8aa) % 27)
		return -1;
	const uint troop = (owner - 0x8aa) / 27;
	if (troop >= _map.troops().size() || _state[owner + 3] >= 0x80)
		return -1;
	if (bounds)
		*bounds = spriteActorBounds(actor);
	return troop;
}

bool DuneEngine::canSelectMapTroop(uint troop) const {
	return troop < _map.troops().size() &&
		(READ_LE_UINT16(_state.begin() + 0x1176) >= 2 ||
		 _map.troops()[troop].place == READ_LE_UINT16(_state.begin() + 0x114e));
}

/** 86FA cycles visible, known actors by the smallest nonzero byte id delta. */
int DuneEngine::nextMapTroop() const {
	const Common::Array<Troop> &troops = _map.troops();
	const byte selected = _state[0x1954];
	uint best = 0x100;
	int next = -1;
	for (uint entry = 0; entry < _mapUnitOrder.size(); entry++) {
		const uint index = _mapUnitOrder[entry];
		if (index >= troops.size() || !troops[index].isKnown())
			continue;
		const byte delta = (byte)(troops[index].id - selected);
		if (delta && delta < best) {
			best = delta;
			next = index;
		}
	}
	return next;
}

bool DuneEngine::mapContactContains(const Common::Point &position) const {
	if (!hasTroopContact())
		return false;
	const int top = _troopContactTop ? _troopContactTop : 5;
	// D6FE uses strict left/top as well as right/bottom edges.
	return position.x > 5 && position.x < 232 &&
		position.y > top && position.y < top + 67;
}

void DuneEngine::updateMapPointer(const Common::Point &position) {
	if (!_onWorld || _onGlobe || _onMixer || _inCabin || _surveyDragging)
		return;
	pollMapTitle((uint16)((_system->getMillis() - _noticeIdleTime) / 5));
	// 5C03 updates the survey's hover label/legend. Strategic cards belong to
	// left/right clicks (5C76/5CE4), not to merely moving the pointer.
	if (!hasSurveyMap())
		return;
	// Pre-hover native saves left these bytes unused. Normalize them before
	// an outside pointer changes the ink and accidentally activates swatch 0.
	if (_state[0x4725] != 245 && _state[0x4725] != 254) {
		_state[0x4724] = 0xff;
		_state[0x4725] = 254;
	}
	const int over = surveyPlaceAt(position, true);
	bool changed = over != _pointedAtPlace;
	// 5692/5605 clears the previous density outline when a place label
	// replaces the legend, or when that label is released again.
	if (changed) {
		_state[0x4724] = 0xff;
		if (over < 0)
			_state[0x4725] = 254;
	}
	_pointedAtPlace = over;
	WRITE_LE_UINT16(_state.begin() + 0x46fc, over < 0 ? 0 : 0x100 + 28 * over);
	if (over < 0) {
		const Common::Rect frame = surveyFrameRect();
		const bool inside = position.x > frame.left && position.x < frame.right &&
			position.y > frame.top && position.y < frame.bottom;
		const byte ink = inside && position.y >= frame.bottom - 10 ? 245 : 254;
		changed |= _state[0x4725] != ink;
		_state[0x4725] = ink; // 57AD/57B2: inverse legend only over its strip.
		if (inside && position.y < frame.bottom - 10 && !_state[0x4722]) {
			// 5746 reads the displayed pixel, including markers, rather than
			// projecting the mouse back into MAP2's immutable deposit IDs.
			const byte pixel = _framebuffer1[position.y * kScreenWidth + position.x];
			const byte density = pixel >= 0x50 && pixel < 0x60 ? pixel - 0x50 : 0xff;
			changed |= _state[0x4724] != density;
			_state[0x4724] = density;
		}
	}
	if (changed) {
		// 5C03/5746 touches only the legend, not the map/actor pool or
		// the retained chief report beneath MAP2.
		drawSurveyLegend();
		const Common::Rect frame = surveyFrameRect();
		const int left = frame.left + 6, top = frame.top + 98;
		_system->copyRectToScreen(_framebuffer1 + top * kScreenWidth + left,
				kScreenWidth, left, top, frame.width() - 12, 8);
		_system->updateScreen();
	}
}

/**
 * Flies to a place.  What each kind of place looks like inside is the game's
 * to say and is not read yet, so a sietch, a village and the rest are shown
 * with the scenes and the picture that suit them.
 */
/**
 * Takes off for a place.  0x42D9 is what the game does when an ornithopter is
 * taken: it loads ORNYPAN.HSQ, sets the byte at DS:0x11C9 to 4 for the orni -
 * eight is the worm - and 0x4709 shifts that down into the low two bits, which
 * is what the rest of the game reads to know the player is in the air.  Where
 * they are is kept as a point of the world rather than a place, the same way
 * as walking out of a building, and 0x4988 draws the ground going by.
 *
 * The player used to be set down at the place outright, so there was no flight
 * and nothing to fly in.
 */
void DuneEngine::takeOffFor(uint index, bool guided,
							 int targetLongitude, int targetLatitude) {
	if (index >= _map.places().size() || _state.size() <= 0x11c9)
		return;

	// Selecting CHANGE DESTINATION during a worm or ornithopter journey runs
	// DOS 497A/4944: it changes the target/course in place, without repeating
	// either vehicle's departure. takeOffFor() is also the normal ornithopter
	// entry point, so only that case may reset the position and consume one.
	const bool ridingWorm = (_state[0x11c9] & (8 | 2)) != 0;
	const bool redirectingFlight = _inDesert && (_state[0x11c9] & 3) != 0;
	if (!ridingWorm)
		retargetSkyFade(); // 4706 follows course selection, before any departure step.

	// 496E/4972 clears the previous steering state before 5119 installs the
	// bearing towards the new target (setFlightCourse below).
	_flightHeading = 0;
	_flightSteered = false;

	if (_state.size() > 0x11c8) {
		_state[0x11c7] = 0;
		_state[0x11c8] = 0;
	}

	_guidedFlightPlace = guided ? (int)index : -1;
	_flightDiscoveryPrompt = false;
	_state[0x11ca] = 0; // 4F09/50C4 returns the airborne position callback.
	if (redirectingFlight && _speakerPerson >= 0) {
		stopSpeech();
		_speakerPerson = -1;
		_speakerFrame = 0;
		_speakerIdleActive = false;
	}
	// A point of the world is a destination in its own right, not only the end
	// of a companion's course: the cabin lets any open stretch be chosen, and
	// the desert's own list - go on, turn back, choose again - is what it is
	// for.  Requiring the clue made every press on open ground do nothing.
	_flightToPoint = targetLongitude >= 0;
	if (_flightToPoint) {
		_flightTargetLongitude = (uint16)targetLongitude;
		_flightTargetLatitude = (int16)targetLatitude;
	} else {
		_flightTargetLongitude = _map.places()[index].longitude;
		_flightTargetLatitude = _map.places()[index].latitude;
	}
	// DOS 4944/496A keeps the active target record in DS:11C5 until 4FF2
	// clears it on landing. A named destination uses its place record; the
	// open-ground branch keeps the last place at DS:1150 as bookkeeping while
	// the chosen world point is carried separately. Redirects replace this
	// word without moving Paul back to the departure place.
	WRITE_LE_UINT16(_state.begin() + 0x11c5,
			_flightToPoint ? READ_LE_UINT16(_state.begin() + 0x1150)
							 : (uint16)(0x100 + 28 * index));

	const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
	if (!redirectingFlight) {
		// 50EA starts a worm at the live desert point. 1150 remembers its
		// previous place only for bookkeeping, never as a position reset.
		const uint16 departure = ridingWorm ? READ_LE_UINT16(_state.begin() + 0x1150) : here;
		if (departure >= 0x100)
			_cameFromPlace = (departure - 0x100) / 28;
		else if (!ridingWorm)
			_cameFromPlace = 0;
	}

	if (_cameFromPlace >= _map.places().size())
		return;
	// A map or cabin destination may lower _onWorld before this call; tear
	// down the map mode before beginning the airborne journey.
	_state[0x46eb] = 0;

	if (!redirectingFlight) {
		clearFlightHazard(); // 4739: new departures clear the warning before request seven.
		// 474B/40D5 uses request seven in the old room. It is not a
		// compass exit: a resident's answer may change who follows, but
		// 40E0 never tests the ordinary room-exit's 47A5 veto.
		// 50EA's named worm destination also enters through 4703/474B.
		// Keep its raw eight: finishVehicleBoarding distinguishes this
		// map departure from the compact selector's folded ten, which
		// owns the worm-call movie in completeWormDeparture.
		_pendingBoardingPlace = index;
		_pendingBoardingGuided = guided;
		_onWorld = _onGlobe = _inCabin = false;
		_state[0x23] = 7;
		releaseSilentLocalSpeaker();
		_state[0x47a7] = 0;
		walkLocalRoomRequests();
		finishVehicleBoarding();
		return;
	}
	completeTakeOffFor(index, guided);
}

/** 36E8/40DD/40E6: finish boarding replies, then leave unwilling followers.
 * Unlike 3FC5, this path has no additional ABD5 voice wait.
 */
void DuneEngine::finishVehicleBoarding() {
	if (_pendingBoardingPlace < 0 || _localRoomRequestPending)
		return;
	_state[0x23] = 0;
	_state[0x11ca] = 0;
	for (uint slot = 0; slot < 16; slot++) {
		const byte *resident = _state.begin() + 0xfd8 + 16 * slot;
		if (READ_LE_UINT16(resident) != READ_LE_UINT16(_state.begin() + 4) ||
				READ_LE_UINT16(resident + 2) != READ_LE_UINT16(_state.begin() + 6) ||
				(resident[15] & 0x42) != 0x42)
			continue;
		const uint person = resident[14];
		markPersonFollowing(person, false); // 9556, including the physical clock.
		stayBehind(person, true); // 9655 changes the panel only after the reply.
	}
	const uint index = _pendingBoardingPlace;
	const bool guided = _pendingBoardingGuided;
	_pendingBoardingPlace = -1;
	_pendingBoardingGuided = false;
	if ((_state[0x11c9] & 3) == 2)
		completeWormDeparture(index);
	else
		completeTakeOffFor(index, guided);
}

void DuneEngine::completeTakeOffFor(uint index, bool guided) {
	const bool ridingWorm = (_state[0x11c9] & (8 | 2)) != 0;
	const bool redirectingFlight = _inDesert && (_state[0x11c9] & 3) != 0;
	const Place &from = _map.places()[_cameFromPlace];
	const byte kindOfPlace = _state.size() > 8 ? _state[8] : 0xff;

	// Choosing a destination from an inner sietch room first brings Paul and
	// his followers outside.  The takeoff is drawn there, with the machine in
	// front of them, before their location becomes a point in the desert.
	if (!redirectingFlight && !ridingWorm && kindOfPlace != 0xff) {
		moveMatchingFollowers((uint16)((kindOfPlace << 8) | 1),
			READ_LE_UINT16(_state.begin() + 6));
		_state[0xb] = 1;
		_state[0xc] = 1;
		WRITE_LE_UINT16(_state.begin() + 4, (uint16)((kindOfPlace << 8) | 1));

		const byte *outside = roomRecord(kindOfPlace, 1);
		if (outside && outside[0])
			_place = (byte)(outside[0] - 1);

		_speech = "";
		_speakerPerson = -1;
		_onWorld = false;
		_inCabin = false;
		showRoomMenu();
		lowerPanelFace(false); // 4745/47DB: before unfolding the selected orni.
		animateOrnithopterTakeoff(kindOfPlace);
		// EF6A keeps time through ORNYTK's blocking frames. Accrue that
		// interval before the first carried scene chooses its sky palette;
		// 1B0D's pending troop work still belongs to the ordinary poll.
		tick(false);
	}
	if (!redirectingFlight && ridingWorm)
		_state[0xe8] = 0; // 4795/47AD: worm departure clears, rather than animates, the face.

	if (!redirectingFlight) {
		WRITE_LE_UINT16(_state.begin() + 0x114e, 0);
		if (!ridingWorm) {
			moveMatchingFollowers(from.longitude, (uint8)from.latitude);
			WRITE_LE_UINT16(_state.begin() + 4, from.longitude);
			WRITE_LE_UINT16(_state.begin() + 6, (uint16)from.latitude & 0xff);
		}
	}

	_state[8] = 0xff; // 475B changes the place kind only; retain the subtype at9.

	// 4 says an ornithopter has been taken, and the low bits say it is in the
	// air; DS:0x487E is 2 for the orni and 1 for the worm, and DS:0x473E is
	// set for the one and cleared for the other.
	if (!redirectingFlight)
		_state[0x11c9] = ridingWorm ? (8 | 2) : (4 | 1);
	// A desert search is a deliberately broken-off course: the original
	// offers BACK TO STARTING POINT and displays the three steering buttons,
	// unlike a direct known-place flight which offers SKIP TO DESTINATION.
	// A journey that is not making for a place of its own - a course the
	// player picked, or a companion's clue - is the broken-off state the three
	// buttons belong to, and it is by them that such a flight is steered.
	if (_state.size() > 0x11cb)
		_state[0x11cb] = (guided || _flightToPoint) ? 0xff : 0;
	if (!redirectingFlight && _state.size() > 0x487e)
		_state[0x487e] = ridingWorm ? 1 : 2;
	if (!redirectingFlight && _state.size() > 0x473e)
		_state[0x473e] = ridingWorm ? 0 : 1;

	_inDesert = true;
	_onWorld = false;
	_flyingTo = (int)index;
	setFlightCourse();
	_flightFrame = 0;
	if (!redirectingFlight) {
		_flightTrail.clear();
		_state[0x4728] = 0; // 49EA initializes a new flight, not a redirect.
		// DOS 4732 resets the carried callback count; 4760 then makes one
		// position step before the live 4F09 timer has begun. 4A1A does not
		// draw a trail until that first live callback.
		WRITE_LE_UINT16(_state.begin() + 0x472b, 0);
		advanceCarriedPosition(false);
		WRITE_LE_UINT16(_state.begin() + 0x4729, 0);

		// 4769 clears the 16-bit last-step tick, not the current timer. Thus
		// the first live poll usually moves again immediately; only a takeoff
		// within the timer's first 0x300 ticks (including after wrap) waits.
		const uint32 nowMs = _system->getMillis();
		_nextFlightStep = FlightStepClock::first(nowMs);
	}
	// DOS 4944 changes an airborne target without touching DS:4729, so a
	// redirect retains its already-running movement timer as well as position.
	stopFlightMovie();
	_flightSkyRestartPending = true;
	_speech = "";
	_speakerPerson = -1;
	_flightInsetSet = false; // 478F/2EB2 prepares a new inset on an airborne redirect too.

	debugC(1, kDebugScript, "taking off from place %u for place %u",
		   _cameFromPlace, index);

	// 476F/4776 restores the initial carried scene through 2DBF/2E07.
	// The room-contact word is cleared even though 3090 retains the physical
	// presence mask while a vehicle carries Paul. Otherwise a conversation
	// at the departure sietch survives into the flight's request context.
	// Airborne redirects enter 478F/2EB2 instead and keep that context.
	if (!redirectingFlight) {
		WRITE_LE_UINT16(_state.begin() + 0x14, 0);
		_state[0x46df] = 0;
	}
	showDesertMenu();
	if (!redirectingFlight)
		_state[0x4727] = 0xff; // 4779/4AB8, before removing the departure machine.
	// 4776/2FBF and4AB8 prepare the first carried scene BEFORE4789 removes
	// the selected machine from the departure cache. Keep that byte intact
	// throughout the outside/takeoff/first-flight paints, like DOS. Redirects
	// and worm rides never consume another machine; DEC wraps even at zero.
	const uint32 leaving = 0x100 + 28 * _cameFromPlace;
	if (!redirectingFlight && !ridingWorm && leaving + 0x15 < _state.size())
		_state[leaving + 0x15]--;
	// 4769/2DBF renders the first carried scene, then 2E52/35E9 checks
	// requests even before the next movement callback. Redirects enter
	// 478F/2EB2 directly and retain their pending request context instead.
	if (!redirectingFlight)
		dispatchCarriedRoomRequests();
}

/** 92C9/D6FE: the occupied companion portrait under the pointer, if any. */
int DuneEngine::companionAt(const Common::Point &at) const {
	// 92C9 stops at an empty first slot; the second slot is not independently
	// selectable in that noncanonical party state. D6FE excludes all four edges.
	if (_state[0x1152] == 0xff)
		return -1;
	for (uint slot = 0; slot < 2; slot++) {
		const byte person = _state[0x1152 + slot];
		const PanelElement &box = _panel[21 + slot];
		if (person != 0xff && person < 16 && at.x > box.x && at.x < box.x2 &&
				at.y > box.y && at.y < box.y2)
			return person;
	}
	return -1;
}

/**
 * Which of the panel's right hand box the pointer is over, or nothing.  The
 * six entries of whichever template stands there carry their own corners, so
 * the box is worked the same way whatever it is showing.
 */
int DuneEngine::compassButtonAt(const Common::Point &at) const {
	for (int i = 1; i != kCompassElements; i++) {
		const PanelElement &e = _panel[kFirstCompassElement + i];

		if (e.sprite == 0xffff || (e.flags & 0x20) != 0)
			continue;

		if (panelControlContains(at, e.x, e.y, e.x2, e.y2))
			return i;
	}

	return -1;
}

/**
 * Turns the course a flight is steered on.  0x5119 adds the button's own
 * number to the byte at DS:0x11C7 - four one way or four the other, which is
 * a sixty-fourth of the way round - and 0x4AE0 marks at DS:0x11C8 that the
 * player has taken the controls.
 */
void DuneEngine::turnFlight(int by) {
	_flightHeading = (byte)(_flightHeading + by);
	_flightSteered = true;

	if (_state.size() > 0x11c8) {
		_state[0x11c7] = _flightHeading;
		_state[0x11c8] = 1;
		WRITE_LE_UINT16(_state.begin() + 0x11cc, 128);
	}
	_nextFlightTurn = _system->getMillis() + 100;

	debugC(1, kDebugScript, "the course is turned to %d", _flightHeading);
}

/**
 * Which of the four steps the course comes nearest to.  The course is a whole
 * turn in a byte, and 0x41EB rounds it by adding a thirty-second before it
 * looks at the quarter, which is what this does.
 */
int DuneEngine::headingWay() const {
	return ((int)_flightHeading + 0x20) % 256 / 0x40;
}

int DuneEngine::wayToFly() const {
	if (_flyingTo < 0 || (uint)_flyingTo >= _map.places().size() || _state.size() <= 7)
		return -1;

	const Place &to = _map.places()[_flyingTo];
	// 407E/B58B/4F49 compares the rounded cell in the actual latitude row
	// with the destination's cached cell. An equatorial-width distance can
	// become zero while the rounded row cell is still adjacent. Manual/free
	// courses do not end when they cross the old destination.
	if (!_flightToPoint && !_flightSteered) {
		const int32 target = _map.groundOffset(to.longitude, to.latitude);
		if (target >= 0 && _map.groundOffset(READ_LE_UINT16(_state.begin() + 4),
				(int8)_state[6]) == target)
			return -1;
	}

	// Movement itself recomputes the named bearing in 4B3B; the direction
	// argument only selects the corresponding desert controls.
	return headingWay();
}

/** Stores the current world point in the DOS ornithopter trail ring. */
void DuneEngine::rememberFlightTrailPoint() {
	enum { kTrailCapacity = (0xe85c - 0xe40c) / 4 }; // 4A00's CS:E40C..E85C ring.
	if (_state.size() <= 6)
		return;

	const Common::Point point((int16)READ_LE_UINT16(_state.begin() + 4), (int8)_state[6]);
	// 4A00 records every movement callback, even if the rounded world point
	// did not change. Such updates still evict old marks after one ring turn.
	if (_flightTrail.size() >= kTrailCapacity)
		_flightTrail.remove_at(0);
	_flightTrail.push_back(point);
}

/** How many world-map points remain to the place a flight is bound for. */
int DuneEngine::distanceToFlightTarget() const {
	if (_flyingTo < 0 || (uint)_flyingTo >= _map.places().size() || _state.size() <= 7)
		return -1;

	const Place &to = _map.places()[_flyingTo];
	const int here = (int)((uint32)READ_LE_UINT16(_state.begin() + 4)
						   * Map::kEquatorWidth / 65536u);
	const int there = (int)((uint32)to.longitude * Map::kEquatorWidth / 65536u);
	const int down = (int)(int8)_state[6];
	const int acrossOff = ABS(there - here);
	return MIN(acrossOff, (int)Map::kEquatorWidth - acrossOff)
		 + ABS((int)to.latitude - down);
}

/** 366F/368B/96D8: select the flight's caller once and speak the report. */
bool DuneEngine::beginFlightDiscovery() {
	WRITE_LE_UINT16(_state.begin() + 0x11f5, 206 + 2 * (_state[0xe1] != 0));
	if (_guidedFlightPlace >= 0 && (uint)_guidedFlightPlace < _map.places().size())
		WRITE_LE_UINT16(_state.begin() + 0x11f3,
			72 + MIN(placeClass(_map.places()[_guidedFlightPlace].kind), 3));
	_state[0x23] = 3;
	return beginCarriedReport();
}

bool DuneEngine::beginCarriedReport() {
	const uint16 followers = READ_LE_UINT16(_state.begin() + 0x1152);
	if (followers == 0xffff) {
		if (_state[0x23] != 4)
			return false;
		// 3637/365C: caption/control fallback without voice or 3551
		// completion. WHAT is disabled and the flight remains paused.
		stopSpeech();
		_speakerPerson = -1;
		_speakerIdleActive = false;
		_speech = command(190);
		_flightDiscoveryPrompt = true;
		_state[0x11ca] = 1;
		_state[0x1fa9] |= 0x40;
		showFlightReportMenu(true, true);
		showFlightWarning();
		return true;
	}
	// 366F reads bit 0x80 of the existing primary seed; it does not advance
	// either generator or prefer a companion according to the story stage.
	const uint companion = (followers >> 8) == 0xff || (_state[0] & 0x80)
		? followers & 0xff : followers >> 8;
	if (companion >= 16)
		return false;
	WRITE_LE_UINT16(_state.begin() + 0x47c4, companion);
	WRITE_LE_UINT16(_state.begin() + 0x47a2, 0xfd8 + 16 * companion);
	_state[0x4728] |= 1; // 368E precedes both vehicle branches of368B.
	const bool orniPortrait = (_state[0x11c9] & 3) == 1;
	if (orniPortrait) {
		// 3618/368B/978E prepares the portrait BEFORE 96D8 can reject
		// every record. A missing paragraph does not undo the pause or
		// consume the request: the ordinary flight controls remain active.
		stopSpeech();
		_speech.clear();
		_speechParts.clear();
		_speakerPerson = companion;
		_speakerReport = 0;
		_state[0x473e] = 1;
		_state[0x47a4] = 1;
		prepareLocalSpeakerIdle();
		_state[0x11ca] = 1;
		showFlightDiscovery();
		waitDesertExhaustion(75); // 361C/E387: no input or pose callbacks.
	}
	preparePersonDialogueContext(companion); // 9F9E/94F3 before conditions.
	const int selected = troopNoticeRecord();
	if (selected < 0) {
		if (orniPortrait) {
			showDesertMenu(false);
			showFlightDiscovery();
		}
		return orniPortrait;
	}
	Dialogue::Record &record = _dialogue.entry(132)[selected];
	stopSpeech();
	_flightDiscoveryPrompt = true; // fillInLine's discovered-place substitution.
	_speakerPerson = companion;
	_speakerReport = 0;
	_dialoguePart = 132;
	_dialogueLine = selected;
	prepareDialogueLine(record, 16);
	if (orniPortrait)
		// 3618 first raises the caller through 368B/978E, then 3624's
		// 96D8 selects the report and enters 978E again. Prepare the new
		// paragraph's pose/budget too, retaining its existing same-bank
		// 99BE registration rather than resetting the animation interval.
		prepareLocalSpeakerParagraph();
	else
		prepareLocalSpeakerIdle();
	_state[0x19] = 0xff;
	rememberBookRecord(record, companion);
	record.flags |= 0x80; // A092/A097; mask 0x20 permits later sightings.
	_state[0x11ca] = 1; // 368B/978E pauses both discovery and warning portraits.
	if (_state[0x23] == 4) {
		_state[0x1fa9] &= (byte)~0x40; // 357C enables the selected voice's WHAT.
	}
	// 3633 enters 3551 directly, not 3542: this report marks the carried
	// poll as handled without consuming queued packets or clearing E7.
	_state[0x47a7]++;
	WRITE_LE_UINT16(_state.begin() + 0x1bea, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1bf8, 0);

	showFlightReportMenu(_state[0x23] == 4, false);
	// 96D8/9EFD retains the caller and the actual request variant.
	playSpeech(record.line, companion, true);
	showFlightDiscovery();
	return true;
}

/** A rejected 96D8 paragraph still leaves 978E's paused portrait visible. */
bool DuneEngine::carriedPortraitVisible() const {
	return _inDesert && _flyingTo >= 0 && !_onWorld && !_onGlobe &&
		!_inCabin && !_onMixer && _speakerPerson >= 0 &&
		_state.size() > 0x11ca && _state[0x11ca];
}

/** 497A/98E6: unregister the report, without 97CF's conversation completion. */
void DuneEngine::releaseCarriedReportForSelector() {
	// 98F5/98E9..98EF clears the portrait/paragraph registrations. It does
	// not call A7A5 or mark the physical person, so leave the recording and
	// follower records alone while the cockpit takes ownership of the screen.
	WRITE_LE_UINT16(_state.begin() + 0x1c06, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1bf8, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1bea, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47c8, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47aa, 0);
	WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
	_speakerIdleActive = false;
	_speakerIdlePerson = -1;
	_speakerPerson = -1;
	_speakerFrame = 0;
	_flightDiscoveryPrompt = false;
	_guidedFlightPlace = -1;
	_speech.clear();
	_speechParts.clear();
}

void DuneEngine::showFlightReportMenu(bool warning, bool withoutCompanion) {
	_menu.clear();
	_menuParts.clear();
	_menuFlags.clear();
	if (warning) {
		_menu.push_back(command(87)); // CHANGE DESTINATION
		_menuParts.push_back(kDiscoveryChange);
		_menuFlags.push_back(0);
	}
	_menu.push_back(command(warning ? 173 : 172)); // IGNORE WARNING / GO TOWARDS
	_menuParts.push_back(kDiscoveryGo);
	_menuFlags.push_back(0);
	_menu.push_back(command(148)); // WHAT
	_menuParts.push_back(kDiscoveryWhat);
	_menuFlags.push_back(withoutCompanion ? 0x4000 : 0);
	_menuKind = kMenuDiscovery;
	_menuFrom = 0;
	_menuUnder = menuRowAt(_system->getEventManager()->getMousePos(), true);

	debugMenuIfChanged();
}

void DuneEngine::showFlightWarning() {
	// 3637/439F: no companion portrait or recording. 14B4 draws the
	// ORNYPAN instruments, frame and grid; D194 prints COMMAND's caption.
	const Common::String speech = _speech;
	_speech.clear();
	drawDesert(false);
	_speech = speech;
	openSpriteSheet("ORNYPAN.HSQ");
	drawSprite(0, 0, 19);
	drawSprite(1, 10, 43);
	drawSprite(2, 79, 45);
	// D1D1 resets the horizontal origin and advances ten pixels at each CR.
	// CX=200C supplies the background/ink colours to D194's tall face.
	_font.setFace(Font::kFaceNormal);
	const Common::String caption = command(190);
	int x = 78, y = 102;
	for (uint i = 0; i < caption.size(); i++) {
		if (caption[i] == '\r') {
			x = 78;
			y += 10;
		} else {
			x = _font.drawChar(_framebuffer1, x, y, caption[i], 0x0c, 0x20);
		}
	}
	for (int i = 1; i <= 3; i++)
		_panel[kFirstCompassElement + i].sprite = 0xffff;
	drawPanel();
	drawMenu();
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/** Redraw the carried report without reselecting or replaying it. */
void DuneEngine::showFlightDiscovery() {
	// 368B/C2F2 paints ORNYCAB with its own partial palette, then 978E
	// installs the companion's palette and draws the selected portrait.
	// Keep the original indices: quantizing these foregrounds to the sky
	// ramp changes both the cockpit and the face. Ordinary flight repaints
	// the terrain/inset but retains the companion's low colours.
	const Common::String speech = _speech;
	_speech.clear();
	// 368B lays ORNYCAB over the flight page that 4B2B already presented.
	// Its transparent window retains that displayed inset, not 4A1A's
	// newer working cache (which may already have grayed the old head).
	Common::Array<byte> shownInset;
	if (_flightInsetSet && !(_state[0x4728] & 0x80)) {
		shownInset.resize(120 * 64);
		for (uint y = 0; y < 64; y++)
			memcpy(shownInset.begin() + y * 120, _framebuffer1 + y * kScreenWidth + 200, 120);
	}
	drawDesert(false, false);
	for (uint y = 0; y < shownInset.size() / 120; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + 200, shownInset.begin() + y * 120, 120);
	_speech = speech;
	// 369A/36CB keeps the worm's landscape; only36AE loads ORNYCAB.
	// The selected paragraph later reaches978E throughA0AA for either
	// vehicle, retaining the common speaker setup and report pause.
	if ((_state[0x11c9] & 3) == 1) {
		openSpriteSheet("ORNYCAB.HSQ");
		drawSprite(0, 0, 0);
	}
	// Retain this vehicle's page for the ordinary pose and mouth callbacks.
	drawCurrentSpeakerFigure();

	// The discovery question interrupts the journey controls.  The reference
	// leaves their framed box empty until Paul answers Gurney.
	for (int i = 1; i <= 3; i++)
		_panel[kFirstCompassElement + i].sprite = 0xffff;
	drawPanel();
	drawMenu();
	drawSpeech(_speech, kSpeechColour);
	finishLocalSpeakerScene();

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0,
							  kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

void DuneEngine::flyTo(uint index) {
	flyTo(index, false);
}

void DuneEngine::flyTo(uint index, bool approach) {
	if (index >= _map.places().size())
		return;

	const Place &place = _map.places()[index];
	const byte arrivingVehicle = _state.size() > 0x11c9 ? _state[0x11c9] & 3 : 0;
	const bool arrivingByOrni = _flyingTo >= 0 && arrivingVehicle == 1;
	const bool walkingArrival = _flyingTo < 0 && _state[8] == 0xff && !arrivingVehicle;
	if (walkingArrival) {
		// 4057/2DBF removes the retained follower picture before drawing
		// the newly entered room. STOP can leave that picture up with no
		// paragraph; it is not an active conversation in the new scene.
		_speakerPerson = -1;
		_speakerIdleActive = false;
		_speakerIdlePerson = -1;
		_speechParts.clear();
		_conversationParentMenu.clear();
		_conversationParentParts.clear();
		_conversationParentFlags.clear();
	}
	// Natural sietch/palace arrivals switch film at 48A2; forts start FORT.HNM
	// through 48DD/C8FB. Villages and Harkonnen sites use reverse ORNYTK
	// without an extra film. SKIP TO DESTINATION bypasses approach films.
	const byte destinationKind = _state[0x100 + 28 * index + 8];
	const bool battlefieldArrival = battleAt(0x100 + 28 * index);
	// 4FB0 skips CA01 for an orni. At a battlefield, 2DD3 then bypasses
	// 488A's approach/stream teardown altogether. Stop native decoding, but
	// retain C950's logical stream gate until a later CA01-style close. D316
	// must not roll occupation/equipment menus after this grounded arrival.
	const bool retainBattleStream = arrivingByOrni && approach && battlefieldArrival &&
			_flightMovie >= 0 && !_flightMovieFallback;
	if (arrivingByOrni && approach && !battlefieldArrival &&
		(destinationKind < 0x21 || (destinationKind >= 0x28 && destinationKind < 0x30)))
		animateOrnithopterApproach(destinationKind);
	stopFlightMovie();
	if (retainBattleStream)
		WRITE_LE_UINT16(_state.begin() + 0x35a6, 1); // Ownership, not a host/DOS handle.
	if (_flyingTo >= 0 && _state[0x11ca] && _speakerPerson >= 0 && !_flightDiscoveryPrompt) {
		// No 3551 report was opened, so there is no GO conversation mark.
		// Landing tears down only the native retained portrait.
		stopSpeech();
		_speakerPerson = -1;
		_speakerIdleActive = false;
		_speakerIdlePerson = -1;
	}

	// 4FC3/E3CC takes a byte from the second persistent generator on carried
	// arrival and keeps it at DS:0xC5. Its low nibble rotates the room's floor
	// positions, preventing every Fremen from repeatedly occupying the same
	// oversized foreground mark. DOS seeded the generator at startup; zero is
	// also a valid saved seed and must advance to one, not be replaced here.
	// 4FC3 rotates floor marks for a carried landing; 3F27's on-foot
	// arrival enters 4002 without that extra RNG draw.
	if (!walkingArrival && _state.size() > 0xd827) {
		_state[0xc5] = (byte)troopRandom(); // E3CC's AL, shared with ordinary polls.
	}

	// Whatever was flown here is parked here now, which 0x4FDC does before the
	// place is drawn - so that the count arriveAt copies out of the record is
	// the one with it in.
	// Only an ornithopter is parked in the place inventory. A worm carries
	// Paul to the same arrival routine but disappears back into the sand; the
	// old broad flight test manufactured an extra orni after every worm ride.
	// 4FCE/4FD2 takes the vehicle from the low pair, then clears the ENTIRE
	// travel byte. The panel mode at 487E is not the parking discriminator.
	_state[0x4727] = 0; // 4FC9/4FCB disables the ordinary poll before parking/room setup.
	if (arrivingByOrni) {
		const uint32 at = 0x100 + 28 * index;

		if (at + 0x15 < _state.size())
			_state[at + 0x15]++;
	}

	// Arriving settles which kind of arrival it is, and the room is chosen by
	// that as much as by the place, so it comes first.
	arriveAt(index);
	roomFor(index);

	// DS:0x101A is not where the player is - it is the place word of the fifth
	// person record, which is Gurney's, and the whole of the original writes
	// it exactly once, at 0x1086, to stand him in the palace.  0x6C71 reads it
	// to find the sietch he was left in.  Writing the player's destination
	// over it on every flight moved him to wherever Paul went next, so his
	// training was credited to the palace the moment Paul flew home and the
	// place he was actually left in was forgotten.  Where the player is, is
	// DS:0x6 and DS:0x114E, and arriveAt has just written both; the followers
	// who are still with Paul take their copy from there below.

	debugC(1, kDebugScript, "flying to place %u, type %d at %d,%d", index, place.type,
		   place.longitude, place.latitude);

	// Arrival returns to the room's own list.  Place details belong to the
	// map tooltip and troop reports; putting the synthesized description in a
	// speech box here covered the lower part of every outside scene, unlike
	// both the known-sietch and hidden-sietch arrivals in the recording.
	_speech.clear();
	_speaker.clear();

	_onWorld = false;
	_inDesert = false;
	_flyingTo = -1;
	_guidedFlightPlace = -1;
	_flightDiscoveryPrompt = false;
	_flightToPoint = false;

	// Back on the ground, so nothing is in the air any more.
	if (_state.size() > 0x11c9)
		_state[0x11c9] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x11c5, 0); // 4FF2: no vehicle's target record.
	_flightTrail.clear();
	_state[0x47a4] = 0; // 2DC7: the fresh grounded room shows floor figures.
	peopleHere(); // 4002/2E07 prepares the physical arrival's account once.

	if (arrivingByOrni && ((_state[8] >= 0x21 && _state[8] < 0x28) ||
			_state[8] >= 0x30)) {
		// The arrival frames show the destination's room controls, not the
		// flight list that was on screen a moment ago. The ordinary caller
		// presents the same room menu again after flyTo returns.
		showRoomMenu(false);
		animateOrnithopterLanding(_state[0x8]);
	}
	// 4FC3 clears the vehicle before 4002/4057 enters the fresh room's
	// 2E07/35AD resident pass. Unlike an interior move through 3FCA, this
	// caller does NOT invent request five. Retain any actual request context
	// and let the same physical, resource-driven walk choose the first reply.
	WRITE_LE_UINT16(_state.begin() + 0x14, 0);
	_state[0x46df] = 0;
	if (arrivingVehicle)
		stopSoundEffect(); // 478C/47CA retire carried-arrival digital playback.
	drawScene();
	if (arrivingVehicle || walkingArrival)
		raisePanelFace(); // 4002/1860/17E6, after clearing11C9 and drawing the room.
	showLocalRoomRequests();
}

/**
 * Says what there is at a place: the region it lies in, how many people are
 * there, and what they are good at.  The names are the interface's own, out
 * of COMMAND1.HSQ, where the twelve regions come first and the nine sietches
 * after them, followed by the six tasks a troop can be set to.
 */
void DuneEngine::describePlace(const Place &place) {
	enum { kFirstTask = 23 };

	// What kind of place it is comes first, which 0x4637 writes before 0x463E
	// writes the name.
	Common::String said = kindOfPlaceName(place.kind) + nameOfPlace(place);

	if (place.people)
		said += Common::String::format(", %d people.", place.people);
	else
		said += ".";

	for (uint i = 0; i != 6; i++) {
		if (!place.skills[i])
			continue;

		static const uint task[6] = { 0, 4, 5, 8, 9, 10 };
		said += Common::String::format(" %s %d.", command(kFirstTask + task[i]).c_str(),
									   place.skills[i]);
	}

	_speech = said;
	_speaker = "";
}

/**
 * Puts the window of the map screen round a point of the world, keeping it on
 * the map from top to bottom and letting it run round the sides.
 */
void DuneEngine::lookAtWorld(int longitude, int latitude) {
	// DOS 0xB6C3 preserves fractional longitude and clamps the centre latitude
	// to +/-75 so the 36 four-line terrain strips remain within the globe.
	_worldLongitude = (uint16)longitude;
	_worldLatitude = CLIP(latitude, -75, 75);
}

/**
 * Puts the window of the map screen round wherever the player is standing,
 * which is where the maps open. 0x5B5D calls 0x407E for Paul's coordinates,
 * whether they belong to a place or a point in the desert. Falling back to
 * place zero while airborne centred every new destination selection on the
 * palace instead of on the current flight position.
 */
void DuneEngine::lookAtPlayer() {
	uint16 longitude;
	int16 latitude;
	if (playerWorldPosition(longitude, latitude))
		lookAtWorld(longitude, latitude);
}

/** Pans the framed survey without disturbing the large map below. */
void DuneEngine::scrollMoveMap(int way) {
	// DS:145E..146D, shared by the original 8831/8858 pan callbacks.
	if (way < 0 || way > 3 || _state.size() < 0x146e)
		return;
	const uint at = 0x145e + way * 4;
	_surveyLongitude = (uint16)(_surveyLongitude + (int16)READ_LE_UINT16(_state.begin() + at));
	_surveyLatitude = clampSurveyLatitude(_surveyLatitude +
		(int16)READ_LE_UINT16(_state.begin() + at + 2), kRouteMapHigh);
	refreshSurveyWindow(); // 8858/8862, not the strategic 8850/5A9A redraw.
}

/**
 * Moves the window north, east, south or west. Longitude wraps at the seam;
 * latitude stops before the polar edge, as in lookAtWorld().
 */
void DuneEngine::scrollWorld(int way) {
	if (_onWorld && !_inCabin && hasSurveyMap()) {
		scrollMoveMap(way);
		return;
	}
	if (way < 0 || way > 3)
		return;
	if (!_inCabin && _menuKind != kMenuWormMap)
		dismissMapTitle(); // 8831 releases the legend before moving the centre.
	// 8831 reads DS:145E..146D for both the strategic map and the cabin.
	// Cabin mode one uses 8846's 197C/197E centre; signed MAP2 mode branches
	// to 8858's separate 1980/1982 centre above. Both use the same vectors.
	const int longitudeStep = 0x1002;
	const int latitudeStep = 12;

	switch (way) {
	case 0: _worldLatitude = MAX(_worldLatitude - latitudeStep, -75); break;
	case 1: _worldLongitude = (uint16)(_worldLongitude + longitudeStep); break;
	case 2: _worldLatitude = MIN(_worldLatitude + latitudeStep, 75); break;
	case 3: _worldLongitude = (uint16)(_worldLongitude - longitudeStep); break;
	default: return;
	}

	if (_menuKind == kMenuWormMap) {
		drawWormMap();
		return;
	}
	if (_inCabin) {
		showCabin();
		return;
	}

	// 8850 closes the old report before 5A9A recreates every actor, even
	// when a polar clamp leaves the centre unchanged. Keep the callback timer.
	refreshStrategicMap();
}

/** 5A1A/5BB0: ordinary map entry, distinct from a room-started troop contact. */
void DuneEngine::openWorldMap() {
	_state[0x46df] = 0; // 18DE, cancelled by3916 only on its next due poll.
	// 186B/5A1A uses temporary balloon mode for map cards, regardless of
	// the retained subtitle preference. Do not overwrite that preference:
	// 187D restores it only when returning to the ground, not on globe entry.
	_state[0x28e7] = 1;
	_state[0xfb] = strategicViewMode(_state[0xfb]); // 186B/186E retains the room's magnitude.
	Common::Array<byte> oldPage(kScreenWidth * kScreenHeight);
	memcpy(oldPage.begin(), _framebuffer1, oldPage.size());
	byte oldPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);
	_speech.clear();
	_speakerPerson = -1;
	_onWorld = true;
	_state[0x11ca] = 1; // 5A60/4ACA suspends position callbacks at map entry.
	// DOS 5A7E installs the signed strategic-map mode before 6715 registers
	// actors. 68EB/6917 use this byte for selected-troop and live-replacement
	// decisions; leaving it at the room value made ordinary map entry differ.
	_state[0x46eb] = 0x80;
	_troopOrdersFromMap = false;
	_state[0x46f3] = 0;
	lookAtPlayer();
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0x194a);
	resetTroopNoticeIdle();
	showMapMenu(false);
	showWorld(false);
	showMapEntryTransition(oldPage.begin(), oldPalette);
}

/** 5BEB only closes this window, never another active map report. */
bool DuneEngine::closeMapTitle() {
	if (_state.size() <= 0xdbe1 || READ_LE_UINT16(_state.begin() + 0xdbe0) != 0x194a)
		return false;
	WRITE_LE_UINT16(_state.begin() + 0xdbe0, 0);
	return true;
}

/** 5BEB/5BFC: release and restore just the opening legend's rectangle. */
bool DuneEngine::dismissMapTitle() {
	if (!closeMapTitle())
		return false;
	// A full 5A9A redraw would expose terrain changes that have not been
	// requested and would replace the retained actor/terrain background.
	const byte *box = _state.begin() + 0x194a;
	paintMapActorChange(Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
			READ_LE_INT16(box + 4), READ_LE_INT16(box + 6)));
	return true;
}

/** 5C03 uses the shared, wrapping 200 Hz input-idle clock. */
bool DuneEngine::pollMapTitle(uint16 idleTicks) {
	if (!_onWorld || _onGlobe || _onMixer || _inCabin ||
			_menuKind != kMenuMap || isPaused() || idleTicks < 1000)
		return false;
	return dismissMapTitle(); // 5C1C uses the same release as menu/click input.
}

/** 5BB0: original map legend descriptor, COMMAND1 text and normal DNCHAR face. */
void DuneEngine::drawMapTitle() {
	const byte *box = _state.begin() + 0x194a;
	const int left = READ_LE_UINT16(box), top = READ_LE_UINT16(box + 2);
	const int right = READ_LE_UINT16(box + 4), bottom = READ_LE_UINT16(box + 6);
	_graphics.fillRect(left, top, right, bottom, box[8]);
	_graphics.fillRect(left + 1, top + 1, right - 1, bottom - 1, box[9]);

	Common::String said = command(225);
	// D03C finds the end of the first numeric field; E2E3 overwrites its
	// last three columns, including leading spaces. Preserve translated text.
	for (uint i = 0; i < said.size(); i++) {
		if (said[i] < '0' || said[i] > '9')
			continue;
		while (i < said.size() && said[i] >= '0' && said[i] <= '9')
			i++;
		if (i >= 3) {
			const Common::String count = Common::String::format("%3u", _state[0x28]);
			for (uint digit = 0; digit < 3; digit++)
				said.setChar(count[digit], i - 3 + digit);
		}
		break;
	}
	_font.setFace(Font::kFaceNormal);
	int y = top + 8;
	Common::String row;
	for (uint i = 0; i <= said.size(); i++) {
		if (i != said.size() && said[i] != '\r') {
			row += (byte)said[i] >= 0x80 ? '@' : said[i]; // D1C9
			continue;
		}
		_font.drawString(_framebuffer1, left + 10, y, row, 0xf0);
		row.clear();
		y += 10;
	}
}

/** DOS 6252: water/equipment, equipment/battle, or the name alone. */
uint DuneEngine::mapPlaceCardKind(uint which) const {
	const uint record = 0x100 + 28 * which;
	if (which >= _map.places().size() || record + 28 > _state.size())
		return 2;
	if (battleAt(record))
		return 1;
	const byte kind = _state[record + 8], flags = _state[record + 10];
	if (!(flags & 0x10) || (kind >= 0x28 && !(flags & 8)))
		return 2;
	if (kind < 0x20 || (kind >= 0x28 && kind < 0x30))
		return 0;
	return kind >= 0x21 && kind < 0x28 ? 1 : 2;
}

Common::Rect DuneEngine::mapPlaceCardBounds(uint which) const {
	if (which >= _map.places().size())
		return Common::Rect();
	const Common::Point mark = placeOnScreen(_map.places()[which]);
	const int height = _state[0x11d0 + mapPlaceCardKind(which)];
	// 5F25 anchors to the place, switches sides at x=195 and only clamps Y.
	int left = mark.x + 15;
	if (left >= 210)
		left -= 130;
	const int top = CLIP(mark.y - height / 2, 4, 148 - height);
	return Common::Rect(left, top, left + 106, top + height);
}

/** DOS 600E: the original place card, using COMMAND1, DNCHAR and ONMAP. */
void DuneEngine::drawMapTooltip(uint which) {
	if (which >= _map.places().size())
		return;
	const Place &place = _map.places()[which];
	if (!place.isKnown() || !Common::Rect(4, 4, 316, 148).contains(placeOnScreen(place)))
		return;
	const Common::Rect bounds = mapPlaceCardBounds(which);
	const uint32 record = 0x100 + 28 * which;
	const uint kind = mapPlaceCardKind(which);
	// 5EE4/5F01 retains a same-kind place window on ordinary redraws,
	// but tears it down without a closing effect and expands the new one
	// when battle/equipment access changes its report kind (and height).
	const bool newKind = _pressedPlace == (int)which && _mapWindowGeometryValid &&
			_mapWindowBounds.height() != bounds.height() && !_noticeTransitionActive;
	if (newKind)
		WRITE_LE_UINT16(_state.begin() + 0x46f8, 0); // 5F91 fast-closes the old kind.
	prepareMapPlaceCard(which, bounds);
	if (newKind) {
		animateMapWindow(placeOnScreen(place), bounds);
	}
	WRITE_LE_UINT16(_state.begin() + 0x46f8, record); // 6016 follows the opening effect.
	_graphics.fillRect(bounds.left, bounds.top, bounds.right, bounds.bottom, _state[0x1670]);
	_graphics.fillRect(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1, _state[0x1671]);
	_font.setFace(Font::kFaceNormal);
	_font.drawString(_framebuffer1, bounds.left + 12, bounds.top + 4, kindOfPlaceName(place.kind), 0x9a);
	const int x = bounds.left + 4;
	int y = bounds.top + 13;
	_font.drawString(_framebuffer1, x, y, nameOfPlace(place), 0x96);
	if (kind == 2)
		return;

	openSpriteSheet("ONMAP.HSQ", false);
	if ((_state[10] & 0x20) && kind == 0) {
		_font.setFace(Font::kFaceAlternate);
		y += 10;
		const int end = _font.drawString(_framebuffer1, x, y, command(107), 0x90);
		if (_state[record + 10] & 0x20) {
			uint count = _state[record + 27];
			_font.drawString(_framebuffer1, end, y, Common::String::format("%3u", count), 0x90);
			y += 7;
			int width = 0, height = 0;
			// 617A compresses the water drops horizontally, like 61D3's
			// vertical equipment stacks. The last drop must fit in the card.
			if (count && currentSpriteSheet->size(0x75, &width, &height)) {
				int step = width + 2;
				const int room = bounds.right - x;
				if (room / (int)count < step) {
					step = MAX(2, (room - width - 2) / (int)count);
					if ((room - width - 2) / (int)count < 2)
						count = MAX(0, (room - width - 2) / 2);
				}
				for (uint copy = 0; copy < count; copy++)
					drawSprite(0x75, x + copy * step, y);
			}
		} else {
			y += 7;
			_font.drawString(_framebuffer1, bounds.left + 10, y, command(108), 0x90);
		}
	}
	_font.setFace(Font::kFaceNormal);
	y += 12;
	if (battleAt(record)) {
		_font.drawString(_framebuffer1, x, y, command(75), 0x9a);
		drawSprite(0x8e + (battleProgress(record) + 15) / 32, x + 47, y + 6);
	} else {
		_font.drawString(_framebuffer1, x, y, command(109), 0x9a);
		drawEquipmentRow(_state.begin() + record + 20, x, y + 10, bounds.bottom, 0x9a, 0x4c60);
	}
}

/**
 * What the cabin writes over its window: the place the pointer stands over,
 * or DESERT and its direction from the present place between marks.  Hidden
 * sietchs have no marker; Tuek's clue is followed by selecting the text
 * "DESERT eastwards", exactly as in the supplied playthrough.
 */
Common::String DuneEngine::nameUnderCabin() {
	enum { kSelect = 86 };

	// 4658/46B5 retains the instruction after FF; 45DE/469B cancels it
	// as soon as the pointer identifies a map point, even while typing.
	const Common::String asks = command(kSelect);

	if (_cabinPointerPoint == -2 || _cabinTyped < asks.size())
		return Common::String(asks.c_str(), MIN<uint>(_cabinTyped, asks.size()));

	const Common::Point mouse = _system->getEventManager()->getMousePos();
	return cabinDestinationName(cabinDestinationAt(mouse));
}

/** 4586's retained DI: a place record, FFF0..7 bearing, or FFFF desert. */
uint16 DuneEngine::cabinDestinationAt(const Common::Point &mouse) const {
	int longitude = 0, latitude = 0;
	if (!cabinWorldPointAt(mouse, &longitude, &latitude))
		return 0;
	const int place = cabinPlaceAt(mouse);
	if (place >= 0)
		return 0x100 + 28 * place;

	// 445D/407E projects Paul from his live position, including while he is
	// flying with no current-place record. The old place-based lookup lost
	// every in-flight DESERT direction after a destination change.
	// 445D clears DS:4749 if 62D6 puts him outside the visible cabin map;
	// 4586 then leaves the label plain DESERT rather than naming a bearing.
	const Common::Point marker = cabinPlayerScreenPosition();
	if (marker.x < 0)
		return 0xffff;
	// 445D stores the icon's left at projected x-13 and its bottom at
	// projected y. 45A3 measures the pointer from left+11, not from Paul's
	// world-map column; the latter skews diagonal bearings at high latitudes.
	const int dx = mouse.x - (marker.x - 2);
	const int dy = mouse.y - marker.y;
	// 45A3 has no minimum-distance gate after the marker is projected.
	// Plain DESERT near a place can also result from an inter-wedge bearing.
	const int direction = cabinDirectionIndex(dx, dy);
	return direction >= 0 ? 0xfff0 + direction : 0xffff;
}

/** 45DE/629D/62A6 names the retained target, not a newly moved pointer. */
Common::String DuneEngine::cabinDestinationName(uint16 destination) {
	if (!destination)
		return Common::String();
	if (destination < 0xfff0) {
		if (destination < 0x100 || (destination - 0x100) % 28)
			return Common::String();
		const uint place = (destination - 0x100) / 28;
		if (place >= _map.places().size())
			return Common::String();
		return kindOfPlaceName(_map.places()[place].kind) + nameOfPlace(_map.places()[place]);
	}
	Common::String desert = command(163);
	if (destination < 0xfff8) {
		desert += ' ';
		desert += command(217 + destination - 0xfff0);
	}
	return desert;
}

/** 445D/62D6: Paul's projected marker exists only inside DS:149C. */
Common::Point DuneEngine::cabinPlayerScreenPosition() const {
	const Common::Rect view(81, 45, 241, 134); // DS:149C
	if (_state.size() <= 0x114f)
		return Common::Point(-1, -1);

	uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	int latitude = (int8)_state[6];
	if (_state[6] == 0x80) {
		const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here < 0x100 || (uint)here + 6 > _state.size())
			return Common::Point(-1, -1);
		longitude = READ_LE_UINT16(_state.begin() + here + 2);
		latitude = (int16)READ_LE_UINT16(_state.begin() + here + 4);
	}
	const Common::Point at = _map.surveyPosition(_worldLongitude,
			_worldLatitude, longitude, latitude, view);
	return view.contains(at) ? at : Common::Point(-1, -1);
}

/** 42E9..4305: board a grounded ornithopter, distinct from airborne retargeting. */
void DuneEngine::boardOrnithopter() {
	retargetSkyFade(); // 42EC, before the cabin resource/view is installed.
	_state[0x473e] = 1;
	_state[0x11c9] = 4; // 42FA: boarded, not yet moving (4709 folds to5).
	WRITE_LE_UINT16(_state.begin() + 0x487e, 2);
	_speech.clear();
	_speakerPerson = -1;
	_onWorld = true;
	_inCabin = true;
	_cabinFromMap = false;
	_cabinTyped = 0;
	_cabinPointerPoint = -1;
	lookAtPlayer();
	enterCabin();
}

/** 4305/430E: roll Cancel over the old scene before installing the cockpit. */
void DuneEngine::enterCabin() {
	Common::Array<Common::String> choices;
	choices.push_back(command(162));
	_menuParts.clear();
	_menuParts.push_back(kMapExit);
	_menuKind = kMenuFlight;
	showMenu(choices, nullptr, false, true);
	drawMenu();
	finishPanelMenuTransition();
	_state[0x11ca] = 1; // 4317/4ACA pauses position callbacks in the selector.
	beginCabinInstruction();
	// 4323 installs cabin mode only after D323 returns; 436E then presents
	// the cockpit. Repaints, hover and panning must not reenter this list.
	showCabin();
}

/** 4377: terrain and site/grid layers, followed by 445D's live Paul marker. */
void DuneEngine::drawCabinMap(bool partial) {
	const Common::Rect view(81, 45, 241, 134); // DS:149C
	// B6C3 writes its polar clamp back to DS:197E. Retain that centre,
	// not only the clamped terrain pixels: the next 8831 pan and 4586
	// destination pick must start from the map the player can actually see.
	_worldLatitude = clampSurveyLatitude(_worldLatitude, view.height());
	if (!partial && _state[0x473e]) {
		// Entry also paints the grid's rim outside the map window. Site
		// sprites and Paul's marker remain clipped to the window below.
		_graphics.setFrameBuffer(_framebuffer1);
		openSpriteSheet("ORNYPAN.HSQ", false);
		drawSprite(2, 79, 45);
	}
	memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	_map.drawTerrainInset(_framebuffer2, kScreenWidth, _worldLongitude,
			_worldLatitude, view, _state.begin() + kMapScratchOrigin, kMapScratchBytes);
	openSpriteSheet("ICONES.HSQ", false);
	// 5DCE rebuilds the visible place list in compact map mode too.
	byte *descriptor = _state.begin() + 0xa5c0;
	const byte *descriptorEnd = _state.begin() + 0xaa70;
	for (uint i = 0; i != _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		if (!place.isKnown())
			continue;
		const Common::Point at = _map.surveyPosition(_worldLongitude,
				_worldLatitude, place.longitude, place.latitude, view);
		if (!view.contains(at))
			continue;
		if (descriptor + 8 <= descriptorEnd) {
			WRITE_LE_UINT16(descriptor, 0x100 + 28 * i);
			WRITE_LE_UINT16(descriptor + 2, at.x);
			WRITE_LE_UINT16(descriptor + 4, at.y | 0x0100);
			descriptor += 6;
		}
		uint sprite = 58 + placeClass(place.kind);
		if (place.kind < 0x20 && distanceFromPaul(place.longitude, place.latitude) >
				READ_LE_UINT16(_state.begin() + 0x1176))
			sprite += 5;
		drawSpriteCentred(sprite, at.x, at.y);
	}
	WRITE_LE_UINT16(descriptor, 0);
	if (_state[0x473e]) { // 4358/4386 skips the cockpit grid for the worm.
		openSpriteSheet("ORNYPAN.HSQ", false);
		drawSprite(2, 79, 45);
	}
	// 44AB restores the marker's rectangle from this clean terrain/site/grid
	// page. Keep it separate from scratch framebuffer two and the visible
	// marker. Partial map refreshes must retain the clean instrument strip.
	if (!partial || _cabinMapPage.size() != kScreenWidth * kScreenHeight) {
		_cabinMapPage.resize(kScreenWidth * kScreenHeight);
		memcpy(_cabinMapPage.begin(), _framebuffer2, _cabinMapPage.size());
	} else {
		for (int y = view.top; y < view.bottom; y++)
			memcpy(_cabinMapPage.begin() + y * kScreenWidth + view.left,
					_framebuffer2 + y * kScreenWidth + view.left, view.width());
	}
	// 4396 draws the grid before 445D/44AB paints ICONES 76. Putting
	// the grid last cuts a vertical line through Paul's marker.
	openSpriteSheet("ICONES.HSQ", false);
	const Common::Point player = cabinPlayerScreenPosition();
	_cabinMarkerRemainder = 0;
	_cabinMarkerTime = _system->getMillis() / 5;
	WRITE_LE_UINT16(_state.begin() + 0x4749, 0);
	if (player.x >= 0) {
		int width = 0, height = 0;
		if (currentSpriteSheet->size(76, &width, &height)) {
			WRITE_LE_UINT16(_state.begin() + 0x4749, player.x - 13);
			WRITE_LE_UINT16(_state.begin() + 0x474b, player.y - height);
			WRITE_LE_UINT16(_state.begin() + 0x474d, player.x - 13 + width);
			WRITE_LE_UINT16(_state.begin() + 0x474f, player.y);
			_state[0x4751] = 1; // 445D falls directly into the first 44AB paint.
			drawSprite(76, player.x - 13, player.y - height);
		}
	}
	_graphics.setFrameBuffer(_framebuffer1);
	for (int y = view.top; y < view.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + view.left,
				_framebuffer2 + y * kScreenWidth + view.left, view.width());
}

/** 44AB copies the clean clipped rectangle before drawing odd phases only. */
void DuneEngine::paintCabinMarker() {
	const Common::Rect view(81, 45, 241, 134);
	Common::Rect box(READ_LE_UINT16(_state.begin() + 0x4749),
			READ_LE_UINT16(_state.begin() + 0x474b),
			READ_LE_UINT16(_state.begin() + 0x474d),
			READ_LE_UINT16(_state.begin() + 0x474f));
	const int left = box.left, top = box.top;
	box.clip(view);
	if (box.isEmpty())
		return;
	const bool visible = (_state[0x4751] & 1) != 0;
	byte *target = visible ? _framebuffer2 : _framebuffer1;
	for (int y = box.top; y < box.bottom; y++)
		memcpy(target + y * kScreenWidth + box.left,
				_cabinMapPage.begin() + y * kScreenWidth + box.left, box.width());
	if (visible) {
		_graphics.setFrameBuffer(_framebuffer2);
		openSpriteSheet("ICONES.HSQ", false);
		drawSprite(76, left, top);
		for (int y = box.top; y < box.bottom; y++)
			memcpy(_framebuffer1 + y * kScreenWidth + box.left,
					_framebuffer2 + y * kScreenWidth + box.left, box.width());
	}
	_graphics.setFrameBuffer(_framebuffer1);
	_system->copyRectToScreen(_framebuffer1 + box.top * kScreenWidth + box.left,
			kScreenWidth, box.left, box.top, box.width(), box.height());
	_system->updateScreen();
}

/** D9D2: one 44AB callback per poll, retaining modulo 300 five-ms ticks. */
bool DuneEngine::pollCabinMarker(uint16 ticks) {
	if ((!_inCabin && _menuKind != kMenuWormMap) || _onGlobe || _onMixer || _cabinMapPage.empty() ||
			!READ_LE_UINT16(_state.begin() + 0x4749))
		return false;
	const uint16 elapsed = ticks + _cabinMarkerRemainder;
	_cabinMarkerRemainder = elapsed % 300;
	if (elapsed < 300)
		return false;
	_state[0x4751]++;
	paintCabinMarker();
	return true;
}

void DuneEngine::updateCabinMarker() {
	const uint32 now = animationClockTicks();
	const uint16 elapsed = animationElapsedTicks(_cabinMarkerTime, now);
	_cabinMarkerTime = now;
	pollCabinMarker(elapsed);
}

/** 4658 registers 46B5 at BP=18h, in the shared 200 Hz scheduler. */
void DuneEngine::beginCabinInstruction() {
	_cabinTyped = 0;
	_cabinPointerPoint = -2;
	_cabinTypingRemainder = 0;
	_cabinTypingTime = _system->getMillis() / 5;
}

bool DuneEngine::pollCabinInstruction(uint16 ticks) {
	if ((!_inCabin && _menuKind != kMenuWormMap) || _onGlobe || _onMixer ||
			_cabinPointerPoint != -2)
		return false;
	// D9D2 uses a 16-bit sum and dispatches at most once, not a catch-up
	// loop. FF leaves the registered callback and the completed text intact.
	const uint16 elapsed = ticks + _cabinTypingRemainder;
	_cabinTypingRemainder = elapsed % 24;
	if (elapsed < 24)
		return false;
	const Common::String text = command(86);
	if (_cabinTyped >= text.size())
		return false;
	const uint before = _cabinTyped;
	byte c;
	do {
		c = text[_cabinTyped++];
	} while (c == ' ' && _cabinTyped < text.size());
	// 46B5 writes only the new glyph(s), including immediately skipped
	// spaces. It does not restore a clean strip or redraw the prefix.
	_font.setFace(Font::kFaceAlternate);
	const Common::String prefix(text.c_str(), before);
	const int x = 85 + _font.stringWidth(prefix);
	_font.drawString(_framebuffer1, x, _state[0x473e] ? 38 : 34,
		Common::String(text.c_str() + before, _cabinTyped - before),
		0x61, _state[0x473e] ? 0x20 : 0xf5);
	const Common::Rect strip(77, 33, 245, 45);
	_system->copyRectToScreen(_framebuffer1 + strip.top * kScreenWidth + strip.left,
		kScreenWidth, strip.left, strip.top, strip.width(), strip.height());
	_system->updateScreen();
	return true;
}

void DuneEngine::updateCabinInstruction() {
	const uint32 now = animationClockTicks();
	const uint16 elapsed = animationElapsedTicks(_cabinTypingTime, now);
	_cabinTypingTime = now;
	pollCabinInstruction(elapsed);
}

/** 4658 and 45DE share the font/pen but use different foreground ink. */
void DuneEngine::drawCabinLabel() {
	_font.setFace(Font::kFaceAlternate);
	const bool cockpit = _state[0x473e] != 0;
	const int y = cockpit ? 38 : 34;
	int x = _font.drawString(_framebuffer1, 85, y, nameUnderCabin(),
			_cabinPointerPoint == -2 ? 0x61 : 0xfe, cockpit ? 0x20 : 0xf5);
	if (_cabinPointerPoint != -2 && _font.charWidth(' ') > 0) {
		// 4641 pads the selected caption with opaque spaces up to x=EDh.
		while (x <= 237)
			x = _font.drawChar(_framebuffer1, x, y, ' ', 0xfe, cockpit ? 0x20 : 0xf5);
	}
}

void DuneEngine::hoverCabinLabel(const Common::Point &mouse) {
	if (_onGlobe || _onMixer || (!_inCabin && _menuKind != kMenuWormMap))
		return;
	int across = 0, down = 0;
	const int point = cabinWorldPointAt(mouse, &across, &down)
			? mouse.y * kScreenWidth + mouse.x : -1;
	if (point == _cabinPointerPoint || (point < 0 && _cabinPointerPoint == -2))
		return;
	const uint16 destination = point < 0 ? 0 : cabinDestinationAt(mouse);
	WRITE_LE_UINT16(_state.begin() + 0x46fc, destination);
	paintCabinDestination(destination);
	if (point >= 0)
		_cabinPointerPoint = point;
	if (_menuKind == kMenuWormMap)
		_wormPointerPoint = point;
}

/** 45DE pads even DI=0, preserving pixels beyond the final opaque space. */
void DuneEngine::paintCabinDestination(uint16 destination) {
	if (!destination) {
		if (_cabinPointerPoint != -2)
			beginCabinInstruction();
	} else {
		_cabinTyped = command(86).size();
		_cabinPointerPoint = 0; // 469B removes typing before naming the target.
	}
	_font.setFace(Font::kFaceAlternate);
	const int y = _state[0x473e] ? 38 : 34;
	const byte behind = _state[0x473e] ? 0x20 : 0xf5;
	int x = _font.drawString(_framebuffer1, 85, y, cabinDestinationName(destination), 0xfe, behind);
	if (_font.charWidth(' ') > 0)
		while (x <= 237)
			x = _font.drawChar(_framebuffer1, x, y, ' ', 0xfe, behind);
	const Common::Rect strip(77, 33, 245, 45);
	_system->copyRectToScreen(_framebuffer1 + strip.top * kScreenWidth + strip.left,
		kScreenWidth, strip.left, strip.top, strip.width(), strip.height());
	_system->updateScreen();
}

/** E3A0 services the selector's registered callbacks, not input or1B0D. */
void DuneEngine::pollCabinConfirmation(uint16 ticks) {
	const AnimationClock::Poll poll(_animationClock, _system->getMillis(), ticks);
	updateSkyFade();
	// 4BB9 remains registered in the compact worm selector. Its BP guard
	// suppresses painting there, not D9D2's modulo-sixteen timer update.
	updateDesertHarvester();
	updateCabinMarker();
	updateCabinInstruction();
}

bool DuneEngine::waitCabinConfirmation(uint16 ticks) {
	const bool fast = ConfMan.getBool("dune_nodelay");
	while (ticks-- && !shouldQuit()) {
		if (!fast) {
			const uint32 edge = _system->getMillis() / 5;
			while (!shouldQuit() && _system->getMillis() / 5 == edge)
				_system->delayMillis(1);
		}
		updateGameMusic(); // ACE6 precedes D9D2's shared clock sample.
		uint16 elapsed = 1;
		if (fast) {
			advanceClockElapsed(5, false, false);
		} else {
			tick(false);
			// Include time spent painting the blank/named label between
			// waits. Restarting a local timer here lost those actual ticks.
			if (_animationClock.sampled())
				elapsed = (uint16)(_system->getMillis() / 5 - _animationClock.lastMillis() / 5);
		}
		pollCabinConfirmation(elapsed);
	}
	return !shouldQuit();
}

/** 450E..4569: acknowledge a fixed destination before changing the course. */
bool DuneEngine::confirmCabinDestination(uint16 destination) {
	if (!destination || ((int16)destination >= 0 && !(_state[0x11c9] & 3) &&
			destination == READ_LE_UINT16(_state.begin() + 0x114e)))
		return false;
	if ((int16)destination >= 0 && (destination < 0x100 || (destination - 0x100) % 28 ||
			(uint)(destination - 0x100) / 28 >= _map.places().size()))
		return false;
	WRITE_LE_UINT16(_state.begin() + 0x46fc, destination);
	paintCabinDestination(destination);
	const int pointer = _cabinPointerPoint;
	// 456C decrements the one-based name word before packing its two
	// bytes into AL; negative DI is used unchanged. AB45 uses raw PZ
	// audio, not dialogue/portrait RNG.
	uint16 voice = destination;
	if ((int16)destination >= 0) {
		const uint16 name = READ_LE_UINT16(_state.begin() + destination) - 1;
		voice = (byte)(((name & 0xff) << 4) | (name >> 8));
	}
	voice += 0x2bc;
	_state[0x47e0] = 0; // AB4F precedes ABCC's busy-channel test.
	// ABCC refuses a second request while digital speech is still active;
	// ABA9 below waits for that same channel, not a restarted recording.
	if (!_mixer->isSoundHandleActive(_speechHandle)) {
		stopSpeech();
		const Common::String filename = Common::String::format("PZ\\PZ%03XI.VOC", voice);
		Resource recording = _archive.openMember(filename);
		if (recording)
			playVoice(recording, filename, -1, false);
	}
	bool finished = true;
	for (uint cycle = 0; cycle < 9 && finished; ++cycle) {
		finished = waitCabinConfirmation(20);
		if (!finished)
			break;
		paintCabinDestination(0);
		finished = waitCabinConfirmation(10);
		paintCabinDestination(destination);
	}
	// ABA9 waits for digital playback without another D9D2 or GUI poll.
	const uint32 started = _system->getMillis();
	while (!ConfMan.getBool("dune_nodelay") && !shouldQuit() &&
			_mixer->isSoundHandleActive(_speechHandle) && _system->getMillis() - started < 5000) {
		updateGameMusic();
		_system->delayMillis(5);
	}
	if (!ConfMan.getBool("dune_nodelay"))
		tick(false);
	stopSpeech();
	_state[0xdc2b] = 0;
	_cabinPointerPoint = pointer;
	_state[0x4732] = 0x80;
	return finished && !shouldQuit();
}

/** Hover/typing changes the instrument strip, not 445D's blink registration. */
void DuneEngine::repaintCabinLabel() {
	if ((!_inCabin && _menuKind != kMenuWormMap) || _cabinMapPage.empty())
		return;
	// The cockpit pen is four rows below the worm's. Include all seven
	// glyph rows, not only DS:14A4's eight-row worm backing strip.
	const Common::Rect strip(77, 33, 245, 45);
	for (int y = strip.top; y < strip.bottom; y++)
		memcpy(_framebuffer1 + y * kScreenWidth + strip.left,
				_cabinMapPage.begin() + y * kScreenWidth + strip.left, strip.width());
	drawCabinLabel();
	_system->copyRectToScreen(_framebuffer1 + strip.top * kScreenWidth + strip.left,
			kScreenWidth, strip.left, strip.top, strip.width(), strip.height());
	_system->updateScreen();
}

/**
 * The ornithopter's own cabin, which is what TAKE AN ORNITHOPTER puts up -
 * not the flat map.  0x42EF loads ORNYPAN.HSQ and 0x43DC lays its three
 * pictures down out of the list of threes at DS:0x14B4: the instruments at
 * the corner and nineteen down, which fills the rest of the view; the frame
 * at ten and forty-three; and the grid over the window at seventy-nine and
 * forty-five.  0x43BC fills the strip DS:0x14A4 keeps - 77 and 33 to 245 and
 * 41 - with the colour 0xF5, which is what the country under the pointer is
 * named on.  The bank carries forty-five colours of its own, which go over
 * the map's without disturbing the rest.
 *
 * The world is drawn into the window the grid stands over, and the list in
 * the panel is the one entry of DS:0x212E: a way out.
 */
void DuneEngine::showCabin() {
	enum {
		kInstruments = 0, kFrame = 1,
		kInstrumentsTop = 19, kFrameLeft = 10, kFrameTop = 43,
		kCancel = 162
	};
	_state[0x46eb] = 1; // 4323 installs the ornithopter cabin mode.

	_graphics.setFrameBuffer(_framebuffer1);
	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	// ORNYPAN begins nineteen pixels down.  The cockpit is open above it and
	// the recording keeps the hour's sky in that strip.
	applySkyPalette(true);
	drawSky();

	// The cabin itself first, and its own colours with it: they go over the
	// map's without disturbing the twelve the ground is drawn in.
	// The place symbols in ICONES use the common low character ramp, which a
	// continuous game inherits from the outside scene's final PERS load.  Put
	// that ramp back explicitly as well: a save loaded directly in the cabin
	// otherwise starts with those palette entries black, making all but the
	// single red pixels of a destination disappear.
	openSpriteSheet("PERS.HSQ");
	// C13E's retained ORNYPAN palette owner leaves the sky/panel fade intact.
	// Only the map's low ramp needs restoration on a native repaint/load.
	applyMapColours(80);

	openSpriteSheet("ORNYPAN.HSQ");
	drawSprite(kInstruments, 0, kInstrumentsTop);
	drawSprite(kFrame, kFrameLeft, kFrameTop);

	drawCabinMap(false);

	// What the pointer stands over, written above the window.  0x43BC's fill
	// of the strip at DS:0x14A4 belongs to the other half of 0x43A2 - the one
	// taken while the cabin is not up - so the words go straight onto the
	// instruments.
	const Common::String over = nameUnderCabin();

	debugC(2, kDebugGraphics, "the cabin says \"%s\"", over.c_str());

	drawCabinLabel();

	// The corner box carries the machine's own controls, which is what the
	// flat map shows as well.
	setCompass(kCompassWorld);

	drawPanel();

	Common::Array<Common::String> choices;
	_menuParts.clear();

	choices.push_back(command(kCancel));
	_menuParts.push_back(kMapExit);

	_menuKind = kMenuFlight;
	_menu = choices;
	_menuFlags.clear();
	_menuFlags.push_back(0);
	_menuFrom = 0;
	_menuUnder = menuRowAt(_system->getEventManager()->getMousePos(), true);

	debugMenuIfChanged();

	drawMenu();

	// Complete the immediate sky case after the low instrument colours.
	// An active388D keeps its displayed colours and registered fade phase.
	applySkyPalette(true);

	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
}

/**
 * Where in the world a spot of the cabin's window stands, or nothing where it
 * is outside the window.
 */
bool DuneEngine::cabinPointsAt(const Common::Point &at, int *across, int *down) const {
	int longitude = 0, latitude = 0;
	if (!cabinWorldPointAt(at, &longitude, &latitude))
		return false;
	*across = (uint32)longitude * Map::kEquatorWidth / 65536u;
	*down = Map::kEquator + latitude;
	return true;
}

/** B5F9: retain the original compact-map target without an equatorial round trip. */
bool DuneEngine::cabinWorldPointAt(const Common::Point &at, int *longitude, int *latitude) const {
	enum {
		kWindowLeft = 81, kWindowTop = 45, kWindowWide = 160, kWindowHigh = 89
	};

	if (at.x < kWindowLeft || at.x >= kWindowLeft + kWindowWide
		|| at.y < kWindowTop || at.y >= kWindowTop + kWindowHigh)
		return false;

	const int band = clampSurveyLatitude(_worldLatitude, kWindowHigh)
		+ at.y - (kWindowTop + (kWindowHigh - 1) / 2);
	if (band <= -(int)Map::kLatitudes || band >= (int)Map::kLatitudes)
		return false;
	const int width = _map.width(ABS(band));
	if (!width)
		return false;
	*longitude = insetTargetLongitude(_worldLongitude, width,
			at.x - (kWindowLeft + kWindowWide / 2));
	*latitude = band;

	return true;
}

/** Maps each immutable MAP2 deposit number to its current survey colour. */
void DuneEngine::spiceDensityColours(byte *colours) const {
	buildSpiceDensityColours(colours, _state.begin(), _state.size());
}

/** 813E projects the selected troop's route and corrects longitude seams. */
void DuneEngine::drawTroopRoutePreview() {
	if (_orderingTroop >= _map.troops().size())
		return;
	const Common::Rect map = surveyMapRect();
	auto project = [this, &map](uint16 longitude, int latitude) {
		return _spiceMap.surveyPosition(_surveyLongitude, _surveyLatitude, longitude, latitude, map);
	};
	const Troop &troop = _map.troops()[_orderingTroop];
	Common::Point previous = project(troop.longitude, troop.latitude);
	uint16 previousLongitude = troop.longitude;
	Common::Array<uint> stops;
	if (_orderingTroop == 2) {
		// 813E reads 4718, including over the first handover report. It must
		// not substitute the live journey queue at 11D3 or the editor cache,
		// which is released when the child menu closes.
		for (uint i = 0; i < 3; i++) {
			const uint place = READ_LE_UINT16(_state.begin() + 0x4718 + 2 * i);
			if (place < 0x100 || (place - 0x100) % 28 ||
					(place - 0x100) / 28 >= _map.places().size())
				break;
			stops.push_back((place - 0x100) / 28);
		}
	} else if (troop.place >= 0x100)
		stops.push_back((troop.place - 0x100) / 28);
	for (uint i = 0; i < stops.size(); i++) {
		if (stops[i] >= _map.places().size())
			break;
		const Place &place = _map.places()[stops[i]];
		const Common::Point next = project(place.longitude, place.latitude);
		Common::Point from = previous, to = next;
		int dx = from.x - to.x;
		if (((int16)(previousLongitude - place.longitude) ^ dx) < 0 && ABS(dx) >= 80) {
			if (from.x >= map.left && from.x < map.right) {
				SWAP(from, to);
				dx = -dx;
			}
			from.x += dx < 0 ? 400 : -400;
		}
		drawPatternedMapLine(_framebuffer1, kScreenWidth, from, to, map, 12, 0x5555);
		previous = next;
		previousLongitude = place.longitude;
	}
}

/** Clip ICONES to the modal survey without damaging its frame or the report. */
void DuneEngine::drawSurveySprite(int sprite, int x, int y) {
	const Common::Rect map = surveyMapRect();
	memset(_framebuffer2, 0, kScreenWidth * kScreenHeight);
	_graphics.setFrameBuffer(_framebuffer2);
	drawSprite(sprite, x, y);
	_graphics.setFrameBuffer(_framebuffer1);
	for (int row = map.top; row < map.bottom; row++)
		for (int column = map.left; column < map.right; column++) {
			const uint offset = row * kScreenWidth + column;
			if (_framebuffer2[offset])
				_framebuffer1[offset] = _framebuffer2[offset];
		}
}

/** 62F2/6314/6322: selected-chief or Paul marker on the 1x survey. */
void DuneEngine::drawSurveyMarker(bool troopMarker) {
	uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	int latitude = (int8)_state[6];
	uint16 place = READ_LE_UINT16(_state.begin() + 0x114e);
	bool settled = _state[6] == 0x80;
	if (troopMarker && _orderingTroop < _map.troops().size()) {
		const Troop &troop = _map.troops()[_orderingTroop];
		longitude = troop.longitude;
		latitude = troop.latitude;
		place = troop.place;
		settled = !(troop.state & 0x40);
	}
	if (settled && place >= 0x100 && (uint)place + 6 <= _state.size()) {
		longitude = READ_LE_UINT16(_state.begin() + place + 2);
		latitude = (int16)READ_LE_UINT16(_state.begin() + place + 4);
	}
	const Common::Rect map = surveyMapRect();
	const Common::Point at = _spiceMap.surveyPosition(_surveyLongitude, _surveyLatitude,
			longitude, latitude, map);
	if (!map.contains(at))
		return; // 62D6 tests the anchor before 6322 draws any part of the sprite.
	openSpriteSheet("ICONES.HSQ", false);
	const int sprite = troopMarker ? 54 : 76;
	int width = 0, height = 0;
	if (currentSpriteSheet->size(sprite, &width, &height))
		// 6330 subtracts from BL only: a top-edge underflow wraps to row 250+.
		drawSurveySprite(sprite, at.x - (troopMarker ? 0 : 13), (byte)(at.y - height));
}

/** 541F/5605/563E starts a fresh survey page with an unhighlighted legend. */
void DuneEngine::resetSurveyHover() {
	_pointedAtPlace = -1;
	WRITE_LE_UINT16(_state.begin() + 0x46fc, 0);
	_state[0x4724] = 0xff;
	_state[0x4725] = 254;
}

/** 543B/5459/5523/5564: retain MAP2's bounds, paint slot and cursor frame. */
void DuneEngine::prepareSurveyWindow() {
	const Common::Rect frame = surveyFrameRect();
	WRITE_LE_UINT16(_state.begin() + 0x4710, frame.left);
	WRITE_LE_UINT16(_state.begin() + 0x4712, frame.top);
	WRITE_LE_UINT16(_state.begin() + 0x4714, frame.right);
	WRITE_LE_UINT16(_state.begin() + 0x4716, frame.bottom);
	const uint16 main = READ_LE_UINT16(_state.begin() + 0xdbe0);
	WRITE_LE_UINT16(_state.begin() + (main && main != 0x4710 ? 0xdbe2 : 0xdbe0), 0x4710);
	_state[0x46eb] = 0xc0;
	// 5564 restores the strategic projection, but DAAA keeps MAP2's frame
	// for cursor controls. They are distinct retained descriptors.
	memcpy(_state.begin() + 0x46e3, _state.begin() + 0x1482, 8);
	WRITE_LE_UINT16(_state.begin() + 0xdc58, 0x4710);
}

/** 58FA/5AD9: remove only MAP2's retained slot and restore the map cursor. */
void DuneEngine::closeSurveyWindow() {
	if (!(_state[0x46eb] & 0x40))
		return;
	_state[0x46eb] &= ~0x40;
	const uint slot = READ_LE_UINT16(_state.begin() + 0xdbe0) == 0x4710 ? 0xdbe0 : 0xdbe2;
	const uint window = READ_LE_UINT16(_state.begin() + slot);
	WRITE_LE_UINT16(_state.begin() + slot, 0);
	if (window && window + 8 <= _state.size()) {
		const byte *box = _state.begin() + window;
		paintMapActorChange(Common::Rect(READ_LE_INT16(box), READ_LE_INT16(box + 2),
				READ_LE_INT16(box + 4), READ_LE_INT16(box + 6))); // 5919/C6AD before 5AD9.
	}
	WRITE_LE_UINT16(_state.begin() + 0xdc58, 0x46e3);
}

/** 542F: refresh MAP2 in place, without rebuilding its retained parent. */
void DuneEngine::refreshSurveyWindow() {
	resetSurveyHover();
	WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
	updateSurveyTerritory(); // 5484/5584 consumes queued captured regions too.
	_graphics.setFrameBuffer(_framebuffer1);
	drawSpiceDensityCard();
	const Common::Rect box = surveyFrameRect();
	_system->copyRectToScreen(_framebuffer1 + box.top * kScreenWidth + box.left,
			kScreenWidth, box.left, box.top, box.width(), box.height());
	_system->updateScreen();
}

/** DOS 541F's 1x survey, shared by route selection and the density display. */
void DuneEngine::drawSpiceDensityCard() {
	prepareSurveyWindow();
	// 5523..5558 draws the selected chief when MAP2 is the second window,
	// over a retained report. Use the registered slot, not a menu-kind alias.
	const bool troopMarker = READ_LE_UINT16(_state.begin() + 0xdbe0) != 0x4710;
	const Common::Rect map = surveyMapRect();
	const int left = map.left - 5, top = map.top - 7;
	uint16 paulLongitude = READ_LE_UINT16(_state.begin() + 4);
	int paulLatitude = (int8)_state[6];
	const uint16 here = READ_LE_UINT16(_state.begin() + 0x114e);
	if (_state[6] == 0x80 && here >= 0x100 && (uint)here + 6 <= _state.size()) {
		paulLongitude = READ_LE_UINT16(_state.begin() + here + 2);
		paulLatitude = (int16)READ_LE_UINT16(_state.begin() + here + 4);
	}
	openSpriteSheet("ONMAP.HSQ", false);
	drawSprite(141, left, top);

	byte colours[256];
	const bool troopMode = _state[0x4722] != 0;
	if (troopMode)
		buildTroopDistributionColours(colours, _state.begin(), _state.size());
	else
		spiceDensityColours(colours);
	_spiceMap.drawSurvey(_framebuffer1, kScreenWidth, _surveyLongitude, _surveyLatitude, map, colours, troopMode,
			_state.begin() + kMapScratchOrigin, kMapScratchBytes);
	auto project = [this, &map](uint16 longitude, int latitude) {
		return _spiceMap.surveyPosition(_surveyLongitude, _surveyLatitude, longitude, latitude, map);
	};
	openSpriteSheet("ICONES.HSQ", false);
	// 5DCE adds every known, visible place to both the drawing and hit list.
	// Replace the previous MAP2 entries after the retained strategic entries.
	// E0DB also visits these markers when choosing a keyboard cursor target.
	byte *descriptor = _state.begin() + 0xa5c0;
	const byte *descriptorEnd = _state.begin() + 0xaa70;
	while (descriptor + 8 <= descriptorEnd && READ_LE_UINT16(descriptor) && !(descriptor[5] & 0x40))
		descriptor += 6;
	for (uint i = 0; i < _map.places().size(); i++) {
		const Place &place = _map.places()[i];
		if (!place.isKnown())
			continue;
		const Common::Point at = project(place.longitude, place.latitude);
		if (!map.contains(at))
			continue;
		if (descriptor + 8 <= descriptorEnd) {
			WRITE_LE_UINT16(descriptor, 0x100 + 28 * i);
			WRITE_LE_UINT16(descriptor + 2, at.x);
			WRITE_LE_UINT16(descriptor + 4, at.y | 0x4000);
			descriptor += 6;
		}
		int sprite = 58 + placeClass(place.kind);
		// 5E15/7C8F choose the farther sietch picture in Paul's latitude units.
		if (place.kind < 0x20) {
			const int dx = ABS((int)(int16)(place.longitude - paulLongitude)) / _map.step(ABS(paulLatitude));
			if (MAX(dx, ABS(place.latitude - paulLatitude)) > READ_LE_UINT16(_state.begin() + 0x1176))
				sprite += 5;
		}
		int width = 0, height = 0;
		if (currentSpriteSheet->size(sprite, &width, &height))
			drawSurveySprite(sprite, at.x - width / 2, at.y - height / 2);
	}
	WRITE_LE_UINT16(descriptor, 0);

	// 54AD outlines the large viewport, not Paul's or the troop's location.
	drawSurveyViewport(_framebuffer1, kScreenWidth, project(_worldLongitude, _worldLatitude), map);

	drawSurveyMarker(troopMarker);

	// The pending route is drawn after the map and its markers.
	if (troopMarker)
		drawTroopRoutePreview();

	drawSurveyLegend();
	// C4F0/1797 restores the raised panel crest before copying this window.
	// Only its intersection is presented: leave the command panel and the
	// surrounding retained map untouched, including during outline effects.
	Common::Rect crest = surveyFrameRect();
	crest.clip(Common::Rect(126, 137, 194, kScreenHeight));
	if (!_state[0x227d] && !crest.isEmpty()) {
		memcpy(_framebuffer2, _framebuffer1, kScreenWidth * kScreenHeight);
		_graphics.setFrameBuffer(_framebuffer2);
		drawFace();
		_graphics.setFrameBuffer(_framebuffer1);
		for (int y = crest.top; y < crest.bottom; y++)
			memcpy(_framebuffer1 + y * kScreenWidth + crest.left,
					_framebuffer2 + y * kScreenWidth + crest.left, crest.width());
	}
}

/** 5605/5692/5746: redraw just the hoverable legend beneath MAP2. */
void DuneEngine::drawSurveyLegend() {
	const Common::Rect map = surveyMapRect();
	const int left = map.left - 5, top = map.top - 7;
	const bool troopMode = _state[0x4722] != 0;
	_graphics.setFrameBuffer(_framebuffer1);
	_graphics.fillRect(left + 6, top + 98, left + 164, top + 106, 245);
	_font.setFace(Font::kFaceAlternate);
	if (_pointedAtPlace >= 0 && (uint)_pointedAtPlace < _map.places().size()) {
		const Place &place = _map.places()[_pointedAtPlace];
		Common::String kind = kindOfPlaceName(place.kind);
		kind.trim();
		_font.drawString(_framebuffer1, left + 6, top + 98,
			kind + " " + nameOfPlace(place), 254);
		const uint16 record = 0x100 + _pointedAtPlace * 28;
		if (record >= 0x138 && place.kind != 0x21) {
			const SurveyTroopCounts counts = countSurveyTroops(_state.begin(), _state.size(), record);
			int x = left + 113;
			if (!(counts.occupation[0] | counts.occupation[1] | counts.occupation[2])) {
				_font.drawString(_framebuffer1, x, top + 98, command(101), 251);
			} else {
				openSpriteSheet("ONMAP.HSQ", false);
				for (uint category = 0; category < 3; category++) {
					if (!counts.occupation[category])
						continue;
					drawSprite(130 + category, x, top + 98);
					// 56ED writes one character ('0' + count), not decimal digits.
					const char label[] = {':', (char)('0' + counts.occupation[category]), 0};
					_font.drawString(_framebuffer1, x + 6, top + 98, label, 251);
					x += 18;
				}
			}
		}
	} else {
		const byte ink = _state[0x4725] == 245 ? 245 : 254;
		_font.drawString(_framebuffer1, left + 6, top + 98, command(troopMode ? 103 : 100),
				ink, ink == 245 ? 254 : 245);
		if (troopMode)
			return;
		_font.drawString(_framebuffer1, left + 89, top + 98, "-", 254);
		_font.drawString(_framebuffer1, left + 154 + _font.stringWidth("-"), top + 98, "+", 254);
		openSpriteSheet("ONMAP.HSQ", false);
		drawSprite(128, left + 95, top + 99);
		drawSprite(129, left + 155, top + 99);
		// 578B/DNVGA 3724 outlines one four-pixel density swatch with XOR
		// 0F. Reapply once to the clean legend on full redraws, not on actor
		// callbacks which retain MAP2's existing foreground pixels.
		// Older native saves left both hover bytes zero; those are not a
		// selected swatch until the pointer has prepared a valid legend ink.
		if ((_state[0x4725] == 245 || _state[0x4725] == 254) && _state[0x4724] < 16) {
			const int x = left + 94 + 4 * _state[0x4724], y = top + 98;
			for (int col = x; col < x + 5; col++) {
				_framebuffer1[y * kScreenWidth + col] ^= 15;
				_framebuffer1[(y + 6) * kScreenWidth + col] ^= 15;
			}
			for (int row = y + 1; row < y + 6; row++) {
				_framebuffer1[row * kScreenWidth + x] ^= 15;
				_framebuffer1[row * kScreenWidth + x + 4] ^= 15;
			}
		}
	}
}

void DuneEngine::showWorld(bool present) {
	const bool stageTroopReport = isStageEventMenu() && _orderingTroop < _map.troops().size();
	_state[0x46df] = 0; // Direct room-contact map entry also uses18BA.
	// MAP/ONMAP reuse 4C60. Returning to a carried scene must prepare its
	// own inset again, rather than display the selector's shared scratch.
	_flightInsetSet = false;
	// 5A1A/18BA/B21 tears down the battlefield callback at map entry.
	// This also repairs older covered checkpoints which kept it registered.
	// Ordinary redraws are harmless: stopping an absent callback is a no-op.
	stopBattleAnimation();
	if (_mapAnimationRedraw) {
		repaintMapActors(present);
		return;
	}
	Common::Array<byte> visible;
	if (_mapRebuildRedraw) {
		visible.resize(kScreenWidth * kScreenHeight);
		memcpy(visible.begin(), _framebuffer1, visible.size());
	}
	memcpy(_state.begin() + 0x46e3, _state.begin() + 0x1482, 8); // 5A66/5564.
	WRITE_LE_UINT16(_state.begin() + 0xdc58, 0x46e3); // 5AD9 before any MAP2 child.
	_graphics.setFrameBuffer(_framebuffer1);
	// ICONES has no palette of its own.  Restore the shared low character ramp
	// explicitly before ONMAP's colours, so a save loaded directly on the map
	// does not turn every place and player marker black.
	if (!_mapRebuildRedraw)
		openSpriteSheet("PERS.HSQ"); // 5D82 preserves the contacted chief's ramp.
	openSpriteSheet("ONMAP.HSQ");

	// The world has its own face for the panel's corner box: the ornithopter's
	// controls rather than the four ways out of a room.
	setCompass(kCompassWorld);

	memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
	_map.drawStrategic(_framebuffer1, kScreenWidth, _worldLongitude, _worldLatitude);
	// 5B69/C560 paints inclusive one-pixel outlines around the 312x144
	// terrain inset. fillRect's right/bottom bounds are exclusive.
	for (int edge = 0; edge < 4; edge++) {
		const byte colour = 246 + edge * 2;
		_graphics.fillRect(edge, edge, 320 - edge, edge + 1, colour);
		_graphics.fillRect(edge, 151 - edge, 320 - edge, 152 - edge, colour);
		_graphics.fillRect(edge, edge, edge + 1, 152 - edge, colour);
		_graphics.fillRect(319 - edge, edge, 320 - edge, 152 - edge, colour);
	}

	drawMapScenery();

	// 5A9A paints Paul before creating the ONMAP actors and selection ring.
	// C412 retains this background. Actor callbacks must not expose terrain
	// or trees changed since the last map redraw, especially offscreen growth.
	_mapCleanPage.resize(kScreenWidth * kScreenHeight);
	memcpy(_mapCleanPage.begin(), _framebuffer1, _mapCleanPage.size());
	openSpriteSheet("ONMAP.HSQ", false);
	drawMapUnits();
	if (_menuKind == kMenuMap && _menuFlags.size() > 1 &&
			READ_LE_UINT16(_state.begin() + 0x1176) >= 2) {
		// 878C uses DS:3CBE after 6715 has registered the visible actors.
		// Recruited troops elsewhere cannot make CONTACT FREMEN TROOPS live.
		_menuFlags[1] = _mapUnitOrder.empty() ? 0x4000 : 0;
	}

	// The legend is a retained window, not a consequence of the contact's
	// return destination. Redraws must not resurrect a dismissed legend.
	if (!_mapRebuildRedraw && _menuKind == kMenuMap && READ_LE_UINT16(_state.begin() + 0xdbe0) == 0x194a)
		drawMapTitle();

	// The panel already exists when 79EE paints a chief's report. Its raised
	// centre overlaps the lower card's last three rows, so restore it first.
	drawPanel();

	// Contacting a troop puts that Fremen's report over the map.
	if (!_mapRebuildRedraw && (_menuKind == kMenuTroopContact || _menuKind == kMenuTroopNotice ||
		_menuKind == kMenuOrders ||
		_menuKind == kMenuEquipment || _menuKind == kMenuProspectorBriefing ||
		_menuKind == kMenuMoveTroop || stageTroopReport))
		drawTroopStatus(_orderingTroop);
	if (!_mapRebuildRedraw && _menuKind == kMenuEquipment)
		drawEquipmentCard(_orderingTroop);

	if (!_mapRebuildRedraw && hasSurveyMap())
		drawSpiceDensityCard();

	drawMenu();
	if (!_mapRebuildRedraw)
		drawMapTroopCard();

	const int tooltip = (_menuKind == kMenuPlace || _menuKind == kMenuMap) ? _pressedPlace : -1;
	if (_mapRebuildRedraw) {
		retainMapForegrounds(visible.begin());
		// 5DAE presents the rebuilt map with the old foregrounds intact.
		// refreshBattleMenu runs 1C18 and 5DC7's separate MAP2 copy next.
	} else if (tooltip >= 0)
		drawMapTooltip((uint)tooltip);
	else if (!_speech.empty() && (_menuKind != kMenuMap || mapTroopCard() < 0) &&
			 _menuKind != kMenuTroopContact &&
			 _menuKind != kMenuTroopNotice && _menuKind != kMenuOrders
			 && _menuKind != kMenuEquipment &&
			 _menuKind != kMenuProspectorBriefing && _menuKind != kMenuMoveTroop && !stageTroopReport)
		drawSpeech(_speech, kSpeechColour);

	if (_surveyDragging && canDragSurvey())
		drawSurveyDragOutline(_framebuffer1, kScreenWidth, _surveyDragPosition);

	if (present) {
		finishPanelMenuTransition();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}
}

/** DNVGA effect 2A for the 0FA7/189A room return. */
void DuneEngine::showRoomConversionTransition(const byte *oldPage, const byte *oldPalette) {
	Common::Array<byte> target(kScreenWidth * kScreenHeight);
	memcpy(target.begin(), _framebuffer1, target.size());
	byte newPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(newPalette, 0, 256);
	_system->getPaletteManager()->setPalette(oldPalette, 0, 256);
	memcpy(_framebuffer1, oldPage, target.size());
	const bool cursorShown = CursorMan.showMouse(false);
	const bool fast = ConfMan.getBool("dune_nodelay");
	uint32 deadline = _system->getMillis();
	uint elapsed = 0;
	for (uint phase = 0; phase < 130 && !shouldQuit(); phase++) {
		if (phase == 65)
			_system->getPaletteManager()->setPalette(newPalette, 0, 256);
		Graphics::dissolveRoomConversion(target.begin(), _framebuffer1, phase);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		updateGameMusic();
		deadline += 15; // 2572: three 200-Hz ticks, including the repeated offset.
		elapsed += 15;
		const uint32 now = _system->getMillis();
		if (!fast && now < deadline)
			_system->delayMillis(deadline - now);
	}
	// C108/C4CD copies the panel too, after the scene-only driver effect.
	memcpy(_framebuffer1, target.begin(), target.size());
	_system->getPaletteManager()->setPalette(newPalette, 0, 256);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
	if (fast)
		advanceClockElapsed(elapsed, false, false);
	CursorMan.showMouse(cursorShown);
}

/** DNVGA effect 0x34 used by DOS 5A1A while entering the strategic map. */
void DuneEngine::showMapEntryTransition(const byte *oldPage, const byte *oldPalette, bool raiseFace) {
	Common::Array<byte> target(kScreenWidth * kScreenHeight);
	memcpy(target.begin(), _framebuffer1, target.size());
	byte newPalette[256 * 3];
	_system->getPaletteManager()->grabPalette(newPalette, 0, 256);
	_system->getPaletteManager()->setPalette(oldPalette, 0, 256);
	memcpy(_framebuffer1, oldPage, target.size());
	const bool cursorShown = CursorMan.showMouse(false);
	const bool fast = ConfMan.getBool("dune_nodelay");
	uint32 deadline = _system->getMillis();
	for (uint phase = 0; phase < 32 && !shouldQuit(); phase++) {
		if (phase == 16)
			_system->getPaletteManager()->setPalette(newPalette, 0, 256);
		Graphics::dissolveMapPage(target.begin(), _framebuffer1, phase);
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
		updateGameMusic();
		deadline += 15; // DNVGA 2572 waits three 200-Hz ticks per pass.
		const uint32 now = _system->getMillis();
		if (!fast && now < deadline)
			_system->delayMillis(deadline - now);
	}
	memcpy(_framebuffer1, target.begin(), target.size());
	_system->getPaletteManager()->setPalette(newPalette, 0, 256);
	_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
	_system->updateScreen();
	if (!raiseFace) {
		// AEFA's book entry shares effect34, but deliberately remains lowered.
	} else if ((_menuKind != kMenuMap || _state[0x46f3]) && !_state[0x11c9] &&
			_state[0xe8] < 10 && _roomPanelFaceValid && !_inDesert) {
		// Direct troop contact restores the room during its first raise.
		// 5A03's room-origin latch distinguishes that entry even though both
		// paths have already installed the ordinary map menu at this point.
		// Ordinary SEE DUNE MAP instead raises over the newly retained map;
		// the actual5A30/5BB0 frames must not briefly expose the old room.
		byte mapBacking[20 * 23];
		memcpy(mapBacking, _openingPanelFaceBacking, sizeof(mapBacking));
		memcpy(_openingPanelFaceBacking, _roomPanelFaceBacking, sizeof(mapBacking));
		raisePanelFace();
		memcpy(_openingPanelFaceBacking, mapBacking, sizeof(mapBacking));
		for (uint y = 0; y < 23; y++)
			memcpy(_framebuffer1 + (137 + y) * kScreenWidth + 150,
					mapBacking + y * 20, 20);
		drawFace();
		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	} else {
		raisePanelFace(); // 5A3A/17E6, including airborne map selectors.
	}
	CursorMan.showMouse(cursorShown);
}

/** The driver's independent timer fades without running ACE6/AD37. */
void DuneEngine::updateMusicFade() {
	if (_music)
		_music->fade();
}

void DuneEngine::updateGameMusic() {
	// AD37 completes a fade and starts the song requested by the new place.
	// D9D2 also services music during the battlefield's synchronous waits.
	updateMusicFade();
	// ACE6 first checks ABCC's digital sample-busy latch. Defer both
	// prepared game requests and CD-style handoffs while a voice plays.
	// The driver's fade still progresses through its independent timer.
	if (_mixer->isSoundHandleActive(_speechHandle))
		return;
	if (_music) {
		// ACE6 reaches AD37 only in game-relative mode; AD37 loads the
		// retained request when playback has ceased, even if its old label
		// names that same song. CD style and MUSIC OFF must not start it.
		if (!musicIsOff() && !(_state[kHowMusicGoes] & kCdStyle) &&
				!_music->fading() && !_music->playing() && !_wantedSong.empty()) {
			_song = _wantedSong;
			_music->play(_song);
			debugC(1, kDebugSound, "the song here is %s", _song.c_str());
		}
	}
	// ACE6 puts on the next CD-style playlist entry when the current one ends.
	if (!musicIsOff() && _state.size() > kHowMusicGoes &&
			(_state[kHowMusicGoes] & kCdStyle) && _music && !_music->playing()) {
		const uint16 ticks = (uint16)(_system->getMillis() / 5);
		WRITE_LE_UINT16(_state.begin() + 0xce7a, ticks);
		if (playlistDelayElapsed(ticks)) {
			_song.clear();
			playNextOfPlaylist(false);
		}
	}
}

/** A25B: retain the real report descriptor and its deferred table offset. */
void DuneEngine::selectLocalSpeakerReport(uint16 line) {
	if ((_speakerPerson != 3 && _speakerPerson != 11 && _speakerPerson != 13) || _onWorld || _onGlobe ||
			_inCabin || _onMixer || _takingOff || (int8)_state[0xea] > 0)
		return;
	const uint16 key = line | 0x800;
	for (uint at = 0x1984; at + 10 <= _state.size(); at += 10) {
		const uint16 next = READ_LE_UINT16(_state.begin() + at);
		if (next > key)
			break;
		if (next != key)
			continue;
		const uint16 kind = READ_LE_UINT16(_state.begin() + at + 2);
		const int variant = localSpeakerVariant();
		const uint offset = (kind == 0x38 ? 0x10 : 0x0a) +
			(variant >= 0 ? 4 * variant + 2 : 0);
		WRITE_LE_UINT16(_state.begin() + 0x47e1, (offset << 8) | 1);
		WRITE_LE_UINT16(_state.begin() + 0x47e4, at + 2);
		_speakerCardValueValid = false;
		_speakerCardText.clear();
		break;
	}
}

/** E353(12): input/audio remain live, but no ordinary pose/hour dispatch. */
void DuneEngine::waitLocalSpeakerReportFrame() {
	if (ConfMan.getBool("dune_nodelay")) {
		advanceClockElapsed(60, false, false);
		return;
	}
	const uint32 began = _system->getMillis();
	while (!shouldQuit() && _system->getMillis() - began < 60) {
		updateGameMusic();
		if (pollLocalSpeechMouth()) {
			_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
			_system->updateScreen();
		}
		// 9B49 ignores E353's carry: a key shortens this wait, not the
		// remaining lowering sequence, and does not select another command.
		if (pollSkip() != kPlaybackFinished)
			break;
		_system->delayMillis(5);
	}
	tick(false);
}

/** 949F/9B49: lower an opened report before traversing the next record. */
void DuneEngine::closeLocalSpeakerReport() {
	if ((_speakerPerson != 3 && _speakerPerson != 11 && _speakerPerson != 13) || _onWorld || _onGlobe ||
			_inCabin || _onMixer || _takingOff || (int8)_state[0xea] > 0)
		return;
	const byte phase = _state[0x47e1];
	const int opening = _state[0x47e2] / 2;
	const bool supported = _speakerPerson == 3 ? opening == 5 || opening == 8 :
		_speakerPerson == 11 ? opening == 5 : opening == 6 + 2 * localSpeakerVariant();
	if (phase == 0x80 && supported) {
		_speakerReportLowering = true;
		_state[0x47e1] = 0x81; // 9C79 bypasses the active-mouth bottom clip.
		const bool voiceActive = _speakerVoiceActive;
		_speakerVoiceActive = false; // 99F6 paints SI, not the speech block.
		_speakerReport = 0;
		_speakerReportLowered = false;
		_speakerSpeechMode = true; // A completed 9AB4 report is already in 9A40.
		_speakerAnimation = opening + 1;
		Common::Array<SpeakerSprite> sprites;
		uint frames = 0;
		collectSpeakerSprites(_speakerPerson, 0, _speakerAnimation, -1, sprites, &frames);
		for (uint frame = 0; frame < frames && !shouldQuit(); frame++) {
			_speakerFrame = frame;
			_speakerVoiceRemaining--; // 99F6 consumes the shared 47CE counter.
			if (!repaintLocalSpeakerFigure()) {
				if (_inDesert)
					drawDesertDialogue();
				else
					drawScene();
			}
			waitLocalSpeakerReportFrame();
		}
		_speakerVoiceActive = voiceActive;
		// Keep the last painted reverse frame. Only the next callback or
		// prepared paragraph selects a new body pose.
		_speakerReportLowered = frames != 0;
	} else if (phase == 1) {
		_speakerReport = 0; // The still-pending report never starts now.
	}
	WRITE_LE_UINT16(_state.begin() + 0x47e1, 0);
	_speakerCardText.clear();
	_speakerReportLowering = false;
}

/** D9D2: service the retained pose and speech callbacks before reading input. */
void DuneEngine::updateLocalSpeaker() {
	if (_endingCoupleActive) {
		updateEndingCouple();
		return;
	}
	// The slower pose callback restores only changed parts from the retained
	// backdrop. Audio-timed mouth cues remain independent of that pose tick.
	// CS:1313's spoken beats use the same 9761/978E/9908 portrait
	// callback as ordinary conversation, but keep the gathering menu.
	// Excluding that menu freezes the face and lets the mouth-close path
	// repaint a room over the desert. Silent poses have no active speaker.
	const bool desertGathering = _menuKind == kMenuPalaceGathering &&
		!_menuParts.empty() && _menuParts[0] >= 310 && _menuParts[0] <= 318;
	// 982B/967B can restore the desert commands over a follower's retained
	// portrait. Its callbacks still belong to the outdoor scene after STOP;
	// treating that menu as no portrait reset the pose through drawScene(),
	// replacing the landscape with an empty indoor room.
	const bool retainedDesertPortrait = _menuKind == kMenuDesert && _speech.empty();
	// A244's answer list keeps the same portrait and 99BE callback. It can
	// be opened outdoors too (for example Stilgar's final attack question).
	const bool desertTalk = _inDesert && _flyingTo < 0 &&
		(_menuKind == kMenuTalk || _menuKind == kMenuShipmentResponse ||
		 desertGathering || retainedDesertPortrait);
	const bool flightTalk = (_inDesert && _flyingTo >= 0 && _flightDiscoveryPrompt &&
		_menuKind == kMenuDiscovery) || carriedPortraitVisible();
	const bool speaking = (!_inDesert || desertTalk || flightTalk) && !_onWorld && _speakerPerson >= 0
		&& _mixer->isSoundHandleActive(_speechHandle);
	const bool idlePortrait = (!_inDesert || desertTalk || flightTalk) && !_onWorld &&
		_speakerIdleActive && _speakerIdlePerson == _speakerPerson && _speakerPerson >= 0 &&
		_menuKind != kMenuTroopNotice &&
		(!_speakerReport || (_speakerPerson == 3 &&
			(_speakerReport <= 4 || _speakerReport == 6)) ||
			(_speakerPerson == 11 && _speakerReport == 4) ||
			(_speakerPerson == 13 && _speakerReport == 5)) &&
		// 9B49 leaves the last reverse pose selected, but does not
		// unregister 99BE. Keep its timer even after the voice expires.
		(_speakerAnimation <= 3 || _speakerPerson >= 12 || _speakerReportLowered ||
			(_speakerPerson == 11 && _speakerReport == 4) ||
			(_speakerPerson == 3 &&
				((_speakerReport >= 1 && _speakerReport <= 4) || _speakerReport == 6)));

	const uint32 portraitTick = animationClockTicks();
	bool portraitRepaint = false;
	if (idlePortrait) {
		const uint16 elapsed = animationElapsedTicks(_speakerIdleTime, portraitTick);
		_speakerIdleTime = portraitTick;
		portraitRepaint = pollLocalSpeakerAnimation(elapsed);
	} else {
		if (speaking && animationClockMillis() >= _nextMouthTime) {
			portraitRepaint = advanceLocalSpeakerAnimation();
			_nextMouthTime = animationClockMillis() + 1000 / 12;
		}
		// A portrait callback is not accumulating time while another screen
		// owns the display; a new local pose re-registers it through 9908.
		_speakerIdleTime = portraitTick;
	}
	if (portraitRepaint && !repaintLocalSpeakerFigure()) {
		if (flightTalk)
			showFlightDiscovery();
		else if (desertTalk)
			drawDesertDialogue();
		else
			drawScene();
	}
	// A7C2/A82E's audio cue painter is independent of 99BE's pose tick.
	if (pollLocalSpeechMouth()) {
		copyFrameRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();
	}

	// The mouth closes again when there is no longer anything to hear.
	if (!speaking && !idlePortrait && !_inCabin && !_onWorld &&
		(_speakerFrame != 0 || _speakerVoiceActive) && _speakerPerson >= 0) {
		_speakerVoiceActive = false;
		_speakerVoiceRemaining = 0;
		_speakerFrame = 0;
		if (flightTalk)
			showFlightDiscovery();
		else if (desertTalk)
			drawDesertDialogue();
		else
			drawScene();
	}
}

/** 98B2: release a silent figure when its presentation is dismissed. */
void DuneEngine::releaseSilentLocalSpeaker() {
	if (_state[0x47c3])
		return; // An active speech figure retains its callbacks.
	WRITE_LE_UINT16(_state.begin() + 0x4540, 0);
	_state[0x47d1] &= 0x3f;
	// The native registration is separate from DOS's retained pose pointer.
	const bool registered = _speakerIdleActive || READ_LE_UINT16(_state.begin() + 0x47c8);
	WRITE_LE_UINT16(_state.begin() + 0x47c8, 0);
	if (!registered)
		return;
	WRITE_LE_UINT16(_state.begin() + 0x1bf8, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1c06, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47ce, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47c6, 0);
	_speakerIdleActive = false;
	_speakerIdlePerson = -1;
	stopSpeech(); // 98E2/9B8B also releases the mouth callback.
}

/** 37B2/98E6: a new landscape releases even an active speech figure. */
void DuneEngine::releaseLocalSpeakerForScene() {
	WRITE_LE_UINT16(_state.begin() + 0x1c06, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1bf8, 0);
	WRITE_LE_UINT16(_state.begin() + 0x1bea, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47c8, 0);
	WRITE_LE_UINT16(_state.begin() + 0x47aa, 0);
	WRITE_LE_UINT16(_state.begin() + 0x479e, 0);
	stopSpeech(); // 9B8B/A7A5, without finishing the conversation.
	_state[0x47c3] = 0;
	WRITE_LE_UINT16(_state.begin() + 0x47ce, 0);
	_state[0x47d1] &= 0x7f;
	WRITE_LE_UINT16(_state.begin() + 0x47c6, 0);
	_speakerIdleActive = false;
	_speakerIdlePerson = -1;
	_speakerVoiceActive = false;
	_speakerSpeechMode = false;
	_speakerVoiceRemaining = 0;
}

/** 9799/9908: another paragraph keeps an existing same-bank99BE timer. */
void DuneEngine::prepareLocalSpeakerParagraph() {
	const bool registered = _speakerIdleActive;
	const int picture = _speakerIdlePicture;
	const uint32 previous = _speakerIdleTime;
	const uint16 remainder = _speakerIdleRemainder;
	prepareLocalSpeakerIdle();
	// 91A0/98B2 removes the old callback only when its bank changes.
	// Otherwise 9932 sees a retained cursor and does not call DA25 again.
	if (registered && _speakerIdleFrameCount > 0 && picture == _speakerIdlePicture) {
		_speakerIdleActive = true; // 9932 retains an existing same-bank timer even for a positive request.
		_speakerIdleTime = previous;
		_speakerIdleRemainder = remainder;
	}
}

/** 93B6/9799 and 9908: select the local speaker's first idle pose. */
void DuneEngine::prepareLocalSpeakerIdle() {
	// 978E calls 4ACA before preparing any local portrait, even on the
	// ground. 97CF clears this pause only for a carried conversation;
	// a subsequent departure explicitly resumes the position callback.
	_state[0x11ca] = 1;
	_speakerFrame = 0;
	_speakerAnimation = 0;
	_speakerIdleVariant = -1;
	_speakerVoiceActive = false;
	_speakerSpeechMode = false;
	_speakerVoiceRemaining = 0;
	_speakerCardValueValid = false;
	_speakerReportLowered = false;
	_speakerCardText.clear();
	_speakerIdleActive = !_onWorld && _speakerPerson >= 0 && _speakerPerson < 16;
	_speakerIdlePerson = _speakerIdleActive ? _speakerPerson : -1;
	_speakerIdlePicture = _speakerIdleActive
		? fremenPicture(_speakerPerson, _speakerPerson == 15 ? _state[0x476c] & 7 : 0) : -1;
	_speakerIdleNextFrame = 0;
	_speakerIdleFrameCount = 0;
	_speakerIdleRemaining = _state.size() > 0x478c ? _state[0x478c] * 4 : 0;
	_speakerIdleTime = _system->getMillis() / 5;
	_speakerIdleRemainder = 0; // DA25 starts a new 99BE callback interval.
	if (!_speakerIdleActive)
		return;
	const int variant = localSpeakerVariant();
	if (_speakerPerson == 14 || _speakerPerson == 15)
		_speakerIdleVariant = variant;
	_speakerAnimation = variant >= 0 ? variant : (troopRandom(true) & 6) / 2;
	if (variant < 0 && _speakerPerson == 12 && (_state[0x10a7] & 0x10))
		_speakerAnimation += 5;
	if (variant >= 0)
		_speakerFrame = _speakerIdleNextFrame = 32; // 996C skips the talking frames.
	Common::Array<SpeakerSprite> sprites;
	uint frames = 0;
	collectSpeakerSprites(_speakerPerson, 0, _speakerAnimation, -1, sprites, &frames);
	_speakerIdleFrameCount = frames;
	// 9936/993B: a positive automatic request still prepares its pose,
	// but must not register a new99BE. Paragraphs can retain an existing
	// same-bank timer through 9932 above; do not cancel that registration.
	_speakerIdleActive = frames != 0 && (int8)_state[0xea] <= 0;
}

/** D9D2's 16-bit accumulation for the registered 99BE portrait callback. */
bool DuneEngine::pollLocalSpeakerAnimation(uint16 ticks) {
	if (!_speakerIdleActive)
		return false;
	const uint16 elapsed = ticks + _speakerIdleRemainder;
	_speakerIdleRemainder = elapsed % 16;
	// A delayed poll calls 99BE once, never once per missed interval.
	return elapsed >= 16 && advanceLocalSpeakerAnimation();
}

/** One registered 99BE portrait tick, apart from the timer and repaint. */
bool DuneEngine::advanceLocalSpeakerAnimation() {
	const int variant = localSpeakerVariant();
	bool textGesture = (_speakerPerson == 3 &&
		((_speakerReport >= 1 && _speakerReport <= 4) || _speakerReport == 6)) ||
		(_speakerPerson == 11 && _speakerReport == 4);
	bool merchantGesture = _speakerPerson == 13 && _speakerReport == 5;
	const int gestureAnimation = merchantGesture ? 6 + 2 * variant : (_speakerReport == 6 ? 8 : 5);
	if ((textGesture || merchantGesture) && _speakerAnimation == gestureAnimation) {
		// 9AB4 holds a report animation for twenty registered callbacks.
		// The first selected frame is zero; the last is nineteen.
		if (_speakerFrame < 19) {
			_speakerFrame++;
			return true;
		}
		_speakerReport = 0;
		if (_state[0x47e1] == 0x81) {
			_state[0x47e1] = 0x80; // 9AB4 retains the completed report for 9B49.
			completeLocalSpeakerReport();
		}
		_speakerAnimation = MAX(variant, 0);
		_speakerFrame = 0;
		textGesture = false;
		merchantGesture = false;
		// The next callback resumes 9A7B's speech-block selector.
	}
	// Action-selected gesture/report animations have their own selector. Keep
	// the existing voiced progression until that DOS path is reconstructed;
	// 99BE's ordinary random idle choices must not replace a held invoice.
	if ((_speakerReport && !textGesture && !merchantGesture) ||
		(_speakerAnimation > 3 && _speakerPerson != 12 && variant < 0 && !textGesture && !_speakerReportLowered)) {
		_speakerFrame++;
		return true;
	}
	if (!_speakerSpeechMode && (!_speakerIdleActive || _speakerIdlePerson != _speakerPerson)) {
		_speakerFrame++;
		return true;
	}
	if (!_speakerSpeechMode) {
		if (_speakerIdleNextFrame >= _speakerIdleFrameCount) {
			if (_speakerIdleRemaining < 0) {
				_speakerSpeechMode = true;
				// 9A3B changes mode without resetting the shared 47CE counter.
				// 9985 still uses its low bits if speech is repeated while
				// 9A7B is retrying the first random speech-block selection.
				_speakerVoiceRemaining = _speakerIdleRemaining;
			} else {
				_speakerAnimation = variant >= 0 ? variant : (troopRandom(true) & 6) / 2;
				if (variant < 0 && _speakerPerson == 12 && (_state[0x10a7] & 0x10))
					_speakerAnimation += 5;
				Common::Array<SpeakerSprite> sprites;
				uint frames = 0;
				collectSpeakerSprites(_speakerPerson, 0, _speakerAnimation, -1, sprites, &frames);
				_speakerIdleFrameCount = frames;
				_speakerIdleNextFrame = variant >= 0 ? 32 : 0;
			}
		}
		if (!_speakerSpeechMode) {
			if (_speakerIdleFrameCount <= 0)
				return false;
			_speakerFrame = _speakerIdleNextFrame++;
			_speakerIdleRemaining--;
			return true;
		}
	}
	// 9A40/9AB4 runs after a silent idle timeout too, without first
	// selecting an extra speech block or spending an extra random word.
	if ((textGesture || merchantGesture) && _speakerVoiceRemaining <= 0) {
		_speakerAnimation = gestureAnimation;
		_speakerFrame = 0;
		_state[0x47e1] = 0x81;
		_state[0x47e2] = 2 * gestureAnimation;
		if (_speakerReport == 6 || _speakerReport == 5) {
			_speakerCardValue = _speakerPerson == 3 ? READ_LE_UINT16(_state.begin() + 0x20) : _state[0x9d];
			_speakerCardValueValid = true;
		}
		return true;
	}
	// 9F1C starts at an eight-frame boundary. Once those eight callbacks have
	// painted, 9A7B retries E3B7 at each callback until its masked high byte
	// is zero, then picks another block in the speech animation. Skipped
	// callbacks leave the previous frame on the page, but do consume RNG.
	if (_speakerVoiceRemaining <= 0) {
		const uint16 choice = troopRandom(true) & (variant >= 0 ? 0x0f18 : 0x0f38);
		if (choice & 0x0f00)
			return false;
		_speakerVoiceAnimation = variant >= 0 ? variant : 4;
		if (variant < 0 && _speakerPerson == 7 && storyStage() >= 200)
			_speakerVoiceAnimation++;
		if (variant < 0 && _speakerPerson == 12 && (_state[0x10a7] & 0x10))
			_speakerVoiceAnimation += 5;
		_speakerVoiceActive = true;
		_speakerVoiceFrame = choice & (variant >= 0 ? 0x18 : 0x38);
		_speakerVoiceRemaining = 8;
	} else if (_speakerVoiceRemaining < 8) {
		_speakerVoiceFrame++;
	}
	_speakerVoiceRemaining--;
	if (_speakerReportLowered) {
		_speakerReportLowered = false;
		_speakerAnimation = MAX(variant, 0);
		_speakerFrame = 0;
	}
	return true;
}

Common::Error DuneEngine::runGame() {
	debugC(1, kDebugGraphics, "the game view is up");

	_onWorld = false;
	_worldLongitude = 0;
	_menuKind = kMenuLines;
	_orderingTroop = 0;
	_scene = 0;

	bool loadedSave = false;
	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0) {
		const int slot = ConfMan.getInt("save_slot");
		const Common::Error error = loadGameState(slot);
		loadedSave = error.getCode() == Common::kNoError;

		if (!loadedSave)
			warning("Could not load Dune save slot %d: %s", slot, error.getDesc().c_str());
	}

	if (!loadedSave) {
		// 00B9/00BC silently consume both opening story records even when
		// the introduction is skipped. Otherwise the first stage change
		// marks an intro record instead of its own automatic paragraph.
		markAutomaticStoryRecord();
		markAutomaticStoryRecord();
		// Where the game begins is in its own memory and not something to settle
		// here.  DS:0x6 holds the place the player is at with 0x80 under it, and
		// DS:0x4 the kind of arrival under the place's own byte, which the data
		// starts at 0x0180 and 0x200A: the first place, under the tenth kind,
		// which is the state the last of the intro's palace scenes leaves behind.
		//
		// Only DS:0x114E, the place itself, is not in the data - it starts at
		// 0xFFFF, nowhere - so arriving works it out and writes it, which the
		// hourly round needs to know which troops are where the player is.

		// The very last thing the intro does before it hands the game over is
		// 0x2B4, one byte: `inc byte [0x115]`, which is the first place's record
		// at DS:0x100 and its ornithopters at +0x15.  The palace holds nothing at
		// all in the data - none of the seven kinds of thing - so without this the
		// machine Jessica tells Paul to go outside and take is never there, the
		// row that takes it is greyed out, and the game cannot be left on the
		// ground.  0x2B8 then puts 0x200A and 0x0180 in the two words below,
		// which the data already holds, so they are read out of it as before.
		enum { kPalaceOrnithopters = 0x115 };

		if (_state.size() > kPalaceOrnithopters)
			_state[kPalaceOrnithopters]++;

		// The data is shipped with the music off - 0x10 stands in DS:0x2943 - and
		// with DS:0x3810 asking for the songs as a CD player's.  The program's own
		// first instruction at 0x19 puts DS:0x3810 back to nought, so the place
		// chooses the song, and the sound's opening at 0xE820 takes the bit out of
		// DS:0x2943 once there is something to play on.  Neither was done here, so
		// with the music read out of where the game keeps it there was none.
		if (_state.size() > kHowMusicGoes)
			_state[kHowMusicGoes] = 0;

		if (_music && _state.size() > kMusicIsOff)
			_state[kMusicIsOff] &= (byte)~kMusicOffBit;

		const uint16 whose = _state.size() > 7 ? READ_LE_UINT16(_state.begin() + 6) : 0;
		const uint place = (whose >> 8) > 0 ? (whose >> 8) - 1 : 0;
		const byte kind = _state.size() > 5 ? (byte)(READ_LE_UINT16(_state.begin() + 4) & 0xff) : 0;
		const uint16 present = _state.size() > 0x13 ? READ_LE_UINT16(_state.begin() + 0x12) : 0;

		introScene(place, kind, present, -1);

		if (ConfMan.hasKey("dune_placebyte"))
			_place = (byte)ConfMan.getInt("dune_placebyte");
		if (ConfMan.hasKey("dune_salfile"))
			_salFile = ConfMan.get("dune_salfile");
		if (ConfMan.hasKey("dune_background"))
			_background = ConfMan.get("dune_background");

		// Something to say until the game knows what is actually being said.
		// A bank of a character is frame upon frame of their face talking; which
		// of them make a whole face, and what the face is drawn over, is not
		// worked out yet, so nobody is shown speaking until it is.
		_speaker = ConfMan.hasKey("dune_speaker") ? ConfMan.get("dune_speaker") : Common::String("LETO.HSQ");
		_nextMouthTime = 0;
		_lastClockTime = 0;
		_hourElapsedMs = 0;
		_speakerFrame = ConfMan.hasKey("dune_portrait") ? ConfMan.getInt("dune_portrait") : 0;
		// A stage to start the story at, for looking at what the game says
		// further on than its beginning.
		if (ConfMan.hasKey("dune_stage"))
			advanceStory((byte)ConfMan.getInt("dune_stage"));

		// The game opens on what there is to do in the room and not on anybody
		// speaking: the panel carries SEE DUNE MAP, the mixer and Duke Leto, and
		// it is the player who takes the talking up.
		_dialoguePart = 1;
		_dialogueLine = _dialogue.count() > 1 && !_dialogue.entry(1).empty()
						? _dialogue.entry(1).size() - 1 : 0;
		// Nobody is drawn large over the room until the player takes a person up
		// and they begin to speak; at the start Duke Leto is stood on a mark like
		// anybody else, which is what the walk at 0x30A0 puts him there for.
		peopleHere();
		_speakerPerson = -1;
		_speakerFrame = 0;
		_speech = "";
		drawScene();
		showRoomMenu();
		raiseOpeningPanelFace();
	}

	// A way of seeing what a bank holds: its sprites laid out in a grid, so
	// that the one a room or a view is built out of can be found by eye.
	if (ConfMan.hasKey("dune_sheet")) {
		const int first = ConfMan.hasKey("dune_sheetfrom") ? ConfMan.getInt("dune_sheetfrom") : 0;

		memset(_framebuffer1, 0, kScreenWidth * kScreenHeight);
		_graphics.setFrameBuffer(_framebuffer1);
		openSpriteSheet(ConfMan.get("dune_sheet").c_str());

		// One sprite on its own where it is asked for, so that a large one can
		// be seen whole; otherwise sixty of them in a grid.
		if (ConfMan.hasKey("dune_sprite")) {
			drawSprite(ConfMan.getInt("dune_sprite"), 0, 0, 0);
		} else {
			for (int i = 0; i != 60; i++)
				drawSprite(first + i, 4 + (i % 10) * 32, 4 + (i / 10) * 32, 0);
		}

		_system->copyRectToScreen(_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
		_system->updateScreen();

		if (ConfMan.hasKey("dune_dump"))
			dumpScreenPPM(ConfMan.get("dune_dump"));

		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_showworld")) {
		_onWorld = true;
		lookAtWorld((ConfMan.getInt("dune_showworld") + 80) * 65536 / Map::kEquatorWidth, -61);
		showWorld();
	}

	if (ConfMan.hasKey("dune_flyto"))
		flyTo(ConfMan.getInt("dune_flyto"));

	// The hour the game stands at, so that the sky's colours through the day
	// and the sun, the moon and the day on the panel can be looked at.
	if (ConfMan.hasKey("dune_hour") && _state.size() > 3) {
		WRITE_LE_UINT16(_state.begin() + 2, (uint16)ConfMan.getInt("dune_hour"));

		drawScene();
		showRoomMenu();
	}

	// The saved DOS random word can be placed at a known phase when checking an
	// hourly equipment hazard. Normal games retain and advance their own word.
	if (ConfMan.hasKey("dune_wormseed") && _state.size() > 0xd827)
		WRITE_LE_UINT16(_state.begin() + 0xd826,
						(uint16)ConfMan.getInt("dune_wormseed"));

	// A room of the place the player is at, by its number in the place's own
	// table, so that every room can be looked at without walking to it.
	if (ConfMan.hasKey("dune_room")) {
		const byte kindOfPlace = _state.size() > 8 ? _state[8] : 0;
		const byte want = (byte)ConfMan.getInt("dune_room");
		const byte *record = roomRecord(kindOfPlace, want);

		if (record && record[0]) {
			_state[0xb] = want;
			WRITE_LE_UINT16(_state.begin() + 4, (uint16)((kindOfPlace << 8) | want));
			_place = (byte)(record[0] - 1);
			debugC(1, kDebugScript, "room %d of kind 0x%02x looks like 0x%02x",
				   want, kindOfPlace, _place);
		}
	}

	if (ConfMan.hasKey("dune_room")) {
		drawScene();
		showRoomMenu();
	}

	if (ConfMan.hasKey("dune_waiting"))
		showWaiting();

	if (ConfMan.hasKey("dune_orders")) {
		// A way of seeing the orders while nothing yet brings troops to a
		// place: meet as many of them as asked for, and put them here.  A
		// place holds the first of its troops at +9 and each troop the next at
		// +1, which is the chain the game walks, so they are strung onto it.
		const int meet = ConfMan.getInt("dune_orders") - 1;
		const uint16 here = _state.size() > 0x114f ? READ_LE_UINT16(_state.begin() + 0x114e) : 0;

		for (int i = 0; i < meet && here + 0xa <= _state.size(); i++) {
			const uint32 at = 0x8aa + i * 27;
			_state[at + 3] &= 0x7f;
			// A troop already living here is already in the linked list.
			// Inserting it at its own head creates a cycle in peopleHere().
			if (READ_LE_UINT16(_state.begin() + at + 4) == here)
				continue;
			WRITE_LE_UINT16(_state.begin() + at + 4, here);

			// Onto the head of the place's chain, so the newest is first.
			_state[at + 1] = _state[here + 9];
			_state[here + 9] = (byte)(i + 1);
		}

		if (meet > 0)
			_map.readTroops(_state.begin(), _state.size());

		showTroopsHere();
	}

	// The first conditions of CONDIT.HSQ and whether they hold, for settling
	// which of them a line of the dialogue is really asking about.
	if (ConfMan.hasKey("dune_conditions")) {
		for (int i = 0; i != ConfMan.getInt("dune_conditions"); i++)
			debug("condition %3d: %-52s %s", i, _condit.describe(i).c_str(),
				  conditionHolds(i) ? "HOLDS" : "-");
	}

	// A compact reading of the place records for checking clue destinations.
	// Hidden marks deliberately have no cockpit icon, so their index, class and
	// coordinates otherwise cannot be compared with the course in a recording.
	if (ConfMan.hasKey("dune_places")) {
		for (uint i = 0; i != _map.places().size(); i++) {
			const Place &place = _map.places()[i];
			debug("place %2u: %-16s type %2u kind %02x, %s, world %3u,%3d",
				  i, nameOfPlace(place).c_str(), (uint)place.type, (uint)place.kind,
				  place.isKnown() ? "known " : "hidden",
				  (uint)((uint32)place.longitude * Map::kEquatorWidth / 65536u),
				  (int)Map::kEquator + place.latitude);
		}
	}

	if (ConfMan.hasKey("dune_dialogue")) {
		const uint person = (uint)ConfMan.getInt("dune_dialogue");
		for (uint part = person * 8; part != person * 8 + 8 && part < _dialogue.count(); part++) {
			const Common::Array<Dialogue::Record> &records = _dialogue.entry(part);
			for (uint line = 0; line != records.size(); line++) {
				const Dialogue::Record &record = records[line];
				debug("dialogue part %u record %u: line %u flags %02x condition %u (%s): %s",
					  part, line, record.line, record.flags, record.condition,
					  conditionHolds(record.condition) ? "holds" : "-",
					  fillInLine(phraseOf(person, record.line)).c_str());
			}
		}
	}

	// A way of walking about the rooms of a place without a hand on the mouse:
	// each letter of the string is one of the four ways out.
	if (ConfMan.hasKey("dune_go")) {
		const Common::String ways = ConfMan.get("dune_go");

		for (uint i = 0; i != ways.size(); i++) {
			switch (ways[i]) {
			case 'n': goThroughEdge(Cursor::North); break;
			case 'e': goThroughEdge(Cursor::East);  break;
			case 's': goThroughEdge(Cursor::South); break;
			case 'w': goThroughEdge(Cursor::West);  break;
			default: break;
			}
		}
	}

	if (ConfMan.hasKey("dune_facegrid") && _state.size() > 0x8aa + 3 * 27) {
		const uint troop = 2;
		const uint32 record = 0x8aa + troop * 27;
		WRITE_LE_UINT16(_state.begin() + 0x4756, record);
		WRITE_LE_UINT16(_state.begin() + 0x4758, record);
		for (uint id = 1; id <= 54; id++) {
			_state[record] = (byte)id;
			_map.readTroops(_state.begin(), _state.size());
			_speech = Common::String::format("Fremen face %u", id);
			drawTroopStatus(troop);
			dumpScreenPPM(Common::String::format("%s-%02u.ppm",
				ConfMan.get("dune_facegrid").c_str(), id));
		}
		return Common::kNoError;
	}

	if (ConfMan.hasKey("dune_talkto"))
		talkTo(ConfMan.getInt("dune_talkto"));

	// Picks one of the lines the person offers, so that what a line does can
	// be seen without a hand on the mouse.
	// One row after another, so that a way through the menus can be taken
	// without a hand on the mouse: taking a person up and then choosing what
	// to say to them is two.
	if (ConfMan.hasKey("dune_choose")) {
		const Common::String rows = ConfMan.get("dune_choose");

		for (uint i = 0; i != rows.size(); i++) {
			if (rows[i] >= '0' && rows[i] <= '9')
				chooseFromMenu((uint)(rows[i] - '0'));
		}
	}

	// A way through the game, one step to a word, so that it can be walked and
	// looked at without a hand on the mouse:
	//
	//   n e s w   go through that way out of the room
	//   0 to 4    choose that row of the list the panel is showing
	//   x,y       press the pointer there
	//   .         put what is on the screen out as a dump of its own
	//
	if (ConfMan.hasKey("dune_play")) {
		const Common::String script = ConfMan.get("dune_play");
		uint step = 0;

		for (uint at = 0; at <= script.size(); at++) {
			if (at != script.size() && script[at] != ' ')
				continue;

			// The word that has just ended.
			uint from = at;
			while (from > 0 && script[from - 1] != ' ')
				from--;

			const Common::String word(script.c_str() + from, at - from);
			if (word.empty())
				continue;

			debugC(1, kDebugScript, "step %u: %s", step, word.c_str());

			if (word.hasPrefix("troop:")) {
				// Takes up a troop by its number rather than by where it
				// happens to sit in the contact list, whose order shifts as
				// troops change what they are doing.
				const uint which = (uint)atoi(word.c_str() + 6);

				debugC(1, kDebugScript, "taking up troop %u", which);
				if (which < _map.troops().size()) {
					showTroopContact(which, true);
					showTroopMenu();
				}
				continue;
			}

			if (word.hasPrefix("wait:")) {
				// Lets the world turn in the middle of a scripted run rather
				// than only at the end of one.  A troop sent across the map
				// takes days to arrive and days more to do anything when it
				// does, so a run which could only wait once could carry out
				// exactly one order.
				const int hours = atoi(word.c_str() + 5);

				debugC(1, kDebugScript, "waiting %d hours", hours);
				for (int i = 0; i < hours; i++) {
					// This diagnostic requests whole game hours, not fixed
					// minutes. Respect the current DOS countdown and sub-tick,
					// including the fresh counter and a preceding FD9 reset.
					const uint32 elapsed = timerTicksUntilHour(
						READ_LE_UINT16(_state.begin() + 0x46db)) * 5 - _hourElapsedMs;
					advanceClock(elapsed);
				}
				showPendingTroopNotice();
				continue;
			}

			if (word.hasPrefix("map:")) {
				// Exercise the same animation update as the interactive map,
				// without advancing the campaign clock or moving the pointer.
				const int frames = atoi(word.c_str() + 4);
				for (int i = 0; i < frames; i++)
					animateMap();
				continue;
			}

			if (word.hasPrefix("fly")) {
				// Carries a flight along as many steps as the word asks for, so
				// that a scripted walk can fly and then go on pressing.
				const int steps = atoi(word.c_str() + 4);

				for (int i = 0; i != steps && _inDesert && _flyingTo >= 0
						&& !_flightDiscoveryPrompt && !_state[0x11ca]; i++) {
					stepThroughDesert(headingWay());
					advanceClock(kFlightStepMs);
					if (_inDesert && _flyingTo >= 0 && !_flightDiscoveryPrompt && !_state[0x11ca])
						drawDesert();
				}
			} else if (word == "n" || word == "e" || word == "s" || word == "w") {
				// The four ways go through the same choice the arrow keys
				// make: over a map they scroll it, and only otherwise do they
				// walk Paul through a door.  Walking him unconditionally left
				// a scripted run unable to scroll a map at all, so a place on
				// the far side of the world could never be brought into the
				// window and pressed.
				static const Cursor kWays[4] = {
					Cursor::North, Cursor::East, Cursor::South, Cursor::West
				};
				const int way = word == "n" ? 0 : word == "e" ? 1
							  : word == "s" ? 2 : 3;

				if (_onWorld || _inCabin) {
					if (_menuKind == kMenuMoveTroop)
						scrollMoveMap(way);
					else
						scrollWorld(way);
				} else {
					goThroughEdge(kWays[way]);
				}
			} else if (word == ".") {
				if (ConfMan.hasKey("dune_dump"))
					dumpScreenPPM(Common::String::format("%s-step%02u.ppm",
														 ConfMan.get("dune_dump").c_str(), step));
			} else if (word.contains(',')) {
				const uint comma = word.find(',');
				pressAt(Common::Point(atoi(Common::String(word.c_str(), comma).c_str()),
									  atoi(word.c_str() + comma + 1)));
			} else if (word.hasPrefix("@")) {
				// A row chosen by what it says rather than by where it sits.
				// The lists are not always in the same order - who stands in a
				// room is kept in the order they were last spoken to - so a
				// scripted walk cannot count on a row's number.
				Common::String wanted(word.c_str() + 1);
				wanted.toUppercase();
				int found = -1;

				for (uint i = 0; i != _menu.size() && found < 0; i++) {
					Common::String row = _menu[i];
					row.toUppercase();
					if (row.contains(wanted))
						found = (int)i;
				}

				debugC(1, kDebugScript, "  %s is row %d", word.c_str(), found);

				// chooseFromMenu wants the row of the panel, not the entry of
				// the list, and the two part company as soon as the list is
				// long enough to turn pages.  Page to the entry first, then
				// press the panel row it has come to sit on.
				if (found >= 0) {
					if (menuTurnsPages()) {
						const uint page = (kMenuRows - 1);
						_menuFrom = ((uint)found / page) * page;
						chooseFromMenu((uint)found - _menuFrom);
					} else {
						chooseFromMenu((uint)found);
					}
				}
			} else {
				chooseFromMenu((uint)atoi(word.c_str()));
			}

			step++;
		}
	}

	// The speaker's own frames, one dump to each, for seeing that a mouth moves.
	if (ConfMan.hasKey("dune_frames") && _speakerPerson >= 0) {
		for (int frame = 0; frame != ConfMan.getInt("dune_frames"); frame++) {
			_graphics.setFrameBuffer(_framebuffer1);
			drawSpeakerFigure(_speakerPerson, frame);

			if (ConfMan.hasKey("dune_dump"))
				dumpScreenPPM(Common::String::format("%s-mouth%02d.ppm",
													 ConfMan.get("dune_dump").c_str(), frame));
		}
	}

	// Presses of the pointer, as pairs of numbers with anything between them,
	// so that a way through the game can be walked without a hand on the mouse.
	if (ConfMan.hasKey("dune_press")) {
		const Common::String points = ConfMan.get("dune_press");
		Common::Array<int> numbers;
		int value = -1;

		for (uint i = 0; i <= points.size(); i++) {
			if (i != points.size() && points[i] >= '0' && points[i] <= '9') {
				value = (value < 0 ? 0 : value * 10) + (points[i] - '0');
				continue;
			}

			if (value >= 0)
				numbers.push_back(value);

			value = -1;
		}

		for (uint i = 0; i + 1 < numbers.size(); i += 2) {
			debugC(1, kDebugScript, "pressing at %d,%d", numbers[i], numbers[i + 1]);
			pressAt(Common::Point(numbers[i], numbers[i + 1]));
		}
	}

	// A reference recording includes the time its player spends reading maps,
	// reports and menus.  A scripted press list intentionally removes those
	// pauses, but later story conditions and economic reports still need the
	// same elapsed world time.  This diagnostic key supplies that omitted time
	// in milliseconds through the ordinary clock; it never changes normal play.
	if (ConfMan.hasKey("dune_elapsed")) {
		const int elapsed = ConfMan.getInt("dune_elapsed");
		if (elapsed > 0) {
			advanceClock((uint32)elapsed);
			showPendingTroopNotice();
		}
	}

	// Goes round everybody the game knows, saying the first thing each of them
	// offers, over and over, to see how far the story can be driven by talking
	// alone and where it comes to a stop.
	if (ConfMan.hasKey("dune_autotalk")) {
		const int rounds = ConfMan.getInt("dune_autotalk");
		byte was = _state[0x2a];

		for (int round = 0; round != rounds && !shouldQuit(); round++) {
			for (uint figure = 0; figure != 68; figure += 2) {
				_menu.clear();
				_menuParts.clear();
				talkTo(figure);

				if (_menuKind == kMenuLines && !_menuParts.empty())
					chooseFromMenu(0);
			}

			debug("after round %d of talking the story stands at %d", round + 1, _state[0x2a]);

			if (_state[0x2a] == was)
				break;

			was = _state[0x2a];
		}

		return Common::kNoError;
	}

	// Carries a flight along by as many steps as asked for, putting each one
	// out as a dump of its own, so that the ground going by can be seen
	// without waiting on the clock.
	if (ConfMan.hasKey("dune_fly")) {
		const int steps = ConfMan.getInt("dune_fly");
		debugC(1, kDebugScript,
			   "accelerating flight by %d steps: desert %d, destination %d, discovery %d",
			   steps, _inDesert ? 1 : 0, _flyingTo, _flightDiscoveryPrompt ? 1 : 0);

		for (int i = 0; i != steps && _inDesert && _flyingTo >= 0
				&& !_flightDiscoveryPrompt && !_state[0x11ca]; i++) {
			stepThroughDesert(headingWay());
			// One accelerated position step stands for the same elapsed time
			// as it does in the live loop. Troops must mine, train and move
			// while Paul flies even though the test does not wait in real time.
			advanceClock(kFlightStepMs);
			if (_inDesert && _flyingTo >= 0 && !_flightDiscoveryPrompt && !_state[0x11ca])
				drawDesert();

			if (ConfMan.hasKey("dune_dump"))
				dumpScreenPPM(Common::String::format("%s-fly%02d.ppm",
													ConfMan.get("dune_dump").c_str(), i));
		}
	}

	if (ConfMan.hasKey("dune_dump")) {
		dumpScreenPPM(ConfMan.get("dune_dump"));
		return Common::kNoError;
	}

	Cursor shown = Cursor::None;
	CursorMan.showMouse(true);

	// Until the game's own logic is written this walks the scenes of the
	// palace, which is enough to see that they all draw.

	MapPanRepeat heldMapPan;
	// Shared with the blocking input polls, which may consume key releases.
	uint32 &nextDesertWalk = _nextDesertWalk;
	int &heldDesertKeyWay = _heldDesertKeyWay;
	auto carryHeldRoomExit = [this, &heldDesertKeyWay, &nextDesertWalk](int way) {
		if (_inDesert && _flyingTo < 0 && _menuKind == kMenuDesert) {
			heldDesertKeyWay = way;
			nextDesertWalk = _system->getMillis() + 1000 / 15;
		}
	};
	while (!shouldQuit()) {
		// 3FC5's ABD5 speech wait returns through 3FC8/3FCA before
		// D83E can enter another ordinary hour/report poll. The timer may
		// have left work pending while the resident's reminder was playing.
		// Resume the retained move first, rather than running that work in
		// the old room once audio becomes inactive. New input still reaches
		// the completion call below during the same poll.
		finishRoomDeparture();
		finishVehicleBoarding();
		finishDeferredRoomCompass();
		if (updateDefeat())
			continue;
		if (showingDefeat()) {
			tick(false); // Keep IRQ time, without dispatching campaign work.
			pollDefeatMenu();
			continue;
		}
		if (_menuKind == kMenuTroopNotice) {
			tick(false); // The timer interrupt continues; 1B0D does not.
			pollTroopNotice();
			continue;
		}
		// D838 runs the companion flash before1B0D and D9D2, once per
		// ordinary poll. Long stalls consume one phase, not missed phases.
		updateCompanionPanelBlink((int8)_state[0xceea] > 0
				? READ_LE_UINT16(_state.begin() + 0xce7a)
				: (uint16)(_system->getMillis() / 5));
		// The interrupt updates the hour counter before D83E enters 1B0D.
		// Inside 1B0D, 2B2A checks queued reports before the pending troop
		// hour. Both precede D9D2 callbacks and the DB4C input poll.
		tick(false);
		const bool firstVisionReady = storyStage() == 20;
		const bool noticeOpened = updateTroopNoticeDispatch();
		if (noticeOpened && firstVisionReady && storyStage() == 21)
			waitForFirstDesertVision(); // 2BC4: no ordinary held-input sample yet.
		// Warning and report transitions wait synchronously while DOS's timer
		// interrupt keeps running. Accrue those ticks before 1B0D's hour body.
		tick(false);
		if (_menuKind == kMenuTroopNotice) {
			pollTroopNotice(); // 2C60 enters its nested wait before 1B0D returns.
			continue;
		}
		// ABCC checked speech on entry to 1B0D. A local 2B2A report may
		// start a voice, but 1B0D does not test ABCC again before its hour.
		// Remote reports remain modal and keep their hour pending.
		const bool localNoticeVoiceStarted = noticeOpened &&
			_menuKind == kMenuTalk && _speakerPerson >= 0;
		const bool troopWorkEligible = _state[0xc2] < 7;
		const bool pendingHour = dispatchPendingHour(localNoticeVoiceStarted);
		// 1B8D/1BEC applies the hour's room and battle changes before D83E
		// can pass a click to the old menu. Retain the late retry below for
		// changes caused by input or deferred behind a native modal.
		if (pendingHour && troopWorkEligible)
			refreshBattleMenu();
		if (pollDesertExhaustion())
			continue; // 2E62 consumes a restored pending walk before old-view input.
		// The strategic-map 6B34 actor callback also runs in D9D2, before
		// D83E refreshes DS:0 and polls input. Resolve its current frame before
		// a click tests the animated troop bounds.
		updateGameMusic(); // D9D2 services ACE6 before the registered callbacks.
		updateRegisteredAnimations();
		refreshInputRandom(); // D84B/D84E follows every registered D9D2 callback.
		// DOS D83E calls 4F0C after D9D2/E3CC but before DB4C reads input.
		// On a poll where both a flight step and steering click are due, the
		// aircraft moves on its previous heading before the turn takes effect.
		pollFlightMovement();
		Common::Event event;
		bool mapPanPressed = false;
		bool pointerButtonEdge = false;
		bool leftButtonReleased = false;
		bool idlePointerSample = true;

		while (pollPointerEvent(event, idlePointerSample)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP ||
					event.type == Common::EVENT_RBUTTONDOWN || event.type == Common::EVENT_RBUTTONUP)
				pointerButtonEdge = true;
			if (event.type == Common::EVENT_LBUTTONUP) {
				leftButtonReleased = true;
				_mapPointerPendingButtons &= ~Common::EventManager::LBUTTON;
				heldMapPan.release((uint16)(_system->getMillis() / 5));
			}
			if (event.type == Common::EVENT_RBUTTONUP)
				_mapPointerPendingButtons &= ~Common::EventManager::RBUTTON;
			if (event.type == Common::EVENT_FOCUS_LOST || event.type == Common::EVENT_MAINMENU)
				heldMapPan.release((uint16)(_system->getMillis() / 5));
			// DB4C/D893: either edge of the left/right buttons is input;
			// pointer motion and the middle button leave the idle clock alone.
			if (event.type == Common::EVENT_KEYDOWN ||
					event.type == Common::EVENT_LBUTTONDOWN ||
					event.type == Common::EVENT_LBUTTONUP ||
					event.type == Common::EVENT_RBUTTONDOWN ||
					event.type == Common::EVENT_RBUTTONUP)
				resetTroopNoticeIdle();
			// Saving is worth nothing if there is no way to ask for it while
			// playing, so the menu key opens ScummVM's own.
			if (event.type == Common::EVENT_MAINMENU) {
				finishSurveyDrag(false);
				// DefaultEventManager has already opened and closed the menu.
				resetPointerButtons(); // Releases may have been consumed by the dialog.
				redraw();
				continue;
			}

			if (event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				quitGame();
				continue;
			}
			if (handleTroopNoticeEvent(event))
				continue;
			if (_mapPointerGlide.steps && event.type == Common::EVENT_LBUTTONDOWN) {
				_mapPointerPendingButtons |= Common::EventManager::LBUTTON;
				continue;
			}
			if (_mapPointerGlide.steps && event.type == Common::EVENT_RBUTTONDOWN) {
				_mapPointerPendingButtons |= Common::EventManager::RBUTTON;
				continue;
			}
			if (handleSurveyDrag(event))
				continue;

			// 45DE changes only the caption strip. Keep the terrain, retained
			// backdrop and independently scheduled marker phase undisturbed.
			// 45DE stops typing on the first actual map hover, not at FF.
			if (event.type == Common::EVENT_MOUSEMOVE &&
					(_inCabin || _menuKind == kMenuWormMap)) {
				hoverCabinLabel(event.mouse);
				continue;
			}

			if (event.type == Common::EVENT_LBUTTONDOWN) {
				const int pan = repeatCompassDirection(event.mouse);
				if (pan >= 0) {
					heldMapPan.press(pan, (uint16)(_system->getMillis() / 5));
					mapPanPressed = true;
				}
				// D918 enables held repeats only for controls with flag 4000h.
				// The on-foot compass uses 0080h, so each press takes one step.
				pressAt(event.mouse);
				continue;
			}
			if (event.type == Common::EVENT_RBUTTONDOWN) {
				// D8B1 gives a held left button or a left edge priority.
				if (!_pointerButtons.down() && !leftButtonReleased)
					rightClickMap(event.mouse);
				continue;
			}

			if (event.type != Common::EVENT_KEYDOWN)
				continue;
			const int mapDirection = mapPointerKey(event.kbd.keycode);
			if (mapDirection >= 0 && !(event.kbd.flags & Common::KBD_SHIFT) && canMoveMapPointer()) {
				if (!event.kbdRepeat)
					_mapPointerKeys |= 1 << mapDirection;
				continue;
			}

			// Arrow-key shortcuts repeat raw B5CF steps on the engine clock.
			// Ignore host key repeats once a held direction has been recorded.
			// The mixer and scripted Continue menus retain underlying view flags.
			// Match pressAt's ownership before any room/map/globe shortcut.
			const bool scriptedEvent = _state[0x4774] &&
				(_menuKind == kMenuProspectorBriefing || _menuKind == kMenuPalaceGathering);
			// The pending paragraph/departure owns navigation just as it owns
			// pointer input. Map entry clears its speech, and direct desert
			// steps bypass goThroughEdge's pending-action checks.
			if ((_localRoomRequestPending || _pendingBoardingPlace >= 0 || _pendingRoomExit >= 0 || _pendingRoomCompass >= 0) &&
					(event.kbd.keycode == Common::KEYCODE_UP ||
					 event.kbd.keycode == Common::KEYCODE_RIGHT ||
					 event.kbd.keycode == Common::KEYCODE_DOWN ||
					 event.kbd.keycode == Common::KEYCODE_LEFT ||
					 event.kbd.keycode == Common::KEYCODE_m))
				continue;
			if ((_onMixer || scriptedEvent) && (event.kbd.keycode == Common::KEYCODE_UP ||
					event.kbd.keycode == Common::KEYCODE_RIGHT ||
					event.kbd.keycode == Common::KEYCODE_DOWN ||
					event.kbd.keycode == Common::KEYCODE_LEFT))
				continue;
			if (_inDesert && _flyingTo < 0 && !_onWorld && !_onGlobe &&
					!_inCabin && _menuKind == kMenuDesert) {
				int way = -1;
				switch (event.kbd.keycode) {
				case Common::KEYCODE_UP: way = 0; break;
				case Common::KEYCODE_RIGHT: way = 1; break;
				case Common::KEYCODE_DOWN: way = 2; break;
				case Common::KEYCODE_LEFT: way = 3; break;
				default: break;
				}
				if (way >= 0) {
					// Host repeats cannot start another walk after a dialog clears the hold.
					if (!event.kbdRepeat && heldDesertKeyWay != way) {
						heldDesertKeyWay = way;
						stepThroughDesert(way);
						nextDesertWalk = _system->getMillis() + 1000 / 15;
					}
					continue;
				}
			}

			// Arrow keys must not walk the desert behind the modal target map.
			// Escape has the same meaning as its visible Cancel row.
			if (_menuKind == kMenuWormMap) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					closeWormMap();
				continue;
			}
			if (_menuKind == kMenuBook) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					closeBook();
				else if (event.kbd.keycode == Common::KEYCODE_LEFT)
					turnBookPage(-1);
				else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
					turnBookPage(1);
				continue;
			}

			// Preserve Shift+arrow as a direct compass shortcut, as on
			// the flat map. Plain directions use the DOS cursor above.
			if (_onGlobe && mapDirection >= 0) {
				if (_menuKind == kMenuGlobe && (event.kbd.flags & Common::KBD_SHIFT)) {
					switch (event.kbd.keycode) {
					case Common::KEYCODE_RIGHT: workGlobeCompass(1); break;
					case Common::KEYCODE_LEFT:  workGlobeCompass(3); break;
					case Common::KEYCODE_UP:    workGlobeCompass(0); break;
					case Common::KEYCODE_DOWN:  workGlobeCompass(2); break;
					default: break;
					}
				}
				continue;
			}

			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				// Like EVENT_MAINMENU, this dialog ends the current pan gesture.
				heldMapPan.release((uint16)(_system->getMillis() / 5));
				openMainMenuDialog();
				resetPointerButtons();
				redraw();
				break;
			case Common::KEYCODE_RIGHT:
				if (_onWorld || _inCabin) {
					if (_menuKind == kMenuMoveTroop)
						scrollMoveMap(1);
					else
						scrollWorld(1);
				} else {
					goThroughEdge(Cursor::East);
					carryHeldRoomExit(1);
				}
				break;
			case Common::KEYCODE_LEFT:
				// The other three ways read the map first and leave the desert
				// to goThroughEdge, which steps it just the same.  This one
				// used to take the desert before the map, so a map put up over
				// a flight could be scrolled every way but west.
				if (_onWorld || _inCabin) {
					if (_menuKind == kMenuMoveTroop)
						scrollMoveMap(3);
					else
						scrollWorld(3);
				} else {
					goThroughEdge(Cursor::West);
					carryHeldRoomExit(3);
				}
				break;
			case Common::KEYCODE_UP:
				if (_onWorld || _inCabin) {
					if (_menuKind == kMenuMoveTroop)
						scrollMoveMap(0);
					else
						scrollWorld(0);
				} else {
					goThroughEdge(Cursor::North);
					carryHeldRoomExit(0);
				}
				break;
			case Common::KEYCODE_DOWN:
				if (_onWorld || _inCabin) {
					if (_menuKind == kMenuMoveTroop)
						scrollMoveMap(2);
					else
						scrollWorld(2);
				} else {
					goThroughEdge(Cursor::South);
					carryHeldRoomExit(2);
				}
				break;
			case Common::KEYCODE_m:
				// A held toggle key must not repeatedly open and close the map.
				if (event.kbdRepeat)
					break;
				// This optional shortcut must use the same 5A7E/5ADF mode,
				// menu and actor lifecycle as the visible map commands. A
				// direct _onWorld flip left DS:46EB at its room value and even
				// changed the retained map flag behind the results globe.
				if (_onWorld && !_onGlobe && !_inCabin && _menuKind == kMenuMap) {
					for (uint row = 0; row < _menuParts.size(); row++) {
						if (_menuParts[row] == kMapExit) {
							chooseFromMenu(row);
							break;
						}
					}
				} else if (!_onWorld && !_onGlobe && !_inCabin && !_onMixer &&
						(_menuKind == kMenuRoom || _menuKind == kMenuDesert)) {
					openWorldMap();
				}
				break;
			default:
				// The prototype troop/message lists are explicit dune_orders and
				// dune_waiting startup diagnostics, not gameplay key bindings. Do
				// not let O/W replace a live contact, route editor or globe menu.
				break;
			}
		}
		updateMapKeyboardPointer();

		// D8DA checks the held button's identity and unsigned fifty-tick
		// delay after input has sampled releases. Hover alone never pans MAP2.
		if (_mapPointerGlide.steps) {
			heldMapPan.release((uint16)(_system->getMillis() / 5));
		} else if (!mapPanPressed && _pointerButtons.down()) {
			const int way = repeatCompassDirection(_system->getEventManager()->getMousePos());
			if (heldMapPan.poll(way, (uint16)(_system->getMillis() / 5))) {
				if (_onGlobe)
					workGlobeCompass(way);
				else
					scrollWorld(way);
			}
		}

		// Repeat the held keyboard shortcut independently of host key repeats.
		if (_inDesert && _flyingTo < 0 && !_onWorld && !_onGlobe &&
				!_inCabin && !_onMixer && _menuKind == kMenuDesert &&
				!_localRoomRequestPending && _pendingBoardingPlace < 0 && _pendingRoomExit < 0 && _pendingRoomCompass < 0) {
			const int way = heldDesertKeyWay;
			if (way >= 0) {
				const uint32 now = _system->getMillis();
				if (!nextDesertWalk)
					nextDesertWalk = now + 1000 / 15;
				else if ((int32)(now - nextDesertWalk) >= 0) {
					nextDesertWalk = now + 1000 / 15;
					stepThroughDesert(way);
				}
			} else {
				nextDesertWalk = 0;
			}
		} else {
			heldDesertKeyWay = -1;
			nextDesertWalk = 0;
		}

		finishRoomDeparture();
		finishVehicleBoarding();
		updateFlightSteering();
		if (_menuKind == kMenuRoom)
			checkSpiceShipment();

		if (_state[0x46d9] && !_state[0x47a9])
			continue; // D841: defeat takes priority over newly queued reports.
		// Retry battle-menu changes deferred by a modal on the next poll, not
		// only at the next game hour; refreshBattleMenu guards active reports.
		refreshBattleMenu();

		const Common::Point pointer = _system->getEventManager()->getMousePos();

		const Cursor wanted = cursorForPosition(pointer);
		if (wanted != shown) {
			setCursor(wanted);
			shown = wanted;
		}

		updateMapPointer(pointer);

		// The row of the list under the pointer is written the other way
		// about, dark on light, and the panel is drawn again as it moves from
		// one row to another.
		updateMenuHover(pointer, pointerButtonEdge);

		_system->updateScreen();
		_system->delayMillis(_onGlobe ? 5 : 10); // Allow B9AE's one-tick callback to run at 200Hz.
	}

	return Common::kNoError;
}

} // End of namespace Dune
