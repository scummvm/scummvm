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

namespace {

#define FLAGS(id, a, b, c) { id, a, Common::UNK_LANG, Common::kPlatformUnknown, b, c, Common::UNK_LANG, Common::UNK_LANG }
#define FLAGS_FAN(id, a, b, c, fanLang, repLang) { id, a, Common::UNK_LANG, Common::kPlatformUnknown, b, c, fanLang, repLang }

#define SNATCHER_FLAGS(a, b, c) FLAGS(Snatcher::GI_SNATCHER, a, b, c)

const SnatcherGameDescription adGameDescs[] = {
	{
		{
			"snatcher",
			"US",
			AD_ENTRY2s(	"DATA_B0.BIN",	"63170d11da5ebc3a66ecf0fc2d23e78a", 24176,
						"SP05.BIN",		"e0486f6c73778cddd11f9618776125f4", 35756),
			Common::EN_ANY,
			Common::kPlatformSegaCD,
			ADGF_TESTING,
			GUIO2(GUIO_NOSPEECHVOLUME, GUIO_MIDISEGACD)
		},
		SNATCHER_FLAGS(MDT_SEGACD, true, false)
	},
	{
		{
			"snatcher",
			"EU",
			AD_ENTRY2s(	"DATA_B0.BIN",	"63170d11da5ebc3a66ecf0fc2d23e78a", 24176,
						"SP05.BIN",		"a4ee2ea44629f64bd5d04e0d3160d7a5", 35756),
			Common::EN_ANY,
			Common::kPlatformSegaCD,
			ADGF_TESTING,
			GUIO2(GUIO_NOSPEECHVOLUME, GUIO_MIDISEGACD)
		},
		SNATCHER_FLAGS(MDT_SEGACD, true, true)
	},

	{ AD_TABLE_END_MARKER, FLAGS(0, MDT_NONE, false, false) }
};

const PlainGameDescriptor gameList[] = {
	{ "snatcher", "Snatcher" },
	{ 0, 0 }
};

} // End of anonymous namespace
