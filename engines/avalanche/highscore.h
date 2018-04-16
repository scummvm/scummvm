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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_HIGHSCORE_H
#define AVALANCHE_HIGHSCORE_H

namespace Avalanche {
class AvalancheEngine;

struct HighScoreData {
	Common::String _name;
	uint16 _score;
	Common::String _rank;
};

class HighScore {
public:
	HighScore(AvalancheEngine *vm);

	void displayHighScores();
	void saveHighScores();
	void loadHighScroes();

private:
	AvalancheEngine *_vm;

	HighScoreData _data[12];

	void produceDefaultHighScores();
	void syncHighScores(Common::Serializer &sz);
};

} // End of namespace Avalanche

#endif // AVALANCHE_HIGHSCORE_H
