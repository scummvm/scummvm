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

#ifndef ACCESS_NOCTROPOLIS_NOCTROPOLIS_GAME_H
#define ACCESS_NOCTROPOLIS_NOCTROPOLIS_GAME_H

#include "access/access.h"

namespace Access {

namespace Noctropolis {

/* _selectCommand enum values are different in Noctropolis */
enum NoctropolisCommands {
	kNoctCmdLook = 0, 		// same as MM/Amazon
	kNoctCmdOpen = 1, 		// same as MM/Amazon
	kNoctCmdMove = 2, 		// same as MM/Amazon
	kNoctCmdGetTake = 3, 	// same as MM/Amazon
	kNoctCmdTalk = 4, 		// *not* the same
	kNoctCmdUse = 5, 		// *not* the same
	kNoctCmdGoto = 6, 		// *not* the same
	kNoctCmdInventory = 7,
	kNoctCmdTravel = 8,
	kNoctCmdOptions = 9,
	kNoctCmdSaveLoad = 10,
};

class NoctropolisEngine : public AccessEngine {
public:
	NoctropolisEngine(OSystem *syst, const AccessGameDescription *gameDesc);

	~NoctropolisEngine();

	void playGame() override;
	void dead(int deathId) override {};
	void establish(int esatabIndex, int sub) override {};

	virtual int16 getScreenWidth() const override { return 640; }
	virtual int16 getScreenHeight() const override { return 400; }

	Player *_stil;

protected:
	void setupGame() override;
	void initObjects() override;

private:
	void doIntro();
	void doFlashLogo();
	void doPublisherLogo();
	void initVariables();
	
	Scripts *_invScript;
};

} // end namespace Noctropolis

} // end namespace Access

#endif // ACCESS_NOCTROPOLIS_NOCTROPOLIS_GAME_H
