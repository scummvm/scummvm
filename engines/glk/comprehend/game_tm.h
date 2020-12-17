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

#ifndef GLK_COMPREHEND_GAME_TM_H
#define GLK_COMPREHEND_GAME_TM_H

#include "glk/comprehend/game_opcodes.h"

namespace Glk {
namespace Comprehend {

class TalismanGame : public ComprehendGameV2 {
private:
	Common::String _savedAction;
private:
	/**
	 * Load strings from the executable
	 */
	void loadStrings();
public:
	TalismanGame();
	~TalismanGame() override {}

	void playGame() override;
	void beforeGame() override;
	void beforeTurn() override;
	void beforePrompt() override;
	void afterPrompt() override;
	void handleAction(Sentence *sentence) override;
	void handleSpecialOpcode() override;
};

} // namespace Comprehend
} // namespace Glk

#endif
