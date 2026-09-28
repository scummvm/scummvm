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

#ifndef NANCY_UI_NDUI_MANAGER_H
#define NANCY_UI_NDUI_MANAGER_H

#include "common/array.h"
#include "common/hash-str.h"

#include "engines/nancy/ui/ndui/common.h"

namespace Graphics {
class ManagedSurface;
}

namespace Nancy {

struct NancyInput;

namespace UI {
namespace NDUI {

class Panel;

// The Nancy16 UI, owned by the scene. It creates the panels from the NDUI chunks of the
// player character's UI IFF and the IFFs it includes, and stands between them and the
// rest of the engine:
// - Actions, whether fired by a control's event or sent by a scene's UIControl record,
//   go to the panel or control they name. The ones naming an "Engine_" target go to
//   the engine handlers instead: inventory, journal, saving and loading, options...
// - The scene tells it about the inventory, captions, conversations, the clock and scene
//   changes, which it passes on to the controls that show them.
// - Input goes to the topmost panel under the mouse, and keys to the focused edit box.
// - What the UI asks of the engine (new game, load, save, quit) waits for the scene to
//   pick it up, since it can replace the UI itself.
// It also keeps the textures the elements draw, and the templates of the controls that
// get created while the game runs, like journal entries and conversation responses.
class Manager {
public:
	// What the UI asked of the engine, which has to wait until the UI is done
	// handling the player's input, since it can replace the UI
	enum RequestType {
		kRequestNone,
		kRequestNewGame,
		kRequestLoad,
		kRequestSave,
		kRequestQuit
	};

	struct Request {
		RequestType type = kRequestNone;
		int slot = -1;
		Common::String name;
	};

	~Manager();

	Request takeRequest();

	// Creates every panel in the IFF with the given name, and in the ones it includes
	void load(const Common::String &name);

	void takeAction(const Action &action);
	void takeAction(int32 type, const Common::String &target, double value);
	void takeAction(int32 type, const Common::String &target, const Common::String &text);
	void handleInput(NancyInput &input);
	void registerGraphics();

	const Graphics::ManagedSurface *getTexture(const Common::String &name);

	// Creates a control from a template, as the last one of a panel
	void createObject(const Common::String &name, const Common::String &templateName, const Common::String &parentName);
	void destroyObject(const Common::String &name);

private:
	Panel *findPanel(const Common::String &name) const;

	Common::Array<Panel *> _panels;
	Common::HashMap<Common::String, Graphics::ManagedSurface *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _textures;
	Common::HashMap<Common::String, Common::Array<byte>, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _templates;

	Request _request;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_MANAGER_H
