/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_SERVICES_SETTINGS_H
#define STARK_SERVICES_SETTINGS_H

#include "common/config-manager.h"
#include "common/language.h"
#include "common/ustr.h"

#include "engines/stark/gfx/texture.h"
#include "engines/stark/services/services.h"

struct ADGameDescription;

namespace Audio {
class Mixer;
}

namespace Stark {

/**
 * Settings services.
 * 
 * Maintains the settings of the game.
 */
class Settings {
public:
	enum BoolSettingIndex {
		kHighModel,
		kSubtitle,
		kSpecialFX,
		kShadow,
		kHighFMV,
		kTimeSkip
	};

	enum IntSettingIndex {
		kVoice,
		kMusic,
		kSfx,
		kSaveLoadPage
	};

	Settings(Audio::Mixer *mixer, const ADGameDescription *gd);
	~Settings() {}

	/**
	 * Is this a demo version of the game?
	 *
	 * This is true either for 4-CD or 2-CD style demos
	 */
	bool isDemo() const {
		return _isDemo;
	}

	/** Get the settings value */
	bool getBoolSetting(BoolSettingIndex index) { return ConfMan.getBool(_boolKey[index]); }
	int getIntSetting(IntSettingIndex index) { return ConfMan.getInt(_intKey[index]); }

	/** Flip the boolean settings */
	void flipSetting(BoolSettingIndex index) { 
		ConfMan.setBool(_boolKey[index], !getBoolSetting(index));
	}

	/** Set the integer settings */
	void setIntSetting(IntSettingIndex index, int value);

	/** Check whether low-resolution fmv is available */
	bool hasLowResFMV() { return _hasLowRes; }

	/** Enable the book of secrets */
	void enableBookOfSecrets() { 
		ConfMan.setBool("xoBfOsterceS", true);
		ConfMan.flushToDisk();
	}

	/** Check whether the book of secrets is enabled */
	bool hasBookOfSecrets() { return ConfMan.hasKey("xoBfOsterceS"); }

	/** Should the game try to load external replacement assets? */
	bool isAssetsModEnabled() const;

	/**
	 * Should the engine apply alpha pre-multiplication when loading replacement PNGs
	 *
	 * When rendering, textures are expected to be in pre-multiplied alpha format.
	 * It's best to have the PNGs in that format on file to speed up loading by removing
	 * the need to convert them. However this option enables the conversion when loading
	 * the files to they can be stored with regular alpha transparency for convenience
	 * when testing.
	 */
	bool shouldPreMultiplyReplacementPNGs() const;

	/** Should linear filtering be used when sampling the background image textures? */
	Gfx::Texture::SamplingFilter getImageSamplingFilter() const;

	/** The codepage text is encoded in or this version of the game */
	Common::CodePage getTextCodePage() const;

	/** Should TrueType fonts be rendered with anti-aliasing? */
	bool isFontAntialiasingEnabled() const;

	/**
	 * Should the font settings from 'gui.ini' be ignored.
	 *
	 * Some versions of the game, especially the GOG.com version have a version of 'gui.ini'
	 * that causes poor visuals. We just ignore the settings from the game and use the
	 * default values from ScummVM.
	 */
	bool shouldIgnoreFontSettings() const;

private:
	Audio::Mixer *_mixer;
	bool _hasLowRes;
	const bool _isDemo;
	const Common::Language _language;

	const char *_boolKey[6];
	const char *_intKey[4];
};

} // End of namespace Stark

#endif // STARK_SERVICES_SETTINGS_H
