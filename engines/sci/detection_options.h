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

#ifndef SCI_DETECTION_OPTIONS_H
#define SCI_DETECTION_OPTIONS_H

#include "common/translation.h"

namespace Sci {

const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_EGA_UNDITHER,
		{
			_s("Skip EGA dithering pass (full color backgrounds)"),
			_s("Skip dithering pass in EGA games, graphics are shown with full colors"),
			"disable_dithering",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_HIGH_RESOLUTION_GRAPHICS,
		{
			_s("Enable high resolution graphics"),
			_s("Enable high resolution graphics/content"),
			"enable_high_resolution_graphics",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_ENABLE_BLACK_LINED_VIDEO,
		{
			_s("Enable black-lined video"),
			_s("Draw black lines over videos to increase their apparent sharpness"),
			"enable_black_lined_video",
			false,
			0,
			0
		}
	},

#ifdef USE_RGB_COLOR
	{
		GAMEOPTION_HQ_VIDEO,
		{
			_s("Use high-quality video scaling"),
			_s("Use linear interpolation when upscaling videos, where possible"),
			"enable_hq_video",
			true,
			0,
			0
		}
	},
#endif

	{
		GAMEOPTION_LARRYSCALE,
		{
			_s("Use high-quality \"LarryScale\" cel scaling"),
			_s("Use special cartoon scaler for drawing character sprites"),
			"enable_larryscale",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_PREFER_DIGITAL_SFX,
		{
			_s("Prefer digital sound effects"),
			_s("Prefer digital sound effects instead of synthesized ones"),
			"prefer_digitalsfx",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	// Jones in the Fast Lane - CD audio tracks or resource.snd
	{
		GAMEOPTION_JONES_CDAUDIO,
		{
			_s("Use CD audio"),
			_s("Use CD audio instead of in-game audio, if available"),
			"use_cdaudio",
			true,
			0,
			0
		}
	},

	// KQ5 and KQ6 and SQ4 CD Windows - windows cursors
	{
		GAMEOPTION_WINDOWS_CURSORS,
		{
			_s("Use Windows cursors"),
			_s("Use the Windows cursors (smaller and monochrome) instead of the DOS ones"),
			"windows_cursors",
			false,
			0,
			0
		}
	},

	// SQ4 CD - silver cursors
	{
		GAMEOPTION_SQ4_SILVER_CURSORS,
		{
			_s("Use silver cursors"),
			_s("Use the alternate set of silver cursors instead of the normal golden ones"),
			"silver_cursors",
			false,
			0,
			0
		}
	},

	// Phantasmagoria 2 - content censoring option
	{
		GAMEOPTION_ENABLE_CENSORING,
		{
			_s("Enable content censoring"),
			_s("Enable the game's built-in optional content censoring"),
			"enable_censoring",
			false,
			0,
			0
		}
	},

	// KQ7 - Upscale videos to double their size (The in-game "Full screen" video setting)
	{
		GAMEOPTION_UPSCALE_VIDEOS,
		{
			_s("Upscale videos"),
			_s("Upscale videos to double their size"),
			"enable_video_upscale",
			true,
			0,
			0
		}
	},

	// SCI16 games: use RGB renderer instead of indexed
	{
		GAMEOPTION_RGB_RENDERING,
		{
			_s("Use RGB rendering"),
			_s("Use RGB rendering to improve screen transitions"),
			"rgb_rendering",
			false,
			0,
			0
		}
	},

	// SCI16 games: use custom per-resource palettes to improve visuals
	{
		GAMEOPTION_PALETTE_MODS,
		{
			_s("Use per-resource modified palettes"),
			_s("Use custom per-resource palettes to improve visuals"),
			"palette_mods",
			false,
			0,
			0
		}
	},

	// SQ1: Enable graphics that were disabled for legal reasons
	{
		GAMEOPTION_SQ1_BEARDED_MUSICIANS,
		{
			_s("Enable bearded musicians"),
			_s("Enable graphics that were disabled for legal reasons"),
			"enable_bearded_musicians",
			false,
			0,
			0
		}
	},

#ifdef USE_TTS
	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
 	},
#endif

	{
		GAMEOPTION_ENABLE_GMM_SAVE,
		{
			_s("Enable saving via the GMM"),
			_s("Allows saving via the GMM. WARNING: saves created via the GMM may be corrupted and unusable. Use at your own risk!"),
			"gmm_save_enabled",
			false,
			0,
			0
		}
 	},

	{
		GAMEOPTION_GK1_ENABLE_AUDIO_POPFIX,
		{
			_s("Repair speech audio"),
			_s("Detect and attempt to repair overflows in DPCM8 audio, which cause noticeable pops and crackles."),
			"audio_popfix_enabled",
			true,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const PopUpOptionsMap popUpOptionsList[] = {
	{
		GAMEOPTION_MIDI_MODE,
		_s("MIDI mode:"),
		_s("When using external MIDI devices (e.g. through USB-MIDI), select your device here"),
		"midi_mode",
		kMidiModeStandard,
		{
			{
				_s("Standard (GM / MT-32)"),
				kMidiModeStandard
			},
			{
				_s("Roland D-110 / D-10 / D-20"),
				kMidiModeD110
			},
			{
				_s("Yamaha FB-01"),
				kMidiModeFB01
			},
			{
				_s("Casio MT-540"),
				kMidiModeMT540
			},
			{
				_s("Casio CT-460 / CSM-1"),
				kMidiModeCT460
			},
			POPUP_OPTIONS_ITEMS_TERMINATOR
		}
	},
	POPUP_OPTIONS_TERMINATOR
};

} // End of namespace Sci

#endif
