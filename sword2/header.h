/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	_HEADER
#define	_HEADER

#include "common/scummsys.h"
//#include "src\driver96.h"

//----------------------------------------------------------
// SYSTEM FILE & FRAME HEADERS	(23sep96 JEL)
//----------------------------------------------------------

//#pragma pack( push )
//#pragma pack( 1 )

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif



//----------------------------------------------------------
// ALL FILES
//----------------------------------------------------------
// Standard File Header
#define	NAME_LEN	34

typedef struct
{
	uint8	fileType;	// byte to define file type (see below)
	uint8	compType;	// type of file compression used ie. on whole file (see below)
	uint32	compSize;	// length of compressed file (ie. length on disk)
	uint32	decompSize;	// length of decompressed file held in memory (NB. frames still held compressed)
	uint8	name[NAME_LEN];	//name of object
} GCC_PACK _standardHeader;

//----------------------------------------------------------
// fileType

// 0 something's wrong!
#define ANIMATION_FILE		1	// all normal animations & sprites including mega-sets & font files which are the same format (but all frames always uncompressed)
#define	SCREEN_FILE			2	// each contains background, palette, layer sprites, parallax layers & shading mask
#define	GAME_OBJECT			3	// each contains object hub + structures + script data
#define	WALK_GRID_FILE		4	// walk-grid data
#define	GLOBAL_VAR_FILE		5	// all the global script variables in one file; "there can be only one"
#define PARALLAX_FILE_null	6	// NOT USED
#define	RUN_LIST			7	// each contains a list of object resource id's
#define	TEXT_FILE			8	// each contains all the lines of text for a location or a character's conversation script
#define	SCREEN_MANAGER		9	// one for each location; this contains special startup scripts
#define MOUSE_FILE			10	// mouse pointers and luggage icons (sprites in General \ Mouse pointers & Luggage icons)
#define WAV_FILE			11	// wav file
#define	ICON_FILE			12	// menu icon (sprites in General \ Menu icons
#define PALETTE_FILE		13	// separate palette file (see also _paletteHeader)
//----------------------------------------------------------
// compType

#define NO_COMPRESSION		0
#define FILE_COMPRESSION	1	// standard whole-file compression (not yet devised!)

//----------------------------------------------------------



//----------------------------------------------------------
// (1) ANIMATION FILES
//----------------------------------------------------------
// an animation file consists of:

// standard file header
// animation header
// a string of CDT entries (one per frame of the anim)
// a 16-byte colour table ONLY if (runTimeComp==RLE16)
// a string of groups of (frame header + frame data)

//----------------------------------------------------------
// Animation Header

typedef	struct
{
	uint8	runTimeComp;	// type of runtime compression used for the frame data (see below)
	uint16	noAnimFrames;	// number of frames in the anim (ie. no. of CDT entries)
	uint16	feetStartX;		// start coords for mega to walk to, before running anim
	uint16	feetStartY;
	uint8	feetStartDir;	// direction to start in before running anim
	uint16	feetEndX;		// end coords for mega to stand at after running anim (vital if anim starts from an off-screen position, or ends in a different place from the start)
	uint16	feetEndY;
	uint8	feetEndDir;		// direction to start in after running anim
	uint16	blend;
} GCC_PACK _animHeader;

//----------------------------------------------------------
// runtimeComp - compression used on each frame of the anim

#define NONE	0	// No frame compression
#define RLE256	1	// James's RLE for 256-colour sprites
#define RLE16	2	// James's RLE for 16- or 17-colour sprites
					// (raw blocks have max 16 colours for 2 pixels per byte,
					// so '0's are encoded only as FLAT for 17-colour sprites eg. George's mega-set)
//----------------------------------------------------------
// CDT Entry

typedef struct
{
	int16	x;				// sprite x-coord OR offset to add to mega's feet x-coord to calc sprite y-coord
	int16	y;				// sprite y-coord OR offset to add to mega's feet y-coord to calc sprite y-coord
	uint32	frameOffset;	// points to start of frame header (from start of file header)
	uint8	frameType;		// 0=print sprite normally with top-left corner at (x,y), otherwise see below...
} GCC_PACK _cdtEntry;

// 'frameType' bit values
#define FRAME_OFFSET	1	// print at (feetX+x,feetY+y), with scaling according to feetY
#define FRAME_FLIPPED	2	// print the frame flipped Left->Right
#define FRAME_256_FAST	4	// Frame has been compressed using Pauls fast RLE 256 compression.

//----------------------------------------------------------
//Frame Header

typedef	struct
{
	uint32	compSize;	// compressed size of frame - NB. compression type is now in Anim Header
	uint16	width;		// dimensions of frame
	uint16	height;
} GCC_PACK _frameHeader;

//----------------------------------------------------------
// Frame Data

// uint8 spriteData[width*height];	// one byte per pixel
//----------------------------------------------------------





//----------------------------------------------------------
// (2) SCREEN FILES
//----------------------------------------------------------
// a screen file consists of:

// standard file header
// multi screen header
// 4*256 bytes of palette data
// 256k palette match table
// 2 background parallax layers
// 1 background layer with screen header
// 2 foreground parallax layers
// a string of layer headers
// a string of layer masks

//----------------------------------------------------------
// Multi screen header
// Goes at the beginning of a screen file after the standard
// header.
// Gives offsets from start of table of each of the components

typedef struct
{
	uint32 palette;
	uint32 bg_parallax[2];
	uint32 screen;
	uint32 fg_parallax[2];
	uint32 layers;
	uint32 paletteTable;
	uint32 maskOffset;
} GCC_PACK _multiScreenHeader;

//------------------------------------------------------------
// Palette Data

typedef struct
{
	uint8	red;
	uint8	green;
	uint8	blue;
	uint8	alpha;
} GCC_PACK _palEntry;

#define	NO_COLOURS	256
// _palEntry palette[NO_COLOURS]

//------------------------------------------------------------
// Screen Header

typedef struct
{
	uint16	width;		// dimensions of the background screen
	uint16	height;
	uint16	noLayers;	// number of layer areas
} GCC_PACK _screenHeader;

//------------------------------------------------------------
// Background Raw Bitmap

// uint8 backgroundData[width*height];	// one byte per pixel

//------------------------------------------------------------
// Layer Header

// Note that all the layer headers are kept together,
// rather than being placed before each layer mask,
// in order to simplify the sort routine.

typedef struct
{
	uint16	x;			// coordinates of top-left pixel of area
	uint16	y;
	uint16	width;
	uint16	height;
	uint32  maskSize;
	uint32	offset;		// where to find mask data (from start of standard file header)
} GCC_PACK _layerHeader;

//------------------------------------------------------------
// Layer Mask

// uint8 layerData[width*height/8];	// 8 pixels to a byte

//------------------------------------------------------------



//----------------------------------------------------------
// (3) SCRIPT OBJECT FILES
//----------------------------------------------------------
// a script object file consists of:

// standard file header
// script object header
// script object data

//----------------------------------------------------------
// Script Object Header

// ???????
// ???????
// ???????

//----------------------------------------------------------
// Script Object Data

//----------------------------------------------------------



//----------------------------------------------------------
// (4) WALK-GRID FILES
//----------------------------------------------------------
// a walk-grid file consists of:

// standard file header
// walk-grid file header
// walk-grid data

//----------------------------------------------------------
// Walk-Grid Header - taken directly from old "header.h" in STD_INC

typedef struct
{
	int32	numBars;	// number of bars on the floor
	int32	numNodes;	// number of nodes
} GCC_PACK _walkGridHeader;

//----------------------------------------------------------
// Walk-Grid Data

// ???????

//----------------------------------------------------------



//----------------------------------------------------------
// (5) PALETTE FILES
//----------------------------------------------------------
// a palette file consists of:

// standard file header
// 4*256 bytes of palette data
// 256k palette match table

//----------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// PCX file header
//--------------------------------------------------------------------------------------------------
typedef	struct
{	uint8	manufacturer;
	uint8	version;
	uint8	encoding;
	uint8	bitsPerPixel;
	int16	xmin,ymin;
	int16	xmax,ymax;
	int16	hres;
	int16	vres;
	char	palette[48];
	char	reserved;
	uint8	colourPlanes;
	int16	bytesPerLine;
	int16	paletteType;
	char	filler[58];
} GCC_PACK _PCXHEAD;
//----------------------------------------------------------
#define	TREE_SIZE	3

typedef	struct	// an object hub - which represents all that remains of the compact concept
{
	int32	type;					// type of object
	uint32	logic_level;			// what level?
	uint32	logic[TREE_SIZE];		// NOT USED
	uint32	script_id[TREE_SIZE];	// need this if script
	uint32	script_pc[TREE_SIZE];	// need this also
} GCC_PACK _object_hub;
//----------------------------------------------------------
//----------------------------------------------------------
// (6) text module header	TW

typedef	struct
{
	uint32	noOfLines;	//how many lines of text are there in this module
} GCC_PACK _textHeader;

//a text file has:

//	_standardHeader
//	_textHeader
//	look up table, to
//	line of text,0
//	line of text,0

//----------------------------------------------------------

// #pragma pack( pop )

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

#endif
