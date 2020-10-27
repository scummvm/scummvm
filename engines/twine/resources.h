/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TWINE_RESOURCES_H
#define TWINE_RESOURCES_H

#include "common/scummsys.h"
#include "twine/gamestate.h"
#include "twine/hqrdepack.h"

namespace TwinE {

/** RESS.HQR FILE */
#define RESSHQR_MAINPAL 0
#define RESSHQR_LBAFONT 1
#define RESSHQR_BLANK 2
#define RESSHQR_SPRITEBOXDATA 3
#define RESSHQR_SPRITESHADOW 4
#define RESSHQR_HOLOPAL 5
#define RESSHQR_HOLOSURFACE 6
#define RESSHQR_HOLOIMG 7
#define RESSHQR_HOLOARROWINFO 8
#define RESSHQR_HOLOTWINMDL 9
#define RESSHQR_HOLOARROWMDL 10
#define RESSHQR_HOLOTWINARROWMDL 11
#define RESSHQR_RELLENTIMG 12
#define RESSHQR_RELLENTPAL 13
#define RESSHQR_MENUIMG 14
#define RESSHQR_INTROSCREEN1IMG 15
#define RESSHQR_INTROSCREEN1PAL 16
#define RESSHQR_INTROSCREEN2IMG 17
#define RESSHQR_INTROSCREEN2PAL 18
#define RESSHQR_INTROSCREEN3IMG 19
#define RESSHQR_INTROSCREEN3PAL 20
#define RESSHQR_GAMEOVERMDL 21

#define RESSHQR_ALARMREDPAL 22
#define RESSHQR_DARKPAL 24

#define RESSHQR_ADELINEIMG 27
#define RESSHQR_ADELINEPAL 28

#define RESSHQR_HOLOPOINTMDL 29
#define RESSHQR_HOLOPOINTANIM 30

#define RESSHQR_LBAIMG 49
#define RESSHQR_LBAPAL 50
#define RESSHQR_PLASMAEFFECT 51
#define RESSHQR_EAIMG 52
#define RESSHQR_EAPAL 53

#define FLA_DRAGON3 "dragon3"
#define FLA_INTROD "introd"
#define FLA_THEEND "the_end"

#define FILE3DHQR_HERONORMAL 0
#define FILE3DHQR_HEROATHLETIC 1
#define FILE3DHQR_HEROAGGRESSIVE 2
#define FILE3DHQR_HERODISCRETE 3
#define FILE3DHQR_HEROPROTOPACK 4

/** Behaviour menu sprite values */
#define SPRITEHQR_KASHES 3
#define SPRITEHQR_LIFEPOINTS 4
#define SPRITEHQR_MAGICPOINTS 5
#define SPRITEHQR_KEY 6
#define SPRITEHQR_CLOVERLEAF 7
#define SPRITEHQR_CLOVERLEAFBOX 41

#define SPRITEHQR_MAGICBALL_GREEN 42
#define SPRITEHQR_MAGICBALL_RED 43
#define SPRITEHQR_MAGICBALL_YELLOW_TRANS 44
#define SPRITEHQR_MAGICBALL_GREEN_TRANS 109
#define SPRITEHQR_MAGICBALL_RED_TRANS 110

#define SPRITEHQR_DIAG_BUBBLE_RIGHT 90
#define SPRITEHQR_DIAG_BUBBLE_LEFT 91

class TwinEEngine;
class Resources {
private:
	TwinEEngine *_engine;

	void preloadInventoryItems();
	/** Init palettes */
	void initPalettes();
	/** Preload all sprites */
	void preloadSprites();
	/** Preload all animations */
	void preloadAnimations();
	void preloadSamples();

public:
	Resources(TwinEEngine *engine) : _engine(engine) {}
	/** Table with all loaded samples */
	uint8 *inventoryTable[NUM_INVENTORY_ITEMS] {nullptr};
	/** Table with all loaded samples sizes */
	uint32 inventorySizeTable[NUM_INVENTORY_ITEMS] {0};

	/** Initialize resource pointers */
	void initResources();

	// main palette
	static constexpr const char *HQR_RESS_FILE = "ress.hqr";
	// dialoges
	static constexpr const char *HQR_TEXT_FILE = "text.hqr";
	// samples
	static constexpr const char *HQR_SAMPLES_FILE = "samples.hqr";
	/**
	 * This file contains isometric grids that are used to display area backgrounds and define 3D shape of the surface.
	 * Each of the entries is associated with the entry of lba_bll.hqr with the same index. lba_bll entries define block
	 * sets for use with the grids. Each grid may use only one set of blocks (one entry of lba_bll.hqr).
	 */
	static constexpr const char *HQR_LBA_GRI_FILE = "lba_gri.hqr";
	// isometric libraries for use in grids.
	static constexpr const char *HQR_LBA_BLL_FILE = "lba_bll.hqr";
	/**
	 * isometric bricks, which are some kind of tiles, that are used for building the terrains in LBA 1 isometric scenes.
	 * One brick is the tiniest piece of a grid, which has 64 x 64 x 25 cells. Bricks cannot be used directly on a grid,
	 * but instead they are grouped into blocks by block libraries, which are then referenced by grids
	 */
	static constexpr const char *HQR_LBA_BRK_FILE = "lba_brk.hqr";
	// scenes (active area content (actors, scripts, etc.))
	static constexpr const char *HQR_SCENE_FILE = "scene.hqr";
	// sprites
	static constexpr const char *HQR_SPRITES_FILE = "sprites.hqr";
	/**
	 * model/animation entities
	 * contains data associating 3D models (Body.hqr) with animations (Anim.hqr) for the game characters.
	 */
	static constexpr const char *HQR_FILE3D_FILE = "file3d.hqr";
	// 3d model data
	static constexpr const char *HQR_BODY_FILE = "body.hqr";
	// animations
	static constexpr const char *HQR_ANIM_FILE = "anim.hqr";
	// inventory objects
	static constexpr const char *HQR_INVOBJ_FILE = "invobj.hqr";

	static constexpr const char *HQR_FLASAMP_FILE = "flasamp.hqr";
	static constexpr const char *HQR_MIDI_MI_DOS_FILE = "midi_mi.hqr";
	static constexpr const char *HQR_MIDI_MI_WIN_FILE = "midi_mi_win.hqr";
	static constexpr const char *HQR_MIDI_MI_WIN_MP3_FILE = "midi_mi_win_mp3.hqr";
	static constexpr const char *HQR_MIDI_MI_WIN_OGG_FILE = "midi_mi_win_ogg.hqr";
};

} // namespace TwinE

#endif
