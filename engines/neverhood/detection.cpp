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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"
#include "common/translation.h"

#include "neverhood/detection.h"

static const PlainGameDescriptor neverhoodGames[] = {
	{"neverhood", "The Neverhood Chronicles"},
	{nullptr, nullptr}
};

namespace Neverhood {

static const ADGameDescription gameDescriptions[] = {

	// Neverhood English version
	{
		"neverhood",
		nullptr,
		AD_ENTRY1s("hd.blb", "22958d968458c9ff221aee38577bb2b2", 4279716),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood English big demo version
	{
		"neverhood",
		"Big Demo",
		AD_ENTRY1s("nevdemo.blb", "e637221d296f9a25ff22eaed96b07519", 117274189),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_BIG_DEMO | ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood English demo version
	{
		"neverhood",
		"Demo",
		AD_ENTRY1s("nevdemo.blb", "05b735cfb1086892bec79b54dca5545b", 22564568),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood earlier English demo version
	{
		"neverhood",
		"Demo",
		AD_ENTRY1s("nevdemo.blb", "9cbc33bc8ebacacfc8071f3e26a9c85f", 22357020),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood lite English demo version
	{
		"neverhood",
		"Demo",
		AD_ENTRY1s("nevdemo.blb", "816741610771f6434a673b6821e88899", 11986100),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood Russian version. Dyadyushka Risech
	{
		"neverhood",
		"DR",
		AD_ENTRY1s("hd.blb", "787951bf094aad9962291e69a707bdde", 4248635),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood Russian version. Fargus
	{
		"neverhood",
		"Fargus",
		AD_ENTRY1s("hd.blb", "c87c69db423f560d3708e9de78751a7f", 4425816),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Neverhood Japanese version
	// Bugreport #11074
	{
		"neverhood",
		_s("Missing game code"), // Reason for being unsupported
		AD_ENTRY1s("hd.blb", "c791725bbbc23c0f8bf78eece4555565", 4308928),
		Common::JA_JPN,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | ADGF_UNSUPPORTED,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Neverhood

static const ExtraGuiOption neverhoodExtraGuiOption1 = {
	_s("Use original save/load screens"),
	_s("Use the original save/load screens instead of the ScummVM ones"),
	"originalsaveload",
	false
};

static const ExtraGuiOption neverhoodExtraGuiOption2 = {
	_s("Skip the Hall of Records storyboard scenes"),
	_s("Allows the player to skip past the Hall of Records storyboard scenes"),
	"skiphallofrecordsscenes",
	false
};

static const ExtraGuiOption neverhoodExtraGuiOption3 = {
	_s("Scale the making of videos to full screen"),
	_s("Scale the making of videos, so that they use the whole screen"),
	"scalemakingofvideos",
	false
};


class NeverhoodMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	NeverhoodMetaEngineDetection() : AdvancedMetaEngineDetection(Neverhood::gameDescriptions, sizeof(ADGameDescription), neverhoodGames) {
		_guiOptions = GUIO2(GUIO_NOSUBTITLES, GUIO_NOMIDI);
	}

	const char *getEngineId() const override {
		return "neverhood";
	}

	const char *getName() const override {
		return "The Neverhood Chronicles";
	}

	const char *getOriginalCopyright() const override {
		return "The Neverhood Chronicles (C) The Neverhood, Inc.";
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

const ExtraGuiOptions NeverhoodMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(neverhoodExtraGuiOption1);
	options.push_back(neverhoodExtraGuiOption2);
	options.push_back(neverhoodExtraGuiOption3);
	return options;
}

REGISTER_PLUGIN_STATIC(NEVERHOOD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, NeverhoodMetaEngineDetection);
