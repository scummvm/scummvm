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

//=============================================================================
//
// ConsoleOutputTarget prints messages onto in-game console GUI (available
// only if the game was compiled in debug mode).
//
//=============================================================================

#ifndef AGS_ENGINE_DEBUGGING_CONSOLEOUTPUTTARGET_H
#define AGS_ENGINE_DEBUGGING_CONSOLEOUTPUTTARGET_H

#include "debug/outputhandler.h"

namespace AGS {
namespace Engine {

using Common::String;
using Common::DebugMessage;

class ConsoleOutputTarget : public AGS::Common::IOutputHandler {
public:
	ConsoleOutputTarget();
	virtual ~ConsoleOutputTarget();

	void PrintMessage(const DebugMessage &msg) override;
};

}   // namespace Engine
}   // namespace AGS

#endif
