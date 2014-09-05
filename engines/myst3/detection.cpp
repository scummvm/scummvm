/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/advancedDetector.h"

#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"

#include "common/savefile.h"

#include "graphics/scaler.h"

namespace Myst3 {

struct Myst3GameDescription {
	ADGameDescription desc;
	const ExecutableVersion *version;
};

static const PlainGameDescriptor myst3Games[] = {
	{ "myst3", "Myst III Exile" },
	{ 0, 0 }
};

static const char *directoryGlobs[] = {
	"bin",
	"M3Data",
	"MYST3BIN",
	"TEXT",
	0
};

// SafeDisk keys
static const SafeDiskKey sdKey100 = { 0xE3A2EB02, 0x190158D2, 0xD5E10A26, 0x6DF705D2 };
static const SafeDiskKey sdKey110 = { 0x8D792101, 0x42AF4572, 0x5581B0CE, 0x984525B4 };
static const SafeDiskKey sdKey122 = { 0x9D15D040, 0x10A5F8C7, 0x3A37C5B2, 0x95516FC0 };

// Executable versions and associated offsets
static const ExecutableVersion v100win = { "1.0 Windows",    kFlagVersion10, "M3.exe",                        0x400000, 0x77108,  0x77040,  0x771D0,  0,       &sdKey100 };
static const ExecutableVersion v100mac = { "1.0 Macintosh",  kFlagVersion10, "Myst III Exile",                0,        0x10FD4,  0x10F10,  0x1109C,  0,       0         };
static const ExecutableVersion v110win = { "1.1 Windows",    kFlagNone,      "M3.exe",                        0x400000, 0x78108,  0x78040,  0x781D0,  0x7D654, &sdKey110 };
static const ExecutableVersion v120mjp = { "1.2 Macintosh",  kFlagNone,      "Myst3 Exile JP",                0,        0x11230,  0x1116C,  0x112F8,  0,       0         };
static const ExecutableVersion v120mac = { "1.2 Macintosh",  kFlagNone,      "Myst3",                         0,        0x112D0,  0x1120C,  0x11398,  0,       0         };
static const ExecutableVersion v120sd  = { "1.2 Windows",    kFlagNone,      "M3.exe",                        0x400000, 0x86108,  0x86040,  0x861D0,  0x8B654, &sdKey110 };
static const ExecutableVersion v120rus = { "1.2 Windows",    kFlagNone,      "M3R.exe",                       0x401000, 0x85108,  0x85040,  0x851D0,  0x8A654, 0         };
static const ExecutableVersion v122sd  = { "1.22 Windows",   kFlagNone,      "M3.exe",                        0x400000, 0x86108,  0x86040,  0x861D0,  0x8B654, &sdKey122 };
static const ExecutableVersion v12xwin = { "1.2x Windows",   kFlagNone,      "M3.exe",                        0x400000, 0x86108,  0x86040,  0x861D0,  0x8B654, 0         };
static const ExecutableVersion v122osx = { "1.22 Mac OS X",  kFlagNone,      "Myst3 OS X US",                 0,        0x118C4,  0x11800,  0x1198C,  0x16E10, 0         };
static const ExecutableVersion v124win = { "1.24 Windows",   kFlagNone,      "M3.exe",                        0x400000, 0x86108,  0x86040,  0x861E8,  0x8B66C, 0         };
static const ExecutableVersion v127win = { "1.27 Windows",   kFlagDVD,       "M3.exe",                        0x400000, 0x86110,  0x86040,  0x861F0,  0x8B674, 0         };
static const ExecutableVersion v127mac = { "1.27 Macintosh", kFlagDVD,       "Myst III Exile for Mac OS 8-9", 0,        0x11378,  0x112A8,  0x11454,  0x168D8, 0         };
static const ExecutableVersion v127xcd = { "1.27 Mac OS X",  kFlagNone,      "Myst III Exile for Mac OS X",   0,        0x11934,  0x11864,  0x11A10,  0x16E94, 0         };
static const ExecutableVersion v127osx = { "1.27 Mac OS X",  kFlagDVD,       "Myst III Exile for Mac OS X",   0,        0x11934,  0x11864,  0x11A10,  0x16E94, 0         };
static const ExecutableVersion ps2ntsc = { "PS2 NTSC-U/C",   kFlagNone,      "SLUS_204.34",                   0xFFF00,  0x14EB10, 0x14EA10, 0x14ECA0, 0,       0         };
static const ExecutableVersion ps2pal =  { "PS2 PAL",        kFlagNone,      "SLES_507.26",                   0xFFF00,  0x14ED10, 0x14EC10, 0x14EEA0, 0,       0         };
static const ExecutableVersion xboxpal = { "Xbox PAL",       kFlagDVD,       "Default.xbe",                   0x2D20,   0xDF148,  0xDF070,  0xDF228,  0xE4740, 0         };

#define MYST3ENTRY(platform, lang, extra, exe, md5exe, version) 				\
{																				\
	{																			\
		"myst3",																\
		extra,																	\
		{																		\
			{ "RSRC.m3r", 0, "a2c8ed69800f60bf5667e5c76a88e481", 1223862 },		\
			{ exe, 0, md5exe, -1 },												\
		},																		\
		lang,																	\
		platform,																\
		ADGF_NO_FLAGS,															\
		GUIO_NONE																\
	},																			\
	version,																	\
},

#define MYST3ENTRY_XBOX(lang, langFile, md5lang) 								\
{																				\
	{																			\
		"myst3",																\
		0,																		\
		{																		\
			{ "RSRC.m3r", 0, "3de23eb5a036a62819186105478f9dde", 1226192 },		\
			{ langFile, 0, md5lang, -1 },										\
		},																		\
		lang,																	\
		Common::kPlatformXbox,													\
		ADGF_UNSTABLE,															\
		GUIO_NONE																\
	},																			\
	&xboxpal																	\
},


static const Myst3GameDescription gameDescriptions[] = {
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "f8ab52da2815c1342eeb42b1bcad5441", &v100win) // 1.0
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "0f2b2703fc79be043007ab4e603d54b8", &v110win) // 1.1
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "314f4d2a3c9d22787719219419dcd480", &v120sd ) // 1.2
	MYST3ENTRY(Common::kPlatformWindows, Common::FR_FRA, 0,     "M3.exe", "3510a29ba7db40fa7310e15aac807e8e", &v120sd ) // 1.2
	MYST3ENTRY(Common::kPlatformWindows, Common::JA_JPN, 0,     "M3.exe", "89f4ceb295420c01116bc4bee7bcd9c4", &v120sd ) // 1.2
	MYST3ENTRY(Common::kPlatformWindows, Common::NL_NLD, 0,     "M3.exe", "fe3429350ce30e2a828c888ae5a39f8b", &v120sd ) // 1.2
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "7a4eaea5f19f621e05e732d657c15fe3", &v12xwin) // 1.2
	MYST3ENTRY(Common::kPlatformWindows, Common::RU_RUS, 0,     "M3R.exe","4ce43b84c1d0869a84b5f361f8914a9f", &v120rus) // 1.2
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "6dbe2eed529684c7fb64f0801462b89c", &v12xwin) // 1.21
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "c8ca22d347f947cd52d95229d0f6a3f3", &v12xwin) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, 0,     "M3.exe", "9d76f2f836ef9382e45f354a532f042d", &v122sd ) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::FR_FRA, 0,     "M3.exe", "f24da8790cf46e3b289569960299b0da", &v12xwin) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::DE_DEU, 0,     "M3.exe", "9e26a4d1762b21fac7a35d7ee441fd75", &v12xwin) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::DE_DEU, 0,     "M3.exe", "8ed262713e49eb07a03e0c4f148476e0", &v122sd ) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::PL_POL, 0,     "M3.exe", "5d3dd344c27c07b6db63a5b58e1e2a25", &v12xwin) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::ES_ESP, 0,     "M3.exe", "d6802549deccc2f7833c4859b210cf7d", &v12xwin) // 1.22
	MYST3ENTRY(Common::kPlatformWindows, Common::JA_JPN, 0,     "M3.exe", "1a3a050413777adcdae967da7a188cc0", &v124win)
	MYST3ENTRY(Common::kPlatformWindows, Common::EN_ANY, "DVD", "M3.exe", "708da0c48c8972025f165df59d823a6b", &v127win)
	MYST3ENTRY(Common::kPlatformWindows, Common::FR_FRA, "DVD", "M3.exe", "b6c35e49d5a416cf4aa34f6439e8d544", &v127win)
	MYST3ENTRY(Common::kPlatformWindows, Common::NL_NLD, "DVD", "M3.exe", "819cdb78736fbdef9c9372c1050585d7", &v127win)
	MYST3ENTRY(Common::kPlatformWindows, Common::DE_DEU, "DVD", "M3.exe", "ec01ddbd45b4f7879571be7ac4f4a54d", &v127win)
	MYST3ENTRY(Common::kPlatformWindows, Common::IT_ITA, "DVD", "M3.exe", "e5e0ca7a6703c3aee5438f9602214ac9", &v127win)
	MYST3ENTRY(Common::kPlatformWindows, Common::ES_ESP, "DVD", "M3.exe", "1c30c1d2e06bab8dcd483f5c49ef1a65", &v127win)

	MYST3ENTRY(Common::kPlatformMacintosh, Common::EN_ANY, 0,     "Myst III Exile",                "13d36737421e7dbc5ac453491935d0f5", &v100mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::JA_JPN, 0,     "Myst3 Exile JP",                "fd212d71d70fb4e8cacbdce175ebeb81", &v120mjp)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::FR_FRA, 0,     "Myst3",                         "bedd73588f94f0aa287084b2ee5d4f12", &v120mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::EN_ANY, "DVD", "Myst III Exile for Mac OS 8-9", "820f34ab68643b60f05e819e45f93cd2", &v127mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::FR_FRA, "DVD", "Myst III Exile for Mac OS 8-9", "2f074a705b56ce42f2a4019e1c01e483", &v127mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::NL_NLD, "DVD", "Myst III Exile for Mac OS 8-9", "9440c777480ce17cd6114ab4a6be6fac", &v127mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::DE_DEU, "DVD", "Myst III Exile for Mac OS 8-9", "52f547897d3e2b1edea0003096521088", &v127mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::IT_ITA, "DVD", "Myst III Exile for Mac OS 8-9", "9b08e5b5c748b9226e7350d99106d2bd", &v127mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::ES_ESP, "DVD", "Myst III Exile for Mac OS 8-9", "ddc0612abbbede1b03540454084aefa9", &v127mac)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::EN_ANY, 0,     "Myst3 OS X US",                 "9bae830cd6a35b4d53edcad101ac4f1b", &v122osx) // CD version
	MYST3ENTRY(Common::kPlatformMacintosh, Common::FR_FRA, 0,     "Myst III Exile for Mac OS X",   "4fe7a9ee4ecd469b60fac4865ae9397c", &v127xcd) // CD version
	MYST3ENTRY(Common::kPlatformMacintosh, Common::DE_DEU, 0,     "Myst III Exile for Mac OS X",   "e19a260340368e70140a9dd58e23d0b4", &v127xcd) // CD version
	MYST3ENTRY(Common::kPlatformMacintosh, Common::EN_ANY, "DVD", "Myst III Exile for Mac OS X",   "d5903cae8c0d07bf7c7a462e53cd5c45", &v127osx)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::FR_FRA, "DVD", "Myst III Exile for Mac OS X",   "39a51a5ff94ca3afbab30addc6ee97aa", &v127osx)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::NL_NLD, "DVD", "Myst III Exile for Mac OS X",   "e628b7ec5d7aab55ec71f011771e3bd0", &v127osx)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::DE_DEU, "DVD", "Myst III Exile for Mac OS X",   "2f2682815504d94378ab82bac0e89b6b", &v127osx)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::IT_ITA, "DVD", "Myst III Exile for Mac OS X",   "1265e9ea6a6001a943b217ca92d83ee6", &v127osx)
	MYST3ENTRY(Common::kPlatformMacintosh, Common::ES_ESP, "DVD", "Myst III Exile for Mac OS X",   "05f37c98a0378c57190da5a4687b7e41", &v127osx)

	// Myst 3 Xbox (PAL)
	MYST3ENTRY_XBOX(Common::EN_ANY, "ENGLISHX.m3t", "c4d012ab02b8ca7d0c7e79f4dbd4e676")
	MYST3ENTRY_XBOX(Common::FR_FRA, "FRENCHX.m3t",  "94c9dcdec8794751e4d773776552751a")
	MYST3ENTRY_XBOX(Common::DE_DEU, "GERMANX.m3t",  "b9b66fcd5d4fbb95ac2d7157577991a5")
	MYST3ENTRY_XBOX(Common::IT_ITA, "ITALIANX.m3t", "3ca266019eba68123f6b7cae57cfc200")
	MYST3ENTRY_XBOX(Common::ES_ESP, "SPANISHX.m3t", "a9aca36ccf6709164249f3fb6b1ef148")

	// Myst 3 Xbox (RUS)
	MYST3ENTRY_XBOX(Common::RU_RUS, "ENGLISHX.m3t", "18cb50f5c5317586a128ca9eb3e03279")

	{
		// Myst 3 PS2 (NTSC-U/C)
		{
			"myst3",
			0,
			AD_ENTRY1s("RSRC.m3r", "c60d37bfd3bb8b0bee143018447bb460", 346618151),
			Common::UNK_LANG,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		&ps2ntsc
	},
	
	{
		// Myst 3 PS2 (PAL)
		{
			"myst3",
			0,
			AD_ENTRY1s("RSRC.m3r", "f0e0c502f77157e6b5272686c661ea75", 91371793),
			Common::UNK_LANG,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
		&ps2pal
	},

	{ AD_TABLE_END_MARKER, 0 }
};

class Myst3MetaEngine : public AdvancedMetaEngine {
public:
	Myst3MetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(Myst3GameDescription), myst3Games) {
		_singleid = "myst3";
		_guioptions = GUIO4(GUIO_NOMIDI, GUIO_NOSFX, GUIO_NOSPEECH, GUIO_NOSUBTITLES);
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Myst III Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Myst III Exile (C) Presto Studios";
	}

	virtual bool hasFeature(MetaEngineFeature f) const {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsDeleteSave) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportCreationDate) ||
			(f == kSavesSupportPlayTime);
	}

	virtual SaveStateList listSaves(const char *target) const {
		SaveStateList saveList;
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles("*.m3s");

		for (uint32 i = 0; i < filenames.size(); i++)
			saveList.push_back(SaveStateDescriptor(i, filenames[i]));

		return saveList;
	}

	void resizeThumbnail(Graphics::Surface *big, Graphics::Surface *small) const {
		assert(big->format.bytesPerPixel == 4 && small->format.bytesPerPixel == 4);

		uint32 *dst = (uint32 *)small->getPixels();
		for (uint i = 0; i < small->h; i++) {
			for (uint j = 0; j < small->w; j++) {
				uint32 srcX = big->w * j / small->w;
				uint32 srcY = big->h * i / small->h;
				uint32 *src = (uint32 *)big->getBasePtr(srcX, srcY);

				// Copy RGBA pixel
				*dst++ = *src;
			}
		}
	}

	SaveStateDescriptor getSaveDescription(const char *target, int slot) const {
		SaveStateList saves = listSaves(target);

		SaveStateDescriptor description = SaveStateDescriptor();
		for (uint32 i = 0; i < saves.size(); i++) {
			if (saves[i].getSaveSlot() == slot) {
				description = saves[i];
			}
		}

		return description;
	}

	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const {
		SaveStateDescriptor saveInfos = getSaveDescription(target, slot);

		if (saveInfos.getDescription().empty()) {
			// Unused slot
			return SaveStateDescriptor();
		}

		// Open save
		Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(saveInfos.getDescription());

		// Read state data
		Common::Serializer s = Common::Serializer(saveFile, 0);
		GameState::StateData data;
		data.syncWithSaveGame(s);

		// Read and resize the thumbnail
		Graphics::Surface *guiThumb = new Graphics::Surface();
		guiThumb->create(kThumbnailWidth, kThumbnailHeight1, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
		resizeThumbnail(data.thumbnail.get(), guiThumb);

		// Set metadata
		saveInfos.setThumbnail(guiThumb);
		saveInfos.setPlayTime(data.secondsPlayed * 1000);

		if (data.saveYear != 0) {
			saveInfos.setSaveDate(data.saveYear, data.saveMonth, data.saveDay);
			saveInfos.setSaveTime(data.saveHour, data.saveMinute);
		}

		if (data.saveDescription != "")
			saveInfos.setDescription(data.saveDescription);

		delete saveFile;

		return saveInfos;
	}

	void removeSaveState(const char *target, int slot) const {
		SaveStateDescriptor saveInfos = getSaveDescription(target, slot);
		g_system->getSavefileManager()->removeSavefile(saveInfos.getDescription());
	}

	virtual int getMaximumSaveSlot() const {
		return 999;
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool Myst3MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Myst3GameDescription *gd = (const Myst3GameDescription *)desc;
	if (gd) {
		*engine = new Myst3Engine(syst, gd);
	}
	return gd != 0;
}

Common::Platform Myst3Engine::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language Myst3Engine::getDefaultLanguage() const {
	return _gameDescription->desc.language;
}

const ExecutableVersion *Myst3Engine::getExecutableVersion() const {
	return _gameDescription->version;
}

} // End of namespace Myst3

#if PLUGIN_ENABLED_DYNAMIC(MYST3)
	REGISTER_PLUGIN_DYNAMIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MYST3, PLUGIN_TYPE_ENGINE, Myst3::Myst3MetaEngine);
#endif
