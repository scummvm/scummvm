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

#ifndef ZVISION_CONSOLE_H
#define ZVISION_CONSOLE_H

#include "gui/debugger.h"

namespace ZVision {

class ZVision;

class Console : public GUI::Debugger {
public:
	Console(ZVision *engine);
	~Console() override {}

private:
	ZVision *_engine;

	bool cmdLoadVideo(int argc, const char **argv);
	bool cmdLoadSound(int argc, const char **argv);
	bool cmdRawToWav(int argc, const char **argv);
	bool cmdSetRenderState(int argc, const char **argv);
	bool cmdGenerateRenderTable(int argc, const char **argv);
	bool cmdSetPanoramaFoV(int argc, const char **argv);
	bool cmdSetPanoramaScale(int argc, const char **argv);
	bool cmdLocation(int argc, const char **argv);
	bool cmdDumpFile(int argc, const char **argv);
	bool cmdDumpFiles(int argc, const char **argv);
	bool cmdDumpImage(int argc, const char **argv);
	bool cmdStateValue(int argc, const char **argv);
	bool cmdStateFlag(int argc, const char **argv);
};

} // End of namespace ZVision
#endif
