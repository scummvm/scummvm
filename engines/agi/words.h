/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef AGI_WORDS_H
#define AGI_WORDS_H

namespace Agi {

#define DICTIONARY_RESULT_UNKNOWN -1
#define DICTIONARY_RESULT_IGNORE   0

struct WordEntry {
	uint16 id;
	Common::String word;
};

class Words {
public:
	Words(AgiEngine *vm);
	~Words();

private:
	AgiEngine *_vm;

	// Dictionary
	Common::Array<WordEntry *> _dictionaryWords[26];

	WordEntry _egoWords[MAX_WORDS];
	uint16  _egoWordCount;

public:
	uint16 getEgoWordCount();
	const char *getEgoWord(int16 wordNr);
	uint16 getEgoWordId(int16 wordNr);

	int  loadDictionary_v1(Common::File &f);
	int  loadDictionary(const char *fname);
	void unloadDictionary();

	void clearEgoWords();
	void parseUsingDictionary(const char *rawUserInput);

private:
	void  cleanUpInput(const char *userInput, Common::String &cleanInput);
	int16 findWordInDictionary(const Common::String &userInput, uint16 userInputLen, uint16 userInputPos, uint16 &foundWordLen);
};

} // End of namespace Agi

#endif /* AGI_WORDS_H */
