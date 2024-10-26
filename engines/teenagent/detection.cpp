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

#include "common/algorithm.h"

#include "base/plugins.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "teenagent/teenagent.h"

static const DebugChannelDef debugFlagList[] = {
	{TeenAgent::kDebugActor, "Actor", "Enable Actor Debug"},
	{TeenAgent::kDebugAnimation, "Animation", "Enable Animation Debug"},
	{TeenAgent::kDebugCallbacks, "Callbacks", "Enable Callbacks Debug"},
	{TeenAgent::kDebugDialog, "Dialog", "Enable Dialog Debug"},
	{TeenAgent::kDebugFont, "Font", "Enable Font Debug"},
	{TeenAgent::kDebugInventory, "Inventory", "Enable Inventory Debug"},
	{TeenAgent::kDebugMusic, "Music", "Enable Music Debug"},
	{TeenAgent::kDebugObject, "Object", "Enable Object Debug"},
	{TeenAgent::kDebugPack, "Pack", "Enable Pack Debug"},
	{TeenAgent::kDebugScene, "Scene", "Enable Scene Debug"},
	{TeenAgent::kDebugSurface, "Surface", "Enable Surface Debug"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor teenAgentGames[] = {
	{ "teenagent", "Teen Agent" },
	{ 0, 0 }
};

static const ADGameDescription teenAgentGameDescriptions[] = {
	{
		"teenagent",
		"",
		{
			{"off.res",     0, "c5263a726d038bb6780a40eb3b83cc87", 2720432},
			{"on.res",      0, "a0d5e5bbf6fab4bdc7f4094ed85f9639", 153907},
			{"ons.res",     0, "a7e2e8def1f0fb46644c20686af0d91a", 173077},
			{"varia.res",   0, "b5ba6925029c7bc285283da8c2d3042d", 209315},
			{"lan_000.res", 0, "06e409b0a43ff0ced014b93fb8f5dd5b", 535599},
			{"lan_500.res", 0, "c098cc17cc27a1cad4319fb6789aa5a7", 9538457},
			{"mmm.res",     0, "d25033d9bc88662d680b56825e892e5c", 42104},
			{"sam_mmm.res", 0, "a0878ad9a1af39d515e2e0471222f080", 229636},
			{"sam_sam.res", 0, "547a48cc1be9cf30744de8b0b47838f2", 769552},
			{"sdr.res",     0, "434c62c1f43b7aa4def62ff276163edb", 14672},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},
	{
		"teenagent",
		"Alt version",
		{
			{"off.res",     0, "c5263a726d038bb6780a40eb3b83cc87", 2720432},
			{"on.res",      0, "a0d5e5bbf6fab4bdc7f4094ed85f9639", 153907},
			{"ons.res",     0, "a7e2e8def1f0fb46644c20686af0d91a", 173077},
			{"varia.res",   0, "b786c48e160e1981b496a30acd3deff9", 216683},
			{"lan_000.res", 0, "06e409b0a43ff0ced014b93fb8f5dd5b", 535599},
			{"lan_500.res", 0, "c098cc17cc27a1cad4319fb6789aa5a7", 9538457},
			{"mmm.res",     0, "d25033d9bc88662d680b56825e892e5c", 42104},
			{"sam_mmm.res", 0, "a0878ad9a1af39d515e2e0471222f080", 229636},
			{"sam_sam.res", 0, "547a48cc1be9cf30744de8b0b47838f2", 769552},
			{"sdr.res",     0, "434c62c1f43b7aa4def62ff276163edb", 14665},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},
	{ // Russian fan translation
		"teenagent",
		"",
		{
			{"off.res",     0, "c5263a726d038bb6780a40eb3b83cc87", 2720432},
			{"on.res",      0, "a0d5e5bbf6fab4bdc7f4094ed85f9639", 153907},
			{"ons.res",     0, "a7e2e8def1f0fb46644c20686af0d91a", 173077},
			{"varia.res",   0, "b786c48e160e1981b496a30acd3deff9", 216683},
			{"lan_000.res", 0, "06e409b0a43ff0ced014b93fb8f5dd5b", 535599},
			{"lan_500.res", 0, "c098cc17cc27a1cad4319fb6789aa5a7", 9538457},
			{"mmm.res",     0, "d25033d9bc88662d680b56825e892e5c", 42104},
			{"sam_mmm.res", 0, "a0878ad9a1af39d515e2e0471222f080", 229636},
			{"sam_sam.res", 0, "547a48cc1be9cf30744de8b0b47838f2", 769552},
			{"cdlogo.res",  0, "d1aacbb7deb718f9d946ba9deec6271d", 64768},
			{"sdr.res",     0, "434c62c1f43b7aa4def62ff276163edb", 14665},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},
	{ // Czech Floppy
		"teenagent",
		"",
		{
			{"off.res",     0, "c5263a726d038bb6780a40eb3b83cc87", 2720432},
			{"on.res",      0, "a0d5e5bbf6fab4bdc7f4094ed85f9639", 153907},
			{"ons.res",     0, "a7e2e8def1f0fb46644c20686af0d91a", 173077},
			{"varia.res",   0, "b786c48e160e1981b496a30acd3deff9", 216683},
			{"lan_000.res", 0, "06e409b0a43ff0ced014b93fb8f5dd5b", 535599},
			{"lan_500.res", 0, "c098cc17cc27a1cad4319fb6789aa5a7", 9538457},
			{"mmm.res",     0, "d25033d9bc88662d680b56825e892e5c", 42104},
			{"sam_mmm.res", 0, "a0878ad9a1af39d515e2e0471222f080", 229636},
			{"sam_sam.res", 0, "547a48cc1be9cf30744de8b0b47838f2", 769552},
			{"sdr.res",     0, "434c62c1f43b7aa4def62ff276163edb", 14672},
			AD_LISTEND
		},
		Common::CS_CZE,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},
	{ // Polish CD
		"teenagent",
		"CD",
		{
			{"off.res",     0, "aaac839a6ef639d68ebc97bc42faa42d", 2720432},
			{"on.res",      0, "a0d5e5bbf6fab4bdc7f4094ed85f9639", 153907},
			{"ons.res",     0, "a7e2e8def1f0fb46644c20686af0d91a", 173077},
			{"varia.res",   0, "b786c48e160e1981b496a30acd3deff9", 216683},
			{"lan_000.res", 0, "06e409b0a43ff0ced014b93fb8f5dd5b", 535599},
			{"lan_500.res", 0, "c098cc17cc27a1cad4319fb6789aa5a7", 9538457},
			{"sam_sam.res", 0, "547a48cc1be9cf30744de8b0b47838f2", 769552},
			{"voices.res",  0, "955aa04517a2b0499adf17d9b7c6f4a1", 37306128},
			{"cdlogo.res",  0, "6bf95a48f366bdf8af3a198c7b723c77", 64768},
			{"sdr.res",     0, "d0b1398c78dc82571ddef5877c9a3a06", 14993},
			AD_LISTEND
		},
		Common::PL_POL,
		Common::kPlatformDOS,
		ADGF_CD,
		GUIO1(GUIO_NOMIDI)
	},
	{ // Polish Floppy
		"teenagent",
		"",
		{
			{"off.res",     0, "aaac839a6ef639d68ebc97bc42faa42d", 2720432},
			{"on.res",      0, "a0d5e5bbf6fab4bdc7f4094ed85f9639", 153907},
			{"ons.res",     0, "a7e2e8def1f0fb46644c20686af0d91a", 173077},
			{"varia.res",   0, "b786c48e160e1981b496a30acd3deff9", 216683},
			{"lan_000.res", 0, "06e409b0a43ff0ced014b93fb8f5dd5b", 535599},
			{"lan_500.res", 0, "c098cc17cc27a1cad4319fb6789aa5a7", 9538457},
			{"sam_sam.res", 0, "547a48cc1be9cf30744de8b0b47838f2", 769552},
			{"voices.res",  0, "955aa04517a2b0499adf17d9b7c6f4a1", 19376128},
			{"cdlogo.res",  0, "6bf95a48f366bdf8af3a198c7b723c77", 64768},
			{"sdr.res",     0, "d0b1398c78dc82571ddef5877c9a3a06", 14993},
			AD_LISTEND
		},
		Common::PL_POL,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{ // Demo
		"teenagent",
		"Demo",
		{
			{"off.res",     0, "441b7dde82ca84a829fc7fe9743e9b78", 906956},
			{"on.res",      0, "25dbb6eed0a80d98edff3c24f09f1ee0", 37654},
			{"ons.res",     0, "394fbd9418e43942127c45a326d10ee1", 50596},
			{"varia.res",   0, "b786c48e160e1981b496a30acd3deff9", 216683},
			{"lan_000.res", 0, "c7241846ec67dd249fe02610cb9b8425", 91729},
			{"lan_500.res", 0, "791e4058a4742abd7c03dc82272623a9", 2109796},
			{"mmm.res",     0, "afbab0a454860f4ccf23005d8d2f4a70", 27073},
			{"sam_mmm.res", 0, "85fcdd0d49062577acf4a9ddafc53283", 148783},
			{"sam_sam.res", 0, "dc39c65ec57ed70612670b7e780f3408", 342219},
			{"sdr.res",     0, "434c62c1f43b7aa4def62ff276163edb", 14672},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},
	{ // Demo alt
		"teenagent",
		"Alt Demo",
		{
			{"off.res",     0, "441b7dde82ca84a829fc7fe9743e9b78", 906956},
			{"on.res",      0, "25dbb6eed0a80d98edff3c24f09f1ee0", 37654},
			{"ons.res",     0, "394fbd9418e43942127c45a326d10ee1", 50596},
			{"varia.res",   0, "8ffe0a75b7299b44a34fdd3831cecacb", 217003},
			{"lan_000.res", 0, "c7241846ec67dd249fe02610cb9b8425", 91729},
			{"lan_500.res", 0, "791e4058a4742abd7c03dc82272623a9", 2109796},
			{"mmm.res",     0, "afbab0a454860f4ccf23005d8d2f4a70", 27073},
			{"sam_mmm.res", 0, "85fcdd0d49062577acf4a9ddafc53283", 148783},
			{"sam_sam.res", 0, "dc39c65ec57ed70612670b7e780f3408", 342219},
			{"sdr.res",     0, "434c62c1f43b7aa4def62ff276163edb", 14672},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER,
};



class TeenAgentMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	TeenAgentMetaEngineDetection() : AdvancedMetaEngineDetection(teenAgentGameDescriptions, teenAgentGames) {
	}

	const char *getName() const override {
		return "teenagent";
	}

	const char *getEngineName() const override {
		return "TeenAgent";
	}

	const char *getOriginalCopyright() const override {
		return "TEENAGENT (C) 1994 Metropolis";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(TEENAGENT_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TeenAgentMetaEngineDetection);
