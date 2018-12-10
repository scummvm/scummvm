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

#ifndef BLADERUNNER_KIA_H
#define BLADERUNNER_KIA_H

#include "common/str.h"

#include "graphics/surface.h"

namespace Common {
struct KeyState;
}

namespace BladeRunner {

class BladeRunnerEngine;
class KIALog;
class KIAScript;
class KIASectionBase;
class KIASectionClues;
class KIASectionDiagnostic;
class KIASectionCrimes;
class KIASectionHelp;
class KIASectionLoad;
class KIASectionSettings;
class KIASectionPogo;
class KIASectionSave;
class KIASectionSuspects;
class KIAShapes;
class Shape;
class UIImagePicker;
class VQAPlayer;

enum KIASections {
	kKIASectionNone       = 0,
	kKIASectionCrimes     = 1,
	kKIASectionSuspects   = 2,
	kKIASectionClues      = 3,
	kKIASectionSettings   = 4,
	kKIASectionHelp       = 5,
	kKIASectionSave       = 6,
	kKIASectionLoad       = 7,
	kKIASectionQuit       = 8,
	kKIASectionDiagnostic = 9,
	kKIASectionPogo       = 10
};

class KIA {
	static const char *kPogo;
	static const int kPlayerActorDialogueQueueCapacity = 31;

	struct ActorDialogueQueueEntry {
		int actorId;
		int sentenceId;
	};

	BladeRunnerEngine *_vm;

	int _transitionId;

	int                _playerVqaTimeLast;
	VQAPlayer         *_playerVqaPlayer;
	int                _playerVqaFrame;
	int                _playerVisualizerState;
	int                _playerPhotographId;
	Shape             *_playerPhotograph;
	int                _playerSliceModelId;
	float              _playerSliceModelAngle;
	Graphics::Surface  _playerImage;
	int                _timeLast;

	ActorDialogueQueueEntry _playerActorDialogueQueue[kPlayerActorDialogueQueueCapacity];
	int                     _playerActorDialogueQueuePosition;
	int                     _playerActorDialogueQueueSize;
	int                     _playerActorDialogueState;

	KIASections           _currentSectionId;
	KIASections           _lastSectionIdKIA;
	KIASections           _lastSectionIdOptions;
	KIASectionBase       *_currentSection;

	KIASectionClues      *_cluesSection;
	KIASectionCrimes     *_crimesSection;
	KIASectionDiagnostic *_diagnosticSection;
	KIASectionHelp       *_helpSection;
	KIASectionLoad       *_loadSection;
	KIASectionSettings   *_settingsSection;
	KIASectionPogo       *_pogoSection;
	KIASectionSave       *_saveSection;
	KIASectionSuspects   *_suspectsSection;

	UIImagePicker        *_buttons;

	VQAPlayer            *_mainVqaPlayer;

	int                   _pogoPos;


public:
	bool              _forceOpen;

	KIALog           *_log;
	KIAScript        *_script;
	KIAShapes        *_shapes;

	Graphics::Surface _thumbnail;

public:
	KIA(BladeRunnerEngine *vm);
	~KIA();

	void reset();

	void openLastOpened();
	void open(KIASections sectionId);
	bool isOpen() const;

	void tick();

	void resume();

	void handleMouseDown(int mouseX, int mouseY, bool mainButton);
	void handleMouseUp(int mouseX, int mouseY, bool mainButton);
	void handleKeyUp(const Common::KeyState &kbd);
	void handleKeyDown(const Common::KeyState &kbd);

	void playerReset();
	void playActorDialogue(int actorId, int sentenceId);
	void playSliceModel(int sliceModelId);
	void playPhotograph(int photographId);
	void playImage(const Graphics::Surface &image);

private:
	static void mouseDownCallback(int buttonId, void *callbackData);
	static void mouseUpCallback(int buttonId, void *callbackData);
	static void loopEnded(void *callbackData, int frame, int loopId);

	void init();
	void unload();
	void switchSection(int sectionId);

	void createButtons(int sectionId);
	void buttonClicked(int buttonId);

	const char *getSectionVqaName(int sectionId);
	int getVqaLoopMain(int sectionId);
	int getVqaLoopTransition(int transitionId);

	int getTransitionId(int oldSectionId, int newSectionId);
	void playTransitionSound(int transitionId);

	void playPrivateAddon();
};

} // End of namespace BladeRunner
#endif
