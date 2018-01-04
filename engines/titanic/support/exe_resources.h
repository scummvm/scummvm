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

#ifndef TITANIC_EXE_RESOURCES_H
#define TITANIC_EXE_RESOURCES_H

namespace Titanic {

class CScriptHandler;

enum FileHandle { HANDLE_STDIN = 0, HANDLE_STDOUT = 1, HANDLE_STDERR = 2 };

enum VocabMode { VOCAB_MODE_NONE = 0, VOCAB_MODE_EN = 3, VOCAB_MODE_DE = 5 };

class CExeResources {
public:
	CScriptHandler *_owner;
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
	VocabMode _vocabMode;
public:
	CExeResources();

	void reset(CScriptHandler *owner, int val1, VocabMode vocabMode);

	/**
	 * Tests whether the vocab mode equals the passed mode
	 */
	bool isVocabMode(int mode) const { return _vocabMode == mode; }

	/**
	 * Returns the vocab mode
	 */
	VocabMode getVocabMode() const { return _vocabMode; }
};

} // End of namespace Titanic

#endif /* TITANIC_EXE_RESOURCES_H */
