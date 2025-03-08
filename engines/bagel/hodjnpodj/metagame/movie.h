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

#ifndef HODJNPODJ_METAGAME_MOVIE_H
#define HODJNPODJ_METAGAME_MOVIE_H

#include "video/avi_decoder.h"
#include "bagel/hodjnpodj/views/view.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class Movie : public View {
private:
	Video::AVIDecoder _video;

public:
	Movie();
	virtual ~Movie() {
	}

	void draw() override;
	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool tick() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
