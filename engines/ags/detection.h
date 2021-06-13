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

struct PluginVersion {
	const char *_plugin;
	int _version;
};

struct AGSGameDescription {
	ADGameDescription desc;
	const PluginVersion *_plugins;
};

extern const PlainGameDescriptor GAME_NAMES[];

extern const AGSGameDescription GAME_DESCRIPTIONS[];

enum AGSSteamVersion { kAGSteam = 0, kWadjetEye = 1 };
enum AGSSpriteFontVersion { kAGSSpriteFont = 0, kClifftopGames = 1 };

} // namespace AGS


class AGSMetaEngineDetection : public AdvancedMetaEngineDetection {
	mutable Common::String _gameid;
	mutable Common::String _extra;
	mutable Common::String _filename;
	mutable Common::String _md5;

	static const DebugChannelDef debugFlagList[];

public:
	AGSMetaEngineDetection();
	~AGSMetaEngineDetection() override {}

	const char *getEngineId() const override {
		return "ags";
	}

	const char *getName() const override {
		return "Adventure Game Studio";
	}

	const char *getOriginalCopyright() const override {
		return "AGS Engine (C) Chris Jones";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGames detectGames(const Common::FSList &fslist) const override;

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra = nullptr) const override;

	bool canPlayUnknownVariants() const override {
		return true;
	}

	GUI::OptionsContainerWidget *buildEngineOptionsWidgetStatic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

#endif
