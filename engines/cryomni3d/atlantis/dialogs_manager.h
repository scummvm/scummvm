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

#ifndef CRYOMNI3D_ATLANTIS_DIALOGS_MANAGER_H
#define CRYOMNI3D_ATLANTIS_DIALOGS_MANAGER_H

#include "graphics/managed_surface.h"

#include "cryomni3d/dialogs_manager.h"

namespace CryOmni3D {
namespace Atlantis {

class CryOmni3DEngine_Atlantis;

class Atlantis_DialogsManager : public DialogsManager {
public:
	explicit Atlantis_DialogsManager(CryOmni3DEngine_Atlantis *engine);

	bool play(const Common::String &sequence);

protected:
	void executeShow(const Common::String &show) override;
	void playDialog(const Common::String &video, const Common::String &sound,
	                const Common::String &text, const SubtitlesSettings &settings) override;
	void displayMessage(const Common::String &text) override;
	uint askPlayerQuestions(const Common::String &video,
	                        const Common::StringArray &questions) override;

private:
	CryOmni3DEngine_Atlantis *_engine;
	Graphics::ManagedSurface _lastImage;
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_DIALOGS_MANAGER_H
