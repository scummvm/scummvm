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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SHARED_H
#define ASYLUM_SHARED_H_

#include "common/singleton.h"

#include "asylum/sceneres.h"

namespace Asylum {

/**
 * Shared Resources are classes that are instantiated at the engine level.
 * Binding them to this singleton simplifies implementation of new classes
 * that will require access to common interfaces and tools.
 *
 * A candidate for a shared resource would be any class that will be utilized
 * by one-to-many instances of another class, but will never need more than
 * one instance itself.
 *
 * Each component herin could technically be a singleton unto itself, but
 * by using this method, destruction can be handled by the engine.
 */
class SharedResources: public Common::Singleton<SharedResources> {
public:

    void setOSystem(OSystem* system) { _system = system; }
    OSystem* getOSystem() { return _system; }

	void setVideo(Video* video) { _video = video; }
	Video* getVideo() { return _video; }

	void setScreen(Screen* screen) { _screen = screen; }
	Screen* getScreen() { return _screen; }

	void setSound(Sound* sound) { _sound = sound; }
	Sound* getSound() { return _sound; }

	void setScene(Scene* scene) { _scene = scene; }
	Scene* getScene() { return _scene; }

	bool pointInPoly(PolyDefinitions *poly, int x, int y);

    void setGameFlag(int flag);
    void clearGameFlag(int flag);
    void toggleGameFlag(int flag);
    bool isGameFlagSet(int flag);
    bool isGameFlagNotSet(int flag);

private:
	friend class Common::Singleton<SingletonBaseType>;
	SharedResources();
	~SharedResources();

    OSystem *_system;
	Video  *_video;
	Screen *_screen;
	Sound  *_sound;
	Scene  *_scene;

    // NOTE
	// Storing the gameflags on the
	// scriptmanager since this makes the
	// most sense
	int _gameFlags[1512];

}; // end of class SharedResources

#define Shared	(::Asylum::SharedResources::instance())

} // end of namespace Asylum

#endif
