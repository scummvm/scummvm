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
			 0)};
