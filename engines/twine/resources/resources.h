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

#ifndef TWINE_RESOURCES_RESOURCES_H
#define TWINE_RESOURCES_RESOURCES_H

#include "common/hashmap.h"
#include "common/scummsys.h"
#include "twine/parser/anim3ds.h"
#include "twine/parser/body.h"
#include "twine/parser/holomap.h"
#include "twine/parser/sprite.h"
#include "twine/parser/text.h"
#include "twine/resources/hqr.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/scene.h"

namespace TwinE {

/** RESS.HQR FILE */
#define RESSHQR_MAINPAL 0
#define RESSHQR_LBAFONT 1
#define RESSHQR_BLANK 2
#define RESSHQR_SPRITEBOXDATA 3
#define RESSHQR_SPRITESHADOW 4
#define RESSHQR_HOLOPAL 5           // lba1
#define RESSHQR_HOLOSURFACE 6       // lba1
#define RESSHQR_HOLOIMG 7           // lba1
#define RESSHQR_HOLOARROWINFO 8     // lba1
#define RESSHQR_HOLOTWINMDL 9       // lba1
#define RESSHQR_HOLOARROWMDL 10     // lba1
#define RESSHQR_HOLOTWINARROWMDL 11 // lba1

#define RESSHQR_BLACKPAL 9   // lba2
#define RESSHQR_ECLAIRPAL 10 // lba2
#define RESSHQR_ARROWBIN 12  // lba2
#define SAMPLE_RAIN 13

#define RESSHQR_GAMEOVERMDL 21

#define RESSHQR_ALARMREDPAL 22
#define RESSHQR_FLAINFO 23
#define RESSHQR_DARKPAL 24

#define RESSHQR_HOLOPOINTMDL 29
#define RESSHQR_HOLOPOINTANIM 30

#define RESSHQR_PLASMAEFFECT 51

#define FLA_DRAGON3 "dragon3"
#define FLA_INTROD "introd"
#define FLA_THEEND "the_end"
#define FLA_BATEAU "bateau"

#define ACF_INTRO "INTRO"

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

#define SPRITEHQR_MAGICBALL_YELLOW 1
#define SPRITEHQR_MAGICBALL_FIRE 13
#define SPRITEHQR_MAGICBALL_GREEN 42
#define SPRITEHQR_MAGICBALL_RED 43
#define SPRITEHQR_MAGICBALL_YELLOW_TRANS 44
#define SPRITEHQR_EXPLOSION_FIRST_FRAME 97 // 7 frames
#define SPRITEHQR_FENCE_1 18
#define SPRITEHQR_FENCE_2 19
#define SPRITEHQR_FENCE_3 22
#define SPRITEHQR_FENCE_4 23
#define SPRITEHQR_FENCE_METAL 35
#define SPRITEHQR_FENCE_METAL_2 54
#define SPRITEHQR_FENCE_METAL_3 83
#define SPRITEHQR_MUSHROOM 92
#define SPRITEHQR_DOOR_WODDEN_1 31
#define SPRITEHQR_DOOR_WODDEN_2 32
#define SPRITEHQR_DOOR_PRISON_WODDEN 37
#define SPRITEHQR_DOOR_PADLOCK 58
#define SPRITEHQR_DOOR_BRICKED_UP 76
#define SPRITEHQR_DOOR_1 104
#define SPRITEHQR_DOOR_2 107
#define SPRITEHQR_DOOR_3 24
#define SPRITEHQR_DOOR_4 11
#define SPRITEHQR_DOOR_5 12
#define SPRITEHQR_DOOR_PRISON_GRID 15
#define SPRITEHQR_DOOR_PRISON_HARMED 16
#define SPRITEHQR_DOOR_PRISON_WITH_F_LETTER 17
#define SPRITEHQR_MAGICBALL_GREEN_TRANS 109
#define SPRITEHQR_MAGICBALL_RED_TRANS 110

#define SPRITEHQR_DIAG_BUBBLE_RIGHT 90
#define SPRITEHQR_DIAG_BUBBLE_LEFT 91

/** Total number of animations allowed in the game */
#define NUM_ANIMS 2083 // 600 for lba1

/** Total number of samples allowed in the game */
#define NUM_SAMPLES 895 // 243 for lba1

class TwinEEngine;

class Resources {
private:
	TwinEEngine *_engine;

	void preloadInventoryItems();
	/** Init standard menu and in-game palette */
	void initPalettes();
	/** Preload all sprites */
	void preloadSprites();

	/** Preload all animations */
	void preloadAnimations();
	void preloadAnim3DS();
	void preloadSamples();
	void loadMovieInfo();

	using MovieInfoMap = Common::HashMap<Common::String, Common::Array<int32> >;
	MovieInfoMap _movieInfo;

	TrajectoryData _trajectories;

	TextData _textData;
	Anim3DSData _anim3DSData;
public:
	Resources(TwinEEngine *engine) : _engine(engine) {}
	~Resources();

	/**
	 * For lba1 this is returning the gif images that are used as a placeholder for the fla movies
	 * For lba2 this is the list of videos that are mapped by their entry index
	 */
	const Common::Array<int32> &getMovieInfo(const Common::String &name) const;

	/** Table with all loaded samples */
	BodyData _inventoryTable[NUM_INVENTORY_ITEMS];

	/** Table with all loaded sprites */
	uint8 *_spriteTable[NUM_SPRITES]{nullptr};
	/** Table with all loaded sprite sizes */
	uint32 _spriteSizeTable[NUM_SPRITES]{0};
	SpriteData _spriteData[NUM_SPRITES];

	AnimData _animData[NUM_ANIMS];

	/** Table with all loaded samples */
	uint8 *_samplesTable[NUM_SAMPLES]{nullptr};
	/** Table with all loaded samples sizes */
	uint32 _samplesSizeTable[NUM_SAMPLES]{0};

	/** Font buffer pointer */
	int32 _fontBufSize = 0;
	uint8 *_fontPtr = nullptr;
	uint8 *_sjisFontPtr = nullptr;

	SpriteData _spriteShadowPtr;
	SpriteBoundingBoxData _spriteBoundingBox;

	BodyData _holomapPointModelPtr;
	BodyData _holomapTwinsenModelPtr;
	BodyData _holomapTwinsenArrowPtr;
	BodyData _holomapArrowPtr;

	/** Initialize resource pointers */
	void initResources();

	const Trajectory *giveTrajPtr(int index) const;
	const TrajectoryData &getTrajectories() const {
		return _trajectories;
	}
	void loadEntityData(EntityData &entityData, int32 &index);

	const TextEntry *getText(TextBankId textBankId, TextId index) const;
	const T_ANIM_3DS *getAnim(int index) const;

	int findSmkMovieIndex(const char *name) const;

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
	 * Bricks are images or sprites in a special format.
	 */
	static constexpr const char *HQR_LBA_BRK_FILE = "lba_brk.hqr";
	// scenes (active area content (actors, scripts, etc.))
	static constexpr const char *HQR_SCENE_FILE = "scene.hqr";
	// full screen images (lba2)
	static constexpr const char *HQR_SCREEN_FILE = "screen.hqr";
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
	static constexpr const char *HQR_ANIM3DS_FILE = "anim3ds.hqr";
	// inventory objects
	static constexpr const char *HQR_INVOBJ_FILE = "invobj.hqr";
	// lba2 holomap
	static constexpr const char *HQR_HOLOMAP_FILE = "holomap.hqr";

	/**
	 * @brief Floppy version of the game uses gifs for replacing the videos
	 */
	static constexpr const char *HQR_FLAGIF_FILE = "fla_gif.hqr";
	static constexpr const char *HQR_FLASAMP_FILE = "flasamp.hqr";
	static constexpr const char *HQR_MIDI_MI_DOS_FILE = "midi_mi.hqr";
	static constexpr const char *HQR_MIDI_MI_WIN_FILE = "midi_mi_win.hqr";

	static constexpr const char *HQR_VIDEO_FILE = "video.hqr"; // lba2 - smk files

	TwineImage adelineLogo() const {
		if (_engine->isLBA1()) {
			return TwineImage(Resources::HQR_RESS_FILE, 27, 28);
		}
		return TwineImage(Resources::HQR_SCREEN_FILE, 0, 1);
	}

	TwineImage lbaLogo() const {
		if (_engine->isLBA1()) {
			return TwineImage(Resources::HQR_RESS_FILE, 49, 50);
		}
		return TwineImage(Resources::HQR_SCREEN_FILE, 60, 61);
	}

	TwineImage eaLogo() const {
		if (_engine->isLBA1()) {
			return TwineImage(Resources::HQR_RESS_FILE, 52, 53);
		}
		return TwineImage(Resources::HQR_SCREEN_FILE, 74, 75);
	}

	TwineImage activisionLogo() const {
		assert(_engine->isLBA2());
		return TwineImage(Resources::HQR_SCREEN_FILE, 72, 73);
	}

	TwineImage virginLogo() const {
		assert(_engine->isLBA2());
		return TwineImage(Resources::HQR_SCREEN_FILE, 76, 77);
	}

	TwineImage relentLogo() const {
		assert(_engine->isLBA1());
		return TwineImage(Resources::HQR_RESS_FILE, 12, 13);
	}

	TwineImage menuBackground() const {
		if (_engine->isLBA1()) {
			return TwineImage(Resources::HQR_RESS_FILE, 14, -1);
		}
		return TwineImage(Resources::HQR_SCREEN_FILE, 4, 5);
	}
};

} // namespace TwinE

#endif
