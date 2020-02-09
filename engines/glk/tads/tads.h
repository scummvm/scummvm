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

#ifndef GLK_TADS
#define GLK_TADS

#include "common/scummsys.h"
#include "glk/glk_api.h"

namespace Glk {
namespace TADS {

/**
 * tads game interpreter
 */
class TADS : public GlkAPI {
protected:
	winid_t story_win, status_win;
	uint mainfg, mainbg;
	uint statusfg, statusbg;
public:
	/**
	 * Constructor
	 */
	TADS(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override { return INTERPRETER_SCOTT; }

	/**
	  * Returns true whether a given feature is supported by the engine
	  */
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;
};

extern TADS *g_vm;

} // End of namespace TADS
} // End of namespace Glk

#endif
