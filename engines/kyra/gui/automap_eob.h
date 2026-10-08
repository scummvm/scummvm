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

#ifndef KYRA_GUI_AUTOMAP_EOB_H
#define KYRA_GUI_AUTOMAP_EOB_H

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "common/array.h"
#include "common/scummsys.h"

namespace Common {
class OutSaveFile;
class SeekableReadStreamEndianWrapper;
} // End of namespace Common

namespace Graphics {
class Font;
struct Surface;
} // End of namespace Graphics

class OSystem;

namespace Kyra {

struct LevelBlockProperty;
class EoBCoreEngine; // TODO: REMOVE

class Automap_EoB {
public:
	Automap_EoB(OSystem *system, LevelBlockProperty **blockData, const uint8 *wllFlags, const uint8 *specialWallTypes, const int8 *wllShapeMap, /* const uint8 *wllVmpMap,*/ int gameID, int lang, bool featureEnabled);
	~Automap_EoB();

	void markVisited(uint16 block);
	void markSeen(uint16 block, int8 dir);

	void draw(int level, uint16 partyBlock, int8 partyDirection);
	void drawPartyIcon(uint16 partyBlock, int8 partyDirection);

private:
	// Geometry shared by drawing and click hit-testing.
	struct AutomapLayout {
		int cell;
		int offX, offY;                 // top-left of the 32x32 grid (mouse hit-test)
		int frame;                      // stone frame thickness
		int mapX, mapY, mapW, mapH;     // parchment region (map + footer strip)
		int sideX, sideY, sideW, sideH; // stone side panel (plaque, legend, keys)
		int plX, plY, plW, plH;         // level/coords plaque inside the side panel
		int footY;                      // top of the selected-note footer strip
	};

	enum {
		kNumLegendStrings = 15,
		kNumControlStrings = 6
	};

	struct TranslateableStrings {
		const char *const legendStrings[kNumLegendStrings];
		const char *const controlStrings[kNumControlStrings];
		const char *const levelNames[2][16];
		const char *const specialMarkerStrings[2][3];
	};

	struct SpecialMarkers {
		int8 level;
		uint16 block;
		int8 iconColor;
	};

	enum IconID : int {
		kIconNone			=	-1,
		kIconPartyNorth		=	0,
		kIconPartyEast		=	1,
		kIconPartySouth		=	2,
		kIconPartyWest		=	3,
		kIconTeleporter		=	4,
		kIconStairsUp		=	5,
		kIconStairsDown		=	6,
		kIconPit			=	7,
		kIconPlate			=	8,
		kIconIllusionWall	=	10,
		kIconWallOfForce	=	12,
		kIconBigObject		=	15,
		kIconDoorButton		=	17,
		kIconSmallObject	=	18,
		kIconNicheNS		=	20,
		kIconNicheEW		=	21,
		kIconPortalNS		=	22,
		kIconPortalEW		=	23,
		kIconDoorNS			=	25,
		kIconDoorEW			=	26,
		kIconSpecial		=	30,
		kIconIDMax			=	35
	};

	AutomapLayout createLayout(int width, int height) const;
	void recalcScaling(int width, int height);
	void createColorTable();
	void createIcons(bool lowResSurface);
	void releaseIcons();
	void drawBackground(int width, int height);
	void drawLegend(int level, uint flags);

	enum IconAlignment : int {
		kAlignTopLeft = -1,
		kAlignTopCenter = 0,
		kAlignRightCenter,
		kAlignBottomCenter,
		kAlignLeftCenter,
		kAlignCenter
	};

	template<typename T> void drawIconImpl(Graphics::Surface &surf, int iconSet, int iconID, int cellX, int cellY, int boxFitWidth, IconAlignment alignment, int extraX, int extraY, int overrideColor);
	typedef void (Automap_EoB::*DrawIconFunc)(Graphics::Surface &surf, int iconSet, int iconID, int cellX, int cellY, int boxFitWidth, IconAlignment alignment, int extraX, int extraY, int overrideColor);
	DrawIconFunc _drawIcon;

	int fitString(const Graphics::Font *f, const Common::String &str, int maxW, int maxSc) const;
	uint16 calcNewBlockPosition(uint16 block, int8 dir) const;
	bool isVisited(uint16 block) const;
	bool isSeen(uint16 block) const;

	enum BreakableObjectType {
		kNoBreakableObject = 0,
		kBreakableBlockObject,
		kBreakableBarrierNS,
		kBreakableBarrierEW,
		kBrokenBarrier
	};

	int checkForBreakableObjectType(uint16 block) const;
	bool isCenteredSwitch(uint16 block) const;

	bool _visible;

	LevelBlockProperty *&_blockData;
	const uint8 *const _wllWallFlags;
	const uint8 *const _specialWallTypes;
	const int8 *const _wllShapeMap;
	//const uint8 *const _wllVmpMap;
	const uint8 _wallOfForceID;
	const uint8 *_portalParams;
	int _portalParamsLen;
	const bool _enabled;

	struct SpecialWallType {
		uint8 wall;
		uint8 icon;
		uint16 legendFlag;
	};
	const SpecialWallType *_specialBlockIDs;
	int _numSpecialBlockIDs;

	static const TranslateableStrings _stringTable[];
	const char *const *_legendStrings;
	const char *const *_controlStrings;
	const char *const *_levelNames;
	const int _numLevelNames;

	static const SpecialMarkers _specialMarkersEOB1[];
	static const SpecialMarkers _specialMarkersEOB2[];
	const SpecialMarkers *_specialMarkers;
	const char *const *_specialMarkerStrings;
	int _numSpecialMarkers;

	AutomapLayout _l;
	int _levelStrScl;
	int _coordStrScl;
	int _legendHeadScl;
	int _legendBodyScl;
	int _reduceSpace;
	int _levelStrY;
	int _coordStrY;

	OSystem *_system;
	Graphics::Surface *_background;
	Graphics::Surface *_frame;
	Common::Array<Graphics::Surface*> _mapIcons;
	Common::Array<Graphics::Surface*> _legendIcons;

private:
	// Colors
	enum ColorIndex {
		kColorStone,
		kColorStoneDark,
		kColorStoneEdge,
		kColorStoneHi,
		kColorRivet,
		kColorPaper,
		kColorPaperHi,
		kColorPaperLo,
		kColorPaperEdge,
		kColorInk,
		kColorInkSoft,
		kColorFloor,
		kColorFloorSeen,
		kColorGrid,
		kColorWall,
		kColorWallSeen,
		kColorWoF,
		kColorDoor,
		kColorStair,
		kColorTele,
		kColorPlate,
		kColorPit,
		kColorLever,
		kColorInteractive,
		kColorNiche,
		kColorPlaqueBg,
		kColorPlaqueEd,
		kColorGold,
		kColorGoldDim,
		kColorPanelTxt,
		kColorSpecial1,
		kColorSpecial2,
		kColorSpecial3,
		kColorSpecial4,
		kColorSpecial5,
		kColorSpecial6,
		kColorSpecial7,
		kColorTransp,
		kColorPartyFrame0,
		kColorPartyFrame1,
		kColorPartyFrame2,
		kColorPartyFrame3,
		kColorPartyFrame4,
		kColorPartyFrame5,
		kColorPartyFrame6,
		kColorPartyFrame7,
		kColorPartyFrame8,
		kColorPartyFrame9,
		kNumColors
	};

	/* const */uint32 *_colors;
	int _partyIconColor;
	int _partyIconColorStep;
};

} // End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL

#endif // KYRA_GUI_AUTOMAP_EOB_H
