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

#ifndef AGS_DETECTION_H
#define AGS_DETECTION_H

#include "engines/advancedDetector.h"

namespace AGS {

enum AGSDebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugPath     = 1 << 1,
	kDebugScan     = 1 << 2,
	kDebugFilePath = 1 << 3,
	kDebugScript   = 1 << 4
};

enum GameFlag {
	GAMEFLAG_FORCE_AA = 1,
	GAMEFLAG_INSTALLER = 2,
};

struct PluginVersion {
	const char *_plugin;
	int _version;

	uint32 sizeBuffer() const {
		uint32 ret = 0;
		ret += ADDynamicDescription::strSizeBuffer(_plugin);
		return ret;
	}
	void *toBuffer(void *buffer) {
		buffer = ADDynamicDescription::strToBuffer(buffer, _plugin);
		return buffer;
	}
};

struct AGSGameDescription {
	ADGameDescription desc;
	const PluginVersion *_plugins;
	const char *_mainNameInsideInstaller;

	uint32 sizeBuffer() const {
		uint32 ret = desc.sizeBuffer();
		if (_plugins) {
			const PluginVersion *p;
			for (p = _plugins; p->_plugin != nullptr; p++) {
				ret += p->sizeBuffer();
			}
			// Make space for alignment
			ret += ADDynamicDescription::alignSizeBuffer();
			// Add all plugins plus the final element
			ret += sizeof(*p) * (p - _plugins + 1);
		}
		ret += ADDynamicDescription::strSizeBuffer(_mainNameInsideInstaller);
		return ret;
	}

	void *toBuffer(void *buffer) {
		buffer = desc.toBuffer(buffer);
		if (_plugins) {
			const PluginVersion *p;
			for (p = _plugins; p->_plugin != nullptr; p++)
				;
			uint count = (p - _plugins + 1);

			// Align for pointers
			buffer = ADDynamicDescription::alignToBuffer(buffer);

			memcpy(buffer, _plugins, sizeof(*p) * count);

			_plugins = (PluginVersion *)buffer;

			PluginVersion *dp = (PluginVersion *)buffer;
			buffer = (PluginVersion *)buffer + count;

			for (; dp->_plugin != nullptr; dp++) {
				buffer = dp->toBuffer(buffer);
			}
		}
		buffer = ADDynamicDescription::strToBuffer(buffer, _mainNameInsideInstaller);
		return buffer;
	}
};

extern const PlainGameDescriptor GAME_NAMES[];

extern const AGSGameDescription GAME_DESCRIPTIONS[];

enum AGSSteamVersion { kAGSteam = 0, kWadjetEye = 1 };
enum AGSSpriteFontVersion { kAGSSpriteFont = 0, kClifftopGames = 1 };

#define GAMEOPTION_NO_SAVE_THUMBNAIL GUIO_GAMEOPTIONS1
#define GAMEOPTION_NO_AUTOSAVE		 GUIO_GAMEOPTIONS2
#define GAMEOPTION_NO_SAVELOAD		 GUIO_GAMEOPTIONS3

} // namespace AGS


class AGSMetaEngineDetection : public AdvancedMetaEngineDetection<AGS::AGSGameDescription> {
	mutable Common::String _gameid;
	mutable Common::String _extra;
	mutable Common::Path _filename;
	mutable Common::String _md5;

	static const DebugChannelDef debugFlagList[];

public:
	AGSMetaEngineDetection();
	~AGSMetaEngineDetection() override {}

	const char *getName() const override {
		return "ags";
	}

	const char *getEngineName() const override {
		return "Adventure Game Studio";
	}

	const char *getOriginalCopyright() const override {
		return "AGS Engine (C) Chris Jones";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override;

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra = nullptr) const override;
};

#endif
