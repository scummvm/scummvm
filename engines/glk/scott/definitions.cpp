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

#include "glk/scott/definitions.h"

namespace Glk {
namespace Scott {

GameInfo::GameInfo() {}

GameInfo::GameInfo(
					const char *title,
					GameIDType gameID,
					GameType type,
					Subtype subType,
					DictionaryType dictionary,
					int numberOfItems,
					int numberOfActions,
					int numberOfWords,
					int numberOfRooms,
					int maxCarried,
					int wordLength,
					int numberOfMessages,
					int numberOfVerbs,
					int numberOfNouns,
					int startOfHeader,
					HeaderType headerStyle,
					int startOfRoomImageList,
					int startOfItemFlags,
					int startOfItemImageList,
					int startOfActions,
					ActionTableType actionsStyle,
					int startOfDictionary,
					int startOfRoomDescriptions,
					int startOfRoomConnections,
					int startOfMessages,
					int startOfItemDescriptions,
					int startOfItemLocations,
					int startOfSystemMessages,
					int startOfDirections,
					int startOfCharacters,
					int startOfImageData,
					int imageAddressOffset,
					int numberOfPictures,
					PaletteType palette,
					int pictureFormatVersion,
					int startOfIntroText)
	: _title(title), _gameID(gameID), _type(type), _subType(subType), _dictionary(dictionary), _numberOfItems(numberOfItems),
	  _numberOfActions(numberOfActions), _numberOfWords(numberOfWords), _numberOfRooms(numberOfRooms), _maxCarried(maxCarried),
	  _wordLength(wordLength), _numberOfMessages(numberOfMessages), _numberOfVerbs(numberOfVerbs), _numberOfNouns(numberOfNouns),
	  _startOfHeader(startOfHeader), _headerStyle(headerStyle), _startOfRoomImageList(startOfRoomImageList),
	  _startOfItemFlags(startOfItemFlags), _startOfItemImageList(startOfItemImageList), _startOfActions(startOfActions),
	  _actionsStyle(actionsStyle), _startOfDictionary(startOfDictionary), _startOfRoomDescriptions(startOfRoomDescriptions),
	  _startOfRoomConnections(startOfRoomConnections), _startOfMessages(startOfMessages), _startOfItemDescriptions(startOfItemDescriptions),
	  _startOfItemLocations(startOfItemLocations), _startOfSystemMessages(startOfSystemMessages), _startOfDirections(startOfDirections),
	  _startOfCharacters(startOfCharacters), _startOfImageData(startOfImageData), _imageAddressOffset(imageAddressOffset),
	  _numberOfPictures(numberOfPictures), _palette(palette), _pictureFormatVersion(pictureFormatVersion), _startOfIntroText(startOfIntroText) {}

} // End of namespace Scott
} // End of namespace Glk
