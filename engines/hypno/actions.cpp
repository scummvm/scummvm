
#include "common/events.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

//Actions

void HypnoEngine::runMenu(Hotspots hs) {
	const Hotspot h = *hs.begin();
	assert(h.type == MakeMenu);

	debug("hotspot actions size: %d", h.actions.size());
	for (Actions::const_iterator itt = h.actions.begin(); itt != h.actions.end(); ++itt) {
		Action *action = *itt;
		if (typeid(*action) == typeid(Quit))
			runQuit((Quit *)action);
		else if (typeid(*action) == typeid(Background))
			runBackground((Background *)action);
		else if (typeid(*action) == typeid(Overlay))
			runOverlay((Overlay *)action);
		else if (typeid(*action) == typeid(Ambient))
			runAmbient((Ambient *)action);

		//else if (typeid(*action) == typeid(Mice))
		//	runMice(h, (Mice*) action);
	}

	//if (h.stype == "SINGLE_RUN")
	//	loadImage("int_main/mainbutt.smk", 0, 0);
	if (h.stype == "AUTO_BUTTONS" && _conversation.empty())
		loadImage("int_main/resume.smk", 0, 0, true);
}

void HypnoEngine::runBackground(Background *a) {
	if (a->condition.size() > 0 && !_sceneState[a->condition])
		return;
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y, false);
}

void HypnoEngine::runOverlay(Overlay *a) {
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y, false);
}

void HypnoEngine::runMice(Mice *a) {
	changeCursor(a->path, a->index);
}

void HypnoEngine::runEscape(Escape *a) {
	_nextHotsToRemove = stack.back();
}

void HypnoEngine::runCutscene(Cutscene *a) {
	stopSound();
	disableCursor();
	_music = "";
	_nextSequentialVideoToPlay.push_back(MVideo(a->path, Common::Point(0, 0), false, true, false));
}

void HypnoEngine::runGlobal(Global *a) {
	if (a->command == "TURNON")
		_sceneState[a->variable] = 1;
	else if (a->command == "TURNOFF")
		_sceneState[a->variable] = 0;
	else
		error("Invalid command %s", a->command.c_str());
}

void HypnoEngine::runPlay(Play *a) {
	if (a->condition.size() > 0 && !_sceneState[a->condition])
		return;
	Common::Point origin = a->origin;

	if (a->flag == "/BITMAP")
		loadImage(a->path, origin.x, origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, false, false));
	}
}

void HypnoEngine::runAmbient(Ambient *a) {
	Common::Point origin = a->origin;
	if (a->flag == "/BITMAP")
		loadImage(a->path, origin.x, origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, a->fullscreen, a->flag == "/LOOP"));
	}
}

void HypnoEngine::runWalN(WalN *a) {
	if (a->condition.size() > 0 && !_sceneState[a->condition])
		return;
	Common::Point origin = a->origin;
	if (a->flag == "/BITMAP")
		loadImage(a->path, origin.x, origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, false, false));
	}
}

void HypnoEngine::runQuit(Quit *a) {
	quitGame();
}

void HypnoEngine::runChangeLevel(ChangeLevel *a) {
	_nextLevel = a->level;
}

void HypnoEngine::runTalk(Talk *a) {
	debug("adding TALK line!");
	_conversation.push_back(a);
	_refreshConversation = true;
}

}