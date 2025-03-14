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

#ifndef HODJNPODJ_VIEWS_RULES_H
#define HODJNPODJ_VIEWS_RULES_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {

class Rules : public View {
private:
	OkButton _okButton;
	GfxSurface _background, _scroll;
	GfxSurface _scrollTop, _scrollBottom, _scrollMiddle;
	GfxSurface _scrollContent;
	const char *_filename, *_soundFilename;
	CBofSound *_dictation = nullptr;
	Common::Array<Common::StringArray> _lines;
	uint _helpPage = 0;
	Common::Rect _moreTopRect, _moreBottomRect;
	Common::String _more;
	int _scrollY = 0;
	Common::Rect _scrollTopRect, _scrollBottomRect;

	void renderPage();

public:
	Rules();
	virtual ~Rules() {}

	static void show(const char *filename,
		const char *soundFilename);

	void draw() override;
	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	bool tick() override;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
