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

//=============================================================================
//
// Game version constants and information
//
//=============================================================================

#ifndef AGS_SHARED_AC_GAME_VERSION_H
#define AGS_SHARED_AC_GAME_VERSION_H

namespace AGS3 {

/*

Game data versions and changes:
-------------------------------

12 : 2.3 + 2.4

Versions above are incompatible at the moment.

18 : 2.5.0
19 : 2.5.1 + 2.52
20 : 2.5.3

Lip sync data added.
21 : 2.5.4
22 : 2.5.5

Variable number of sprites.
24 : 2.5.6
25 : 2.6.0

Encrypted global messages and dialogs.
26 : 2.6.1

Wait() must be called with parameter > 0
GetRegionAt() clips the input values to the screen size
Color 0 now means transparent instead of black for text windows
SetPlayerCharacter() does nothing if the new character is already the player character.
27 : 2.6.2

Script modules. Fixes bug in the inventory display.
Clickable GUI is selected with regard for the drawing order.
Pointer to the "player" variable is now accessed via a dynamic object.
31 : 2.7.0
32 : 2.7.2

35 : 3.0.0

Room names are serialized when game is compiled in "debug" mode.
36 : 3.0.1

Interactions are now scripts. The number for "not set" changed from 0 to -1 for
a lot of variables (views, sounds).
Deprecated switch between low-res and high-res native coordinates.
37 : 3.1.0

Dialogs are now scripts. New character animation speed.
39 : 3.1.1

Individual character speech animation speed.
40 : 3.1.2

Audio clips
41 : 3.2.0
42 : 3.2.1

43 : 3.3.0
Added few more game options.

44 : 3.3.1
Added custom dialog option highlight colour.

45 : 3.4.0.1
Support for custom game resolution.

46 : 3.4.0.2-.3
Audio playback speed.
Custom dialog option rendering extension.

47 : 3.4.0.4
Custom properties changed at runtime.
Ambient lighting

48 : 3.4.1
OPT_RENDERATSCREENRES, extended engine caps check, font vertical offset.

49 : 3.4.1.2
Font custom line spacing.

50 : 3.5.0.8
Sprites have "real" resolution. Expanded FontInfo data format.
Option to allow legacy relative asset resolutions.

3.6.0 :
Format value is defined as AGS version represented as NN,NN,NN,NN.
Fonts have adjustable outline
3.6.0.11:
New font load flags, control backward compatible font behavior
3.6.0.16:
Idle animation speed, modifiable hotspot names, fixed video frame
3.6.0.21:
Some adjustments to gui text alignment.
3.6.1:
In RTL mode all text is reversed, not only wrappable (labels etc).
3.6.1.10:
Disabled automatic SetRestartPoint.
3.6.1.14:
Extended game object names, resolving hard length limits.
*/

enum GameDataVersion {
	kGameVersion_Undefined = 0,
	kGameVersion_230 = 12,
	kGameVersion_240 = 12,
	kGameVersion_250 = 18,
	kGameVersion_251 = 19, // same as 2.52
	kGameVersion_253 = 20,
	kGameVersion_254 = 21,
	kGameVersion_255 = 22,
	kGameVersion_256 = 24,
	kGameVersion_260 = 25,
	kGameVersion_261 = 26,
	kGameVersion_262 = 27,
	kGameVersion_270 = 31,
	kGameVersion_272 = 32,
	kGameVersion_300 = 35,
	kGameVersion_301 = 36,
	kGameVersion_310 = 37,
	kGameVersion_311 = 39,
	kGameVersion_312 = 40,
	kGameVersion_320 = 41,
	kGameVersion_321 = 42,
	kGameVersion_330 = 43,
	kGameVersion_331 = 44,
	kGameVersion_340_1 = 45,
	kGameVersion_340_2 = 46,
	kGameVersion_340_4 = 47,
	kGameVersion_341 = 48,
	kGameVersion_341_2 = 49,
	kGameVersion_350 = 50,
	kGameVersion_360 = 3060000,
	kGameVersion_360_11 = 3060011,
	kGameVersion_360_16 = 3060016,
	kGameVersion_360_21 = 3060021,
	kGameVersion_361 = 3060100,
	kGameVersion_361_10 = 3060110,
	kGameVersion_361_14 = 3060114,
	kGameVersion_Current = kGameVersion_361_14
};

} // namespace AGS3

#endif
