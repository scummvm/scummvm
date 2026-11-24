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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/file.h"

#include "ngi/detection.h"
#include "ngi/ngi.h"

static const DebugChannelDef debugFlagList[] = {
	{NGI::kDebugPathfinding, "path", "Pathfinding"},
	{NGI::kDebugDrawing, "drawing", "Drawing"},
	{NGI::kDebugLoading, "loading", "Scene loading"},
	{NGI::kDebugAnimation, "animation", "Animation"},
	{NGI::kDebugBehavior, "behavior", "Behavior"},
	{NGI::kDebugMemory, "memory", "Memory management"},
	{NGI::kDebugEvents, "events", "Event handling"},
	{NGI::kDebugInventory, "inventory", "Inventory"},
	{NGI::kDebugSceneLogic, "scenelogic", "Scene Logic"},
	{NGI::kDebugInteractions, "interactions", "Interactions"},
	{NGI::kDebugXML, "xml", "XML"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor ngiGames[] = {
	{"ngi", 		"Nikita Game Interface game"},
	{"mdream",		"Magic Dream"},

	{"fullpipe", "Full Pipe"}, // Полная труба

	// qdEngine
	// "Pilot Brothers. The Other Side of the Earth", // Братья Пилоты. Обратная сторона Земли

	{"pilot7", "Pilot Brothers. The Olympiad"}, // Братья Пилоты. Олимпиада
	{"popovich", "The Three Bogatyrs: Alesha Popovich and Tugarin the Serpent"}, // 3 богатыря. Алеша Попович и Тугарин Змей
	{"lostpilots", "Pilot Brothers. The Mystery of Atlantic Herring"}, // Братья Пилоты. Загадка атлантической сельди
	{"dobrynya", "The Three Bogatyrs: Dobrynia Nikitich and the Serpent Gorynych"}, // 3 богатыря. Добрыня Никитич и Змей Горыныч
	{"vladimir", "Prince Vladimir"}, // Князь Владимир
	{"luntik", "Moonzy. Getting Ready for School"}, // Лунтик. Подготовка к школе
	{"luntik2", "Moonzy. Mathematics for Kids"}, // Лунтик. Математика для малышей
	{"luntik3", "Moonzy. Russian for Kids"}, // Лунтик. Русский язык для малышей
	{"luntik4", "Moonzy. English for Kids"}, // Лунтик. Английский язык для малышей
	{"38_parrots", "38 Parrots. Getting Ready for School"}, // 38 попугаев. Подготовка к школе
	{"luntik5", "Moonzy. Education Games for Kids"}, // Лунтик. Развивающие задания для малышей
	{"38_parrots2", "38 Parrots. Mathematics"}, // 38 попугаев. Математика
	{"fullkatya", "UmaNetto. Hullo There!"}, // UmaNetto. Полный превед!
	{"muromets", "The Three Bogatyrs: Ilya Muromets and the Nightingale-Robber"}, // 3 богатыря. Илья Муромец и Соловей-разбойник
	{"luntik6", "Moonzy Learns Letters"}, // Лунтик учит буквы
	{"luntik7", "Moonzy Learns Numbers"}, // Лунтик учит цифры

	// OGRE engine
	// "Ranetki", // Ранетки

	// LithTech engine by Monolith Inc.
	// "Ghosts from Bookland or A Book Adventure", // Привидения из Букленда или книжное приключение

	{"luntik8", "Moonzy Explores the World"}, // Лунтик познает мир
	{"luntik9", "Moonzy Learns to Draw"}, // Лунтик учится рисовать
	{"luntik10", "Moonzy Studies Rules"}, // Лунтик учит правила
	{"masterbook", "The Book Of Masters"}, // Книга мастеров
	{"38_parrots3", "38 Parrots. Starship Troopers"}, // 38 попугаев. Звездный десант
	{"luntik11", "Moonzy. Want to Know"}, // Лунтик. Хочу всё знать
	{"luntik12", "Moonzy. Training Memory and Attention"}, // Лунтик. Тренируем память и внимание
	{"luntik13", "Moonzy. Geography for Kids"}, // Лунтик. География для малышей
	{"winx2", "Winx Club 2. Stella's Date"}, // Winx Club. Свидание Стеллы
	{"mm_school_training", "Masha and The Bear. Getting Ready for School"}, // Маша и медведь. Подготовка к школе
	{"trybog", "The Three Bogatyrs and the Shamakhan Queen"}, // 3 богатыря и Шамаханская царица
	{"winx4", "Winx Club 4. Bloom's Birthday"}, // Winx Club. День рождения Блум
	{"masha02", "Masha and The Bear. Education Activities for Kids"}, // Маша и медведь. Развивающие задания для малышей
	{"winx_pets", "Winx Club 6. Flora's Pets"}, // Winx Club. Флора и ее питомцы
	{"winx_turmoil", "Winx Club 7. Turmoil in the Virtuality"}, // Winx Club. Переполох в виртуальности
	{"winx_party", "Winx Club 8. The Party"}, // Winx Club. Вечеринка
	{"vasilisa", "Ivan Tsarevich and the Gray Wolf"}, // Иван Царевич и серый волк
	{"vasilisa2", "Ivan Tsarevich. Vasilisa's Wedding"}, // Иван Царевич. Свадьба Василисы
	{"barboskins_1", "Barboskiny. First Exposure"}, // Барбоскины. Первое знакомство
	{"masha03", "Masha and The Bear. Hide-and-seek"}, // Маша и медведь. Прятки
	{"barboskins_2", "Barboskiny. Burying the Truth"}, // Барбоскины. Заметая следы
	{"masha04", "Masha and The Bear. Let's Be Friends"}, // Маша и медведь. Давайте дружить
	{"phineasferb", "Phineas and Ferb. New Inventions"}, // Финес и Ферб. Новые изобретения
	{"masha05", "Masha and The Bear. Active Games"}, // Маша и Медведь. Активные игры
	{nullptr, nullptr}
};

namespace NGI {

// Catalog placeholders until detection signatures are available.
#define NGI_STUB(id) { \
	{ id, nullptr, AD_ENTRY1s(id "stub.gam", "00000000000000000000000000000000", 0), \
	  Common::RU_RUS, Common::kPlatformWindows, ADGF_UNSUPPORTED | ADGF_DROPPLATFORM, GUIO1(GUIO_NOMIDI) }, \
	0 \
}

static const NGIGameDescription gameDescriptions[] = {

	// Magic Dream Russian version
	{
		{
			"mdream",
			nullptr,
			AD_ENTRY1s("0001.nl", "079d02921a938ec9740598316450d526", 11848423),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_MDREAM
	},

	// Full Pipe Russian version
	// Полная труба
	// 2003/11. Pipe Studio
	{
		{
			"fullpipe",
			nullptr,
			AD_ENTRY1s("4620.sc2", "a1a8f3ed731b0dfea43beaa3016fdc71", 554),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe German version
	{
		{
			"fullpipe",
			nullptr,
			AD_ENTRY1s("4620.sc2", "e4f24ffe4dc84cafc648b951e66c1fb3", 554),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Estonian version
	{
		{
			"fullpipe",
			nullptr,
			AD_ENTRY1s("4620.sc2", "571f6b4b68b02003e35bc12c1a1d3fe3", 466),
			Common::ET_EST,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Lithuanian version
	{
		{
			"fullpipe",
			nullptr,
			AD_ENTRY1s("4620.sc2", "599bb585a1863733d0010cb37d838d51", 510),
			Common::LT_LTU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe English version
	{
		{
			"fullpipe",
			nullptr,
			AD_ENTRY1s("4620.sc2", "bffea807345fece14089768fc141af83", 510),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe English Steam version
	{
		{
			"fullpipe",
			"Steam",
			AD_ENTRY1s("4620.sc2", "66ef399644434e88f3951acd882742b6", 510),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Russian Demo version
	{
		{
			"fullpipe",
			"Demo",
			AD_ENTRY1s("4620.sc2", "a0c71b47fc35a5e163fcd8d0972639bb", 70),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Russian 1997 Demo version
	// Contains General MIDI file
	{
		{
			"fullpipe",
			"Demo",
			AD_ENTRY1s("0001.nl", "c19d5281671c8fe2584bd02209bc4a10", 8998307),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | ADGF_DEMO,
			nullptr
		},
		GID_FULLPIPE
	},

	// Full Pipe German Demo version
	{
		{
			"fullpipe",
			"Demo",
			AD_ENTRY1s("4620.sc2", "e5e98df537e56b39c33ae1d5c90976fe", 510),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Android version
	{
		{
			"fullpipe",
			nullptr,
			AD_ENTRY2s("Full-Pipe-v1-0-0.apk", "71f99c6d83f68169695aa30f055d2bab", 48598381,
					   "main.7.com.absolutist.fullpipepremium.obb", "e5570f4162cd1bbbe457eadc5875bcf2", 162825463),
			Common::UNK_LANG,
			Common::kPlatformAndroid,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Братья Пилоты. Обратная сторона Земли
	// 2004/06. Pipe Studio
	// qdengine:pilots3

	// Братья Пилоты. Олимпиада
	// 2004/08. Pipe Studio
	NGI_STUB("pilot7"),

	// 3 богатыря. Алеша Попович и Тугарин Змей
	// 2005/02. Pipe Studio
	NGI_STUB("popovich"),

	// Братья Пилоты. Загадка атлантической сельди
	// 2006/01. Pipe Studio
	NGI_STUB("lostpilots"),

	// 3 богатыря. Добрыня Никитич и Змей Горыныч
	// 2006/03. Pipe Studio
	NGI_STUB("dobrynya"),

	// Князь Владимир
	// 2006/05. Pipe Studio
	NGI_STUB("vladimir"),

	// Лунтик. Подготовка к школе
	// 2006/11. Pipe Studio
	NGI_STUB("luntik"),

	// Лунтик. Математика для малышей
	// 2007/03. Pipe Studio
	NGI_STUB("luntik2"),

	// Лунтик. Русский язык для малышей
	// 2007/06. Pipe Studio
	NGI_STUB("luntik3"),

	// Лунтик. Английский язык для малышей
	// 2007/09. Pipe Studio
	NGI_STUB("luntik4"),

	// 38 попугаев. Подготовка к школе
	// 2007/09. Pipe Studio
	NGI_STUB("38_parrots"),

	// Лунтик. Развивающие задания для малышей
	// 2007/10. Pipe Studio
	NGI_STUB("luntik5"),

	// 38 попугаев. Математика
	// 2007/11. Pipe Studio
	NGI_STUB("38_parrots2"),

	// UmaNetto. Полный превед!
	// 2007/12. Pipe Studio
	NGI_STUB("fullkatya"),

	// 3 богатыря. Илья Муромец и Соловей-разбойник
	// 2007/12. Pipe Studio
	NGI_STUB("muromets"),

	// Лунтик учит буквы
	// 2008/08. Pipe Studio
	NGI_STUB("luntik6"),

	// Лунтик учит цифры
	// 2008/10. Pipe Studio
	NGI_STUB("luntik7"),

	// Ранетки
	// 2008/11. Pipe Studio
	// OGRE engine, 3D

	// Привидения из Букленда или книжное приключение
	// 2008/12. Pipe Studio
	// LithTech engine by Monolith Inc.

	// Лунтик познает мир
	// 2009/01. Pipe Studio
	NGI_STUB("luntik8"),

	// Лунтик учится рисовать
	// 2009/04. Pipe Studio
	NGI_STUB("luntik9"),

	// Лунтик учит правила
	// 2009/08. Pipe Studio
	NGI_STUB("luntik10"),

	// Книга мастеров
	// 2009/10. Pipe Studio
	NGI_STUB("masterbook"),

	// 38 попугаев. Звездный десант
	// 2009/11. Pipe Studio
	NGI_STUB("38_parrots3"),

	// Лунтик. Хочу всё знать
	// 2010/02. Pipe Studio
	NGI_STUB("luntik11"),

	// Лунтик. Тренируем память и внимание
	// 2010/04. Pipe Studio
	NGI_STUB("luntik12"),

	// Лунтик. География для малышей
	// 2010/08. Pipe Studio
	NGI_STUB("luntik13"),

	// Winx Club. Свидание Стеллы
	// 2010/11. Pipe Studio
	NGI_STUB("winx2"),

	// Маша и медведь. Подготовка к школе
	// 2010/12. Pipe Studio
	NGI_STUB("mm_school_training"),

	// 3 богатыря и Шамаханская царица
	// 2010/12. Pipe Studio
	NGI_STUB("trybog"),

	// Winx Club. День рождения Блум
	// 2011/03. Pipe Studio
	NGI_STUB("winx4"),

	// Маша и медведь. Развивающие задания для малышей
	// 2011/04. Pipe Studio
	NGI_STUB("masha02"),

	// Winx Club. Флора и ее питомцы
	// 2011/08. Pipe Studio
	NGI_STUB("winx_pets"),

	// Winx Club. Переполох в виртуальности
	// 2011/10. Pipe Studio
	NGI_STUB("winx_turmoil"),

	// Winx Club. Вечеринка
	// 2011/10. Pipe Studio
	NGI_STUB("winx_party"),

	// Иван Царевич и серый волк
	// 2011/12. Pipe Studio
	NGI_STUB("vasilisa"),

	// Иван Царевич. Свадьба Василисы
	// 2011/12. Pipe Studio
	NGI_STUB("vasilisa2"),

	// Барбоскины. Первое знакомство
	// 2012/05. Pipe Studio
	NGI_STUB("barboskins_1"),

	// Маша и медведь. Прятки
	// 2012/08. Pipe Studio
	NGI_STUB("masha03"),

	// Барбоскины. Заметая следы
	// 2012/10. Pipe Studio
	NGI_STUB("barboskins_2"),

	// Маша и медведь. Давайте дружить
	// 2012/12. Pipe Studio
	NGI_STUB("masha04"),

	// Финес и Ферб. Новые изобретения
	// 2012/12. Pipe Studio
	NGI_STUB("phineasferb"),

	// Маша и Медведь. Активные игры
	// 2013/04. Pipe Studio
	NGI_STUB("masha05"),

	{ AD_TABLE_END_MARKER, 0 }
};

#undef NGI_STUB

} // End of namespace NGI

class NGIMetaEngineDetection : public AdvancedMetaEngineDetection<NGI::NGIGameDescription> {
public:
	NGIMetaEngineDetection() : AdvancedMetaEngineDetection(NGI::gameDescriptions, ngiGames) {
	}

	const char *getName() const override {
		return "ngi";
	}

	const char *getEngineName() const override {
		return "Nikita Game Interface";
	}

	const char *getOriginalCopyright() const override {
		return "Full Pipe (C) Pipe Studio";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(NGI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, NGIMetaEngineDetection);
