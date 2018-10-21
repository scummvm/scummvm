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

#ifndef GARGOYLE_GLK_TYPES_H
#define GARGOYLE_GLK_TYPES_H

#include "common/scummsys.h"
#include "common/stream.h"

namespace Gargoyle {

typedef uint32 glui32;
typedef int32 glsi32;
class Window;

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
 * These types are opaque object identifiers. They're pointers to opaque
 * C structures, which are defined differently by each library.
 */
typedef struct glk_fileref_struct *frefid_t;
typedef struct glk_schannel_struct *schanid_t;

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
	gestalt_GarglkText             = 0x1100,
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
	style_NUMSTYLES    = 11,
};

enum WinType {
	wintype_AllTypes   = 0,
	wintype_Pair       = 1,
	wintype_Blank      = 2,
	wintype_TextBuffer = 3,
	wintype_TextGrid   = 4,
	wintype_Graphics   = 5,
};

enum WinMethod {
	winmethod_Left    = 0x00,
	winmethod_Right   = 0x01,
	winmethod_Above   = 0x02,
	winmethod_Below   = 0x03,
	winmethod_DirMask = 0x0f,

	winmethod_Fixed        = 0x10,
	winmethod_Proportional = 0x20,
	winmethod_DivisionMask = 0xf0,

	winmethod_Border     = 0x000,
	winmethod_NoBorder   = 0x100,
	winmethod_BorderMask = 0x100,
};

enum FileUsage {
	fileusage_Data        = 0x00,
	fileusage_SavedGame   = 0x01,
	fileusage_Transcript  = 0x02,
	fileusage_InputRecord = 0x03,
	fileusage_TypeMask    = 0x0f,

	fileusage_TextMode    = 0x100,
	fileusage_BinaryMode  = 0x000,
};

enum FileMode {
	filemode_Write       = 0x01,
	filemode_Read        = 0x02,
	filemode_ReadWrite   = 0x03,
	filemode_WriteAppend = 0x05,
};

enum SeekMode {
	seekmode_Start   = 0,
	seekmode_Current = 1,
	seekmode_End     = 2,
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
	stylehint_just_RightFlush = 3,
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

struct event_struct {
	glui32 type;
	Window *win;
	glui32 val1, val2;
};
typedef event_struct event_t;

struct stream_result_struct {
	glui32 readcount;
	glui32 writecount;
};
typedef stream_result_struct stream_result_t;

#ifdef GLK_MODULE_DATETIME

struct glktimeval_struct {
	glsi32 high_sec;
	glui32 low_sec;
	glsi32 microsec;
};
typedef glktimeval_struct glktimeval_t;

struct glkdate_struct {
	glsi32 year;     ///< full (four-digit) year */
	glsi32 month;    ///< 1-12, 1 is January
	glsi32 day;      ///< 1-31
	glsi32 weekday;  ///< 0-6, 0 is Sunday
	glsi32 hour;     ///< 0-23
	glsi32 minute;   ///< 0-59
	glsi32 second;   ///< 0-59, maybe 60 during a leap second
	glsi32 microsec; ///< 0-999999
};
typedef glkdate_struct glkdate_t;

union gidispatch_rock_t {
	glui32 num;
	void *ptr;
};

#endif /* GLK_MODULE_DATETIME */

} // End of namespace Gargoyle

#endif
