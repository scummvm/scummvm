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

#ifndef SCUMM_HE_MOONBASE_NET_MAIN_H
#define SCUMM_HE_MOONBASE_NET_MAIN_H

namespace Scumm {

class ScummEngine_v100he;

//this is a dummy based on ai_main.h Scumm::AI

class Net {
public:
	Net(ScummEngine_v100he *vm);

	int hostGame(char *sessionName, char *userName);
	int joinGame(char *IP, char *userName);
	int addUser(char *shortName, char *longName);
	int removeUser();
	int whoSentThis();
	int whoAmI();
private:

public:
	//getters

private:
	//mostly getters

public:
	//fields

	ScummEngine_v100he *_vm;
};

} // End of namespace Scumm

#endif
