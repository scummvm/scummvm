/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_SERVICES_SERVICES_H
#define STARK_SERVICES_SERVICES_H

#include "common/singleton.h"
#include "common/scummsys.h"

namespace Common {
class RandomSource;
}

namespace Stark {

namespace Gfx {
class Driver;
}

class ArchiveLoader;
class DialogPlayer;
class Diary;
class FontProvider;
class GameInterface;
class Global;
class ResourceProvider;
class StaticProvider;
class Scene;
class UserInterface;
class Settings;
class StateProvider;
class GameChapter;
class GameMessage;

/**
 * Public services available as a singleton
 */
class StarkServices : public Common::Singleton<StarkServices> {
public:
	StarkServices() {
		archiveLoader = nullptr;
		dialogPlayer = nullptr;
		diary = nullptr;
		gfx = nullptr;
		global = nullptr;
		resourceProvider = nullptr;
		randomSource = nullptr;
		scene = nullptr;
		staticProvider = nullptr;
		gameInterface = nullptr;
		userInterface = nullptr;
		fontProvider = nullptr;
		settings = nullptr;
		gameChapter = nullptr;
		gameMessage = nullptr;
		stateProvider = nullptr;
	}

	ArchiveLoader *archiveLoader;
	DialogPlayer *dialogPlayer;
	Diary *diary;
	Gfx::Driver *gfx;
	Global *global;
	ResourceProvider *resourceProvider;
	Common::RandomSource *randomSource;
	Scene *scene;
	StaticProvider *staticProvider;
	GameInterface *gameInterface;
	UserInterface *userInterface;
	FontProvider *fontProvider;
	Settings *settings;
	GameChapter *gameChapter;
	GameMessage *gameMessage;
	StateProvider *stateProvider;
};

/** Shortcuts for accessing the services. */
#define StarkArchiveLoader      StarkServices::instance().archiveLoader
#define StarkDialogPlayer       StarkServices::instance().dialogPlayer
#define StarkDiary              StarkServices::instance().diary
#define StarkGfx                StarkServices::instance().gfx
#define StarkGlobal             StarkServices::instance().global
#define StarkResourceProvider   StarkServices::instance().resourceProvider
#define StarkRandomSource       StarkServices::instance().randomSource
#define StarkScene              StarkServices::instance().scene
#define StarkStaticProvider     StarkServices::instance().staticProvider
#define StarkGameInterface      StarkServices::instance().gameInterface
#define StarkUserInterface      StarkServices::instance().userInterface
#define StarkFontProvider       StarkServices::instance().fontProvider
#define StarkSettings           StarkServices::instance().settings
#define StarkGameChapter        StarkServices::instance().gameChapter
#define StarkGameMessage        StarkServices::instance().gameMessage
#define StarkStateProvider      StarkServices::instance().stateProvider

} // End of namespace Stark

#endif // STARK_SERVICES_SERVICES_H
