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

#include "common/config-manager.h"
#include "image/bmp.h"

#include "ultima/ultima8/gumps/cru_demo_gump.h"

#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/audio/music_process.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruDemoGump)

CruDemoGump::CruDemoGump()
	: ModalGump(), _background(nullptr) {
}

CruDemoGump::CruDemoGump(Common::SeekableReadStream *bmprs, uint32 flags, int32 layer)
		: ModalGump(0, 0, 640, 480, 0, flags, layer), _background(nullptr)
{
	Image::BitmapDecoder decoder;
	_background = RenderSurface::CreateSecondaryRenderSurface(640, 480);

	uint32 color = TEX32_PACK_RGB(0, 0, 0);
	_background->fill32(color, 0, 0, 640, 480); // black background

	if (decoder.loadStream(*bmprs)) {
		// This does an extra copy via the ManagedSurface, but it's a once-off.
		const Graphics::Surface *bmpsurf = decoder.getSurface();
		Graphics::ManagedSurface ms(bmpsurf);
		ms.setPalette(decoder.getPalette(), 0, decoder.getPaletteColorCount());
		Common::Rect srcRect(640, 480);
		_background->Blit(ms, srcRect, 0, 0);
	} else {
		warning("couldn't load bitmap background for demo screen.");
	}
}

CruDemoGump::~CruDemoGump() {
	delete _background;
}

void CruDemoGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Mouse::get_instance()->pushMouseCursor(Mouse::MOUSE_NONE);

	MusicProcess *musicproc = MusicProcess::get_instance();
	if (musicproc) {
		if (GAME_IS_REMORSE) {
			musicproc->playMusic(21);
		} else {
			// TODO: What music do we play for Regret demo?
			musicproc->playMusic(18);
		}
	}
}

void CruDemoGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();

	ModalGump::Close(no_del);

	// Just let it play out?
	//MusicProcess *musicproc = MusicProcess::get_instance();
	//if (musicproc) musicproc->restoreMusic();
}

void CruDemoGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Common::Rect srcRect(640, 480);
	surf->Blit(*_background->getRawSurface(), srcRect, 0, 0);
}

bool CruDemoGump::OnKeyDown(int key, int mod) {
	if (key == Common::KEYCODE_ESCAPE)
		Close();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
