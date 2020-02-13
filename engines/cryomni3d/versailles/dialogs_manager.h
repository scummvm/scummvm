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

#ifndef CRYOMNI3D_VERSAILLES_DIALOGS_MANAGER_H
#define CRYOMNI3D_VERSAILLES_DIALOGS_MANAGER_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "graphics/managed_surface.h"

#include "cryomni3d/dialogs_manager.h"
#include "cryomni3d/font_manager.h"

namespace CryOmni3D {
namespace Versailles {

class CryOmni3DEngine_Versailles;
typedef void (CryOmni3DEngine_Versailles::*ShowCallback)();

class Versailles_DialogsManager : public DialogsManager {
public:
	Versailles_DialogsManager(CryOmni3DEngine_Versailles *engine, bool padAudioFileName);

	// This overload will hide the base one and this is what we want
	bool play(const Common::String &sequence);

	void registerShowCallback(const Common::String &showName, ShowCallback callback) { _shows[showName] = callback; }

protected:
	void executeShow(const Common::String &show) override;
	void playDialog(const Common::String &video, const Common::String &sound,
	                const Common::String &text, const SubtitlesSettings &settings) override;
	void displayMessage(const Common::String &text) override;
	uint askPlayerQuestions(const Common::String &video,
	                        const Common::StringArray &questions) override;

private:
	CryOmni3DEngine_Versailles *_engine;
	Common::HashMap<Common::String, ShowCallback> _shows;
	bool _padAudioFileName;

	void loadFrame(const Common::String &video);

	Graphics::ManagedSurface _lastImage;
};

} // End of namespace Versailles
} // End of namespace CryOmni3D

#endif
