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

namespace Bolt {

const PlainGameDescriptor boltGames[] = {
	{ "carnival", "Cartoon Carnival" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"carnival",
		nullptr,
		{
			{"BOOTHS.BLT", 0, "db13e86a850b9844bf1bab19e9ec2f6f", 1326163},
			{"FRED.BLT",   0, "19540d4ebce8bedd6e5d8523ad82b333", 302133 },
			{"GEORGE.BLT", 0, "57cfd68c99dfb13f139886eaf36b09ca", 1017533},
			{"HUCK.BLT",   0, "c4587a4d613616bfde5e0af9ccf8465e", 1225354},
			{"SCOOBY.BLT", 0, "c0918e36e532a24e2fc4942261d907aa", 3733392},
			{"TOPCAT.BLT", 0, "33ed048eef63c54962ef37998cc19366", 579091 },
			{"YOGI.BLT",   0, "e47d619a79b94f0dfff72a49599fdbcb", 5533358},
			AD_LISTEND
		},
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"carnival",
		nullptr,
		{
			{"ASSETS/BOOTHS.BLT", 0, "db13e86a850b9844bf1bab19e9ec2f6f", 1326163},
			{"ASSETS/FRED.BLT",   0, "19540d4ebce8bedd6e5d8523ad82b333", 302133 },
			{"ASSETS/GEORGE.BLT", 0, "57cfd68c99dfb13f139886eaf36b09ca", 1017533},
			{"ASSETS/HUCK.BLT",   0, "c4587a4d613616bfde5e0af9ccf8465e", 1225354},
			{"ASSETS/SCOOBY.BLT", 0, "c0918e36e532a24e2fc4942261d907aa", 3733392},
			{"ASSETS/TOPCAT.BLT", 0, "33ed048eef63c54962ef37998cc19366", 579091 },
			{"ASSETS/YOGI.BLT",   0, "e47d619a79b94f0dfff72a49599fdbcb", 5533358},
			AD_LISTEND
		},
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"carnival",
		"Demo",
		{
			{"BOOTHS.BLT", 0, "3fc80c0feaaa300720a3ed921496233a", 878177 },
			{"GEORGE.BLT", 0, "ca29738fe8655b9adec81aa48914c213", 388257 },
			{"SCOOBY.BLT", 0, "4dfbdd87fdd2ec05fbdc7ae6435fb239", 677438 },
			{"YOGI.BLT",   0, "06da5c2b889b5ecca371e99d8209347c", 5209352},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},
	{
		"carnival",
		"Demo",
		{
			{"ASSETS/BOOTHS.BLT", 0, "3fc80c0feaaa300720a3ed921496233a", 878177 },
			{"ASSETS/GEORGE.BLT", 0, "ca29738fe8655b9adec81aa48914c213", 388257 },
			{"ASSETS/SCOOBY.BLT", 0, "4dfbdd87fdd2ec05fbdc7ae6435fb239", 677438 },
			{"ASSETS/YOGI.BLT",   0, "06da5c2b889b5ecca371e99d8209347c", 5209352},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Bolt
