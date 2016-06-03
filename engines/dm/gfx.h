#ifndef GFX_H
#define GFX_H

#include "common/scummsys.h"
#include "dm/dm.h"

namespace DM {

extern uint16 gPalSwoosh[16];
extern uint16 gPalMousePointer[16];
extern uint16 gPalCredits[16];
extern uint16 gPalEntrance[16];
extern uint16 gPalDungeonView[6][16];



typedef struct {
	uint16 X1;
	uint16 X2;
	uint16 Y1;
	uint16 Y2;
} Box; // @ BOX_BYTE, BOX_WORD

struct Frame;
enum WallSet {
	kWallSetStone = 0 // @ C0_WALL_SET_STONE
};

enum FloorSet {
	kFloorSetStone = 0 // @ C0_FLOOR_SET_STONE
};

enum StairIndex {
	kStairsNativeIndex_Up_Front_D3L = 0, // @ G0675_i_StairsNativeBitmapIndex_Up_Front_D3L
	kStairsNativeIndex_Up_Front_D3C = 1, // @ G0676_i_StairsNativeBitmapIndex_Up_Front_D3C
	kStairsNativeIndex_Up_Front_D2L = 2, // @ G0677_i_StairsNativeBitmapIndex_Up_Front_D2L
	kStairsNativeIndex_Up_Front_D2C = 3, // @ G0678_i_StairsNativeBitmapIndex_Up_Front_D2C
	kStairsNativeIndex_Up_Front_D1L = 4, // @ G0679_i_StairsNativeBitmapIndex_Up_Front_D1L
	kStairsNativeIndex_Up_Front_D1C = 5, // @ G0680_i_StairsNativeBitmapIndex_Up_Front_D1C
	kStairsNativeIndex_Up_Front_D0C_Left = 6, // @ G0681_i_StairsNativeBitmapIndex_Up_Front_D0C_Left
	kStairsNativeIndex_Down_Front_D3L = 7, // @ G0682_i_StairsNativeBitmapIndex_Down_Front_D3L
	kStairsNativeIndex_Down_Front_D3C = 8, // @ G0683_i_StairsNativeBitmapIndex_Down_Front_D3C
	kStairsNativeIndex_Down_Front_D2L = 9, // @ G0684_i_StairsNativeBitmapIndex_Down_Front_D2L
	kStairsNativeIndex_Down_Front_D2C = 10, // @ G0685_i_StairsNativeBitmapIndex_Down_Front_D2C
	kStairsNativeIndex_Down_Front_D1L = 11, // @ G0686_i_StairsNativeBitmapIndex_Down_Front_D1L
	kStairsNativeIndex_Down_Front_D1C = 12, // @ G0687_i_StairsNativeBitmapIndex_Down_Front_D1C
	kStairsNativeIndex_Down_Front_D0C_Left = 13, // @ G0688_i_StairsNativeBitmapIndex_Down_Front_D0C_Left
	kStairsNativeIndex_Side_D2L = 14, // @ G0689_i_StairsNativeBitmapIndex_Side_D2L
	kStairsNativeIndex_Up_Side_D1L = 15, // @ G0690_i_StairsNativeBitmapIndex_Up_Side_D1L
	kStairsNativeIndex_Down_Side_D1L = 16, // @ G0691_i_StairsNativeBitmapIndex_Down_Side_D1L
	kStairsNativeIndex_Side_D0L = 17 // @ G0692_i_StairsNativeBitmapIndex_Side_D0L
};

enum ViewWall {
	kViewWall_D3L_RIGHT = 0, // @ C00_VIEW_WALL_D3L_RIGHT 
	kViewWall_D3R_LEFT = 1, // @ C01_VIEW_WALL_D3R_LEFT  
	kViewWall_D3L_FRONT = 2, // @ C02_VIEW_WALL_D3L_FRONT 
	kViewWall_D3C_FRONT = 3, // @ C03_VIEW_WALL_D3C_FRONT 
	kViewWall_D3R_FRONT = 4, // @ C04_VIEW_WALL_D3R_FRONT 
	kViewWall_D2L_RIGHT = 5, // @ C05_VIEW_WALL_D2L_RIGHT 
	kViewWall_D2R_LEFT = 6, // @ C06_VIEW_WALL_D2R_LEFT  
	kViewWall_D2L_FRONT = 7, // @ C07_VIEW_WALL_D2L_FRONT 
	kViewWall_D2C_FRONT = 8, // @ C08_VIEW_WALL_D2C_FRONT 
	kViewWall_D2R_FRONT = 9, // @ C09_VIEW_WALL_D2R_FRONT 
	kViewWall_D1L_RIGHT = 10, // @ C10_VIEW_WALL_D1L_RIGHT
	kViewWall_D1R_LEFT = 11, // @ C11_VIEW_WALL_D1R_LEFT 
	kViewWall_D1C_FRONT = 12  // @ C12_VIEW_WALL_D1C_FRONT
};



enum Color {
	kColorNoTransparency = 255,
	kColorBlack = 0,
	kColorDarkGary = 1,
	kColorLightGray = 2,
	kColorDarkBrown = 3,
	kColorCyan = 4,
	kColorLightBrown = 5,
	kColorDarkGreen = 6,
	kColorLightGreen = 7,
	kColorRed = 8,
	kColorGold = 9,
	kColorFlesh = 10,
	kColorYellow = 11,
	kColorDarkestGray = 12,
	kColorLightestGray = 13,
	kColorBlue = 14,
	kColorWhite = 15
};


struct Viewport {
	// TODO: should probably add width and height, seems redundant right meow
	uint16 posX, posY;
};

extern Viewport gDefultViewPort;
extern Viewport gDungeonViewport;


#define kAlcoveOrnCount 3
#define kFountainOrnCount 1

#define kFloorSetGraphicCount 2 // @ C002_FLOOR_SET_GRAPHIC_COUNT
#define kWallSetGraphicCount 13 // @ C013_WALL_SET_GRAPHIC_COUNT
#define kStairsGraphicCount 18 // @ C018_STAIRS_GRAPHIC_COUNT
#define kDoorSetGraphicsCount 3 // @ C003_DOOR_SET_GRAPHIC_COUNT
#define kDoorButtonCount 1 // @ C001_DOOR_BUTTON_COUNT
#define kNativeBitmapIndex 0 // @ C0_NATIVE_BITMAP_INDEX
#define kNativeCoordinateSet 1 // @ C1_COORDINATE_SET
#define kCreatureTypeCount 27 // @ C027_CREATURE_TYPE_COUNT
#define kExplosionAspectCount 4 // @ C004_EXPLOSION_ASPECT_COUNT
#define kObjAspectCount 85 // @ C085_OBJECT_ASPECT_COUNT
#define kProjectileAspectCount 14 // @ C014_PROJECTILE_ASPECT_COUNT


#define kDoorButton 0 // @ C0_DOOR_BUTTON
#define kWallOrnInscription 0 // @ C0_WALL_ORNAMENT_INSCRIPTION
#define kFloorOrnFootprints 15 // @ C15_FLOOR_ORNAMENT_FOOTPRINTS
#define kDoorOrnDestroyedMask 15 // @ C15_DOOR_ORNAMENT_DESTROYED_MASK
#define kDoorOrnThivesEyeMask 16 // @ C16_DOOR_ORNAMENT_THIEVES_EYE_MASK

class DisplayMan {
	DMEngine *_vm;
	uint16 _screenWidth;
	uint16 _screenHeight;
	byte *_vgaBuffer;

	/// Related to graphics.dat file
	uint16 _grapItemCount; // @ G0632_ui_GraphicCount
	uint32 *_packedItemPos;
	byte *_packedBitmaps;
	byte **_bitmaps;

	// pointers 13,14 and [15-19] are owned by this array
	byte *_wallSetBitMaps[25];	// @G[0696..0710]_puc_Bitmap_WallSet_...
	uint16 _stairIndices[kStairsGraphicCount];

	// pointers are not owned by these fields
	byte *_floorBitmap;
	byte *_ceilingBitmap;
	byte *_palChangesProjectile[4]; // @G0075_apuc_PaletteChanges_Projectile

	DisplayMan(const DisplayMan &other); // no implementation on purpose
	void operator=(const DisplayMan &rhs); // no implementation on purpose

	byte *getCurrentVgaBuffer();
	// the original function has two position parameters, but they are always set to zero
	void loadIntoBitmap(uint16 index, byte *destBitmap); // @ F0466_EXPAND_GraphicToBitmap
	void unpackGraphics();

	void drawFloorPitOrStairsBitmapFlippedHorizontally(StairIndex relIndex, Frame &frame); // @ F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally
	void drawFloorPitOrStairsBitmap(StairIndex relIndex, Frame &frame); // @ F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap
	void drawWallSetBitmap(byte *bitmap, Frame &f); // @ F0100_DUNGEONVIEW_DrawWallSetBitmap
	void drawWallSetBitmapWithoutTransparency(byte *bitmap, Frame &f); // @ F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency
	void drawSquareD3L(direction dir, int16 posX, int16 posY); // @ F0116_DUNGEONVIEW_DrawSquareD3L
	void drawSquareD3R(direction dir, int16 posX, int16 posY); // @ F0117_DUNGEONVIEW_DrawSquareD3R
	void drawSquareD3C(direction dir, int16 posX, int16 posY); // @ F0118_DUNGEONVIEW_DrawSquareD3C_CPSF
	void drawSquareD2L(direction dir, int16 posX, int16 posY); // @ F0119_DUNGEONVIEW_DrawSquareD2L
	void drawSquareD2R(direction dir, int16 posX, int16 posY); // @ F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
	void drawSquareD2C(direction dir, int16 posX, int16 posY); // @ F0121_DUNGEONVIEW_DrawSquareD2C
	void drawSquareD1L(direction dir, int16 posX, int16 posY); // @ F0122_DUNGEONVIEW_DrawSquareD1L
	void drawSquareD1R(direction dir, int16 posX, int16 posY); // @ F0122_DUNGEONVIEW_DrawSquareD1R
	void drawSquareD1C(direction dir, int16 posX, int16 posY); // @ F0124_DUNGEONVIEW_DrawSquareD1C
	void drawSquareD0L(direction dir, int16 posX, int16 posY); // @ F0125_DUNGEONVIEW_DrawSquareD0L
	void drawSquareD0R(direction dir, int16 posX, int16 posY); // @ F0126_DUNGEONVIEW_DrawSquareD0R
	void drawSquareD0C(direction dir, int16 posX, int16 posY); // @ F0127_DUNGEONVIEW_DrawSquareD0C

	void loadWallSet(WallSet set); // @ F0095_DUNGEONVIEW_LoadWallSet
	void loadFloorSet(FloorSet set); // @ F0094_DUNGEONVIEW_LoadFloorSet

	void applyCreatureReplColors(int replacedColor, int replacementColor); // @ F0093_DUNGEONVIEW_ApplyCreatureReplacementColors

	bool isDrawnWallOrnAnAlcove(int16 wallOrnOrd, ViewWall viewWallIndex); // @ F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF

	// some methods use this for a stratchpad, don't make assumptions about content between function calls
	byte *_tmpBitmap;
public:
	DisplayMan(DMEngine *dmEngine);
	~DisplayMan();

	void setUpScreens(uint16 width, uint16 height);
	void loadGraphics(); // @ F0479_MEMORY_ReadGraphicsDatHeader, F0460_START_InitializeGraphicData
	void loadCurrentMapGraphics();
	void loadPalette(uint16 *palette);

	/// Gives the width of an IMG0 type item
	uint16 width(uint16 index);
	/// Gives the height of an IMG1 type item
	uint16 height(uint16 index);

	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  byte *destBitmap, uint16 destWidth,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = kColorNoTransparency, Viewport &viewport = gDefultViewPort);
	void blitToBitmapShrinkWithPalChange(byte *srcBitmap, int16 srcWidth, int16 srcHight,
										 byte *destBitmap, int16 destWidth, int16 destHeight, byte *palChange); // @ F0129_VIDEO_BlitShrinkWithPaletteChanges

	void blitToBitmap(byte *srcBitmap, uint16 srcWidth, uint16 srcHeight, byte *destBitmap, uint16 destWidth, uint16 destX = 0, uint16 destY = 0);
	void blitToScreen(byte *srcBitmap, uint16 srcWidth, uint16 srcX, uint16 srcY,
					  uint16 destFromX, uint16 destToX, uint16 destFromY, uint16 destToY,
					  Color transparent = kColorNoTransparency, Viewport &viewport = gDefultViewPort);

	void flipBitmapHorizontal(byte *bitmap, uint16 width, uint16 height);
	void flipBitmapVertical(byte *bitmap, uint16 width, uint16 height);

	void clearBitmap(byte *bitmap, uint16 width, uint16 height, Color color);
	void clearScreen(Color color);
	void drawDungeon(direction dir, int16 posX, int16 posY); // @ F0128_DUNGEONVIEW_Draw_CPSF
	void updateScreen();

	int16 _championPortraitOrdinal; // @ G0289_i_DungeonView_ChampionPortraitOrdinal
	int16 _currMapAlcoveOrnIndices[kAlcoveOrnCount]; // @ G0267_ai_CurrentMapAlcoveOrnamentIndices
	int16 _currMapFountainOrnIndices[kFountainOrnCount]; // @ G0268_ai_CurrentMapFountainOrnamentIndices
	int16 _currMapWallOrnInfo[16][2]; // @ G0101_aai_CurrentMapWallOrnamentsInfo
	int16 _currMapFloorOrnInfo[16][2]; // @ G0102_aai_CurrentMapFloorOrnamentsInfo
	int16 _currMapDoorOrnInfo[17][2]; // @ G0103_aai_CurrentMapDoorOrnamentsInfo
	byte *_currMapAllowedCreatureTypes; // @ G0264_puc_CurrentMapAllowedCreatureTypes
	byte _currMapWallOrnIndices[16]; // @ G0261_auc_CurrentMapWallOrnamentIndices
	byte _currMapFloorOrnIndices[16]; // @ G0262_auc_CurrentMapFloorOrnamentIndices
	byte _currMapDoorOrnIndices[18]; // @ G0263_auc_CurrentMapDoorOrnamentIndices

	int16 _currMapViAltarIndex; // @ G0266_i_CurrentMapViAltarWallOrnamentIndex

	Thing _inscriptionThing; // @ G0290_T_DungeonView_InscriptionThing
};

}

#endif
