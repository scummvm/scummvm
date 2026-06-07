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

#ifndef MACS2_DETECTION_TABLES_H
#define MACS2_DETECTION_TABLES_H

namespace Macs2 {

const PlainGameDescriptor macs2Games[] = {
	{"sis", "Schatz im Silbersee"},
	{0, 0}};

const ADGameDescription gameDescriptions[] = {
	{"sis",
	 nullptr,
	 // Original file
	 AD_ENTRY1s("RESOURCE.MCS", "b8646d5cc2e45442a414220b1a65ba73", 8621636),
	 Common::DE_DEU,
	 Common::kPlatformDOS,
	 ADGF_UNSTABLE,
	 GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TTS, GAMEOPTION_ENHANCEMENTS)},
	{"sis",
	 nullptr,
	 // Adjusted file
	 AD_ENTRY1s("RESOURCE.MCS", "5a6cdeecdabae42872ab9278ab895bad", 8621636),
	 Common::DE_DEU,
	 Common::kPlatformDOS,
	 ADGF_UNSTABLE,
	 GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TTS, GAMEOPTION_ENHANCEMENTS)},

	// GMACS II Interpreter V1.00 - Written & Copyright (C) 1993 by Arndt Hasch - Copyright by LINEL
	{"sis",
	 "Demo",
	 AD_ENTRY1s("RESOURCE.MCS", "779c5f7d11ac61b7b941ec0f1778d837", 2376278),
	 Common::DE_DEU,
	 Common::kPlatformDOS,
	 ADGF_DEMO | ADGF_UNSTABLE,
	 GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TTS, GAMEOPTION_ENHANCEMENTS)},

	// English fan translation (original + macs2_translation.dat)
	{"sis",
	 "w/translation",
	 AD_ENTRY2s("RESOURCE.MCS", "b8646d5cc2e45442a414220b1a65ba73", 8621636,
	            "macs2_translation.dat", nullptr, AD_NO_SIZE),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 GF_TRANSLATED | ADGF_UNSTABLE,
	 GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TTS, GAMEOPTION_ENHANCEMENTS)},
	{"sis",
	 "w/translation",
	 AD_ENTRY2s("RESOURCE.MCS", "5a6cdeecdabae42872ab9278ab895bad", 8621636,
	            "macs2_translation.dat", nullptr, AD_NO_SIZE),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 GF_TRANSLATED | ADGF_UNSTABLE,
	 GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TTS, GAMEOPTION_ENHANCEMENTS)},

	// Demo - English fan translation
	{"sis",
	 "Demo/w/translation",
	 AD_ENTRY2s("RESOURCE.MCS", "779c5f7d11ac61b7b941ec0f1778d837", 2376278,
	            "macs2_translation.dat", nullptr, AD_NO_SIZE),
	 Common::EN_ANY,
	 Common::kPlatformDOS,
	 GF_TRANSLATED | ADGF_DEMO | ADGF_UNSTABLE,
	 GUIO3(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_TTS, GAMEOPTION_ENHANCEMENTS)},

	AD_TABLE_END_MARKER};

} // End of namespace Macs2

#endif // MACS2_DETECTION_TABLES_H
