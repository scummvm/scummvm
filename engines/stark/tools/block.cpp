/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/tools/block.h"

#include "engines/stark/tools/command.h"

#include "common/debug.h"

namespace Stark {
namespace Tools {

Block::Block() :
		_follower(nullptr),
		_trueBranch(nullptr),
		_falseBranch(nullptr) {

}

void Block::appendCommand(Command *command) {
	_commands.push_back(command);
	command->setBlock(this);
}

bool Block::isEmpty() const {
	return _commands.empty();
}

void Block::print() const {
	for (uint i = 0; i < _commands.size(); i++) {
		_commands[i]->printCall();
	}
}

void Block::setBranches(Block *trueBranch, Block *falseBranch) {
	_trueBranch = trueBranch;
	_falseBranch = falseBranch;

	trueBranch->addPredecessor(this);
	falseBranch->addPredecessor(this);
}

void Block::setFollower(Block *follower) {
	_follower = follower;
	follower->addPredecessor(this);
}

void Block::addPredecessor(Block *predecessor) {
	_predecessors.push_back(predecessor);
}

} // End of namespace Tools
} // End of namespace Stark


