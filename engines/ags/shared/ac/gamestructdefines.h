//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_CN_AC__GAMESTRUCTDEFINES_H
#define __AGS_CN_AC__GAMESTRUCTDEFINES_H

#include "util/geometry.h"
#include "core/types.h"

#define PAL_GAMEWIDE        0
#define PAL_LOCKED          1
#define PAL_BACKGROUND      2
#define MAXGLOBALMES        500
#define GLOBALMESLENGTH     500
#define MAXLANGUAGE         5
#define LEGACY_MAX_FONTS    30
#define OPT_DEBUGMODE       0
#define OPT_SCORESOUND      1
#define OPT_WALKONLOOK      2
#define OPT_DIALOGIFACE     3
#define OPT_ANTIGLIDE       4
#define OPT_TWCUSTOM        5
#define OPT_DIALOGGAP       6
#define OPT_NOSKIPTEXT      7
#define OPT_DISABLEOFF      8
#define OPT_ALWAYSSPCH      9
#define OPT_SPEECHTYPE      10
#define OPT_PIXPERFECT      11
#define OPT_NOWALKMODE      12
#define OPT_LETTERBOX       13
#define OPT_FIXEDINVCURSOR  14
#define OPT_NOLOSEINV       15
#define OPT_HIRES_FONTS     16
#define OPT_SPLITRESOURCES  17
#define OPT_ROTATECHARS     18
#define OPT_FADETYPE        19
#define OPT_HANDLEINVCLICKS 20
#define OPT_MOUSEWHEEL      21
#define OPT_DIALOGNUMBERED  22
#define OPT_DIALOGUPWARDS   23
#define OPT_CROSSFADEMUSIC  24
#define OPT_ANTIALIASFONTS  25
#define OPT_THOUGHTGUI      26
#define OPT_TURNTOFACELOC   27
#define OPT_RIGHTLEFTWRITE  28  // right-to-left text writing
#define OPT_DUPLICATEINV    29  // if they have 2 of the item, draw it twice
#define OPT_SAVESCREENSHOT  30
#define OPT_PORTRAITSIDE    31
#define OPT_STRICTSCRIPTING 32  // don't allow MoveCharacter-style commands
#define OPT_LEFTTORIGHTEVAL 33  // left-to-right operator evaluation
#define OPT_COMPRESSSPRITES 34
#define OPT_STRICTSTRINGS   35  // don't allow old-style strings
#define OPT_NEWGUIALPHA     36
#define OPT_RUNGAMEDLGOPTS  37
#define OPT_NATIVECOORDINATES 38 // defines coordinate relation between game logic and game screen
#define OPT_GLOBALTALKANIMSPD 39
#define OPT_HIGHESTOPTION_321 39
#define OPT_SPRITEALPHA     40
#define OPT_HIGHESTOPTION_330 OPT_SPRITEALPHA
#define OPT_SAFEFILEPATHS   41
#define OPT_HIGHESTOPTION_335 OPT_SAFEFILEPATHS
#define OPT_DIALOGOPTIONSAPI 42 // version of dialog options API (-1 for pre-3.4.0 API)
#define OPT_BASESCRIPTAPI   43 // version of the Script API (ScriptAPIVersion) used to compile game script
#define OPT_SCRIPTCOMPATLEV 44 // level of API compatibility (ScriptAPIVersion) used to compile game script
#define OPT_RENDERATSCREENRES 45 // scale sprites at the (final) screen resolution
#define OPT_RELATIVEASSETRES 46 // relative asset resolution mode (where sprites are resized to match game type)
#define OPT_WALKSPEEDABSOLUTE 47 // if movement speeds are independent of walkable mask resolution
#define OPT_HIGHESTOPTION   OPT_WALKSPEEDABSOLUTE
#define OPT_NOMODMUSIC      98
#define OPT_LIPSYNCTEXT     99
#define PORTRAIT_LEFT       0
#define PORTRAIT_RIGHT      1
#define PORTRAIT_ALTERNATE  2
#define PORTRAIT_XPOSITION  3
#define FADE_NORMAL         0
#define FADE_INSTANT        1
#define FADE_DISSOLVE       2
#define FADE_BOXOUT         3
#define FADE_CROSSFADE      4
#define FADE_LAST           4   // this should equal the last one

// Legacy font flags
//#define FFLG_LEGACY_NOSCALE 0x01 // TODO: is this from legacy format, ever used?
#define FFLG_LEGACY_SIZEMASK 0x3f
#define MAX_LEGACY_FONT_SIZE 63
// Contemporary font flags
#define FFLG_SIZEMULTIPLIER  0x01  // size data means multiplier
// Font outline types
#define FONT_OUTLINE_NONE -1
#define FONT_OUTLINE_AUTO -10

#define DIALOG_OPTIONS_HIGHLIGHT_COLOR_DEFAULT  14 // Yellow

#define MAXVIEWNAMELENGTH 15
#define MAXLIPSYNCFRAMES  20
#define MAX_GUID_LENGTH   40
#define MAX_SG_EXT_LENGTH 20
#define MAX_SG_FOLDER_LEN 50

enum GameResolutionType
{
    kGameResolution_Undefined   = -1,
    // definition of 320x200 in very old versions of the engine (somewhere pre-2.56)
    kGameResolution_Default     = 0,
    kGameResolution_320x200     = 1,
    kGameResolution_320x240     = 2,
    kGameResolution_640x400     = 3,
    kGameResolution_640x480     = 4,
    kGameResolution_800x600     = 5,
    kGameResolution_1024x768    = 6,
    kGameResolution_1280x720    = 7,
    kGameResolution_Custom      = 8,
    kNumGameResolutions,

    kGameResolution_LastLoRes   = kGameResolution_320x240,
    kGameResolution_FirstHiRes  = kGameResolution_640x400
};

inline bool IsLegacyHiRes(GameResolutionType resolution)
{
    return resolution > kGameResolution_LastLoRes;
}

Size ResolutionTypeToSize(GameResolutionType resolution, bool letterbox = false);

// Automatic numbering of dialog options (OPT_DIALOGNUMBERED)
enum DialogOptionNumbering
{
    kDlgOptNoNumbering = -1,
    kDlgOptKeysOnly    =  0, // implicit key shortcuts
    kDlgOptNumbering   =  1  // draw option indices and use key shortcuts
};

// Version of the script api (OPT_BASESCRIPTAPI and OPT_SCRIPTCOMPATLEV).
// If the existing script function meaning had changed, that may be
// possible to find out which implementation to use by checking one of those
// two options.
// NOTE: please remember that those values are valid only for games made with
// 3.4.0 final and above.
enum ScriptAPIVersion
{
    kScriptAPI_Undefined = INT32_MIN,
    kScriptAPI_v321 = 0,
    kScriptAPI_v330 = 1,
    kScriptAPI_v334 = 2,
    kScriptAPI_v335 = 3,
    kScriptAPI_v340 = 4,
    kScriptAPI_v341 = 5,
    kScriptAPI_v350 = 6,
    kScriptAPI_v3507= 7,
    kScriptAPI_v351 = 8,
    kScriptAPI_Current = kScriptAPI_v351
};

// Determines whether the graphics renderer should scale sprites at the final
// screen resolution, as opposed to native resolution
enum RenderAtScreenRes
{
    kRenderAtScreenRes_UserDefined  = 0,
    kRenderAtScreenRes_Enabled      = 1,
    kRenderAtScreenRes_Disabled     = 2,
};

// Method to use when blending two sprites with alpha channel
enum GameSpriteAlphaRenderingStyle
{
    kSpriteAlphaRender_Legacy = 0,
    kSpriteAlphaRender_Proper
};

// Method to use when blending two GUI elements with alpha channel
enum GameGuiAlphaRenderingStyle
{
    kGuiAlphaRender_Legacy = 0,
    kGuiAlphaRender_AdditiveAlpha,
    kGuiAlphaRender_Proper
};


// Sprite flags (serialized as 8-bit)
#define SPF_HIRES           0x01  // sized for high native resolution (legacy option)
#define SPF_HICOLOR         0x02  // is 16-bit
#define SPF_DYNAMICALLOC    0x04  // created by runtime script
#define SPF_TRUECOLOR       0x08  // is 32-bit
#define SPF_ALPHACHANNEL    0x10  // has alpha-channel
#define SPF_VAR_RESOLUTION  0x20  // variable resolution (refer to SPF_HIRES)
#define SPF_HADALPHACHANNEL 0x80  // the saved sprite on disk has one

// General information about sprite (properties, size)
struct SpriteInfo
{
    uint32_t Flags;
    int      Width;
    int      Height;

    SpriteInfo();

    //
    // Legacy game support
    //
    // Gets if sprite should adjust its base size depending on game's resolution
    inline bool IsRelativeRes() const { return (Flags & SPF_VAR_RESOLUTION) != 0; }
    // Gets if sprite belongs to high resolution; hi-res sprites should be
    // downscaled in low-res games, and low-res sprites should be upscaled
    // in hi-res games
    inline bool IsLegacyHiRes() const { return (Flags & SPF_HIRES) != 0; }
};

// Various font parameters, defining and extending font rendering behavior.
// While FontRenderer object's main goal is to render single line of text at
// the strictly determined position on canvas, FontInfo may additionally
// provide instructions on adjusting drawing position, as well as arranging
// multiple lines, and similar cases.
struct FontInfo
{
    enum AutoOutlineStyle : int
    {
        kRounded = 0,
        kSquared = 1,
    };

    // General font's loading and rendering flags
    uint32_t      Flags;
    // Font size, in points (basically means pixels in AGS)
    int           SizePt;
    // Factor to multiply base font size by
    int           SizeMultiplier;
    // Outlining font index, or auto-outline flag
    char          Outline;
    // Custom vertical render offset, used mainly for fixing broken fonts
    int           YOffset;
    // custom line spacing between two lines of text (0 = use font height)
    int           LineSpacing;
    // When automatic outlining, thickness of the outline (0 = use legacy thickness)
    int           AutoOutlineThickness;
    // When automatic outlining, style of the outline
    AutoOutlineStyle AutoOutlineStyle;

    FontInfo();
};

#endif // __AGS_CN_AC__GAMESTRUCTDEFINES_H
