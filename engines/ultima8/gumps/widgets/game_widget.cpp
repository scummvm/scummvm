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

#include "ultima8/misc/pent_include.h"
#include "ultima8/gumps/widgets/game_widget.h"

#include "ultima8/ultima8.h"
#include "ultima8/games/game_info.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/gumps/widgets/button_widget.h"
#include "ultima8/gumps/pentagram_menu_gump.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(GameWidget, Gump)


GameWidget::GameWidget(int X, int Y, Pentagram::istring &game)
	: Gump(X, Y, 443, 109), highlight(false) {
	info = Ultima8Engine::get_instance()->getGameInfo(game);
	assert(info);
}

GameWidget::~GameWidget() {

}

Pentagram::istring GameWidget::getGameName() {
	return info->name;
}
void GameWidget::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	// setup children
	Gump *w = new TextWidget(65, 2, info->getGameTitle(),
	                         false, 0, 350, 180);
	w->InitGump(this, false);

	// FIXME: localize these strings
	std::string gamename = "Game:";
	gamename += " ";
	gamename += info->name;
	w = new TextWidget(65, 29, gamename, false, 0, 350, 180);
	w->InitGump(this, false);

	std::string path = "Path:";
	path += " ";
	w = new TextWidget(65, 50, path, false, 0, 350, 180);
	w->InitGump(this, false);

	std::string version = info->getPrintableVersion();
	w = new TextWidget(360, 70, version, false, 2, 70, 0, Font::TEXT_RIGHT);
	w->InitGump(this, false);

	w = new ButtonWidget(13, 86, "Play Game", false, 1, 0x80D000D0);
	w->SetIndex(static_cast<int32>(GAME_PLAY));
	w->InitGump(this, false);

	w = new ButtonWidget(122, 86, "Load Savegame", false, 1, 0x80D000D0);
	w->SetIndex(static_cast<int32>(GAME_LOAD));
	w->InitGump(this, false);

	w = new ButtonWidget(270, 86, "Settings", false, 1, 0x80D000D0);
	w->SetIndex(static_cast<int32>(GAME_SETTINGS));
	w->InitGump(this, false);

	w = new ButtonWidget(361, 86, "Remove", false, 1, 0x80D000D0);
	w->SetIndex(static_cast<int32>(GAME_REMOVE));
	w->InitGump(this, false);
}

void GameWidget::ChildNotify(Gump *child, uint32 message) {
	if (child->IsOfType<ButtonWidget>() &&
	        message == ButtonWidget::BUTTON_CLICK) {
		int32 index_ = child->GetIndex();

		if (parent)
			parent->ChildNotify(this, static_cast<uint32>(index_));
	}
}

uint16 GameWidget::TraceObjId(int mx, int my) {
	uint16 objId_ = Gump::TraceObjId(mx, my);
	if (!objId_) objId_ = getObjId();
	return objId_;
}

Gump *GameWidget::OnMouseDown(int button, int mx, int my) {
	Gump *g = Gump::OnMouseDown(button, mx, my);
	if (!g) g = this;
	return g;
}

void GameWidget::OnMouseOver() {
	highlight = true;
}

void GameWidget::OnMouseLeft() {
	highlight = false;
}


void GameWidget::PaintThis(RenderSurface *surf, int32 lerp_factor, bool /*scaled*/) {
	PentagramMenuGump *p = p_dynamic_cast<PentagramMenuGump *>(GetParent());
	Texture *coversImage = p->getCovers();
	Texture *flagsImage = p->getFlags();

	// Note: we're not painting a background to make this widget transparent

	// outer border
	surf->Fill32(0xFFFFFFFF, 0, 0, 443, 1);
	surf->Fill32(0xFFFFFFFF, 0, 0, 1, 109);
	surf->Fill32(0xFFFFFFFF, 0, 108, 443, 1);
	surf->Fill32(0xFFFFFFFF, 442, 0, 1, 109);

	// line above buttons
	surf->Fill32(0xFFFFFFFF, 0, 81, 443, 1);

	// line to the right of box graphics
	surf->Fill32(0xFFFFFFFF, 57, 0, 1, 81);

	// line below game description
	surf->Fill32(0xFFFFFFFF, 57, 23, 443 - 57, 1);

	// box graphics
//	surf->Fill32(0xFFAFAFFF,1,1,56,80);
	surf->Blit(coversImage, (info->type - 1) * 56, 0, 56, 80, 1, 1); // HACK...

	if (highlight)
		surf->Fill32(0xFF30308F, 58, 1, 443 - 57 - 2, 22);

	// flag
//	surf->Fill32(0xFFAFFFAF,415,4,24,16);
	surf->Blit(flagsImage, (info->language - 1) * 24, 0, 24, 16, 415, 4); // HACK...


#if 0
	Pentagram::Font *font = FontManager::get_instance()->getTTFont(1);
	assert(font);

	// FIXME: convert these into ButtonWidgets, localize texts,
	//        adjust sizes/positions depending on texts
	surf->Fill32(0xFFE0E0E0, 13, 85, 87, 1);
	surf->Fill32(0xFFC0C0C0, 13, 86, 1, 18);
	surf->Fill32(0xFF808080, 99, 86, 1, 18);
	surf->Fill32(0xFF404040, 13, 104, 87, 1);

	unsigned int rem;
	std::string button = "Play Game";
	RenderedText *t = font->renderText(button, rem, 0, 0);
	t->draw(surf, 19, 99);
	delete t;

	surf->Fill32(0xFFE0E0E0, 122, 85, 126, 1);
	surf->Fill32(0xFFC0C0C0, 122, 86, 1, 18);
	surf->Fill32(0xFF808080, 247, 86, 1, 18);
	surf->Fill32(0xFF404040, 122, 104, 126, 1);

	button = "Load Savegame";
	t = font->renderText(button, rem, 0, 0);
	t->draw(surf, 128, 99);
	delete t;

	surf->Fill32(0xFFE0E0E0, 270, 85, 69, 1);
	surf->Fill32(0xFFC0C0C0, 270, 86, 1, 18);
	surf->Fill32(0xFF808080, 338, 86, 1, 18);
	surf->Fill32(0xFF404040, 270, 104, 69, 1);

	button = "Settings";
	t = font->renderText(button, rem, 0, 0);
	t->draw(surf, 276, 99);
	delete t;

	surf->Fill32(0xFFE0E0E0, 361, 85, 67, 1);
	surf->Fill32(0xFFC0C0C0, 361, 86, 1, 18);
	surf->Fill32(0xFF808080, 427, 86, 1, 18);
	surf->Fill32(0xFF404040, 361, 104, 67, 1);

	button = "Remove";
	t = font->renderText(button, rem, 0, 0);
	t->draw(surf, 367, 99);
	delete t;
#endif
}

} // End of namespace Ultima8
