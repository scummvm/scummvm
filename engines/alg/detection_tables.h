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

static const ADGameDescription gameDescriptions[] = {
	{
		// Crime Patrol (v1.00) (Single Speed CD-ROM Version)
		"cpatrols",
		"",
		AD_ENTRY1s("CPSS.LIB", "feddb53975c9832c0f54055c15350389", 193353403),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Crime Patrol (v1.00) (Double Speed CD-ROM Version)
		"cpatrold",
		"",
		AD_ENTRY1s("CPDS.LIB", "43579f72207298f154f6fb2b1a24e193", 303710700),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Crime Patrol Demo
		"cpatroldemo",
		"",
		AD_ENTRY1s("CP.LIB", "0621e198afb7be96279beec770cd8461", 16859660),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Drug Wars (v1.00) (Single Speed CD-ROM Version)
		"dwarss",
		"",
		AD_ENTRY1s("DWSS.LIB", "f041a2b106d3ba27b03b5695e5263172", 191903386),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Drug Wars (v1.00) (Double Speed CD-ROM Version)
		"dwarsd",
		"",
		AD_ENTRY1s("DWDS.LIB", "f00bc0d980eac72b6bbfa691808b62ae", 320739868),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Drug Wars Demo
		"dwarsdemo",
		"",
		AD_ENTRY1s("DWDEMO.LIB", "1f0cf57c8aeb326c37777c4ad82e7889", 24435449),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Who Shot Johnny Rock? (v1.00) (Single Speed CD-ROM Version)
		"johnrocs",
		"",
		AD_ENTRY1s("JOHNROC.LIB", "3cbf7843ef2fdf23716301dceaa2eb10", 141833752),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Who Shot Johnny Rock? (v1.00) (Double Speed CD-ROM Version)
		"johnrocd",
		"",
		AD_ENTRY1s("JOHNROCD.LIB", "93c38b5fc7d1ae6e9dccf4f7a1c313a8", 326535618),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// The Last Bounty Hunter (v1.00)
		"lbhunter",
		"",
		AD_ENTRY1s("BHDS.LIB", "6fad52a6a72830ab3373cbe3e0a3a779", 281473503),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// The Last Bounty Hunter Demo
		"lbhunterdemo",
		"",
		AD_ENTRY1s("BHDEMO.LIB", "af5fbbd5e18d96225077eb6bf2cac680", 28368775),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Mad Dog McCree (v1.03a)
		"maddog",
		"",
		AD_ENTRY1s("MADDOG.LIB", "df27e760531dba600cb3ebc23a2d98d1", 114633310),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Mad Dog II: The Lost Gold (v1.00) (Single Speed CD-ROM Version)
		"maddog2s",
		"",
		AD_ENTRY1s("MADDOG2.LIB", "7b54bca3932b28d8776eaed16a9f43b5", 185708043),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Mad Dog II: The Lost Gold (v1.00) (Double Speed CD-ROM Version)
		"maddog2d",
		"",
		AD_ENTRY1s("MADDOG2D.LIB", "1660b1728573481483c50206ad92a0ca", 291119013),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Space Pirates (v1.00) (Single Speed CD-ROM Version)
		"spiratess",
		"",
		AD_ENTRY1s("SPSS.LIB", "c006d9f85fd86024b57d69875f23c473", 175141152),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Space Pirates (v1.00) (Double Speed CD-ROM Version)
		"spiratesd",
		"",
		AD_ENTRY1s("SPDS.LIB", "223d3a339d542905c437a6a63cf6dbd8", 273506701),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		// Space Pirates Demo
		"spiratesdemo",
		"",
		AD_ENTRY1s("SP.LIB", "a1a1b7c9ed28ff2484ab8362825c3973", 14556553),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Alg
