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

#ifndef ACCESS_MARTIAN_GAME_H
#define ACCESS_MARTIAN_GAME_H

#include "access/access.h"

namespace Access {

namespace Martian {

class MartianEngine : public AccessEngine {
private:
	bool _skipStart;
	SpriteResource *_introObjects;
	Common::MemoryReadStream *_creditsStream;
	/**
	 * Do the game introduction
	 */
	void doCredits();

	bool showCredits();

	/**
	 * Setup variables for the game
	 */
	void setupGame();

	void initObjects();
	void configSelect();
	void initVariables();
protected:
	/**
	 * Play the game
	 */
	void playGame() override;

	void dead(int deathId) override;

	void setNoteParams();
	void displayNote(const Common::String &msg);
public:
	SpriteResource *_spec7Objects;

	MartianEngine(OSystem *syst, const AccessGameDescription *gameDesc);
	~MartianEngine() override;

	void doSpecial5(int param1);
	void showDeathText(Common::String msg);
	void establish(int esatabIndex, int sub) override {};
};

} // End of namespace Martian

} // End of namespace Access

#endif /* ACCESS_MARTIAN_GAME_H */
