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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/palette_fader_process.h"
#include "ultima/ultima8/graphics/fade_to_modal_process.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_class stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(FadeToModalProcess)


FadeToModalProcess::FadeToModalProcess(ModalGump *modal)
	: _modal(modal), _nextState(FS_OpenFadeOut), _fader(nullptr)
{
	setRunPaused();
}

FadeToModalProcess::~FadeToModalProcess(void) {
}

void FadeToModalProcess::onWakeUp() {
	if (_nextState == FS_CloseFadeIn) {
		// Jump in now and make sure the fade in is started (ie, we go to black)
		// before the modal is closed, otherwise a single frame of the thing
		// behind it will be shown first.
		_fader = new PaletteFaderProcess(0x00000000, true, 0x7FFF, 30, false);
		_fader->run();
	}
}

void FadeToModalProcess::run() {
	switch (_nextState) {
		case FS_OpenFadeOut:
		{
			_fader = new PaletteFaderProcess(0x00000000, false, 0x7FFF, 30, true);
			Kernel::get_instance()->addProcess(_fader);
			_fader->setRunPaused();
			_nextState = FS_ShowGump;
			waitFor(_fader);
			break;
		}
		case FS_ShowGump:
		{
			// kernel will delete the fader object for us
			_fader = nullptr;
			_modal->InitGump(0);
			_modal->setRelativePosition(Gump::CENTER);
			_modal->CreateNotifier();
			// Reset the palette before showing the modal
			PaletteManager::get_instance()->untransformPalette(PaletteManager::Pal_Game);
			_nextState = FS_CloseFadeIn;
			waitFor(_modal->GetNotifyProcess());
			break;
		}
		case FS_CloseFadeIn:
		{
			// Already created a new fader in onWakeUp..
			Kernel::get_instance()->addProcess(_fader);
			_fader->setRunPaused();
			_nextState = FS_Finshed;
			waitFor(_fader);
			break;
		}
		case FS_Finshed:
		{
			// kernel will delete the fader object for us
			_fader = nullptr;
			terminate();
			break;
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
