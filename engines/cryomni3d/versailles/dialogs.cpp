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

#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

bool CryOmni3DEngine_Versailles::preprocessDialog(const Common::String &sequence) {
	if (_inventory.inInventoryByNameID(96) && _inventory.inInventoryByNameID(98)) {
		_dialogsMan["{JOUEUR-TROUVE-TITRE-ET-PAMPHLET}"] = 'Y';
	}
	if (_inventory.inInventoryByNameID(126)) {
		_dialogsMan["{JOUEUR_POSSEDE_EPIGRAPHE}"] = 'Y';
	}

	if (_currentLevel == 1 && _currentPlaceId == 3) {
		playInGameVideo("11D_LEB1");
	}

	_dialogsMan["{JOUEUR-VU-MEMORANDUM-DANS-LUSTRE-DU-SALON-DE-LA-GUERRE}"] = 'N';
	if (_currentLevel == 5 && _gameVariables[GameVariables::kSeenMemorandum] &&
	        !_inventory.inInventoryByNameID(140)) {
		_dialogsMan["{JOUEUR-VU-MEMORANDUM-DANS-LUSTRE-DU-SALON-DE-LA-GUERRE}"] = 'Y';
	}

	if (_currentLevel == 1 && _currentPlaceId == 1 && currentGameTime() == 3 &&
	        sequence.hasPrefix("13F_HUI") && _gameVariables[GameVariables::kWarnedIncomplete] == 0 &&
	        _dialogsMan["{JOUEUR-TROUVE-TITRE-ET-PAMPHLET}"] == 'Y' &&
	        (!_inventory.inInventoryByNameID(96) || !_inventory.inInventoryByNameID(98))) {
		displayMessageBoxWarp(18);
		_gameVariables[GameVariables::kWarnedIncomplete] = 1;
		return 0;
	}
	if (_currentLevel == 2 && _currentPlaceId == 11 && currentGameTime() == 4 &&
	        sequence.hasPrefix("24Z_BON") && _gameVariables[GameVariables::kWarnedIncomplete] == 0 &&
	        _dialogsMan["{JOUEUR-MONTRE-TITRE-FABLE-APPARU-SUR-ESQUISSE}"] == 'Y' &&
	        (!_inventory.inInventoryByNameID(101) || !_inventory.inInventoryByNameID(103))) {
		displayMessageBoxWarp(18);
		_gameVariables[GameVariables::kWarnedIncomplete] = 1;
		return 0;
	}
	if (_currentLevel == 3 && _currentPlaceId == 10 && currentGameTime() == 3 &&
	        sequence.hasPrefix("31O_SUIA") && _gameVariables[GameVariables::kWarnedIncomplete] == 0 &&
	        _dialogsMan["CROISSY-ACCEPTE-TEXTE"] == 'Y' &&
	        (!_inventory.inInventoryByNameID(121) || !_inventory.inInventoryByNameID(119) ||
	         !_inventory.inInventoryByNameID(115) ||
	         _gameVariables[GameVariables::kGotMedalsSolution] == 0)) {
		displayMessageBoxWarp(18);
		_gameVariables[GameVariables::kWarnedIncomplete] = 1;
		return 0;
	}
	if (_currentLevel == 4 && _currentPlaceId == 10 && currentGameTime() == 3 &&
	        sequence.hasPrefix("42C_BON") && _gameVariables[GameVariables::kWarnedIncomplete] == 0 &&
	        _dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] == 'Y' &&
	        (!_inventory.inInventoryByNameID(127) ||
	         _gameVariables[GameVariables::kUsedVaubanBlueprint1] == 0 ||
	         _gameVariables[GameVariables::kUsedVaubanBlueprint2] == 0)) {
		displayMessageBoxWarp(18);
		_gameVariables[GameVariables::kWarnedIncomplete] = 1;
		return 0;
	}
	if (_currentLevel == 5 && _currentPlaceId == 10 && currentGameTime() == 3 &&
	        sequence.hasPrefix("42C_BON") && _gameVariables[GameVariables::kWarnedIncomplete] == 0 &&
	        _dialogsMan["{JOUEUR-MONTRE-PAMPHLET-RELIGION}"] == 'Y' &&
	        (!_inventory.inInventoryByNameID(127) ||
	         _gameVariables[GameVariables::kUsedVaubanBlueprint1] == 0 ||
	         _gameVariables[GameVariables::kUsedVaubanBlueprint2] == 0)) {
		displayMessageBoxWarp(18);
		_gameVariables[GameVariables::kWarnedIncomplete] = 1;
		return 0;
	}
	if (_currentLevel == 6 && _currentPlaceId == 14 && currentGameTime() == 2 &&
	        sequence.hasPrefix("61_BON") && _gameVariables[GameVariables::kWarnedIncomplete] == 0) {
		displayMessageBoxWarp(19);
		_gameVariables[GameVariables::kWarnedIncomplete] = 1;
		return 0;
	}
	return 1;
}

void CryOmni3DEngine_Versailles::postprocessDialog(const Common::String &sequence) {
	if (_currentLevel == 1) {
		if (_dialogsMan["{LEVEL1_FINI}"] == 'Y') {
			playTransitionEndLevel(1);
		}
	} else if (_currentLevel == 2) {
		_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS}"] = 'N';
		_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS2}"] = 'N';
		_dialogsMan["{JOUEUR-PRESENTE-FAUX-CROQUIS3}"] = 'N';
		_dialogsMan["{LE JOUEUR-PRESENTE-AUTRES-ESQUISSES-OU-ESQUISSE-NON-TRIEES}"] = 'N';
		_dialogsMan["{LE JOUEUR-PRESENTE-ESQUISSES-TRIEES}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TITRE-FABLE-APPARU-SUR-ESQUISSE}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-ESQUISSE-DETRUITE}"] = 'N';
		if (_dialogsMan["{LEVEL2_FINI}"] == 'Y') {
			playTransitionEndLevel(2);
		}
	} else if (_currentLevel == 3) {
		if (currentGameTime() == 1 && _dialogsMan["LULLY-DONNE-MISSION1-JOUEUR"] == 'Y') {
			setGameTime(2, 3);
		}
		if (!_gameVariables[GameVariables::kGotMedalsSolution] &&
		        _dialogsMan["MONSIEUR-DONNE-SOLUTION-MEDAILLES"] == 'Y') {
			playInGameVideo("32M_MR2");
			_gameVariables[GameVariables::kGotMedalsSolution] = 1;
		}
		if (!_gameVariables[GameVariables::kDecipherScore] &&
		        _dialogsMan["LULLY-DIT-CHAT-PENDU-JOUEUR"] == 'Y') {
			_gameVariables[GameVariables::kDecipherScore] = 1;
			collectObject(118);
			setGameTime(3, 3);
		}
		if (currentGameTime() == 3 && _dialogsMan["CROISSY-ACCEPTE-TEXTE"] == 'Y') {
			setGameTime(4, 3);
		}
		if (_dialogsMan["{LEVEL3_FINI}"] == 'Y') {
			playTransitionEndLevel(3);
		}
		if (sequence == "32M_MR" && _dialogsMan["MONSIEUR-DONNE-SOLUTION-MEDAILLES"] == 'Y') {
			_dialogsMan["{JOUEUR-MONTRE-MEDAILLES-MONSIEUR}"] = 'Y';
		}
		_dialogsMan["{JOUEUR-MONTRE-PAMPHLET-ARCHITECTURE}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-EPIGRAPHE-MEDAILLES}"] = 'N';
		_dialogsMan["{JOUEUR-MONTRE-TOUT-AUTRE-CHOSE}"] = 'N';
	} else if (_currentLevel == 4) {
		if (_dialogsMan["{LEVEL4_FINI}"] == 'Y') {
			playTransitionEndLevel(4);
		}
	} else if (_currentLevel == 5) {
		if (sequence == "54I_BON" && _dialogsMan["BONTEMPS-DIT-PROMENADE"] == 'Y') {
			collectObject(141);
			playTransitionEndLevel(5);
		}
		if (sequence == "52A4_LAC" && _gameVariables[GameVariables::kStateLampoonReligion] != 3 &&
		        _dialogsMan["LACHAIZE-DIT-REFORME"] == 'Y' && _dialogsMan["LACHAIZE-DIT-DRAGONNADES"] == 'Y' &&
		        _dialogsMan["LACHAIZE-TROUVE-ECROUELLES"] == 'Y') {
			_inventory.removeByNameID(125);
			_gameVariables[GameVariables::kStateLampoonReligion] = 3;
			collectObject(125);
			_inventory.deselectObject();
		}
	}
}

void CryOmni3DEngine_Versailles::updateGameTimeDialVariables() {
	_dialogsMan["{CURRENT_GAME_TIME1}"] = 'N';
	_dialogsMan["{CURRENT_GAME_TIME2}"] = 'N';
	_dialogsMan["{CURRENT_GAME_TIME3}"] = 'N';
	_dialogsMan["{CURRENT_GAME_TIME4}"] = 'N';
	_dialogsMan["{CURRENT_GAME_TIME5}"] = 'N';
	switch (currentGameTime()) {
	case 1:
		_dialogsMan["{CURRENT_GAME_TIME1}"] = 'Y';
		break;
	case 2:
		_dialogsMan["{CURRENT_GAME_TIME2}"] = 'Y';
		break;
	case 3:
		_dialogsMan["{CURRENT_GAME_TIME3}"] = 'Y';
		break;
	case 4:
		_dialogsMan["{CURRENT_GAME_TIME4}"] = 'Y';
		break;
	case 5:
		_dialogsMan["{CURRENT_GAME_TIME5}"] = 'Y';
		break;
	default:
		error("Invalid current game time %d", currentGameTime());
	}
}

void CryOmni3DEngine_Versailles::setupDialogShows() {
	_dialogsMan.registerShowCallback("(BONTEMPS-MONTRE-TROISIEME-TITRE-DE-FABLE)",
	                                 &CryOmni3DEngine_Versailles::dialogShowBontempsShowThird);
	_dialogsMan.registerShowCallback("(HUISSIER DONNE PAMPHLET SUR LA FAMILLE ROYALE)",
	                                 &CryOmni3DEngine_Versailles::dialogShowHuissierShowPamphlet);
	_dialogsMan.registerShowCallback("(MONSEIGNEUR TRIE LES ESQUISSES)",
	                                 &CryOmni3DEngine_Versailles::dialogShowMonseigneurSorts);
	_dialogsMan.registerShowCallback("(ANIMATION LE BRUN REGARDE LES ESQUISSES)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLeBrunWatches);
	_dialogsMan.registerShowCallback("(OUVERTURE DES PORTES)",
	                                 &CryOmni3DEngine_Versailles::dialogShowDoorsOpen);
	_dialogsMan.registerShowCallback("(GARDE SUISSE DONNE CLEF PETITE PORTE)",
	                                 &CryOmni3DEngine_Versailles::dialogShowSwissGuardGives);
	_dialogsMan.registerShowCallback("(LULLY CORRIGE LA PARTITION.)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLullyCorrects);
	_dialogsMan.registerShowCallback("(BONTEMPS DONNE AUTORISATION)",
	                                 &CryOmni3DEngine_Versailles::dialogShowBontempsGivesAuth);
	_dialogsMan.registerShowCallback("(CROISSY PART)",
	                                 &CryOmni3DEngine_Versailles::dialogShowCroissyLeave);
	_dialogsMan.registerShowCallback("(MAINTENON-DONNE-PAMPHLET-RELIGION)",
	                                 &CryOmni3DEngine_Versailles::dialogShowMaintenonGives);
	_dialogsMan.registerShowCallback("(LA CHAIZE REND TEXTE)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLaChaizeGivesBack);
	_dialogsMan.registerShowCallback("(LA CHAIZE " "\x83" "CRIT DRAGONNADES)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLaChaizeWrites);
	_dialogsMan.registerShowCallback("(LACHAIZE-DONNE-PAMPHLET-JOUEUR)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLaChaizeGivesPamphlet);
	_dialogsMan.registerShowCallback("(BONTEMPS-DONNE-CLEF-DES-COMBLES)",
	                                 &CryOmni3DEngine_Versailles::dialogShowBontempsGivesKey);
	_dialogsMan.registerShowCallback("(LE DUC DU MAINE S'EN VA)",
	                                 &CryOmni3DEngine_Versailles::dialogShowDuMaineLeaves);
	_dialogsMan.registerShowCallback("(SC" "\xe9" "NE DE TRANSITION)",
	                                 &CryOmni3DEngine_Versailles::dialogShowTransitionScene);
	_dialogsMan.registerShowCallback("(FIN DU JEU)", &CryOmni3DEngine_Versailles::dialogShowEndOfGame);
	_dialogsMan.registerShowCallback("(LEBRUN-DONNE-FAUSSES-ESQUISSES)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLeBrunGives);
	_dialogsMan.registerShowCallback("(LEBRUN_S_EN_VA)",
	                                 &CryOmni3DEngine_Versailles::dialogShowLeBrunLeave);
}

void CryOmni3DEngine_Versailles::dialogShowBontempsShowThird() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowHuissierShowPamphlet() {
	collectObject(101);
	_inventory.deselectObject();
}

void CryOmni3DEngine_Versailles::dialogShowMonseigneurSorts() {
	_inventory.removeByNameID(105);
	collectObject(106);
	_gameVariables[GameVariables::kSketchState] = 2; // Sketches sorted
	_inventory.deselectObject();
	setGameTime(3, 2);
	_dialogsMan["MONSEIGNEUR-ATTEND-ESQUISSES"] = 'N';
}

void CryOmni3DEngine_Versailles::dialogShowLeBrunWatches() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowDoorsOpen() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowSwissGuardGives() {
	collectObject(123);
	_dialogsMan["{JOUEUR-POSSEDE-CLE}"] = 'Y';
}

void CryOmni3DEngine_Versailles::dialogShowLullyCorrects() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowBontempsGivesAuth() {
	collectObject(120);
}

void CryOmni3DEngine_Versailles::dialogShowCroissyLeave() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowMaintenonGives() {
	collectObject(125);
	_inventory.deselectObject();
}

void CryOmni3DEngine_Versailles::dialogShowLaChaizeGivesBack() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowLaChaizeWrites() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowLaChaizeGivesPamphlet() {
	// Nothing to do
}

void CryOmni3DEngine_Versailles::dialogShowBontempsGivesKey() {
	collectObject(140);
	_inventory.deselectObject();
}

void CryOmni3DEngine_Versailles::dialogShowDuMaineLeaves() {
	playInGameVideo("62S_DUC1");
	_inventory.removeByNameID(144);
	setPlaceState(19, 1);
}

void CryOmni3DEngine_Versailles::dialogShowTransitionScene() {
	playTransitionEndLevel(6);
}

void CryOmni3DEngine_Versailles::dialogShowEndOfGame() {
	doGameOver();
}

void CryOmni3DEngine_Versailles::dialogShowLeBrunGives() {
	collectObject(107);
	_inventory.deselectObject();
}

void CryOmni3DEngine_Versailles::dialogShowLeBrunLeave() {
	playInGameVideo("11D_LEB3");
	setGameTime(2, 1);
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
