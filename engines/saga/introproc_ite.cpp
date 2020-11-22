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

// Intro sequence scene procedures

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/animation.h"
#include "saga/events.h"
#include "saga/font.h"
#include "saga/itedata.h"
#include "saga/sndres.h"
#include "saga/palanim.h"
#include "saga/music.h"

#include "saga/scene.h"
#include "saga/resource.h"

namespace Saga {

#define INTRO_FRAMETIME 90
#define INTRO_CAPTION_Y 170
#define INTRO_DE_CAPTION_Y 160
#define INTRO_IT_CAPTION_Y 160
#define INTRO_FR_CAPTION_Y 160
#define INTRO_VOICE_PAD 50
#define INTRO_VOICE_LETTERLEN 90

#define DISSOLVE_DURATION 3000
#define LOGO_DISSOLVE_DURATION 1000

// Intro scenes
#define RID_ITE_INTRO_ANIM_SCENE 1538
#define RID_ITE_CAVE_SCENE_1 1542
#define RID_ITE_CAVE_SCENE_2 1545
#define RID_ITE_CAVE_SCENE_3 1548
#define RID_ITE_CAVE_SCENE_4 1551
#define RID_ITE_VALLEY_SCENE 1556
#define RID_ITE_TREEHOUSE_SCENE 1560
#define RID_ITE_FAIREPATH_SCENE 1564
#define RID_ITE_FAIRETENT_SCENE 1567

// Intro scenes - DOS demo
#define RID_ITE_INTRO_ANIM_SCENE_DOS_DEMO 298
#define RID_ITE_CAVE_SCENE_DOS_DEMO 302
#define RID_ITE_VALLEY_SCENE_DOS_DEMO 310

// ITE intro music
#define MUSIC_INTRO 9
#define MUSIC_TITLE_THEME 10

LoadSceneParams ITE_IntroList[] = {
	{RID_ITE_INTRO_ANIM_SCENE, kLoadByResourceId, Scene::SC_ITEIntroAnimProc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_CAVE_SCENE_1, kLoadByResourceId, Scene::SC_ITEIntroCave1Proc, false, kTransitionFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_CAVE_SCENE_2, kLoadByResourceId, Scene::SC_ITEIntroCave2Proc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_CAVE_SCENE_3, kLoadByResourceId, Scene::SC_ITEIntroCave3Proc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_CAVE_SCENE_4, kLoadByResourceId, Scene::SC_ITEIntroCave4Proc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_VALLEY_SCENE, kLoadByResourceId, Scene::SC_ITEIntroValleyProc, false, kTransitionFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_TREEHOUSE_SCENE, kLoadByResourceId, Scene::SC_ITEIntroTreeHouseProc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_FAIREPATH_SCENE, kLoadByResourceId, Scene::SC_ITEIntroFairePathProc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_FAIRETENT_SCENE, kLoadByResourceId, Scene::SC_ITEIntroFaireTentProc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE}
};

LoadSceneParams ITE_DOS_Demo_IntroList[] = {
	{RID_ITE_INTRO_ANIM_SCENE_DOS_DEMO, kLoadByResourceId, Scene::SC_ITEIntroAnimProc, false, kTransitionNoFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_CAVE_SCENE_DOS_DEMO, kLoadByResourceId, Scene::SC_ITEIntroCaveDemoProc, false, kTransitionFade, 0, NO_CHAPTER_CHANGE},
	{RID_ITE_VALLEY_SCENE_DOS_DEMO, kLoadByResourceId, Scene::SC_ITEIntroValleyProc, false, kTransitionFade, 0, NO_CHAPTER_CHANGE},
};

int Scene::ITEStartProc() {
	LoadSceneParams firstScene;
	LoadSceneParams tempScene;
	bool dosDemo = (_vm->getFeatures() & GF_ITE_DOS_DEMO);
	int scenesCount = (!dosDemo) ? ARRAYSIZE(ITE_IntroList) : ARRAYSIZE(ITE_DOS_Demo_IntroList);

	for (int i = 0; i < scenesCount; i++) {
		tempScene = (!dosDemo) ? ITE_IntroList[i] : ITE_DOS_Demo_IntroList[i];
		tempScene.sceneDescriptor = _vm->_resource->convertResourceId(tempScene.sceneDescriptor);
		_vm->_scene->queueScene(tempScene);
	}

	firstScene.loadFlag = kLoadBySceneNumber;
	firstScene.sceneDescriptor = _vm->getStartSceneNumber();
	firstScene.sceneSkipTarget = true;
	firstScene.sceneProc = NULL;
	firstScene.transitionType = kTransitionFade;
	firstScene.actorsEntrance = 0;
	firstScene.chapter = -1;

	_vm->_scene->queueScene(firstScene);

	return SUCCESS;
}

EventColumns *Scene::queueIntroDialogue(EventColumns *eventColumns, int n_dialogues, const IntroDialogue dialogue[]) {
	TextListEntry textEntry;
	TextListEntry *entry;
	Event event;
	int voiceLength;
	int i;

	// Queue narrator dialogue list
	textEntry.knownColor = kKnownColorSubtitleTextColor;
	textEntry.effectKnownColor = (_vm->getPlatform() == Common::kPlatformPC98) ? kKnownColorSubtitleEffectColorPC98 : kKnownColorTransparent;
	textEntry.useRect = true;
	textEntry.rect.left = (_vm->getPlatform() == Common::kPlatformPC98) ? 10 : 0;
	textEntry.rect.right = _vm->getDisplayInfo().width - (_vm->getPlatform() == Common::kPlatformPC98 ? 10 : 0);
	if (_vm->getLanguage() == Common::DE_DEU) {
		textEntry.rect.top = INTRO_DE_CAPTION_Y;
	} else if (_vm->getLanguage() == Common::IT_ITA) {
		textEntry.rect.top = INTRO_IT_CAPTION_Y;
	} else if (_vm->getLanguage() == Common::FR_FRA) {
		textEntry.rect.top = INTRO_FR_CAPTION_Y;
	} else {
		textEntry.rect.top = INTRO_CAPTION_Y;
	}
	textEntry.rect.bottom = _vm->getDisplayInfo().height;
	textEntry.font = kKnownFontMedium;
	textEntry.flags = (FontEffectFlags)(kFontOutline | kFontCentered);

	for (i = 0; i < n_dialogues; i++) {
		textEntry.text = dialogue[i].i_str;

		// For the Japanese version align each string to the bottom of the screen
		if (_vm->getLanguage() == Common::JA_JPN)
			textEntry.rect.top = textEntry.rect.bottom - _vm->_font->getHeight(textEntry.font, textEntry.text, textEntry.rect.width(), textEntry.flags);

		entry = _vm->_scene->_textList.addEntry(textEntry);

		if (_vm->_subtitlesEnabled) {
			// Display text
			event.type = kEvTOneshot;
			event.code = kTextEvent;
			event.op = kEventDisplay;
			event.data = entry;
			event.time = (i == 0) ? 0 : INTRO_VOICE_PAD;
			eventColumns = _vm->_events->chain(eventColumns, event);
		}

		if (_vm->_voicesEnabled) {
			// Play voice
			event.type = kEvTOneshot;
			event.code = kVoiceEvent;
			event.op = kEventPlay;
			event.param = dialogue[i].i_voice_rn;
			event.time = 0;
			_vm->_events->chain(eventColumns, event);
		}

		voiceLength = _vm->_sndRes->getVoiceLength(dialogue[i].i_voice_rn);
		if (voiceLength < 0) {
			// Set a default length if no speech file is present
			voiceLength = strlen(dialogue[i].i_str) * INTRO_VOICE_LETTERLEN;
		}

		// Remove text
		event.type = kEvTOneshot;
		event.code = kTextEvent;
		event.op = kEventRemove;
		event.data = entry;
		event.time = voiceLength;
		_vm->_events->chain(eventColumns, event);
	}

	return eventColumns;
}

// Queue a page of credits text. The original interpreter did word-wrapping
// automatically. We currently don't.

EventColumns *Scene::queueCredits(int delta_time, int duration, int n_credits, const IntroCredit credits[]) {
	int game;
	Common::Language lang;
	bool hasWyrmkeepCredits = (Common::File::exists("credit3n.dlt") ||	// PC
							   Common::File::exists("credit3m.dlt"));	// Mac

	// The assumption here is that all WyrmKeep versions have the same
	// credits, regardless of which operating system they're for.

	lang = _vm->getLanguage();

	if (hasWyrmkeepCredits)
		game = kITECreditsWyrmKeep;
	else if (_vm->getPlatform() == Common::kPlatformMacintosh)
		game = kITECreditsMac;
	else if (_vm->getPlatform() == Common::kPlatformPC98)
		game = kITECreditsPC98;
	else if (_vm->getFeatures() & GF_EXTRA_ITE_CREDITS)
		game = kITECreditsPCCD;
	else
		game = kITECreditsPC;

	int lineHeight = 0;
	int paragraph_spacing;
	KnownFont font = kKnownFontSmall;
	int i;

	int n_paragraphs = 0;
	int credits_height = 0;

	for (i = 0; i < n_credits; i++) {
		if (credits[i].lang != lang && credits[i].lang != Common::UNK_LANG) {
			continue;
		}

		if (!(credits[i].game & game)) {
			continue;
		}

		switch (credits[i].type) {
		case kITECreditsHeader:
			font = kKnownFontSmall;
			// First glance at disasm might suggest that the 12 here is a typo (instead of 11). But it isn't.
			// I take into account the extra pixel per paragraph here which the original code will consider
			// elsewhere. In tbe second (queueing) loop below I have to be a bit more elaborate to get this
			// right (using extra variable yOffs2), but here it works fine like this...
			lineHeight = (_vm->getPlatform() == Common::kPlatformPC98) ? 12 : _vm->_font->getHeight(font) + 4;
			n_paragraphs++;
			break;
		case kITECreditsText:
			font = kKnownFontMedium;
			lineHeight = (_vm->getPlatform() == Common::kPlatformPC98) ? (_vm->_font->getHeight(font) << 1) : _vm->_font->getHeight(font) + 2;
			break;
		default:
			error("Unknown credit type");
		}

		credits_height += lineHeight;
	}

	paragraph_spacing = (200 - credits_height) / (n_paragraphs + 3);
	int y = (_vm->getPlatform() == Common::kPlatformPC98) ? paragraph_spacing + 80 : paragraph_spacing;

	TextListEntry textEntry;
	TextListEntry *entry;
	Event event;
	EventColumns *eventColumns = NULL;

	textEntry.knownColor = (_vm->getPlatform() == Common::kPlatformPC98) ? kKnownColorBrightWhite : kKnownColorSubtitleTextColor;
	textEntry.effectKnownColor = (_vm->getPlatform() == Common::kPlatformPC98) ? kKnownColorVerbTextShadow : kKnownColorTransparent;
	textEntry.flags = (FontEffectFlags)(((_vm->getPlatform() == Common::kPlatformPC98) ? kFontShadow : kFontOutline) | kFontCentered);
	textEntry.point.x = 160;
	int yOffs = 0;
	int yOffs2 = 0;

	for (i = 0; i < n_credits; i++) {
		if (credits[i].lang != lang && credits[i].lang != Common::UNK_LANG) {
			continue;
		}

		if (!(credits[i].game & game)) {
			continue;
		}

		switch (credits[i].type) {
		case kITECreditsHeader:
			font = kKnownFontSmall;
			lineHeight = (_vm->getPlatform() == Common::kPlatformPC98) ? 11 : _vm->_font->getHeight(font) + 4;
			yOffs = (_vm->getPlatform() == Common::kPlatformPC98) ? -3 : 0;
			y = y + paragraph_spacing + yOffs2;
			break;
		case kITECreditsText:
			font = kKnownFontMedium;
			lineHeight = (_vm->getPlatform() == Common::kPlatformPC98) ? (_vm->_font->getHeight(font) << 1) : _vm->_font->getHeight(font) + 2;
			yOffs = 0;
			break;
		default:
			break;
		}

		textEntry.text = credits[i].string;
		textEntry.font = font;
		textEntry.point.y = y + yOffs;

		if (_vm->getPlatform() == Common::kPlatformPC98) {
			textEntry.point.y >>= 1;
			yOffs2 = 1;
		}

		entry = _vm->_scene->_textList.addEntry(textEntry);

		// Display text
		event.type = kEvTOneshot;
		event.code = kTextEvent;
		event.op = kEventDisplay;
		event.data = entry;
		event.time = delta_time;
		eventColumns = _vm->_events->queue(event);

		// Remove text
		event.type = kEvTOneshot;
		event.code = kTextEvent;
		event.op = kEventRemove;
		event.data = entry;
		event.time = duration;
		_vm->_events->chain(eventColumns, event);

		y += lineHeight;
	}

	return eventColumns;
}

int Scene::SC_ITEIntroAnimProc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroAnimProc(param);
}

// Handles the introductory Dreamer's Guild / NWC logo animation scene.
int Scene::ITEIntroAnimProc(int param) {
	Event event;
	EventColumns *eventColumns;
	bool isMac = _vm->getPlatform() == Common::kPlatformMacintosh;
	bool isMultiCD = _vm->getPlatform() == Common::kPlatformUnknown;
	bool hasWyrmkeepCredits = (Common::File::exists("credit3n.dlt") ||	// PC
							   Common::File::exists("credit3m.dlt"));	// Mac
	bool isDemo = Common::File::exists("scriptsd.rsc");

	switch (param) {
	case SCENE_BEGIN:{
		// Background for intro scene is the first frame of the
		// intro animation; display it and set the palette
		event.type = kEvTOneshot;
		event.code = kBgEvent;
		event.op = kEventDisplay;
		event.param = kEvPSetPalette;
		event.time = 0;
		eventColumns = _vm->_events->queue(event);

		debug(3, "Intro animation procedure started.");
		debug(3, "Linking animation resources...");

		_vm->_anim->setFrameTime(0, INTRO_FRAMETIME);

		// Link this scene's animation resources for continuous
		// playback
		int lastAnim;

		if (hasWyrmkeepCredits || isMultiCD || isDemo)
			lastAnim = isMac ? 3 : 2;
		else
			lastAnim = isMac ? 4 : 5;

		for (int i = 0; i < lastAnim; i++)
			_vm->_anim->link(i, i+1);

		_vm->_anim->setFlag(lastAnim, ANIM_FLAG_ENDSCENE);

		debug(3, "Beginning animation playback.");

		// Begin the animation
		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = 0;
		event.time = 0;
		_vm->_events->chain(eventColumns, event);

		// Queue intro music playback
		_vm->_events->chainMusic(eventColumns, MUSIC_INTRO, true);
		}
		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::ITEIntroCaveCommonProc(int param, int caveScene) {
	Event event;
	EventColumns *eventColumns = NULL;
	const IntroDialogue *dialogue;

	int lang = 0;

	if (_vm->getLanguage() == Common::DE_DEU)
		lang = 1;
	else if (_vm->getLanguage() == Common::IT_ITA)
		lang = 2;
	else if (_vm->getLanguage() == Common::FR_FRA)
		lang = 3;
	else if (_vm->getLanguage() == Common::JA_JPN)
		lang = 4;

	int n_dialogues = 0;

	switch (caveScene) {
	case 1:
		n_dialogues = ARRAYSIZE(introDialogueCave1[lang]);
		dialogue = introDialogueCave1[lang];
		break;
	case 2:
		n_dialogues = ARRAYSIZE(introDialogueCave2[lang]);
		dialogue = introDialogueCave2[lang];
		break;
	case 3:
		n_dialogues = ARRAYSIZE(introDialogueCave3[lang]);
		dialogue = introDialogueCave3[lang];
		break;
	case 4:
		n_dialogues = ARRAYSIZE(introDialogueCave4[lang]);
		dialogue = introDialogueCave4[lang];
		break;
	default:
		error("Invalid cave scene");
	}

	switch (param) {
	case SCENE_BEGIN:
		if (caveScene > 1) {
			// Start 'dissolve' transition to new scene background
			event.type = kEvTContinuous;
			event.code = kTransitionEvent;
			event.op = kEventDissolve;
			event.time = 0;
			event.duration = DISSOLVE_DURATION;
			eventColumns = _vm->_events->queue(event);
		}

		// Begin palette cycling animation for candles
		event.type = kEvTOneshot;
		event.code = kPalAnimEvent;
		event.op = kEventCycleStart;
		event.time = 0;
		eventColumns = _vm->_events->chain(eventColumns, event);

		// Queue narrator dialogue list
		queueIntroDialogue(eventColumns, n_dialogues, dialogue);

		// End scene after last dialogue over
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = INTRO_VOICE_PAD;
		_vm->_events->chain(eventColumns, event);

		break;
	case SCENE_END:
		break;

	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::ITEIntroCaveDemoProc(int param) {
	Event event;
	EventColumns *eventColumns = NULL;

	switch (param) {
	case SCENE_BEGIN:
		// Begin palette cycling animation for candles
		event.type = kEvTOneshot;
		event.code = kPalAnimEvent;
		event.op = kEventCycleStart;
		event.time = 0;
		eventColumns = _vm->_events->chain(eventColumns, event);

		// Queue narrator dialogue list
		for (int i = 0; i < 11; i++) {
			// Play voice
			event.type = kEvTOneshot;
			event.code = kVoiceEvent;
			event.op = kEventPlay;
			event.param = i;
			event.time = _vm->_sndRes->getVoiceLength(i);
			_vm->_events->chain(eventColumns, event);
		}

		// End scene after last dialogue over
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = INTRO_VOICE_PAD;
		_vm->_events->chain(eventColumns, event);

		break;
	case SCENE_END:
		break;

	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroCaveDemoProc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCaveDemoProc(param);
}

// Handles first introductory cave painting scene
int Scene::SC_ITEIntroCave1Proc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCaveCommonProc(param, 1);
}

// Handles second introductory cave painting scene
int Scene::SC_ITEIntroCave2Proc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCaveCommonProc(param, 2);
}

// Handles third introductory cave painting scene
int Scene::SC_ITEIntroCave3Proc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCaveCommonProc(param, 3);
}

// Handles fourth introductory cave painting scene
int Scene::SC_ITEIntroCave4Proc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroCaveCommonProc(param, 4);
}

int Scene::SC_ITEIntroValleyProc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroValleyProc(param);
}

// Handles intro title scene (valley overlook)
int Scene::ITEIntroValleyProc(int param) {
	Event event;
	EventColumns *eventColumns;

	int n_credits = ARRAYSIZE(creditsValley);

	switch (param) {
	case SCENE_BEGIN:
		// Begin title screen background animation
		_vm->_anim->setCycles(0, -1);

		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = 0;
		event.time = 0;
		eventColumns = _vm->_events->queue(event);

		// Begin ITE title theme music
		_vm->_music->stop();

		_vm->_events->chainMusic(eventColumns, MUSIC_TITLE_THEME);

		// Pause animation before logo
		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventStop;
		event.param = 0;
		event.time = 3000;
		_vm->_events->chain(eventColumns, event);

		// Display logo
		event.type = kEvTContinuous;
		event.code = kTransitionEvent;
		event.op = kEventDissolveBGMask;
		event.time = 0;
		event.duration = LOGO_DISSOLVE_DURATION;
		_vm->_events->chain(eventColumns, event);

		// Remove logo
		event.type = kEvTContinuous;
		event.code = kTransitionEvent;
		event.op = kEventDissolve;
		event.time = 3000;
		event.duration = LOGO_DISSOLVE_DURATION;
		_vm->_events->chain(eventColumns, event);

		// Unpause animation before logo
		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.time = 0;
		event.param = 0;
		_vm->_events->chain(eventColumns, event);

		// Queue game credits list
		eventColumns = queueCredits(9000, CREDIT_DURATION1, n_credits, creditsValley);

		// End scene after credit display
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = 1000;
		_vm->_events->chain(eventColumns, event);

		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroTreeHouseProc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroTreeHouseProc(param);
}

// Handles second intro credit screen (treehouse view)
int Scene::ITEIntroTreeHouseProc(int param) {
	Event event;
	EventColumns *eventColumns;

	int n_credits1 = ARRAYSIZE(creditsTreeHouse1);
	int n_credits2 = ARRAYSIZE(creditsTreeHouse2);

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = kEvTContinuous;
		event.code = kTransitionEvent;
		event.op = kEventDissolve;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;
		eventColumns = _vm->_events->queue(event);

		if (_vm->_anim->hasAnimation(0)) {
			// Begin title screen background animation
			_vm->_anim->setFrameTime(0, 100);

			event.type = kEvTOneshot;
			event.code = kAnimEvent;
			event.op = kEventPlay;
			event.param = 0;
			event.time = 0;
			_vm->_events->chain(eventColumns, event);
		}

		// Queue game credits list
		queueCredits(DISSOLVE_DURATION + 2000, CREDIT_DURATION1, n_credits1, creditsTreeHouse1);
		eventColumns = queueCredits(DISSOLVE_DURATION + 7000, CREDIT_DURATION1, n_credits2, creditsTreeHouse2);

		// End scene after credit display
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = 1000;
		_vm->_events->chain(eventColumns, event);

		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroFairePathProc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroFairePathProc(param);
}

// Handles third intro credit screen (path to puzzle tent)
int Scene::ITEIntroFairePathProc(int param) {
	Event event;
	EventColumns *eventColumns;

	int n_credits1 = ARRAYSIZE(creditsFairePath1);
	int n_credits2 = ARRAYSIZE(creditsFairePath2);

	switch (param) {
	case SCENE_BEGIN:
		// Start 'dissolve' transition to new scene background
		event.type = kEvTContinuous;
		event.code = kTransitionEvent;
		event.op = kEventDissolve;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;
		eventColumns = _vm->_events->queue(event);

		// Begin title screen background animation
		_vm->_anim->setCycles(0, -1);

		event.type = kEvTOneshot;
		event.code = kAnimEvent;
		event.op = kEventPlay;
		event.param = 0;
		event.time = 0;
		_vm->_events->chain(eventColumns, event);

		// Queue game credits list
		queueCredits(DISSOLVE_DURATION + 2000, CREDIT_DURATION1, n_credits1, creditsFairePath1);
		eventColumns = queueCredits(DISSOLVE_DURATION + 7000, CREDIT_DURATION1, n_credits2, creditsFairePath2);

		// End scene after credit display
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = 1000;
		_vm->_events->chain(eventColumns, event);

		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

int Scene::SC_ITEIntroFaireTentProc(int param, void *refCon) {
	return ((Scene *)refCon)->ITEIntroFaireTentProc(param);
}

// Handles fourth intro credit screen (treehouse view)
int Scene::ITEIntroFaireTentProc(int param) {
	Event event;
	EventColumns *eventColumns;

	switch (param) {
	case SCENE_BEGIN:

		// Start 'dissolve' transition to new scene background
		event.type = kEvTContinuous;
		event.code = kTransitionEvent;
		event.op = kEventDissolve;
		event.time = 0;
		event.duration = DISSOLVE_DURATION;
		eventColumns = _vm->_events->queue(event);
		_vm->_events->chain(eventColumns, event);

		// Queue PC98 extra credits
		eventColumns = queueCredits(DISSOLVE_DURATION, CREDIT_DURATION1, ARRAYSIZE(creditsTent), creditsTent);

		// End scene after momentary pause
		event.type = kEvTOneshot;
		event.code = kSceneEvent;
		event.op = kEventEnd;
		event.time = (_vm->getPlatform() == Common::kPlatformPC98) ? 5000 - CREDIT_DURATION1 : 5000;
		_vm->_events->chain(eventColumns, event);

		break;
	case SCENE_END:
		break;
	default:
		warning("Illegal scene procedure parameter");
		break;
	}

	return 0;
}

} // End of namespace Saga
