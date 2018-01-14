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


namespace Common {
struct KeyState;
}

namespace Graphics {
struct Surface;
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

class KIA {
	static const char *kPogo;

	struct ActorDialogueQueueEntry {
		int actorId;
		int sentenceId;
	};

	BladeRunnerEngine *_vm;

	int _forceOpen;
	int _transitionId;

	int _lastSectionIdKIA;
	int _lastSectionIdOptions;

	int        _playerVqaTimeLast;
	VQAPlayer *_playerVqaPlayer;
	int        _playerVqaFrame;
	int        _playerVisualizerState;
	int        _playerPhotographId;
	Shape     *_playerPhotograph;
	int        _playerSliceModelId;
	float      _playerSliceModelAngle;
	int        _timeLast;

	ActorDialogueQueueEntry *_playerActorDialogueQueue;
	int                      _playerActorDialogueQueueCapacity;
	int                      _playerActorDialogueQueuePosition;
	int                      _playerActorDialogueQueueSize;
	int                      _playerActorDialogueState;

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
	int        _currentSectionId;
	KIALog    *_log;
	KIAScript *_script;
	KIAShapes *_shapes;

public:
	KIA(BladeRunnerEngine *vm);
	~KIA();

	void openLastOpened();
	void openOptions();

	void tick();

	void handleMouseDown(int mouseX, int mouseY, bool mainButton);
	void handleMouseUp(int mouseX, int mouseY, bool mainButton);
	void handleKeyUp(const Common::KeyState &kbd);
	void handleKeyDown(const Common::KeyState &kbd);

	void playerReset();
	void playActorDialogue(int actorId, int sentenceId);
	void playSliceModel(int sliceModelId);
	void playPhotograph(int photographId);

private:
	static void mouseDownCallback(int buttonId, void *callbackData);
	static void mouseUpCallback(int buttonId, void *callbackData);
	static void loopEnded(void *callbackData, int frame, int loopId);

	void open(int sectionId);
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
