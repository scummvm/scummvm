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
#include "bagel/metaengine.h"

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

#define HS_LEFT					110

#define HSH_TOP					50
#define HSM_TOP					HSH_TOP + S_HEIGHT + 2
#define HSL_TOP					HSM_TOP + S_HEIGHT + 2
#define HSNP_TOP				HSL_TOP + S_HEIGHT + 2

#define PS_LEFT					310

#define PSH_TOP					50
#define PSM_TOP					PSH_TOP + S_HEIGHT + 2
#define PSL_TOP					PSM_TOP + S_HEIGHT + 2
#define PSNP_TOP				PSL_TOP + S_HEIGHT + 2

#define G_WIDTH					130
#define G_HEIGHT				21

#define G_TOP					185
#define GA_LEFT					55
#define GG_LEFT					GA_LEFT + G_WIDTH + 5
#define GR_LEFT					GG_LEFT + G_WIDTH + 5

#define S_WIDTH					130
#define S_HEIGHT				21

#define HS_LEFT					110

#define HSH_TOP					50
#define HSM_TOP					HSH_TOP + S_HEIGHT + 2
#define HSL_TOP					HSM_TOP + S_HEIGHT + 2
#define HSNP_TOP				HSL_TOP + S_HEIGHT + 2

#define PS_LEFT					310

#define PSH_TOP					50
#define PSM_TOP					PSH_TOP + S_HEIGHT + 2
#define PSL_TOP					PSM_TOP + S_HEIGHT + 2
#define PSNP_TOP				PSL_TOP + S_HEIGHT + 2


static const byte anGameValues[18] = {
	MG_GAME_ARCHEROIDS, MG_GAME_ARTPARTS, MG_GAME_BARBERSHOP, MG_GAME_BATTLEFISH,
	MG_GAME_BEACON, MG_GAME_CRYPTOGRAMS, MG_GAME_DAMFURRY, MG_GAME_FUGE,
	MG_GAME_GARFUNKEL, MG_GAME_LIFE, MG_GAME_MANKALA, MG_GAME_MAZEODOOM,
	MG_GAME_NOVACANCY, MG_GAME_PACRAT, MG_GAME_PEGGLEBOZ, MG_GAME_RIDDLES,
	MG_GAME_THGESNGGME, MG_GAME_WORDSEARCH
};

static const char *aszGames[18] = {
	"Archeroids", "Art Parts", "Barbershop Quintet", "Battlefish", "Beacon", "Cryptograms",
	"Dam Furry Animals", "Fuge", "Garfunkel", "Life", "Mankala", "Maze O' Doom",
	"No Vacancy", "Pack-Rat", "Peggleboz", "Riddles", "TH GESNG GAM", "Word Search"
};

static const byte anGeoOrder[18] = { 9, 12, 11, 0, 7, 13, 5, 16, 17, 1, 4, 14, 3, 10, 15, 2, 6, 8 };


GrandTour::GrandTour() : Dialog("GrandTour"),
	_playButton("PLAY", "Play",           RectWH(PLAY_LEFT, F_TOP - 25, F_WIDTH, F_HEIGHT), this),
	_saveButton("SAVE", "Save",           RectWH(SAVE_LEFT, F_TOP - 25, F_WIDTH, F_HEIGHT), this),
	_restoreButton("LOAD", "Restore",     RectWH(RESTORE_LEFT, F_TOP - 25, F_WIDTH, F_HEIGHT), this),
	_audioButton("AUDIO", "Audio",        RectWH(AUDIO_LEFT, F_TOP, F_WIDTH, F_HEIGHT), this),
	_top10Button("TOP10", "Top 10 List",  RectWH(TOP10_LEFT, F_TOP, F_WIDTH, F_HEIGHT), this),
	_mainMenuButton("CLOSE", "Main Menu", RectWH(LEAVE_LEFT, F_TOP, F_WIDTH, F_HEIGHT), this),

	pHSHButton("HHARD", "Hard",          RectWH(HS_LEFT, HSH_TOP, S_WIDTH, S_HEIGHT), this),
	pHSMButton("HMEDIUM", "Medium",      RectWH(HS_LEFT, HSM_TOP, S_WIDTH, S_HEIGHT), this),
	pHSLButton("HEASY", "Easy",          RectWH(HS_LEFT, HSL_TOP, S_WIDTH, S_HEIGHT), this),
	pHSNPButton("HNONE", "Count Me Out", RectWH(HS_LEFT, HSNP_TOP, S_WIDTH, S_HEIGHT), this),
	pPSHButton("PHARD", "Hard",          RectWH(PS_LEFT, PSH_TOP, S_WIDTH, S_HEIGHT), this),
	pPSMButton("PMEDIUM", "Medium",      RectWH(PS_LEFT, PSM_TOP, S_WIDTH, S_HEIGHT), this),
	pPSLButton("PEASY", "Easy",          RectWH(PS_LEFT, PSL_TOP, S_WIDTH, S_HEIGHT), this),
	pPSNPButton("PNONE", "Count Me Out", RectWH(PS_LEFT, PSNP_TOP, S_WIDTH, S_HEIGHT), this),

	pGAButton("ALPHA", "Alphabetically", RectWH(GA_LEFT, G_TOP, G_WIDTH, G_HEIGHT), this),
	pGGButton("GEOG",  "Geographically", RectWH(GG_LEFT, G_TOP, G_WIDTH, G_HEIGHT), this),
	pGRButton("RANDOM", "Randomly",      RectWH(GR_LEFT, G_TOP, G_WIDTH, G_HEIGHT), this)
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

bool GrandTour::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		replaceView("TitleMenu");
		return true;
	}

	return false;
}

bool GrandTour::msgGame(const GameMessage &msg) {
	if (msg._name == "BUTTON") {
		if (msg._stringValue == "CLOSE") {
			replaceView("TitleMenu");
			return true;
		} else if (msg._stringValue == "TOP10") {
			addView("TopScores");
			return true;
		}
	} else if (msg._name == "RADIOBUTTON") {
		if (msg._stringValue == "HHARD") {
			m_pgtGTStruct->nHodjSkillLevel = SKILLLEVEL_HIGH;
			m_pgtGTStruct->bPlayingHodj = true;
		} else if (msg._stringValue == "HMEDIUM") {
			m_pgtGTStruct->nHodjSkillLevel = SKILLLEVEL_MEDIUM;
			m_pgtGTStruct->bPlayingHodj = true;
		} else if (msg._stringValue == "HEASY") {
			m_pgtGTStruct->nHodjSkillLevel = SKILLLEVEL_LOW;
			m_pgtGTStruct->bPlayingHodj = true;
		} else if (msg._stringValue == "HNONE") {
			m_pgtGTStruct->nHodjSkillLevel = NOPLAY;
			m_pgtGTStruct->bPlayingHodj = true;
		}

		else if (msg._stringValue == "PHARD") {
			m_pgtGTStruct->nPodjSkillLevel = SKILLLEVEL_HIGH;
			m_pgtGTStruct->bPlayingHodj = false;
		} else if (msg._stringValue == "PMEDIUM") {
			m_pgtGTStruct->nPodjSkillLevel = SKILLLEVEL_MEDIUM;
			m_pgtGTStruct->bPlayingHodj = false;
		} else if (msg._stringValue == "PEASY") {
			m_pgtGTStruct->nPodjSkillLevel = SKILLLEVEL_LOW;
			m_pgtGTStruct->bPlayingHodj = false;
		} else if (msg._stringValue == "PNONE") {
			m_pgtGTStruct->nPodjSkillLevel = NOPLAY;
			m_pgtGTStruct->bPlayingHodj = false;
		}

		else if (msg._stringValue == "ALPHA") {
			m_pgtGTStruct->nGameSelection = GAME_ALPHA;
		} else if (msg._stringValue == "GEOG") {
			m_pgtGTStruct->nGameSelection = GAME_GEO;
		} else if (msg._stringValue == "RANDOM") {
			m_pgtGTStruct->nGameSelection = GAME_RAND;
		}

		updateRadioButtons();
		return true;
	}

	return false;
}

void GrandTour::draw() {
	Dialog::draw();
	uint nOldTextAlign;
	Common::String cNextGame;
	Common::String cLastGame;
	int nGameCode;
	int nGamesCompleted = 0;
	int i;

	GfxSurface s = getSurface();
	s.setFontSize(FONT_SIZE);

	cNextGame = "RANDOM";
	nGameCode = getNextGameCode(false);
	for (i = 0; i < 18; i++) {
		if ((m_pgtGTStruct->nGameSelection != GAME_RAND) && (anGameValues[i] == nGameCode)) {
			cNextGame = aszGames[i];
			break;
		}
	}

	//	Hodj's Text
	s.setTextColor(BLUE);
	s.writeString("Hodj", Common::Point(HS_LEFT - 40, HSH_TOP));
	s.writeString("Score :", Common::Point(HS_LEFT, HSNP_TOP + S_HEIGHT + 2));
	s.writeString(Common::String::format("%d", m_pgtGTStruct->nHodjScore),
		Common::Point(HS_LEFT + 50, HSNP_TOP + S_HEIGHT + 2));

	if (m_pgtGTStruct->bPlayingHodj) {
		s.writeString("Hodj", Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 96));

		cLastGame = "NONE";
		for (i = 0; i < 18; i++) {
			if (anGameValues[i] == m_pgtGTStruct->nHodjLastGame) {
				cLastGame = aszGames[i];
			}
			if (m_pgtGTStruct->abHGamePlayed[i])
				nGamesCompleted++;
		}

		s.writeString(cLastGame, Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 24));
		s.writeString(Common::String::format("%d", m_pgtGTStruct->nHodjLastScore),
			Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 42));
		s.writeString(cNextGame, Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 60));
		s.writeString(Common::String::format("%d", nGamesCompleted),
			Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 78));
	}

	//	Podj's Text
	s.setTextColor(RED);
	s.writeString("Podj", Common::Point(PS_LEFT - 40, PSH_TOP));
	s.writeString("Score :", Common::Point(PS_LEFT, PSNP_TOP + S_HEIGHT + 2));
	s.writeString(Common::String::format("%d", m_pgtGTStruct->nPodjScore),
		Common::Point(PS_LEFT + 50, PSNP_TOP + S_HEIGHT + 2));

	if (m_pgtGTStruct->bPlayingHodj == false) {
		s.writeString("Podj", Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 96));

		cLastGame = "NONE";
		for (i = 0; i < 18; i++) {
			if (anGameValues[i] == m_pgtGTStruct->nPodjLastGame) {
				cLastGame = aszGames[i];
			}
			if (m_pgtGTStruct->abPGamePlayed[i])
				nGamesCompleted++;
		}

		s.writeString(cLastGame, Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 24));
		s.writeString(Common::String::format("%d", m_pgtGTStruct->nPodjLastScore),
			Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 42));
		s.writeString(cNextGame, Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 60));
		s.writeString(
			Common::String::format("%i Out Of 18", nGamesCompleted),
			Common::Point(HS_LEFT + S_WIDTH + 5, G_TOP + 78));
	}

	//	Other Text
	s.setTextColor(PURPLE);
	s.writeString("Game Order:", Common::Point(GA_LEFT, G_TOP - 20));

	s.writeString("Last Game :", RectWH(HS_LEFT, G_TOP + 24, S_WIDTH, 16),
		PURPLE, Graphics::kTextAlignRight);
	s.writeString("Last Game's Score :", RectWH(HS_LEFT, G_TOP + 42, S_WIDTH, 16),
		PURPLE, Graphics::kTextAlignRight);
	s.writeString("Next Game :", RectWH(HS_LEFT, G_TOP + 60, S_WIDTH, 16),
		PURPLE, Graphics::kTextAlignRight);
	s.writeString("Games Completed :", RectWH(HS_LEFT, G_TOP + 78, S_WIDTH, 16),
		PURPLE, Graphics::kTextAlignRight);
	s.writeString("About To Play :", RectWH(HS_LEFT, G_TOP + 96, S_WIDTH, 16),
		PURPLE, Graphics::kTextAlignRight);
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

void GrandTour::updateRadioButtons() {
	pHSHButton.setCheck(m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_HIGH);
	pHSMButton.setCheck(m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_MEDIUM);
	pHSLButton.setCheck(m_pgtGTStruct->nHodjSkillLevel == SKILLLEVEL_LOW);
	pHSNPButton.setCheck(m_pgtGTStruct->nHodjSkillLevel == NOPLAY);

	pPSHButton.setCheck(m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_HIGH);
	pPSMButton.setCheck(m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_MEDIUM);
	pPSLButton.setCheck(m_pgtGTStruct->nPodjSkillLevel == SKILLLEVEL_LOW);
	pPSNPButton.setCheck(m_pgtGTStruct->nPodjSkillLevel == NOPLAY);

	pGAButton.setCheck(m_pgtGTStruct->nGameSelection == GAME_ALPHA);
	pGGButton.setCheck(m_pgtGTStruct->nGameSelection == GAME_GEO);
	pGRButton.setCheck(m_pgtGTStruct->nGameSelection == GAME_RAND);

	redraw();
}

int GrandTour::getNextGameCode(bool bExecute) {
	int	i;
	int	nReturnValue = -1;
	bool bThereAreGamesToBePlayed = false;

	nReturnValue = -1;

	if (m_pgtGTStruct->nGameSelection == GAME_ALPHA) {
		for (i = 0; i < 18; i++) {
			if (m_pgtGTStruct->bPlayingHodj) {
				if (m_pgtGTStruct->abHGamePlayed[i] == false) {
					if (bExecute) {
						m_pgtGTStruct->nHodjLastGame = anGameValues[i];
						m_pgtGTStruct->abHGamePlayed[i] = true;
					}
					nReturnValue = anGameValues[i];
					break;
				}
			} else {
				if (m_pgtGTStruct->abPGamePlayed[i] == false) {
					if (bExecute) {
						m_pgtGTStruct->nPodjLastGame = anGameValues[i];
						m_pgtGTStruct->abPGamePlayed[i] = true;
					}
					nReturnValue = anGameValues[i];
					break;
				}
			}
		}
	} else {
		if (m_pgtGTStruct->nGameSelection == GAME_GEO) {
			for (i = 0; i < 18; i++) {
				if (m_pgtGTStruct->bPlayingHodj) {
					if (m_pgtGTStruct->abHGamePlayed[anGeoOrder[i]] == false) {
						if (bExecute) {
							m_pgtGTStruct->nHodjLastGame = anGameValues[anGeoOrder[i]];
							m_pgtGTStruct->abHGamePlayed[anGeoOrder[i]] = true;
						}
						nReturnValue = anGameValues[anGeoOrder[i]];
						break;
					}
				} else {
					if (m_pgtGTStruct->abPGamePlayed[anGeoOrder[i]] == false) {
						if (bExecute) {
							m_pgtGTStruct->nPodjLastGame = anGameValues[anGeoOrder[i]];
							m_pgtGTStruct->abPGamePlayed[anGeoOrder[i]] = true;
						}
						nReturnValue = anGameValues[anGeoOrder[i]];
						break;
					}
				}
			}
		} else {
			if ((m_pgtGTStruct->bPlayingHodj == false) && (m_pgtGTStruct->nHodjSkillLevel != NOPLAY)) {
				i = 0;
				nReturnValue = m_pgtGTStruct->nCurrGameCode;
				while (anGameValues[i] != nReturnValue) {
					i++;
				};
				if (bExecute) {
					m_pgtGTStruct->nPodjLastGame = anGameValues[i];
					m_pgtGTStruct->abPGamePlayed[i] = true;
				}
			} else {
				for (i = 0; i < 18; i++) {
					if (m_pgtGTStruct->bPlayingHodj) {
						if (m_pgtGTStruct->abHGamePlayed[i] == false) {
							bThereAreGamesToBePlayed = true;
							break;
						}
					} else {
						if (m_pgtGTStruct->abPGamePlayed[i] == false) {
							bThereAreGamesToBePlayed = true;
							break;
						}
					}
				}
				if (bThereAreGamesToBePlayed) {
					do {
						i = getRandomNumber(17);
						if (m_pgtGTStruct->bPlayingHodj) {
							if (m_pgtGTStruct->abHGamePlayed[i] == false) {
								if (bExecute) {
									m_pgtGTStruct->nHodjLastGame = anGameValues[i];
									m_pgtGTStruct->abHGamePlayed[i] = true;
								}
								nReturnValue = anGameValues[i];
							}
						} else {
							if (m_pgtGTStruct->abPGamePlayed[i] == false) {
								if (bExecute) {
									m_pgtGTStruct->nPodjLastGame = anGameValues[i];
									m_pgtGTStruct->abPGamePlayed[i] = true;
								}
								nReturnValue = anGameValues[i];
							}
						}
					} while (nReturnValue == -1);
				}
			}
		}
	}
	return nReturnValue;
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
