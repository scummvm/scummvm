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

namespace Alg {

static const AlgGameDescription gameDescriptions[] = {
	{
		// Crime Patrol (v1.00) (DOS)
		{
			"cpatrol",
			"",
			AD_ENTRY1s("CPDS.LIB", "43579f72207298f154f6fb2b1a24e193", 303710700),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_SINGLE_SPEED_VERSION)
		},
		GType_CRIME_PATROL,
	},
	{
		// Crime Patrol Demo (DOS)
		{
			"cpatrol",
			"Demo",
			AD_ENTRY1s("CP.LIB", "0621e198afb7be96279beec770cd8461", 16859660),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO0()
		},
		GType_CRIME_PATROL,
	},
	{
		// Drug Wars (v1.00) (DOS)
		{
			"dwars",
			"",
			AD_ENTRY1s("DWDS.LIB", "f00bc0d980eac72b6bbfa691808b62ae", 320739868),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_SINGLE_SPEED_VERSION)
		},
		GType_DRUG_WARS,
	},
	{
		// Drug Wars Demo (DOS)
		{
			"dwars",
			"Demo",
			AD_ENTRY1s("DWDEMO.LIB", "1f0cf57c8aeb326c37777c4ad82e7889", 24435449),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO0()
		},
		GType_DRUG_WARS,
	},
	{
		// Who Shot Johnny Rock? (v1.00) (DOS)
		{
			"johnroc",
			"",
			AD_ENTRY1s("JOHNROCD.LIB", "93c38b5fc7d1ae6e9dccf4f7a1c313a8", 326535618),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_SINGLE_SPEED_VERSION)
		},
		GType_WSJR,
	},
	{
		// The Last Bounty Hunter (v1.00) (DOS)
		{
			"lbhunter",
			"",
			AD_ENTRY1s("BHDS.LIB", "6fad52a6a72830ab3373cbe3e0a3a779", 281473503),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_LAST_BOUNTY_HUNTER,
	},
	{
		// The Last Bounty Hunter Demo (DOS)
		{
			"lbhunter",
			"demo",
			AD_ENTRY1s("BHDEMO.LIB", "af5fbbd5e18d96225077eb6bf2cac680", 28368775),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GAMEOPTION_SINGLE_SPEED_VERSION)
		},
		GType_LAST_BOUNTY_HUNTER,
	},
	{
		// Mad Dog McCree (v1.03a) (DOS)
		{
			"maddog",
			"",
			AD_ENTRY1s("MADDOG.LIB", "df27e760531dba600cb3ebc23a2d98d1", 114633310),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_MADDOG,
	},
	{
		// Mad Dog II: The Lost Gold (v1.00) (DOS)
		{
			"maddog2",
			"",
			AD_ENTRY1s("MADDOG2D.LIB", "1660b1728573481483c50206ad92a0ca", 291119013),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_SINGLE_SPEED_VERSION)
		},
		GType_MADDOG2,
	},
	{
		// Space Pirates (v1.00) (DOS)
		{
			"spirates",
			"",
			AD_ENTRY1s("SPDS.LIB", "223d3a339d542905c437a6a63cf6dbd8", 273506701),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_SINGLE_SPEED_VERSION)
		},
		GType_SPACE_PIRATES,
	},
	{
		// Space Pirates Demo (DOS)
		{
			"spirates",
			"Demo",
			AD_ENTRY1s("SP.LIB", "a1a1b7c9ed28ff2484ab8362825c3973", 14556553),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO0()
		},
		GType_SPACE_PIRATES,
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Alg
