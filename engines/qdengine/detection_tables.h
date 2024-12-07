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


namespace QDEngine {

const PlainGameDescriptor GAME_NAMES[] = {
	{ "qdengine", "QDEngine game" },
	{ "karliknos", "Little Longnose" }, // Карлик Нос
	{ "nupogodi3", "Wait for it! Issue 3. Song for a Hare" }, // Ну, погоди! Выпуск 3. Песня для зайца
	{ "pilots3", "Pilot Brothers 3: Back Side of the Earth" }, // Братья Пилоты. Обратная сторона Земли
	{ "pilots3d", "Pilot Brothers 3D. The Case of Garden Pests" }, // Братья Пилоты 3D. Дело об Огородных вредителях
	{ "pilots3d-2", "Pilot Brothers 3D-2. Kennel Club Secrets" }, // Братья Пилоты 3D-2. Тайны Клуба Собаководов
	{ "mng", "Mom don't Worry" }, // Мама не горюй
	{ "rybalka", "Features of National Fishing" }, // Особенности национальной рыбалки
	{ "maski", "Mask Show" }, // Маски-шоу
	{ "3mice1", "Marvellous Mice Adventures: Meeting Sea Rat" }, // Три маленькие белые мышки. Визит Морской крысы
	{ "shveik", "The Adventures of the Good Soldier Schweik" }, // Похождения бравого солдата Швейка
	{ "klepa", "Klepa: Symbolic Rivets" },  // Клёпа: Символические Заклепки
	{ "3mice2", "Marvellous Mice Adventures: Sea Rat's Birthday" }, // Три маленькие белые мышки. День рождения морской крысы
	{ "dogncat", "Dog-n-cat: Island of Dr Ratiarty" }, // Агентство "КотоПес": Остров доктора Крысарди
	{ "dogncat2", "Dog-n-cat: In the Footsteps of Unprecedented Beasts" }, // Агентство "КотоПес": По следам невиданных зверей

	{ 0, 0 }
};

#define SUPPORT_STATUS ADGF_UNSTABLE

#define GAMEelf(id,extra,md5,size,exefile,exemd5,exesize,lang,flags) { \
        id, \
        nullptr, \
        AD_ENTRY2s("qd_game.qml", md5, size, exefile, exemd5, exesize), \
        lang, \
        Common::kPlatformWindows, \
        (SUPPORT_STATUS | ADGF_DROPPLATFORM | flags), \
        GUIO1(GUIO_NONE) \
    }

#define GAME(id,md5,size,exefile,exemd5,exesize) GAMEelf(id,nullptr,md5,size,exefile,exemd5,exesize,Common::RU_RUS,0)
#define GAMEd(id,md5,size,exefile,exemd5,exesize) GAMEelf(id,nullptr,md5,size,exefile,exemd5,exesize,Common::RU_RUS,ADGF_DEMO)
#define GAMEl(id,md5,size,exefile,exemd5,exesize,lang) GAMEelf(id,nullptr,md5,size,exefile,exemd5,exesize,lang,0)
#define GAMEe(id,extra,md5,size,exefile,exemd5,exesize) GAMEelf(id,extra,md5,size,exefile,exemd5,exesize,Common::RU_RUS,0)

const ADGameDescription GAME_DESCRIPTIONS[] = {
	// Карлик Нос
	// 2003/10/03. K D-Lab / Melnitsa
	GAME("karliknos", "a3f1b86c07bf72f688e7f2b5f20aa7f9", 3225385,
			"qd_game.exe", "fcc99749c0ec55d4b112450755ae97b8", 1808636),
	// Nykštukas Nosis
	//GAMEl("karliknos", "???", 3225385, Common::LT_LIT),

#undef SUPPORT_STATUS
#define SUPPORT_STATUS ADGF_NO_FLAGS
	// Ну, погоди! Выпуск 3. Песня для зайца
	// 2003/12/11. K D-Lab
	GAME("nupogodi3", "7394a5d28590680cfeb6a037101a9c81", 1675230,
			"qd_game.exe", "d5e942e1d588fbd127c812551c0b67ef", 909312),
	// Na, palauk! Zuikio dainos
	GAMEl("nupogodi3", "3fbcdd27bf0a3defb6195c92b9b0a95f", 1675036,
			"qd_game.exe", "18b4260f3f25ac0f394a8c618203dcba", 1855208, Common::LT_LTU),
	// Jen počkej a Píseň pro zajíce
	GAMEl("nupogodi3", "c0fab62fe6f3a339e96b1dd4a034e40a", 1674743,
			"qd_game.exe", "d5e942e1d588fbd127c812551c0b67ef", 909312, Common::CS_CZE),
	// No počkaj! 3: Pieseň pre zajaca
	//GAMEl("nupogodi3", "???", 1674743,
	//		"qd_game.exe", "???", 909312, Common::SK_SVK),

#undef SUPPORT_STATUS
#define SUPPORT_STATUS ADGF_UNSTABLE

	// Братья Пилоты. Обратная сторона Земли
	// 2004/06/11. K D-Lab / Pipe Studio
	// GOG release is NOT on this engine
	GAME("pilots3", "33dfc61046e5af85b570eda24e0afe6d", 14452732,
			"bp4.exe", "fd5bcdcc1fc16c78aa51d67e550cc674", 950272),

	// Особенности национальной рыбалки
	// 2004/09/15. Melnitsa / Lazy Games
	GAME("rybalka", "4113d7f0f67cf9563f4c62f999898426", 16892191,
			"onr.exe", "30e87b2fa28bab268e0a98e0898ef09c", 745472),

	// Братья Пилоты 3D. Дело об Огородных вредителях
	// 2004/10/22. K D-Lab
	GAME("pilots3d", "c836992c240990f199947e8a362ea867", 5741533,
			"qd_game.exe", "34684d88c8be61b60b2b6707e49b2ba0", 733184),

	// Братья Пилоты 3D-2. Тайны Клуба Собаководов
	// 2005/04/29. K D-Lab
	GAMEe("pilots3d-2", "Part I", "7f9eeee513eadbd0f2431bca880df1d1", 4266775,
			"qd_game.exe", "708813f2e8b77f62385f558dfc333abe", 753664),
	GAMEe("pilots3d-2", "Part II", "dbf676d0d9f4f8d968352918e7ecaffe", 9689557,
			"qd_game.exe", "708813f2e8b77f62385f558dfc333abe", 753664),

	// Мама не горюй
	// 2005/08/05. Melnitsa / Lazy Games
	GAME("mng", "9a1b9134f914f22682fbc0604f4eed81", 20704192,
			"mama.exe", "233ac221814568e5c278f952e76495ce", 757760),
	GAMEd("mng", "da3a6e591d4896ca3176c5866aef278f", 2541094,
			"qd_game.exe", "233ac221814568e5c278f952e76495ce", 757760),

#undef SUPPORT_STATUS
#define SUPPORT_STATUS ADGF_NO_FLAGS
	// Маски-шоу
	// 2005/12/16. Lazy Games
	GAME("maski", "9a40b7491cac0dd80657cfe52e28ea74", 23128230,
			"maski.exe", "1b7a4076bf1ed15e3eb642caeb337337", 761856),

	// Три маленькие белые мышки и пираты северного моря
	GAMEd("3mice1", "e6ad21f2f3241116b0f66a1d98f8e3f3", 403092,
			"qd_game.exe", "da2dc5265cb7c55bcf1b24cd9deae9d1", 761856),

	// Три маленькие белые мышки. Визит Морской крысы
	// 2007/03/01. Lazy Games
	GAME("3mice1", "63e79178e66c520a9094622ccdcaf9c5", 10006622,
			"qd_game.exe", "b8b3545b022f8e0975a8a948c03ad76e", 917504),
	// Příběhy myší rodinky I
	GAMEl("3mice1", "a6060f3d17ea2e50a03216332e517817", 10022470,
			"qd_game.exe", "b8b3545b022f8e0975a8a948c03ad76e", 917504, Common::CS_CZE),
	// Były Sobie Myszki Trzy: Na Spotkanie Cioci Myszy
	GAMEl("3mice1", "6fe8feb6cf76f4642f6d6cb612b1d446", 10012908,
			"qd_game.exe", "b8b3545b022f8e0975a8a948c03ad76e", 917504, Common::PL_POL),

	// Похождения бравого солдата Швейка
	// 2007/09/07. Lazy Games
	GAME("shveik", "3877afbb780996a5608a5b358dd59726", 30233418,
			"shveik.exe", "a56cb058b46e47754019652d27d974b3", 962560),

#undef SUPPORT_STATUS
#define SUPPORT_STATUS ADGF_UNSTABLE

	// Клёпа: Символические Заклепки
	// 2007/10/19. K-D Logic
	GAME("klepa", "a09e1f072942d88c06abccfb820f46a9", 20260975,
			"qd_game.exe", "a56cb058b46e47754019652d27d974b3", 962560),

#undef SUPPORT_STATUS
#define SUPPORT_STATUS ADGF_NO_FLAGS
	// Три маленькие белые мышки. День рождения морской крысы
	// 2007/12/27. Lazy Games
	GAME("3mice2", "93d8311ff9c00453f25192743c9e98d9", 8176962,
			"qd_game.exe", "ffe20c2dbb131b01fccc1211a41e76e7", 962560),
	// Příběhy myší rodinky II
	//GAMEl("3mice2", "???", 8176962, Common::CZ_CZE),
	GAMEd("3mice2", "dfd98feb2e7d3345a7babdeb3ed3e9a7", 800666, // Demo1
			"demo1.exe", "ffe20c2dbb131b01fccc1211a41e76e7", 962560),

	// Były Sobie Myszki Trzy: Urodzinowe Śledztwo
	GAMEl("3mice2", "9128a0d801e547cb651c5fc7e09bee04", 8182142,
			"qd_game.exe", "ffe20c2dbb131b01fccc1211a41e76e7", 962560, Common::PL_POL),

	{ // Demo2, original
	  // Video is in MP4 format which is not supported
		"3mice2",
		"Original",
		AD_ENTRY3s("qd_game.qml", "6af4c6f11cf0994670bedb78efe22267", 1124576, // Demo2
				   "demo2.exe",   "ffe20c2dbb131b01fccc1211a41e76e7", 962560,
				   "Resource/Video/martha.mpg", "02850c5fc074eba22b368cca0ff57c98", 9810104),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM | ADGF_DEMO | GF_BROKEN_VIDEOS,
		GUIO1(GUIO_NONE)
	},

	{ // Demo2, original
	  // Video is recoded with: ffmpeg -i martha.mpg -b:v 6000k -maxrate:v 9000k martha-new.mpeg
	  // The full game has this video also broken: the video frames get frozen at the last several seconds
		"3mice2",
		"Reencoded video",
		AD_ENTRY3s("qd_game.qml", "6af4c6f11cf0994670bedb78efe22267", 1124576,
				   "demo2.exe",   "ffe20c2dbb131b01fccc1211a41e76e7", 962560,
				   "Resource/Video/martha.mpg", "4dc93c37c0cdd75c01c58412f68e4874", 32499712),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

#undef SUPPORT_STATUS
#define SUPPORT_STATUS ADGF_UNSTABLE
	// Агентство "КотоПес": Остров доктора Крысарди
	// 2008/09/19 KD Vision Games
	GAME("dogncat", "db4e0081fdaf575e82910cf7399cef62", 17759363,
			"qd_game.exe", "ed659f0fc79e772f1f1ff8107fa94dea", 962560),

	// Агентство "КотоПес": По следам невиданных зверей
	// 2008/03/21 KD Vision Games
	GAME("dogncat2", "c250f79a8e404b13a588e6a03e3a6d20", 10483617,
			"qd_game.exe", "ed659f0fc79e772f1f1ff8107fa94dea", 962560),

	AD_TABLE_END_MARKER
};

} // namespace Qdengine
