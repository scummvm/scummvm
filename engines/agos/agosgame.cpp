#define FILE_MD5_BYTES 5000

static const AGOSGameDescription gameDescriptions[] = {
	// Elvira 1 - English Amiga Floppy
	{
		{
			"elvira1",
			"Floppy",
			{
	{ "gameamiga",		GAME_BASEFILE,	"7bdaff4a118d8035047cf9b1393b3fa0"},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1"},
	{ "start",		GAME_RESTFILE,	"69fb4f12108b39ae659f108cad4d3efe"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED
	},

	// Elvira 1 - English Amiga Demo
	{
		{
			"elvira1",
			"Demo",

			{
	{ "agos.mdf",		GAME_MENUFILE,	"825bc8eecd599f4c26732902ba2c2c77"},
	{ "englishdemo",	GAME_BASEFILE,	"7bbc2dfe8619ef579004ff57674c6e92"},
	{ "icon.dat",		GAME_ICONFILE,	"68b329da9893e34099c7d8ad5cb9c940"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_DEMO
	},

	// Elvira 1 - English Atari ST Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamest",		GAME_BASEFILE,	"8942859018fcfb2dbed13e83d974d1ab"},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1"},
	{ "start",		GAME_RESTFILE,	"69fb4f12108b39ae659f108cad4d3efe"},
	{ "tbllist",		GAME_TBLFILE,	"5b6ff494bf7e24213758598ef4ac0a8b"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED
	},

	// Elvira 1 - English DOS Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"a49e132a1f18306dd5d1ec2fe435e178"},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45"},
	{ "start",		GAME_RESTFILE,	"69fb4f12108b39ae659f108cad4d3efe"},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 1 - German DOS Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"d0b593143e21fc150c044819df2c0b98"},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45"},
	{ "start",		GAME_RESTFILE,	"69fb4f12108b39ae659f108cad4d3efe"},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 1 - French DOS Floppy
	{
		{
			"elvira1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"9076d507d60cc454df662316438ec843"},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45"},
	{ "start",		GAME_RESTFILE,	"69fb4f12108b39ae659f108cad4d3efe"},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformPC
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - English Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"4aa163967f5d2bd319f8350d6af03186"},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d"},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955"},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b"},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e"},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED
	},

	// Elvira 2 - English Atari ST Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamest",		GAME_BASEFILE,	"1b1acd637d32bee79859b7cc9de070e7"},
	{ "icon.dat",		GAME_ICONFILE,	"9a4eaf4df0cdf5cc85a5134150f96589"},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955"},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b"},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e"},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAtariST
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED
	},

	// Elvira 2 - English DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"3313254722031b22d833a2cf45a91fd7"},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9"},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955"},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b"},
	{ "stripped.txt",	GAME_STRFILE,	"c2533277b7ff11f5495967d55355ea17"},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - English DOS Floppy, Alternate
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"1282fd5c520861ae2b73bf653afef547"},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9"},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955"},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b"},
	{ "stripped.txt",	GAME_STRFILE,	"c2533277b7ff11f5495967d55355ea17"},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - German DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"d1979d2fbc5fb5276563578ca55cbcec"},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9"},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955"},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b"},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7"},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Elvira 2 - French DOS Floppy
	{
		{
			"elvira2",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"4bf28ab00f5324fd938e632595742382"},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9"},
	{ "menus.dat",		GAME_MENUFILE,	"a2fdc88a77c8bdffec6b36cbeda4d955"},
	{ "start",		GAME_RESTFILE,	"4d380a35ba941d03ee5084c71d20055b"},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7"},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformPC
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE
	},

	// Waxworks - English Amiga Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"eca24fe7c3e005caca47cecac56f7245"},
	{ "icon.pkd",		GAME_ICONFILE,	"4822a91c18b1b2005ac17fc617f7dcbe"},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e"},
	{ "start",		GAME_RESTFILE,	"036b647973d6884cdfc2042a3d12df83"},
	{ "stripped.txt",	GAME_STRFILE,	"6faaebff2786216900061eeb978f10af"},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0"},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE | GF_CRUNCHED | GF_CRUNCHED_GAMEPC
	},

	// Waxworks - English DOS Floppy
	{
		{
			"waxworks",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"7751e9358e894e32ef40ef3b3bae0f2a"},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a"},
	{ "menus.dat",		GAME_MENUFILE,	"3409eeb8ca8b46fc04da99de67573f5e"},
	{ "roomslst",		GAME_RMSLFILE,	"e3758c46ab8f3c23a1ac012bd607108d"},
	{ "start",		GAME_RESTFILE,	"036b647973d6884cdfc2042a3d12df83"},
	{ "stripped.txt",	GAME_STRFILE,	"f259e3e07a1cde8d0404a767d815e12c"},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0"},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English Acorn CD Demo
	{
		{
			"simon1",
			"CD Demo",

			{
	{ "data", 	GAME_GMEFILE,	"b4a7526ced425ba8ad0d548d0ec69900"},
	{ "gamebase", 	GAME_BASEFILE,	"425c7d1957699d35abca7e12a08c7422"},
	{ "icondata", 	GAME_ICONFILE, 	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped", 	GAME_STRFILE,	"d9de7542612d9f4e0819ad0df5eac56b"},
	{ "tbllist", 	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAcorn
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Acorn CD
	{
		{
			"simon1",
			"CD",

			{
	{ "data", 	GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806"},
	{ "gamebase", 	GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6"},
	{ "icondata", 	GAME_ICONFILE, 	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped", 	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist", 	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAcorn
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Amiga ECS Floppy
	{
		{
			"simon1",
			"ECS Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"bb94a688e247695d912cce9d0173d73a"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"f5fc67db3b8c5283cda51c43b98a74f8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_32COLOR | GF_CRUNCHED | GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English Amiga ECS Demo
	{
		{
			"simon1",
			"ECS Demo",

			{
	{ "gameamiga",		GAME_BASEFILE,	"4696309eed9d7335c62ebb87a0f006ad"},
	{ "icon.pkd",		GAME_ICONFILE,	"ebc96af15bfaf75ba8210326b9260d2f"},
	{ "stripped.txt",	GAME_STRFILE,	"8edde5b9498dc9f31da1093028da467c"},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_32COLOR | GF_CRUNCHED | GF_CRUNCHED_GAMEPC | GF_OLD_BUNDLE | GF_DEMO
	},

	// Simon the Sorcerer 1 - English Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"6c9ad2ff571d34a4cf0c696cf4e13500"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_CRUNCHED | GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - French Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"bd9828b9d4e5d89b50fe8c47a8e6bc07"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"2297baec985617d0d5612a0124bac359"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_CRUNCHED | GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - German Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",

			{
	{ "gameamiga",		GAME_BASEFILE,	"a2de9553f3b73064369948b5af38bb30"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_CRUNCHED | GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English Amiga CD32
	{
		{
			"simon1",
			"CD32",

			{
	{ "gameamiga",		GAME_BASEFILE,	"bab7f19237cf7d7619b6c73631da1854"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1CD32,
		GF_TALKIE | GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English Amiga CD32 alternative?
	{
		{
			"simon1",
			"CD32",

			{
	{ "gameamiga",		GAME_BASEFILE,	"ec5358680c117f29b128cbbb322111a4"},
	{ "icon.pkd",		GAME_ICONFILE,	"8ce5a46466a4f8f6d0f780b0ef00d5f5"},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_SIMON1,
		GID_SIMON1CD32,
		GF_TALKIE | GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy Demo
	{
		{
			"simon1",
			"Floppy Demo",

			{
	{ "gdemo",		GAME_BASEFILE,	"2be4a21bc76e2fdc071867c130651439"},
	{ "icon.dat",		GAME_ICONFILE,	"55af3b4d93972bc58bfee38a86b76c3f"},
	{ "stripped.txt",	GAME_STRFILE,	"33a2e329b97b2a349858d6a093159eb7"},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE | GF_DEMO
	},

	// Simon the Sorcerer 1 - English DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"c392e494dcabed797b98cbcfc687b33a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy with Russian patch
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::RU_RUS,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom)
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"9f93d27432ce44a787eef10adb640870"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom) with Russian patch
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::RU_RUS,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - French DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"34759d0d4285a2f4b21b8e03b8fcefb3"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"aa01e7386057abc0c3e27dbaa9c4ba5b"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - German DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"063015e6ce7d90b570dbc21fe0c667b1"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - Italian DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"65c9b2dea57df84ef55d1eaf384ebd30"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::IT_ITA,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - Spanish DOS Floppy
	{
		{
			"simon1",
			"Floppy",

			{
	{ "gamepc",		GAME_BASEFILE,	"5374fafdea2068134f33deab225feed3"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::ES_ESP,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - English DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806"},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English DOS CD alternate?
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"c0b948b6821d2140f8b977144f21027a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"64f73e94639b63af846ac4a8a94a23d8"},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - French DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"3cfb9d1ff4ec725af9924140126cf69f"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"638049fa5d41b81fb6fb11671721b871"},
	{ "stripped.txt",	GAME_STRFILE,	"ef51ac74c946881ae4d7ca66cc7a0d1e"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - German DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"7db9912acac4f1d965a64bdcfc370ba1"},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Hebrew DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"bc66e9c0b296e1b155a246917133f71a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"a34b2c8642f2e3676d7088b5c8b3e884"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::HB_ISR,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Italian DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"52e315e0e02feca86d15cc82e3306b6c"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::IT_ITA,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Italian DOS CD alternate
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"104efd83c8f3edf545982e07d87f66ac"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			// FIXME: DOS version which uses WAV format
			Common::IT_ITA,
			Common::kPlatformWindows
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - Spanish DOS CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"439f801ba52c02c9d1844600d1ce0f5e"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"eff2774a73890b9eac533db90cd1afa1"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::ES_ESP,
			Common::kPlatformPC
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Windows CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"c7c12fea7f6d0bfd22af5cdbc8166862"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"b1b18d0731b64c0738c5cc4a2ee792fc"},
	{ "stripped.txt",	GAME_STRFILE,	"a27e87a9ba21212d769804b3df47bfb2"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - English Windows CD with Russian patch
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"4536a706412b36d628f12142bfa97af0"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"b1b18d0731b64c0738c5cc4a2ee792fc"},
	{ "stripped.txt",	GAME_STRFILE,	"a27e87a9ba21212d769804b3df47bfb2"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::RU_RUS,
			Common::kPlatformWindows
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 1 - German Windows CD
	{
		{
			"simon1",
			"CD",

			{
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"acd9cc438525b142d93b15c77a6f551b"},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformWindows
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English DOS Floppy
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"27c8e7feada80c75b70b9c2f6088d519"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"eefcc32b1f2c0482c1a59a963a146345"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0
	},

	// Simon the Sorcerer 2 - English DOS Floppy with Russian patch
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"7edfc633dd50f8caa719c478443db70b"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"eefcc32b1f2c0482c1a59a963a146345"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::RU_RUS,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate?
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"604d04315935e77624bd356ac926e068"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate? with Russian patch
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"eb8bde3685842a8fd38f60bc476ef8e9"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::RU_RUS,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0
	},

	// Simon the Sorcerer 2 - German DOS Floppy
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"eb6e3e37fe52993f948d7e2d6b869828"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"5fa9d080b04c610f526bd685be1bf747"},
	{ "stripped.txt",	GAME_STRFILE,	"fd30df01cc248ecbaef302af855e0212"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0
	},

	// Simon the Sorcerer 2 - Italian DOS Floppy
	{
		{
			"simon2",
			"Floppy",

			{
	{ "game32",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"f306a397565d7f13bec7ecf14c723de7"},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::IT_ITA,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0
	},

	// Simon the Sorcerer 2 - English DOS CD Demo
	{
		{
			"simon2",
			"CD Demo",

			{
	{ "gsptr30",		GAME_BASEFILE,	"3794c15887539b8578bacab694ccf08a"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"f8c9e6df1e55923a749e115ba74210c4"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD Demo
	{
		{
			"simon2",
			"CD Demo",

			{
	{ "gsptr30",		GAME_BASEFILE,	"7596ef2644fde56ee5fad0dcd483a01e"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"3f308f201f6b1ddf7c2704c1fc43a3e1"},
	{ "stripped.txt",	GAME_STRFILE,	"4c68cf64e581a9bd638a56c900b08bfe"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"8c301fb9c4fcf119d2730ccd2a565eb3"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"9c535d403966750ae98bdaf698375a38"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English DOS CD alternate?
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"8d6dcc65577e285dbca03ff6d7d9323c"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - French DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"43b3a04d2f0a0cbd1b024c814856561a"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"8af0e02c0c3344db64dffc12196eb59d"},
	{ "stripped.txt",	GAME_STRFILE,	"5ea27977b4d7dcfd50eb5074e162ebbf"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"0d05c3f4c06c9a4ceb3d2f5bc0b18e11"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"6c5fdfdd0eab9038767c2d22858406b2"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German DOS CD alternate?
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"ec9f0f24fd895e7ea72e3c8e448c0240"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Hebrew DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"952a2b1be23c3c609ba8d988a9a1627d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"a2b249a82ea182af09789eb95fb6c5be"},
	{ "stripped.txt",	GAME_STRFILE,	"de9dbc24158660e153483fa0cf6c3172"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::HB_ISR,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Italian DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"344aca58e5ad5e25c517d5eb1d85c435"},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			// FIXME: DOS version which uses WAV format
			Common::IT_ITA,
			Common::kPlatformWindows
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Spanish DOS CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"268dc322aa73bcf27bb016b8e8ceb889"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"4f43bd06b6cc78dbd25a7475ca964eb1"},
	{ "stripped.txt",	GAME_STRFILE,	"d13753796bd81bf313a2449f34d8b112"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::ES_ESP,
			Common::kPlatformPC
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Czech Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"eb8217f9ec4628d12ca606033146c48c"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"2d1074788501c55dcd9e59269ea0aaed"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::CZ_CZE,
			Common::kPlatformWindows
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - English Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"e749c4c103d7e7d51b34620ed76c5a04"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"9609a933c541fed2e00c6c3479d7c181"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformWindows
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - German Windows CD 1.1
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"9e858b3bb189c134c3a5f34c3385a8d3"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"16d574da07e93bcae43cee353dab8c7e"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformWindows
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - Polish Windows CD
	{
		{
			"simon2",
			"CD",

			{
	{ "gsptr30",		GAME_BASEFILE,	"657fd873f5d0637097ee02315b447e6f"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"7b9afcf82a94722707e0d025c0192be8"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
	{ NULL, 0, NULL}
			},
			Common::PL_POL,
			Common::kPlatformWindows
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE
	},

	// The Feeble Files - English Amiga CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "gfxindex.dat",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2"},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - German Amiga CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "gfxindex.dat",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2"},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformAmiga
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - English Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - French Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformMacintosh
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - German Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformMacintosh
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - Spanish Macintosh CD
	{
		{
			"feeble",
			"CD",

			{
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "setup",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::ES_ESP,
			Common::kPlatformMacintosh
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE
	},

	// The Feeble Files - English Windows 2CD
	{
		{
			"feeble",
			"2CD",

			{
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - English Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"a8746407a5b20a7da0da0a14c380af1c"},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - French Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed"},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::FR_FRA,
			Common::kPlatformWindows
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - German Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformWindows
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - Italian Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"80576f2e1ed4c912b63921fe77af313e"},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::IT_ITA,
			Common::kPlatformWindows
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// The Feeble Files - Spanish Windows 4CD
	{
		{
			"feeble",
			"4CD",

			{
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3"},
	{ "save.999",		GAME_RESTFILE,	"71512fc98501a8071a26b683a31dde78"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
	{ NULL, 0, NULL}
			},
			Common::ES_ESP,
			Common::kPlatformWindows
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Demon in my Pocket
	{
		{
			"dimp",
			"CD",

			{
	{ "Gdimp",		GAME_BASEFILE,	"0b1e89ae1dc2e012b7fa7a987b4ac42a"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_PP,
		GID_DIMP,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Jumble
	{
		{
			"jumble",
			"CD",

			{
	{ "Gjumble",		GAME_BASEFILE,	"d54cce46d339038d1a6b74ea213655bc"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_PP,
		GID_JUMBLE,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - NoPatience
	{
		{
			"puzzle",
			"CD",

			{
	{ "Gpuzzle",		GAME_BASEFILE,	"3f80dac8e0d85401a1058a560fe49ab6"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_PP,
		GID_PUZZLE,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - English
	{
		{
			"swampy",
			"CD",

			{
	{ "Gswampy",		GAME_BASEFILE,	"3a6d4d7b2433e660f2483f9396cc87a2"},
	{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformWindows
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures - German
	{
		{
			"swampy",
			"CD",

			{
	{ "Gswampy",		GAME_BASEFILE,	"4b44f26f51c9c4a16210b4ff9875d24c"},
	{ NULL, 0, NULL}
			},
			Common::DE_DEU,
			Common::kPlatformWindows
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE
	},
	{ { NULL, NULL,	{ { NULL, 0, NULL }, { NULL, 0, NULL } }, Common::UNK_LANG, Common::kPlatformUnknown }, 0, 0, 0 }
};
