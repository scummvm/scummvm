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

#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/metagame/views/grand_tour.h"
#include "bagel/hodjnpodj/metagame/bgen/mgstat.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define FONT_SIZE 8

#define	NOPLAY		-1
#define GAME_ALPHA	0
#define GAME_GEO	1
#define GAME_RAND	2

// Button positioning constants
#define F_WIDTH					110
#define F_HEIGHT                20
#define F_TOP                   325
#define PLAY_LEFT				75
#define SAVE_LEFT				PLAY_LEFT + F_WIDTH + 5
#define RESTORE_LEFT			SAVE_LEFT + F_WIDTH + 5
#define	AUDIO_LEFT				PLAY_LEFT
#define TOP10_LEFT              AUDIO_LEFT + F_WIDTH + 5
#define LEAVE_LEFT				TOP10_LEFT + F_WIDTH + 5

#define S_WIDTH					130
#define S_HEIGHT				21

#define HS_LEFT					SCROLL_LEFT + 110

#define HSH_TOP					SCROLL_TOP + 50
#define HSM_TOP					HSH_TOP + S_HEIGHT + 2
#define HSL_TOP					HSM_TOP + S_HEIGHT + 2
#define HSNP_TOP				HSL_TOP + S_HEIGHT + 2

#define PS_LEFT					SCROLL_LEFT + 310

#define PSH_TOP					SCROLL_TOP + 50
#define PSM_TOP					PSH_TOP + S_HEIGHT + 2
#define PSL_TOP					PSM_TOP + S_HEIGHT + 2
#define PSNP_TOP				PSL_TOP + S_HEIGHT + 2

#define G_WIDTH					130
#define G_HEIGHT				21

#define G_TOP					SCROLL_TOP + 185
#define GA_LEFT					SCROLL_LEFT + 55
#define GG_LEFT					GA_LEFT + G_WIDTH + 5
#define GR_LEFT					GG_LEFT + G_WIDTH + 5

GrandTour::GrandTour() : Dialog("GrandTour"),
	_playButton("PLAY", "Play",          RectWH(PLAY_LEFT, F_TOP - 25, F_WIDTH, F_HEIGHT), this),
	_saveButton("SAVE", "Save",          RectWH(SAVE_LEFT, F_TOP - 25, F_WIDTH, F_HEIGHT), this),
	_restoreButton("LOAD", "Restore",    RectWH(RESTORE_LEFT, F_TOP - 25, F_WIDTH, F_HEIGHT), this),
	_audioButton("AUDIO", "Audio",       RectWH(AUDIO_LEFT, F_TOP, F_WIDTH, F_HEIGHT), this),
	_top10Button("TOP10", "Top 10 List", RectWH(TOP10_LEFT, F_TOP, F_WIDTH, F_HEIGHT), this),
	_mainMenuButton("CLOSE", "Main Menu",RectWH(LEAVE_LEFT, F_TOP, F_WIDTH, F_HEIGHT), this)
{
}

bool GrandTour::msgOpen(const OpenMessage &msg) {
	Dialog::msgOpen(msg);
	g_engine->_bReturnToGrandTour = true;
	_grandTour.reset();

	adjustScore();

	return true;
}

bool GrandTour::msgClose(const CloseMessage &msg) {
	Dialog::msgClose(msg);
	g_engine->_bReturnToGrandTour = false;

	return true;
}

bool GrandTour::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		if (msg._stringValue == "CLOSE") {
			replaceView("TitleMenu");
			return true;
		}
	}

	return false;
}

void GrandTour::draw() {
	Dialog::draw();

	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);
}

void GrandTour::adjustScore() {
	int nGameScore = 0;
	int32 lTemp = 0l;

	switch (m_pgtGTStruct->nCurrGameCode) {
	case MG_GAME_ARCHEROIDS: // 1 or 0
	case MG_GAME_ARTPARTS: // 1 or 0
	case MG_GAME_BATTLEFISH: // 1 or 0
	case MG_GAME_MANKALA: // 1 or 0
	case MG_GAME_MAZEODOOM: // 1 or 0
	case MG_GAME_RIDDLES: // 1 or 0
		nGameScore = m_pgtGTStruct->stMiniGame.lScore * 100;
		break;

	case MG_GAME_BEACON:
		//  %
		nGameScore = MIN<int>(100, (m_pgtGTStruct->stMiniGame.lScore * 2));
		break;

	case MG_GAME_LIFE:
		// number
		nGameScore = MIN<int>(100, ((m_pgtGTStruct->stMiniGame.lScore * 25) / 10));
		break;

	case MG_GAME_THGESNGGME:
		// %
		nGameScore = MIN<int>(100, ((m_pgtGTStruct->stMiniGame.lScore * 15) / 10));
		break;

	case MG_GAME_CRYPTOGRAMS:
		// number
		nGameScore = m_pgtGTStruct->stMiniGame.lScore;
		break;

	case MG_GAME_PEGGLEBOZ: // number
	case MG_GAME_GARFUNKEL: // number
	case MG_GAME_WORDSEARCH: // number
		nGameScore = (m_pgtGTStruct->stMiniGame.lScore * 100) / 25;
		break;

	case MG_GAME_BARBERSHOP:
		// number of cards discarded
		nGameScore = (m_pgtGTStruct->stMiniGame.lScore * 100) / 62;
		break;

	case MG_GAME_NOVACANCY:
		// number
		nGameScore = 100 - (((m_pgtGTStruct->stMiniGame.lScore) * 100) / 45);
		break;

	case MG_GAME_DAMFURRY:
		// number
		nGameScore = ((m_pgtGTStruct->stMiniGame.lScore) * 100) / 60;
		break;

	case MG_GAME_FUGE:
		// number
		nGameScore = ((m_pgtGTStruct->stMiniGame.lScore) * 100) / 53;
		break;

	case MG_GAME_PACRAT:
		// number
		lTemp = m_pgtGTStruct->stMiniGame.lScore * 100;

		switch (m_pgtGTStruct->stMiniGame.nSkillLevel) {
		case SKILLLEVEL_LOW:
			nGameScore = (int)(lTemp / 2373);
			break;
		case SKILLLEVEL_MEDIUM:
			nGameScore = (int)(lTemp / 14280);
			break;
		case SKILLLEVEL_HIGH:
			nGameScore = (int)(lTemp / 28584);
			break;
		}
		break;

	default:
		nGameScore = 0;
		break;
	}

	if (m_pgtGTStruct->bPlayingHodj) {
		m_pgtGTStruct->nHodjLastScore = nGameScore;
		m_pgtGTStruct->nHodjScore += nGameScore;
	} else {
		m_pgtGTStruct->nPodjLastScore = nGameScore;
		m_pgtGTStruct->nPodjScore += nGameScore;
	}
}

/*------------------------------------------------------------------------*/

void GRANDTRSTRUCT::reset() {
	bMidGrandTour = false;
	nHodjSkillLevel = SKILLLEVEL_LOW;
	nPodjSkillLevel = NOPLAY;
	nGameSelection = GAME_ALPHA;
	nCurrGameCode = 0;
	nHodjScore = 0;
	nPodjScore = 0;
	nHodjLastGame = 0;
	nPodjLastGame = 0;
	nHodjLastScore = 0;
	nPodjLastScore = 0;
	bPlayMusic = true;
	bPlayFX = true;
	bPlayingHodj = true;
	Common::fill(abHGamePlayed, abHGamePlayed + 18, false);
	Common::fill(abPGamePlayed, abPGamePlayed + 18, false);

	stMiniGame.lCrowns = 0;
	stMiniGame.lScore = 0;
	stMiniGame.nSkillLevel = SKILLLEVEL_LOW;
	stMiniGame.bSoundEffectsEnabled = bPlayFX;
	stMiniGame.bMusicEnabled = bPlayMusic;
	stMiniGame.bPlayingMetagame = true;
	stMiniGame.bPlayingHodj = true;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
