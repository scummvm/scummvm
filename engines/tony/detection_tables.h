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

namespace Tony {

static const TonyGameDescription gameDescriptions[] = {
	{
		// Tony Tough English
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "57c4a3860cf899443c357e0078ea6f49", 366773 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "aebc6eb607ee19cc94bfe9c11898bb8c", 243003502 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Czech
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "a8283a101878f3ca105f1f83f07e2c40", 386491 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "3384bdcb70d1e1ecedbde26e79683ede", 299019523 },
				AD_LISTEND
			},
			Common::CS_CZE,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough French "Collection Aventure" provided by Strangerke
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "e890c6a41238827bdfa9874a65618b69", 374135 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "aebc6eb607ee19cc94bfe9c11898bb8c", 243003502 },
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Spanish provided by Pakolmo
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "bcca7985db2fba9c1c4a0886618ec835", 515967 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb", 0, "aebc6eb607ee19cc94bfe9c11898bb8c", 243003502 },
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough German "Shoe Box" provided by Strangerke
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "ccf7ab939a34de1b13df538596431684", 389554 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "6a3c2f5426ab762bf4dc9826796aa320", 279745055 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough German "Gamestar" provided in bug #6138
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "187de6f88f4083808cb66342ab55a7fd", 389904 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, 0, -1 },	// FIXME
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Italian provided by Fabio Barzagli
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "1dc896cdb945170d7408598f803411c1", 380001 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "f9f1ac8f63a909bb3ed972490dae65c4", 286130226 },
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Italian provided by Giovanni Bajo
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "6202816f991b15af82aab84e3e4be011", 380183 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "f9f1ac8f63a909bb3ed972490dae65c4", 286130226 },
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Polish provided by iGom bug #11546
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "c212a81e34edf92bc177a80f24780bd2", 380200 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "544b1db3a41b0f89567267d0664183bb", 321349288 },
				AD_LISTEND
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Polish provided by Fabio Barzagli
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "89733ea710669acc8e7900b115f4afef", 389625 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "544b1db3a41b0f89567267d0664183bb", 310906270 },
				AD_LISTEND
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Russian, reported in bug #6589
		{
			"tony",
			0,
			{
				{ "roasted.mpc", 0, "377d6e24adeedc6c5c09c31b92231218", 391536 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071 },
				{ "voices.vdb",  0, "af4061f49b934086710f0d41e6250a15", 325225827 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough English Demo
		{
			"tony",
			"Extracted Demo",
			{
				{ "roasted.mpc", 0, "1e247922ec869712bfd96625bc4d3c7c", 39211 },
				{ "roasted.mpr", 0, "06203dbbc85fdd1e6dc8fc211c1a6207", 14972409 },
				{ "voices.vdb",  0, "f9f1ac8f63a909bb3ed972490dae65c4", 20189260 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},

#if 0
	// Disabling this detection entry. The package contains _only_ standard list
	// of InnoSetup files:
	//
	// DATA.TAG     SETUP.INI    _ISDEL.EXE   _sys1.cab    data1.cab    layout.bin
	// setup.bmp    setup.lid    SETUP.EXE    _INST32I.EX_ _setup.dll   _user1.cab
	// lang.dat     os.dat       setup.ins
	//
	// As a result, adding this entry will lead to a false "unknown variant for tony-demo"
	// For practically all games which use InnoSetup.
	//
	// The current AdvancedDetector code always will show the end user the detected
	// game AND this entry, which is very annoying
	//
	// The only potential solution is to make AD work with the InnoSetup archives,
	// that is, make it look inside of the archive and if it contains the matching
	// file names, only then report it as an unknown variant.
	{
		// Tony Tough English Demo (Compressed)
		{
			"tony",
			"Demo",
			{
				{ "data1.cab", 0, "7d8b6d308f96aee3968ad7910fb11e6d", 58660608 },
				{ "data.tag", 0, "e9af151040745e83081e691356abeed7", 137 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | GF_COMPRESSED | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
	},
#endif

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Tony
