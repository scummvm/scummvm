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

namespace Prisoner {

const PlainGameDescriptor prisonerGames[] = {
	{"prisoner", "Prisoner of Ice"},
	{ 0, 0 }
};

// Every entry is detected by its language file (?_KLANG.BIN), the engine takes the language
// letter from it.
const ADGameDescription gameDescriptions[] = {
	// Prisoner GOG version
	{
		"prisoner",
		0,
		AD_ENTRY1s("a_klang.bin", "f20001d885aee4e6320052a4368bf8cf", 561),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)},

	// Prisoner English version
	{
		"prisoner",
		0,
		AD_ENTRY1s("e_klang.bin", "898cc3ba5b382cf9eed2918bea56ac64", 561),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)
	},

	// Prisoner Multilingual Version - English
	{
		"prisoner",
		0,
		AD_ENTRY1s("e_klang.bin", "c8e85c96425a2c5bd535410ef39fd9fe", 561),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)
	},

	// Prisoner Multilingual Version - German
	{
		"prisoner",
		0,
		AD_ENTRY1s("d_klang.bin", "2c5e71abf3c7908f80db62fe850a459b", 561),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)
	},

	// Prisoner Multilingual Version - French
	{
		"prisoner",
		0,
		AD_ENTRY1s("f_klang.bin", "08866dc389a00b2c3c611b782c68220c", 561),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)
	},

	// Prisoner Multilingual Version - Dutch
	{
		"prisoner",
		0,
		AD_ENTRY1s("n_klang.bin", "dd8bcbe13bbf2cc2f5039dd4254b74b7", 561),
		Common::NL_NLD,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)
	},

	// Prisoner Multilingual Version - Swedish
	{
		"prisoner",
		0,
		AD_ENTRY1s("w_klang.bin", "68ee7ecba3965975bb857d41026be4b7", 561),
		Common::SV_SWE,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO4(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MUSIC, GUIO_MIDIGM, GUIO_MIDIADLIB)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Prisoner
