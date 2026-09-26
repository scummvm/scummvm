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
 */

#include "ripper/scenes/scene.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/dialogue.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

Scene::Scene(RipperEngine *engine) : _engine(engine),
		_chooser(*engine->getScripts()->getDialogue()) {
}

void Scene::prepare(const char *reason, uint cursorIndex, bool cursorVisible) {
	// RunKaDialogueScene at 0x2aef5, RunCainDialogueScene at 0x2c160, and
	// RunTubeSwitchScene at 0x25e18 preserve the incoming indexed palette and
	// replace the active control/chooser ownership while their local loop runs.
	_engine->getToolbar()->leave();
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition(reason);
	else
		_chooser.clearPending();
	_savedPalette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(_savedPalette.data(), 0, 256);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(cursorIndex);
	_engine->getCursor()->dispatchSelectionIndexChange(cursorIndex);
	_engine->getCursor()->setVisible(cursorVisible);
	debugC(2, kDebugScene,
		"Ripper: prepared self-contained scene reason=%s cursor=%u visible=%d paletteBytes=%u",
		reason, cursorIndex, cursorVisible, _savedPalette.size());
}

void Scene::finish(const char *reason, int cursorIndex, bool cursorVisible) {
	if (_chooser.isPending())
		_chooser.dismissForSceneTransition(reason);
	_engine->getToolbar()->leave();
	if (_savedPalette.size() == 256 * 3)
		g_system->getPaletteManager()->setPalette(_savedPalette.data(), 0, 256);
	if (cursorIndex >= 0)
		_engine->getCursor()->update(cursorIndex);
	_engine->getCursor()->setVisible(cursorVisible);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(2, kDebugScene,
		"Ripper: finished self-contained scene reason=%s cursor=%d visible=%d paletteRestored=%d",
		reason, cursorIndex, cursorVisible, _savedPalette.size() == 256 * 3);
}

void Scene::stopAudio(Audio::SoundHandle &handle) {
	_engine->getMedia()->stopSoundEffect(handle);
}

} // End of namespace Ripper
