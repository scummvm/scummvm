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

#include "base/plugins.h"
#include "common/file.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "drascula/detection.h"

static const PlainGameDescriptor drasculaGames[] = {
	{"drascula", "Drascula: The Vampire Strikes Back"},
	{0, 0}
};

namespace Drascula {

static const DrasculaGameDescription gameDescriptions[] = {

	//// Packed versions //////////////////////////////////////////////////////

	{
		// Drascula English version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				// HACK: List packet.001 twice to ensure this detector entry
				// is ranked just as high as the others (which each have two
				// detection files).
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.002", 1, "4401123400f22f212b89f15fb4b43013", 721122},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.002", 1, "7b83cedb9bb326ed5143e5c459508d43", 722383},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula German version (original packed files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.003", 1, "e8f4dc6091037329bab4ddb1cba35807", 719728},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version (original packed version)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "0253e924af223f5fe52537023385159b", 32564209),
			Common::IT_ITA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.005", 1, "58caac54b891f5d7f335e710e45e5d29", 16209623},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Spanish version (original packed version)
		{
			"drascula",
			0,
			AD_ENTRY1s("packet.001", "3c971aba65a037d29d0b479cad6f5943", 31702652),
			Common::ES_ESP,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Spanish version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.004", 1, "a289d3cf80d50f25ec569b653248437e", 17205838},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Russian version (ScummVM repacked files)
		{
			"drascula",
			0,
			{
				{"packet.001", 0, "c6a8697396e213a18472542d5f547cb4", 32847563},
				{"packet.006", 1, "e464b99de7f226391337510d5c328258", 697173},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	//// Unpacked versions ////////////////////////////////////////////////////

	{
		// Drascula English version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "09b2735953edcd43af115c65ae00b10e", 1595),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "eeeee96b82169003630e08992248296c", 608),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula French version (updated - bug #3612236)
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "1f9fbded768bee061cc22bc5bdeab540", 611),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula German version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "72e46089033d56bad1c179ac36e2a9d2", 610),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "02b49a18328d0bf2efe6ba658c9c7a1d", 2098),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Italian version (updated - bug #3612236)
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "ccaee939bb3b344c048f28f9205710d1", 2925),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{
		// Drascula Spanish version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "0746ed1a5cc8d9728f790c29813f4b43", 23059),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},
	{
		// Drascula Russian version
		{
			"drascula",
			0,
			AD_ENTRY1s("14.ald", "4dfab170eae935a2e9889196df427a4a", 1426),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_LINKSPEECHTOSFX)
		},
	},

	{ AD_TABLE_END_MARKER }
};

static const ExtraGuiOption drasculaExtraGuiOption = {
	_s("Use original save/load screens"),
	_s("Use the original save/load screens instead of the ScummVM ones"),
	"originalsaveload",
	false
};

class DrasculaMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	DrasculaMetaEngineDetection() : AdvancedMetaEngineDetection(Drascula::gameDescriptions, sizeof(Drascula::DrasculaGameDescription), drasculaGames) {
		_guiOptions = GUIO1(GUIO_NOMIDI);
	}

	const char *getEngineId() const override {
		return "drascula";
	}

	const char *getName() const override {
		return "Drascula: The Vampire Strikes Back";
	}

	const char *getOriginalCopyright() const override {
		return "Drascula: The Vampire Strikes Back (C) 2000 Alcachofa Soft, (C) 1996 Digital Dreams Multimedia, (C) 1994 Emilio de Paz";
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

const ExtraGuiOptions DrasculaMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(drasculaExtraGuiOption);
	return options;
}

} // End of namespace Drascula

REGISTER_PLUGIN_STATIC(DRASCULA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Drascula::DrasculaMetaEngineDetection);
