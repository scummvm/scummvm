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

#ifndef GLK_GLK_TYPES_H
#define GLK_GLK_TYPES_H

#include "common/scummsys.h"
#include "common/stream.h"

namespace Glk {

class Window;


/**
 * List of the different sub-engines the engine will likely eventually support
 */
enum InterpreterType {
	INTERPRETER_ADRIFT,
	INTERPRETER_ADVSYS,
	INTERPRETER_AGILITY,
	INTERPRETER_AGT,
	INTERPRETER_ALAN2,
	INTERPRETER_ALAN3,
	INTERPRETER_ARCHETYPE,
	INTERPRETER_BOCFEL,
	INTERPRETER_COMPREHEND,
	INTERPRETER_GEAS,
	INTERPRETER_GLULX,
	INTERPRETER_HUGO,
	INTERPRETER_JACL,
	INTERPRETER_LEVEL9,
	INTERPRETER_MAGNETIC,
	INTERPRETER_QUEST,
	INTERPRETER_SCARE,
	INTERPRETER_SCOTT,
	INTERPRETER_TADS2,
	INTERPRETER_TADS3,
	INTERPRETER_ZCODE
};

/**
 * These are the compile-time conditionals that reveal various Glk optional modules.
 */
#define GLK_MODULE_LINE_ECHO
#define GLK_MODULE_LINE_TERMINATORS
#define GLK_MODULE_UNICODE
#define GLK_MODULE_UNICODE_NORM
#define GLK_MODULE_IMAGE
#define GLK_MODULE_SOUND
#define GLK_MODULE_SOUND2
#define GLK_MODULE_HYPERLINKS
#define GLK_MODULE_DATETIME
#define GLK_MODULE_GARGLKTEXT

/**
 * Usurp C1 space for ligatures and smart typography glyphs
 */
enum Enc {
	ENC_LIG_FI = 128,
	ENC_LIG_FL = 129,
	ENC_LSQUO = 130,
	ENC_RSQUO = 131,
	ENC_LDQUO = 132,
	ENC_RDQUO = 133,
	ENC_NDASH = 134,
	ENC_MDASH = 135,
	ENC_FLOWBREAK = 136
};

/**
 * These are the Unicode versions
 */
enum UniChars {
	UNI_LIG_FI = 0xFB01,
	UNI_LIG_FL = 0xFB02,
	UNI_LSQUO = 0x2018,
	UNI_RSQUO = 0x2019,
	UNI_LDQUO = 0x201c,
	UNI_RDQUO = 0x201d,
	UNI_NDASH = 0x2013,
	UNI_MDASH = 0x2014
};

enum Gestalt {
	gestalt_Version                = 0,
	gestalt_CharInput              = 1,
	gestalt_LineInput              = 2,
	gestalt_CharOutput             = 3,
	gestalt_CharOutput_CannotPrint = 0,
	gestalt_CharOutput_ApproxPrint = 1,
	gestalt_CharOutput_ExactPrint  = 2,
	gestalt_MouseInput             = 4,
	gestalt_Timer                  = 5,
	gestalt_Graphics               = 6,
	gestalt_DrawImage              = 7,
	gestalt_Sound                  = 8,
	gestalt_SoundVolume            = 9,
	gestalt_SoundNotify            = 10,
	gestalt_Hyperlinks             = 11,
	gestalt_HyperlinkInput         = 12,
	gestalt_SoundMusic             = 13,
	gestalt_GraphicsTransparency   = 14,
	gestalt_Unicode                = 15,
	gestalt_UnicodeNorm            = 16,
	gestalt_LineInputEcho          = 17,
	gestalt_LineTerminators        = 18,
	gestalt_LineTerminatorKey      = 19,
	gestalt_DateTime               = 20,
	gestalt_Sound2                 = 21,
	gestalt_GarglkText             = 0x1100
};

enum Style {
	style_Normal       = 0,
	style_Emphasized   = 1,
	style_Preformatted = 2,
	style_Header       = 3,
	style_Subheader    = 4,
	style_Alert        = 5,
	style_Note         = 6,
	style_BlockQuote   = 7,
	style_Input        = 8,
	style_User1        = 9,
	style_User2        = 10,
	style_NUMSTYLES    = 11
};

enum WinType {
	wintype_AllTypes   = 0,
	wintype_Pair       = 1,
	wintype_Blank      = 2,
	wintype_TextBuffer = 3,
	wintype_TextGrid   = 4,
	wintype_Graphics   = 5
};

enum WinMethod {
	winmethod_Left         = 0x00,
	winmethod_Right        = 0x01,
	winmethod_Above        = 0x02,
	winmethod_Below        = 0x03,
	winmethod_Arbitrary    = 0x04,		///< Newly introduced for ScummVM Glk
	winmethod_DirMask      = 0x0f,

	winmethod_Fixed        = 0x10,
	winmethod_Proportional = 0x20,
	winmethod_DivisionMask = 0xf0,

	winmethod_Border     = 0x000,
	winmethod_NoBorder   = 0x100,
	winmethod_BorderMask = 0x100
};

enum StyleHint {
	stylehint_Indentation     = 0,
	stylehint_ParaIndentation = 1,
	stylehint_Justification  = 2,
	stylehint_Size            = 3,
	stylehint_Weight          = 4,
	stylehint_Oblique         = 5,
	stylehint_Proportional    = 6,
	stylehint_TextColor       = 7,
	stylehint_BackColor       = 8,
	stylehint_ReverseColor    = 9,
	stylehint_NUMHINTS        = 10,

	stylehint_just_LeftFlush  = 0,
	stylehint_just_LeftRight  = 1,
	stylehint_just_Centered   = 2,
	stylehint_just_RightFlush = 3
};

/**
 * These constants define the classes of opaque objects. It's a bit ugly to put
 * them in this header file, since more classes may be added in the future.
 * But if you find yourself stuck with an obsolete version of this file,
 * adding new class definitions will be easy enough -- they will be numbered
 * sequentially, and the numeric constants can be found in the Glk specification.
 */
enum giDisp {
	gidisp_Class_Window   = 0,
	gidisp_Class_Stream   = 1,
	gidisp_Class_Fileref  = 2,
	gidisp_Class_Schannel = 3
};

enum zcolor {
	zcolor_Transparent = 0x7FFFFFFCU,
	zcolor_Cursor      = 0x7FFFFFFDU,
	zcolor_Current     = 0x7FFFFFFEU,
	zcolor_Default     = 0x7FFFFFFFU
};

#ifdef GLK_MODULE_IMAGE

enum ImageAlign {
	imagealign_InlineUp     = 1,
	imagealign_InlineDown   = 2,
	imagealign_InlineCenter = 3,
	imagealign_MarginLeft   = 4,
	imagealign_MarginRight  = 5
};

#endif /* GLK_MODULE_IMAGE */

union gidispatch_rock_t {
	uint num;
	void *ptr;

	gidispatch_rock_t() : ptr(nullptr) {}
};

union gluniversal_union {
	uint _uint;          ///< Iu
	int32 _sint;         ///< Is
	void *_opaqueref;    ///< Qa, Qb, Qc...
	byte _uch;           ///< Cu
	int8 _sch;           ///< Cs
	char _ch;            ///< Cn
	char *_charstr;		 ///< S
	uint32 *_unicharstr; ///< U
	void *_array;        ///< all # arguments
	uint _ptrflag;       ///< [ ... ] or *?
};
typedef gluniversal_union gluniversal_t;

struct gidispatch_intconst_struct {
	const char *name;
	uint32 val;
};
typedef gidispatch_intconst_struct gidispatch_intconst_t;

typedef uint32 glui32;

} // End of namespace Glk

#endif
