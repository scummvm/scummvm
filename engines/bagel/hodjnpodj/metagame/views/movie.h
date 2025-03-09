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

#ifndef HODJNPODJ_METAGAME_VIEWS_MOVIE_H
#define HODJNPODJ_METAGAME_VIEWS_MOVIE_H

#include "video/avi_decoder.h"
#include "bagel/hodjnpodj/views/view.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

// Movie Identifiers
#define MOVIE_ID_NONE   0
#define MOVIE_ID_LOGO   1
#define MOVIE_ID_TITLE  2
#define MOVIE_ID_INTRO  3
#define MOVIE_ID_ENDING 4

#define LOGO_MOVIE      "video/logo.avi"
#define TITLE_MOVIE     "video/title.avi"
#define STARTUP_MOVIE   "video/intro.avi"
#define HODJ_WIN_MOVIE  "video/hodj.avi"
#define PODJ_WIN_MOVIE  "video/podj.avi"

class Movie : public View {
private:
	Video::AVIDecoder _video;
	int _movieId = 0;
	Graphics::Palette _savedPalette;

public:
	Movie();
	virtual ~Movie() {
	}

	void close();
	void draw() override;
	bool msgOpen(const OpenMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool tick() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
