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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/textconsole.h"

#include "efh/detection.h"
#include "efh/efh.h"

namespace Efh {

static const PlainGameDescriptor efhGames[] = {
	// Games
	{"efh", "Escape From Hell"},
	{nullptr, nullptr}
};

static const EfhGameDescription gameDescriptions[] = {

	// Escape From Hell English - Unpacked version
	{
		{"efh", nullptr, AD_ENTRY1s("escape.exe", "2702f8f713e113a853a925d29aecc709", 147312),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeEfh
	},
	// Escape From Hell English
	{
		{"efh", nullptr, AD_ENTRY1s("escape.exe", "1ca4ae3f2ea66c30d1ef3e257a86cd05", 141487),
		 Common::EN_ANY,
		 Common::kPlatformDOS,
		 ADGF_UNSTABLE,
		 GUIO0()},
		kGameTypeEfh},
	{AD_TABLE_END_MARKER, kGameTypeNone}
};

static const DebugChannelDef debugFlagList[] = {
	{Efh::kDebugEngine, "engine", "Engine debug level"},
	{Efh::kDebugUtils, "utils", "Utils debug level"},
	{Efh::kDebugGraphics, "graphics", "Graphics debug level"},
	DEBUG_CHANNEL_END};

class EfhMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	EfhMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(EfhGameDescription), efhGames) {
	}

	const char *getEngineName() const override {
		return "Efh";
	}

	const char *getName() const override {
		return "efh";
	}

	const char *getOriginalCopyright() const override {
		return "Escape From Hell (C) Electronic Arts, 1990";
	}
};

} // End of namespace efh

REGISTER_PLUGIN_STATIC(EFH_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Efh::EfhMetaEngineDetection);
