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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

const GameInfo _games[NUMGAMES] = {
	GameInfo("The Golden Baton",
			 BATON,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 48,  // Number of items
			 171, // Number of actions
			 76,  // Number of words
			 31,  // Number of rooms
			 5,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 77, // number_of_verbs
			 76, // number_of_nouns;

			 0x2349, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b56, // actions
			 UNCOMPRESSED,
			 0x473a,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x2450, // start_of_system_messages
			 0x277e, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 30,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("The Golden Baton C64",
			 BATON_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 48,  // Number of items
			 166, // Number of actions
			 78,  // Number of words
			 31,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 80, // number_of_verbs
			 79, // number_of_nouns;

			 0x1dd9,                // header
			 MYSTERIOUS_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df1, // actions
			 UNCOMPRESSED,
			 0x2861,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x38f1, // start_of_image_data;
			 0,      // image_address_offset
			 30,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("The Time Machine",
			 TIME_MACHINE,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 62,  // Number of items
			 164, // Number of actions
			 87,  // Number of words
			 44,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 88, // number_of_verbs
			 87, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x475f,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0, // start_of_characters;
			 FOLLOWS,
			 0,     // image_address_offset
			 43,    // number_of_pictures;
			 ZXOPT, // palette
			 99,    // picture_format_version;
			 0),
	GameInfo("The Time Machine C64",
			 TIME_MACHINE_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 62,  // Number of items
			 161, // Number of actions
			 85,  // Number of words
			 44,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 73,  // Number of messages

			 87, // number_of_verbs
			 85, // number_of_nouns;

			 0x1dd9,                // header
			 MYSTERIOUS_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df1, // actions
			 UNCOMPRESSED,
			 0x2811,  // dictionary
			 0x2b6d,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 0x2f0f,  // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 0x37ce,  // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3872, // start_of_image_data;
			 0,      // image_address_offset
			 43,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Arrow of Death part 1",
			 ARROW1,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 64,  // Number of items
			 150, // Number of actions
			 90,  // Number of words
			 52,  // Number of rooms
			 5,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 91, // number_of_verbs
			 83, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x46b3,  // dictionary
			 0x4a41,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 51,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Arrow of Death part 1 C64",
			 ARROW1_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 64,  // Number of items
			 150, // Number of actions
			 90,  // Number of words
			 52,  // Number of rooms
			 5,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 91, // number_of_verbs
			 82, // number_of_nouns;

			 0x1dd9,                // header
			 MYSTERIOUS_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df1, // actions
			 UNCOMPRESSED,
			 0x2761,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x0408, // start_of_system_messages
			 0x0408, // start of directions

			 0,      // start_of_characters;
			 0x38e2, // start_of_image_data;
			 0,      // image_address_offset
			 51,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Arrow of Death part 2",
			 ARROW2,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 91,  // Number of items
			 190, // Number of actions
			 83,  // Number of words
			 65,  // Number of rooms
			 9,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 84, // number_of_verbs
			 83, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x49b7,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 64,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Arrow of Death part 2 C64",
			 ARROW2_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 90,  // Number of items
			 176, // Number of actions
			 82,  // Number of words
			 65,  // Number of rooms
			 9,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 81, // number_of_verbs
			 82, // number_of_nouns;

			 0x1dd9,                         // header
			 ARROW_OF_DEATH_PT_2_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df1, // actions
			 UNCOMPRESSED,
			 0x2901,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3cac, // start_of_image_data;
			 0,      // image_address_offset
			 64,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Escape from Pulsar 7",
			 PULSAR7,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 90,  // Number of items
			 220, // Number of actions
			 145, // Number of words
			 45,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 146, // number_of_verbs
			 145, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x4b1d,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 44,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Escape from Pulsar 7 C64",
			 PULSAR7_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 88,  // Number of items
			 195, // Number of actions
			 145, // Number of words
			 45,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 146, // number_of_verbs
			 102, // number_of_nouns;

			 0x1dd9, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1da3, // actions
			 UNCOMPRESSED,
			 0x29e3,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3bf4, // start_of_image_data;
			 0,      // image_address_offset
			 44,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Circus",
			 CIRCUS,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 65,  // Number of items
			 165, // Number of actions
			 97,  // Number of words
			 36,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 98, // number_of_verbs
			 97, // number_of_nouns;

			 0x2349, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b56, // actions
			 UNCOMPRESSED,
			 0x471a,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x277E, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 35,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Circus C64",
			 CIRCUS_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 65,  // Number of items
			 165, // Number of actions
			 97,  // Number of words
			 36,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 72,  // Number of messages

			 98, // number_of_verbs
			 96, // number_of_nouns;

			 0x1dd9,                // header
			 MYSTERIOUS_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df1, // actions
			 UNCOMPRESSED,
			 0x2851,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3914, // start_of_image_data;
			 0,      // image_address_offset
			 35,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Feasibility Experiment",
			 FEASIBILITY,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 65,  // Number of items
			 164, // Number of actions
			 82,  // Number of words
			 59,  // Number of rooms
			 5,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 83, // number_of_verbs
			 82, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x47bf,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 58,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Feasibility Experiment C64",
			 FEASIBILITY_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 65,  // Number of items
			 156, // Number of actions
			 79,  // Number of words
			 59,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 56, // number_of_verbs
			 80, // number_of_nouns;

			 0x1dd9, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df3, // actions
			 UNCOMPRESSED,
			 0x27c3,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3876, // start_of_image_data;
			 0,      // image_address_offset
			 58,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("The Wizard of Akyrz",
			 AKYRZ,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 49,  // Number of items
			 201, // Number of actions
			 85,  // Number of words
			 40,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 86, // number_of_verbs
			 85, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x497d,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 39,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("The Wizard of Akyrz C64",
			 AKYRZ_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 49,  // Number of items
			 199, // Number of actions
			 85,  // Number of words
			 40,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 67, // number_of_verbs
			 85, // number_of_nouns;

			 0x1dd9,                // header
			 MYSTERIOUS_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df1, // actions
			 UNCOMPRESSED,
			 0x2a71,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3bce, // start_of_image_data;
			 0,      // image_address_offset
			 39,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Perseus and Andromeda",
			 PERSEUS,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 60,  // Number of items
			 178, // Number of actions
			 130, // Number of words
			 40,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 131, // number_of_verbs
			 130, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x4823,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 39,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Perseus and Andromeda C64",
			 PERSEUS_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 59,  // Number of items
			 165, // Number of actions
			 130, // Number of words
			 40,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 131, // number_of_verbs
			 82,  // number_of_nouns;

			 0x1dd9, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df3, // actions
			 UNCOMPRESSED,
			 0x2853,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3d91, // start_of_image_data;
			 0,      // image_address_offset
			 39,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Ten Little Indians",
			 INDIANS,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 73,  // Number of items
			 161, // Number of actions
			 85,  // Number of words
			 63,  // Number of rooms
			 5,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 86, // number_of_verbs
			 85, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x47b7,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 62,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Ten Little Indians C64",
			 INDIANS_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 73,  // Number of items
			 161, // Number of actions
			 82,  // Number of words
			 63,  // Number of rooms
			 5,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 64, // number_of_verbs
			 82, // number_of_nouns;

			 0x1dd9,             // header
			 INDIANS_C64_HEADER, // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df0, // actions
			 UNCOMPRESSED,
			 0x2810,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3a46, // start_of_image_data;
			 0,      // image_address_offset
			 62,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Waxworks",
			 WAXWORKS,
			 OLD_STYLE,                // type
			 MYSTERIOUS,               // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 57,  // Number of items
			 189, // Number of actions
			 106, // Number of words
			 41,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 107, // number_of_verbs
			 106, // number_of_nouns;

			 0x2351, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x3b81, // actions
			 UNCOMPRESSED,
			 0x48d3,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x24c0, // start_of_system_messages
			 0x2780, // start of directions

			 0,       // start_of_characters;
			 FOLLOWS, // start_of_image_data;
			 0,       // image_address_offset
			 40,      // number_of_pictures;
			 ZXOPT,   // palette
			 99,      // picture_format_version;
			 0),
	GameInfo("Waxworks C64",
			 WAXWORKS_C64,
			 NO_TYPE,                                          // type
			 static_cast<Subtype>(MYSTERIOUS | ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,                         // dictionary type

			 57,  // Number of items
			 189, // Number of actions
			 105, // Number of words
			 41,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 82,  // Number of messages

			 91,  // number_of_verbs
			 105, // number_of_nouns;

			 0x1dd9, // header
			 EARLY,  // header style

			 0, // no room images
			 0, // no item flags
			 0, // no item images

			 0x1df3, // actions
			 UNCOMPRESSED,
			 0x29d3,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x408, // start_of_system_messages
			 0x408, // start of directions

			 0,      // start_of_characters;
			 0x3f31, // start_of_image_data;
			 0,      // image_address_offset
			 40,     // number_of_pictures;
			 C64A,   // palette
			 99,     // picture_format_version;
			 0),
	GameInfo("Questprobe 1: The Hulk",
			 HULK,
			 NO_TYPE,                  // type
			 ENGLISH,                  // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 54,  // Number of items
			 261, // Number of actions
			 128, // Number of words
			 20,  // Number of rooms
			 10,  // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 128, // number_of_verbs
			 129, // number_of_nouns;

			 0x4bf4,      // header
			 HULK_HEADER, // header style

			 0x270c, // room images
			 0,      // item flags
			 0x26c8, // item images

			 0x6087, // actions
			 HULK_ACTIONS,
			 0x4cc4,  // dictionary
			 0x51cd,  // start_of_room_descriptions;
			 0x7111,  // start_of_room_connections;
			 0x575e,  // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 0x5f3d,  // start_of_item_locations;

			 0x2553, // start_of_system_messages
			 0x28f7, // start of directions

			 0x281b, // start_of_characters;
			 0x2782, // start_of_image_data
			 0,      // image_address_offset
			 43,     // number_of_pictures;
			 ZXOPT,  // palette
			 0,      // picture_format_version;
			 0),
	GameInfo("Robin of Sherwood",
			 ROBIN_OF_SHERWOOD,
			 SHERWOOD_VARIANT,       // type
			 ENGLISH,                // subtype
			 FOUR_LETTER_COMPRESSED, // dictionary type

			 87,  // Number of items
			 295, // Number of actions
			 114, // Number of words
			 93,  // Number of rooms
			 10,  // Max carried items
			 4,   // Word length
			 98,  // Number of messages

			 115, // number_of_verbs
			 109, // number_of_nouns;

			 0x3b5a, // header
			 LATE,   // header style

			 0,       // 0x3d99 room images, zero because it needs custom handling
			 0x3db8,  // item flags
			 FOLLOWS, // item images

			 0x409b, // actions
			 COMPRESSED,
			 0x4dc3, // dictionary
			 0,      // 0x9b53 start_of_room_descriptions, zero because of custom handling
			 0x3e67, // start_of_room_connections
			 0x5147, // start_of_messages
			 0x5d65, // start_of_item_descriptions
			 0x4d6b, // start_of_item_locations

			 0x250b, // start_of_system_messages
			 0x26b5, // start of directions

			 0x614f, // start_of_characters
			 0x66bf, // start_of_image_data
			 0x6765, // image_address_offset
			 83,     // number_of_pictures
			 ZXOPT,  // palette
			 4,      // picture_format_version
			 0),
	GameInfo("Robin of Sherwood C64",
			 ROBIN_OF_SHERWOOD_C64,
			 SHERWOOD_VARIANT,                    // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 FOUR_LETTER_COMPRESSED,              // dictionary type

			 87,  // Number of items
			 296, // Number of actions
			 114, // Number of words
			 93,  // Number of rooms
			 10,  // Max carried items
			 4,   // Word length
			 98,  // Number of messages

			 115, // number_of_verbs
			 109, // number_of_nouns;

			 0x1f85,           // header
			 ROBIN_C64_HEADER, // header style

			 0,       // room images, zero because it needs custom handling
			 0x201c,  // item flags
			 FOLLOWS, // item images

			 0x252b, // actions
			 COMPRESSED,
			 0x320a, // dictionary
			 0,      // 0x9b53 start_of_room_descriptions, zero because of custom handling
			 0x20cc, // start_of_room_connections
			 0x358e, // start_of_messages
			 0x4215, // start_of_item_descriptions
			 0x1fa5, // start_of_item_locations

			 0x428, // start_of_system_messages
			 0x428, // start of directions

			 0x45ff, // start_of_characters
			 0x4b6f, // start_of_image_data
			 0x4c23, // image_address_offset
			 90,     // number_of_pictures
			 C64B,   // palette
			 4,      // picture_format_version
			 0),
	GameInfo("Gremlins",
			 GREMLINS,
			 GREMLINS_VARIANT,         // type
			 ENGLISH,                  // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 99,  // Number of items
			 236, // Number of actions
			 126, // Number of words
			 42,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 98,  // Number of messages

			 115, // number_of_verbs
			 126, // number_of_nouns;

			 0x2370, // header
			 LATE,   // header style

			 0x3a09,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images

			 0x3afe,     // actions
			 COMPRESSED, // actions_style;
			 0x45e5,     // dictionary
			 FOLLOWS,    // start_of_room_descriptions;
			 FOLLOWS,    // start_of_room_connections;
			 FOLLOWS,    // start_of_messages;
			 FOLLOWS,    // start_of_item_descriptions;
			 FOLLOWS,    // start_of_item_locations;
			 0x2426,     // start_of_system_messages
			 0x25f6,     // start of directions

			 0x5be1, // start_of_characters;
			 0x63e1, // start_of_image_data
			 0x64e1, // image_address_offset
			 78,     // number_of_pictures;
			 ZXOPT,  // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Gremlins (alternative)",
			 GREMLINS_ALT,
			 GREMLINS_VARIANT,         // type
			 ENGLISH,                  // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 99,  // Number of items
			 236, // Number of actions
			 126, // Number of words
			 42,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 98,  // Number of messages

			 115, // number_of_verbs
			 126, // number_of_nouns;

			 0x2378, // header
			 LATE,   // header style

			 0x3a0d,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images

			 0x3b02,     // actions
			 COMPRESSED, // actions_style;
			 0x45e5,     // dictionary
			 FOLLOWS,    // start_of_room_descriptions;
			 FOLLOWS,    // start_of_room_connections;
			 FOLLOWS,    // start_of_messages;
			 FOLLOWS,    // start_of_item_descriptions;
			 FOLLOWS,    // start_of_item_locations;
			 0x242e,     // start_of_system_messages
			 0x25FE,     // start of directions

			 0x5bdd, // start_of_characters;
			 0x63dd, // start_of_image_data
			 0x64dd, // image_address_offset
			 78,     // number_of_pictures;
			 ZXOPT,  // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Gremlins (German)",
			 GREMLINS_GERMAN,
			 GREMLINS_VARIANT, // type
			 LOCALIZED,        // subtype
			 GERMAN,           // dictionary type

			 99,  // Number of items
			 236, // Number of actions
			 126, // Number of words
			 42,  // Number of rooms
			 6,   // Max carried items
			 5,   // Word length
			 98,  // Number of messages

			 115, // number_of_verbs
			 126, // number_of_nouns

			 0x237d, // header
			 LATE,   // header style

			 0x3a07,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images

			 0x3afc, // actions
			 COMPRESSED,
			 0x45d9,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x23de, // start_of_system_messages
			 0x2623, // start of directions

			 0x643e, // start_of_characters
			 0x6c3e, // start_of_image_data
			 0x6d3e, // image_address_offset
			 72,     // number_of_pictures;
			 ZX,     // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Gremlins C64",
			 GREMLINS_C64,
			 GREMLINS_VARIANT,                    // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,            // dictionary type

			 99,  // Number of items
			 243, // Number of actions
			 126, // Number of words
			 42,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 115, // number_of_verbs
			 126, // number_of_nouns;

			 0x4584,              // header
			 GREMLINS_C64_HEADER, // header style

			 0x465e,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images

			 0x4751,     // actions
			 COMPRESSED, // actions_style;
			 0x527f,     // dictionary
			 FOLLOWS,    // start_of_room_descriptions;
			 FOLLOWS,    // start_of_room_connections;
			 FOLLOWS,    // start_of_messages;
			 FOLLOWS,    // start_of_item_descriptions;
			 0x4596,     // start_of_item_locations;
			 0x0a5e,     // start_of_system_messages
			 0x0a5e,     // start of directions

			 0x6a01, // start_of_characters;
			 0x7201, // start_of_image_data
			 0x7301, // image_address_offset
			 91,     // number_of_pictures;
			 C64B,   // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Gremlins (German, C64)",
			 GREMLINS_GERMAN_C64,
			 GREMLINS_VARIANT,                      // type
			 static_cast<Subtype>(LOCALIZED | C64), // subtype
			 FIVE_LETTER_COMPRESSED,                // dictionary type

			 99,  // Number of items
			 243, // Number of actions
			 125, // Number of words
			 42,  // Number of rooms
			 6,   // Max carried items
			 5,   // Word length
			 98,  // Number of messages

			 125, // number_of_verbs
			 124, // number_of_nouns

			 0x4bb2,              // header
			 GREMLINS_C64_HEADER, // header style

			 0x4c8c,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images

			 0x4d81, // actions
			 COMPRESSED,
			 0x585e,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 0x4bc4,  // start_of_item_locations;

			 0x1325, // start_of_system_messages
			 0x1325, // start of directions

			 0x76c5, // start_of_characters
			 0x7cfd, // start_of_image_data
			 0x7db3, // image_address_offset
			 91,     // number_of_pictures;
			 C64B,   // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Gremlins (Spanish)",
			 GREMLINS_SPANISH,
			 GREMLINS_VARIANT, // type
			 LOCALIZED,        // subtype
			 SPANISH,          // dictionary type

			 99,  // Number of items
			 236, // Number of actions
			 126, // Number of words
			 42,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 98,  // Number of messages

			 115, // number_of_verbs
			 126, // number_of_nouns

			 0x23c5, // header
			 LATE,   // header style

			 0x3993,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images

			 0x3a88, // actions
			 COMPRESSED,
			 0x455f,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x2426, // start_of_system_messages
			 0x25c4, // start of directions

			 0x6171, // start_of_characters
			 0x6971, // start_of_image_data
			 0x6A71, // image_address_offset
			 74,     // number_of_pictures;
			 ZXOPT,  // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Seas of Blood",
			 SEAS_OF_BLOOD,
			 SEAS_OF_BLOOD_VARIANT,  // type
			 ENGLISH,                // subtype
			 FOUR_LETTER_COMPRESSED, // dictionary type

			 125, // Number of items
			 344, // Number of actions
			 134, // Number of words
			 83,  // Number of rooms
			 10,  // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 69,  // number_of_verbs
			 134, // number_of_nouns;

			 0x494d, // header
			 LATE,   // header style

			 0,       // no room images
			 0x4961,  // item flags
			 FOLLOWS, // item images

			 FOLLOWS, // actions
			 COMPRESSED,
			 0x591b, // dictionary
			 0x67cb, // start_of_room_descriptions;
			 0x5c4b, // start_of_room_connections;
			 0x5ebb, // start_of_messages;
			 0x6ce0, // start_of_item_descriptions;
			 0x5e3d, // start_of_item_locations;

			 0x24ed, // start_of_system_messages
			 0x26fc, // start of directions

			 0x7389, // start_of_characters;
			 0x7a89, // start_of_image_data;
			 0x7b9f, // image_address_offset
			 139,    // number_of_pictures;
			 ZXOPT,  // palette
			 4,      // picture_format_version;
			 0),
	GameInfo("Seas of Blood C64",
			 SEAS_OF_BLOOD_C64,
			 SEAS_OF_BLOOD_VARIANT,               // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 FOUR_LETTER_COMPRESSED,              // dictionary type

			 125, // Number of items
			 347, // Number of actions
			 134, // Number of words
			 82,  // Number of rooms
			 10,  // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 69,  // number_of_verbs
			 134, // number_of_nouns;

			 0x527d,                   // header
			 SEAS_OF_BLOOD_C64_HEADER, // header style

			 0,       // no room images
			 0x5a73,  // item flags
			 FOLLOWS, // item images

			 0x5b69, // actions
			 COMPRESSED,
			 0x6a2d, // dictionary
			 0x78dd, // start_of_room_descriptions;
			 0x6d5d, // start_of_room_connections;
			 0x6fcd, // start_of_messages;
			 0x7df2, // start_of_item_descriptions;
			 0x6f4f, // start_of_item_locations;

			 0x0b9d, // start_of_system_messages
			 0x0b9d, // start of directions

			 0x84a5, // start_of_characters;
			 0x8ba5, // start_of_image_data;
			 0x8CBB, // image_address_offset
			 139,    // number_of_pictures;
			 C64B,   // palette
			 4,      // picture_format_version;
			 0),
	GameInfo("The Sorcerer of Claymorgue Castle",
			 CLAYMORGUE,
			 NO_TYPE,                  // type
			 ENGLISH,                  // subtype
			 FIVE_LETTER_UNCOMPRESSED, // dictionary type

			 75,  // Number of items
			 267, // Number of actions
			 109, // Number of words
			 32,  // Number of rooms
			 10,  // Max carried items
			 5,   // Word length
			 79,  // Number of messages

			 110, // number_of_verbs
			 108, // number_of_nouns;

			 0x246c, // header
			 EARLY,  // header style

			 0x3bf6,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images
			 FOLLOWS, // actions
			 UNCOMPRESSED,
			 0x4ecf,  // dictionary
			 0x53f7,  // start_of_room_descriptions;
			 0x4d6f,  // start_of_room_connections;
			 0x5605,  // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 0x4e35,  // start_of_item_locations;

			 0x24e2, // start_of_system_messages
			 0x2877, // start of directions

			 0x6007,  // start_of_characters;
			 0x6807,  // start_of_image_data
			 -0x3fe5, // image_address_offset
			 37,      // number_of_pictures;
			 ZXOPT,   // palette
			 1,       // picture_format_version;
			 0),
	GameInfo("The Sorcerer of Claymorgue Castle C64",
			 CLAYMORGUE_C64,
			 NO_TYPE,                             // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 FIVE_LETTER_UNCOMPRESSED,            // dictionary type

			 75,  // Number of items
			 265, // Number of actions
			 108, // Number of words
			 32,  // Number of rooms
			 10,  // Max carried items
			 5,   // Word length
			 79,  // Number of messages

			 110, // number_of_verbs
			 108, // number_of_nouns;

			 0x47d6, // header
			 EARLY,  // header style

			 0x471f,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images
			 0x47f0,  // actions
			 UNCOMPRESSED,
			 0x5891,  // dictionary
			 FOLLOWS, // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0xa2c, // start_of_system_messages
			 0xa2c, // start of directions

			 0x6b1a, // start_of_characters;
			 0x731a, // start_of_image_data
			 0x6b1a, // image_address_offset
			 44,     // number_of_pictures;
			 C64B,   // palette
			 1,      // picture_format_version;
			 0),
	GameInfo("Adventureland",
			 ADVENTURELAND,
			 NO_TYPE,                   // type
			 ENGLISH,                   // subtype
			 THREE_LETTER_UNCOMPRESSED, // dictionary type

			 65,  // Number of items
			 181, // Number of actions
			 69,  // Number of words
			 33,  // Number of rooms
			 6,   // Max carried items
			 3,   // Word length
			 75,  // Number of messages

			 70, // number_of_verbs
			 69, // number_of_nouns;

			 0x2473, // header
			 EARLY,  // header style

			 0x3ebe,  // room images
			 FOLLOWS, // item flags
			 0x3f1f,  // item images
			 0x3f5e,  // actions
			 UNCOMPRESSED,
			 0x4c10,  // dictionary
			 0x4e40,  // start_of_room_descriptions;
			 0x4abe,  // start_of_room_connections;
			 0x52c3,  // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 0x4b8a,  // start_of_item_locations;

			 0x24eb, // start_of_system_messages
			 0x285e, // start of directions

			 0x631e,  // start_of_characters;
			 FOLLOWS, // start_of_image_data
			 -0x3fe5, // image_address_offset
			 41,      // number_of_pictures;
			 ZXOPT,   // palette
			 1,       // picture_format_version;
			 0),
	GameInfo("Adventureland C64",
			 ADVENTURELAND_C64,
			 NO_TYPE,                             // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 THREE_LETTER_UNCOMPRESSED,           // dictionary type

			 62,  // Number of items
			 170, // Number of actions
			 69,  // Number of words
			 33,  // Number of rooms
			 6,   // Max carried items
			 3,   // Word length
			 75,  // Number of messages

			 70, // number_of_verbs
			 69, // number_of_nouns;

			 0x4146, // header
			 EARLY,  // header style

			 0x6364, // room images
			 0x85a7, // item flags
			 0x63c5, // item images
			 0x4160, // actions
			 UNCOMPRESSED,
			 0x4c10,  // dictionary
			 0x4e40,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 0x6404,  // start_of_item_locations;

			 0x188, // start_of_system_messages
			 0x188, // start of directions

			 0x6482,  // start_of_characters;
			 0x6c82,  // start_of_image_data
			 -0x1102, // image_address_offset
			 41,      // number_of_pictures;
			 C64B,    // palette
			 1,       // picture_format_version;
			 0),
	GameInfo("Savage Island part I",
			 SAVAGE_ISLAND,
			 SAVAGE_ISLAND_VARIANT,    // type
			 ENGLISH,                  // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 58,  // Number of items
			 259, // Number of actions
			 84,  // Number of words
			 34,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 82, // number_of_verbs
			 84, // number_of_nouns;

			 0x236d,  // header
			 LATE,    // header style
			 0x390c,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images
			 0x39a6,  // actions
			 COMPRESSED,
			 0x4484,  // dictionary
			 0x47c7,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 0x4b91,  // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x2423, // start_of_system_messages
			 0x25f3, // start of directions

			 0x570f,  // start_of_characters;
			 FOLLOWS, // start_of_image_data
			 0x600f,  // image_address_offset
			 37,      // number_of_pictures;
			 ZXOPT,   // palette
			 3,       // picture_format_version;
			 0),
	GameInfo("Savage Island part I (C64)",
			 SAVAGE_ISLAND_C64,
			 SAVAGE_ISLAND_VARIANT,               // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,            // dictionary type

			 58,  // Number of items
			 259, // Number of actions
			 84,  // Number of words
			 34,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 99,  // Number of messages

			 85, // number_of_verbs
			 84, // number_of_nouns;

			 0x46dc,  // header
			 EARLY,   // header style
			 0x4645,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images
			 0x46f6,  // actions
			 COMPRESSED,
			 0x51d6,  // dictionary
			 0x5528,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
					  //        0x58f6, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x0a5f, // start_of_system_messages
			 0x0a5f, // start of directions

			 0x6A02, // start_of_characters;
			 0x7202, // start_of_image_data
			 0x7302, // image_address_offset
			 37,     // number_of_pictures;
			 C64B,   // palette
			 3,      // picture_format_version;
			 0),
	GameInfo("Savage Island part II",
			 SAVAGE_ISLAND2,
			 NO_TYPE,                  // type
			 ENGLISH,                  // subtype
			 FOUR_LETTER_UNCOMPRESSED, // dictionary type

			 48,  // Number of items
			 241, // Number of actions
			 79,  // Number of words
			 30,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 95,  // Number of messages

			 74, // number_of_verbs
			 79, // number_of_nouns;

			 0x236d,  // header
			 LATE,    // header style
			 0x390c,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images
			 0x398e,  // actions
			 COMPRESSED,
			 0x43f0,  // dictionary
			 0x46ed,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x2423, // start_of_system_messages
			 0x25f3, // start of directions

			 0x57d0,  // start_of_characters;
			 FOLLOWS, // start_of_image_data
			 0x60d0,  // image_address_offset
			 21,      // number_of_pictures;
			 ZXOPT,   // palette
			 3,       // picture_format_version;
			 0),
	GameInfo("Savage Island part II (C64)",
			 SAVAGE_ISLAND2_C64,
			 NO_TYPE,                             // type
			 static_cast<Subtype>(ENGLISH | C64), // subtype
			 FOUR_LETTER_UNCOMPRESSED,            // dictionary type

			 48,  // Number of items
			 241, // Number of actions
			 78,  // Number of words
			 30,  // Number of rooms
			 6,   // Max carried items
			 4,   // Word length
			 94,  // Number of messages

			 79, // number_of_verbs
			 79, // number_of_nouns;

			 0x4654,  // header
			 EARLY,   // header style
			 0x6310,  // room images
			 FOLLOWS, // item flags
			 FOLLOWS, // item images
			 0x466e,  // actions
			 COMPRESSED,
			 0x50d2,  // dictionary
			 0x53e8,  // start_of_room_descriptions;
			 FOLLOWS, // start_of_room_connections;
			 FOLLOWS, // start_of_messages;
			 FOLLOWS, // start_of_item_descriptions;
			 FOLLOWS, // start_of_item_locations;

			 0x0a50, // start_of_system_messages
			 0x0a50, // start of directions

			 0x6A02, // start_of_characters;
			 0x7202, // start_of_image_data
			 0x7302, // image_address_offset
			 21,     // number_of_pictures;
			 C64B,   // palette
			 3,      // picture_format_version;
			 0)
};
