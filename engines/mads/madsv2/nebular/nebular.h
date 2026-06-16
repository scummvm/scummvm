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

#ifndef MADS_NEBULAR_H
#define MADS_NEBULAR_H

#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

struct MADSSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;

	void readSavegameHeader(Common::SeekableReadStream *src);
	void writeSavegameHeader(Common::WriteStream *dest);
};

class RexNebularEngine : public MADSV2Engine {
private:
	void showRecipe();

public:
	RexNebularEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		MADSV2Engine(syst, gameDesc) {}
	~RexNebularEngine() override {}

	Common::Error run() override;
	void syncRoom(Common::Serializer &s) override;

	void global_init_code() override;
	void section_music(int section_num) override;
	void global_section_constructor() override;
	void global_daemon_code() override;
	void global_pre_parser_code() override {}
	void global_parser_code() override;
	void global_error_code() override;
	void global_room_init() override {}
	void global_sound_driver() override;
	void player_keep_walking() override;
};

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
