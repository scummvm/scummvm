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

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"

#include "common/config-manager.h"
#include "common/textconsole.h"

namespace Parallaction {

#ifdef USE_TTS

// Transcribed for TTS; only English is translated in-game
static const char *openingCreditsSecondLine[] = {
	"Grafica Totale: Max M.",		// Italian
	"Graphiques Totaux: Max M.",	// French
	"Total Graphics: Max M.",		// English
	"Gesamtgrafik: Max M."			// German
};

static const char *openingCreditsThirdLine[] = {
	"Design del Gioco: Mr. Tzutzumi",	// Italian
	"Conception de Jeux: Mr. Tzutzumi",	// French
	"Game Design: Mr. Tzutzumi",		// English
	"Spieldesign: Mr. Tzutzumi"			// German
};

// Transcribed for TTS; only Italian is translated in-game
static const char *endCreditsItalian[] = {
	"La Cassiera",
	"La Segretaria",
	"Il Taxista",
	"Il Giornalaio",
	"Passante",
	"Il Segretario",
	"L'Uccello",
	"L'Inserviente",
	"Il Priore",
	"Il Suicida",
	"Il Direttore",
	"Passante",
	"Fratello Shinpui",
	"Apollo il Gorilla",
	"Il Portiere",
	"Il Guru",
	"Fratello Baka",
	"Josko il Barman",
	"Figaro L' Estetista",
	"Il Guardiano del Museo",
	"Passante",
	"Il Losco Max",
	"Mister Y",
	"Il Punk",
	"Passante",
	"Il Signor Bemutsu",
	"L' Annunciatore",
	"La Punkina",
	"L' Imperatore",
	"Il Dottor Ki",
	"Il Cuoco",
	"Il Punk",
	"Passante",
	"Il Losco Figuro",
	"Chan L'Onesto",
	"",
	"La Geisha",
	"Kos 'O Professore",
	"Il Giocatore di Pacinko"
};

static const char *endCreditsFrench[] = {
	"La Cassi\350re",
	"La Secr\351taire",
	"Le Chaffeur de Taxi",
	"Le Marchand de Journaux",
	"La Pi\351tonne",
	"Le Secr\351taire",
	"La Corneille",
	"Le Oshiya",
	"Le Prieur",
	"Le Candidat au Suicide",
	"Le Directeur",
	"La Pi\351tonne",
	"Fr\350re Shinpui",
	"Apollo le Garde du Corps",
	"Le Concierge",
	"Le Guru",
	"Fr\350re Baka",
	"Josko le Barman",
	"Figaro L'esth\351ticien",
	"Le Gardien du Mus\351e",
	"La Pi\351tonne",
	"Le Ombrag\351e Max",
	"Monsieur Y",
	"Le Punk",
	"La Pi\351tonne",
	"Monsieur Bemutsu",
	"Le Annonceur",
	"Le Punk",
	"L'Empereur",
	"Le Docteur Ki",
	"Le Chef",
	"Le Punk",
	"Le Pi\351ton",
	"Le Louche Personnage",
	"Honest Chan",
	"",
	"La Geisha",
	"Professeur Kos",
	"Le Joueur de Pachinko"
};

static const char *endCreditsEnglish[] = {
	"Cashier",
	"Secretary",
	"Taxi-driver",
	"Newspaper Seller",
	"Pedestrian",
	"Secretary",
	"Grackle",
	"Oshiya",
	"Prior",
	"Suicidal Man",
	"Governor",
	"Pedestrian",
	"Brother Shinpui",
	"Apollo the Bodyguard",
	"Door-keeper",
	"Guru",
	"Brother Baka",
	"Josko the Barman",
	"Figaro the Beautician",
	"Museum Custodian",
	"Pedestrian",
	"Sullen Max",
	"Mister Y",
	"Punk",
	"Pedestrian",
	"Mister Bemutsu",
	"Announcer",
	"Punk",
	"Emperor",
	"Doctor Ki",
	"Cook",
	"Punk",
	"Pedestrian",
	"Shady Type",
	"Honest Chan",
	"",
	"Geisha",
	"Professor Kos",
	"Pachinko Player"
};

static const char *endCreditsGerman[] = {
	"Die Kassiererin",
	"Die Sekret\344rin",
	"Der Taxifahrer",
	"Der Zeitungsverk\344ufer",
	"Die Passantin",
	"Der Sekret\344r",
	"Des Grakula",
	"Der Oshiya",
	"Der Prior",
	"Der Selbstm\366rder",
	"Der Direktor",
	"Die Passantin",
	"Bruder Shinpui",
	"Apollo der Bodyguard",
	"Der Portier",
	"Der Guru",
	"Bruder Baka",
	"Josko der Barman",
	"Figaro, der Kosmetiker",
	"Der Museumsw\344rter",
	"Die Passantin",
	"Schattig Max",
	"Herr Y",
	"Der Punker",
	"Die Passantin",
	"Herr Bemutsu",
	"Der Ansager",
	"Der Punker",
	"Der Kaiser",
	"Doktor Ki",
	"Der Koch",
	"Der Punker",
	"Der Passant",
	"Der Dunkler Typ",
	"Honest Chan",
	"",
	"Die Geisha",
	"Professor Kos",
	"Der Pachinko-Spieler"
};

static const int kNumberOfCredits = ARRAYSIZE(endCreditsItalian);

#endif

#define INST_ON							1
#define INST_OFF						2
#define INST_X							3
#define INST_Y							4
#define INST_Z							5
#define INST_F							6
#define INST_LOOP						7
#define INST_ENDLOOP					8
#define INST_SHOW						9
#define INST_INC						10
#define INST_DEC						11
#define INST_SET						12
#define INST_PUT						13
#define INST_CALL						14
#define INST_WAIT						15
#define INST_START						16
#define INST_SOUND						17
#define INST_MOVE						18
#define INST_ENDSCRIPT					19

#define SetOpcodeTable(x) table = &x;



typedef Common::Functor1Mem<CommandContext&, void, CommandExec_ns> OpcodeV1;
#define COMMAND_OPCODE(op) table->push_back(new OpcodeV1(this, &CommandExec_ns::cmdOp_##op))
#define DECLARE_COMMAND_OPCODE(op) void CommandExec_ns::cmdOp_##op(CommandContext& ctxt)

typedef Common::Functor1Mem<ProgramContext&, void, ProgramExec_ns> OpcodeV2;
#define INSTRUCTION_OPCODE(op) table->push_back(new OpcodeV2(this, &ProgramExec_ns::instOp_##op))
#define DECLARE_INSTRUCTION_OPCODE(op) void ProgramExec_ns::instOp_##op(ProgramContext& ctxt)

extern const char *_instructionNamesRes_ns[];


DECLARE_INSTRUCTION_OPCODE(on) {
	InstructionPtr inst = ctxt._inst;

	inst->_a->_flags |= kFlagsActive;
	inst->_a->_flags &= ~kFlagsRemove;

#ifdef USE_TTS
	if (scumm_stricmp(inst->_a->_name, "telo3") == 0) {
		_vm->setTTSVoice(kNarratorVoiceID);
		_vm->sayText(openingCreditsSecondLine[_vm->getInternLanguage()], Common::TextToSpeechManager::INTERRUPT);
	} else if (scumm_stricmp(inst->_a->_name, "game") == 0) {
		_vm->setTTSVoice(kNarratorVoiceID);
		_vm->sayText(openingCreditsThirdLine[_vm->getInternLanguage()], Common::TextToSpeechManager::INTERRUPT);
	}
#endif
}


DECLARE_INSTRUCTION_OPCODE(off) {
	ctxt._inst->_a->_flags |= kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(loop) {
	InstructionPtr inst = ctxt._inst;

	ctxt._program->_loopCounter = inst->_opB.getValue();
	ctxt._program->_loopStart = ctxt._ip;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (ctxt._program->_loopStart == 11 && scumm_stricmp(_vm->_location._name, "test") == 0) {
		// Delay moving on from test results until TTS is done or the player clicks, 
		// so the TTS system can speak them fully
		Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
		if (ttsMan != nullptr && ConfMan.getBool("tts_enabled") && ttsMan->isSpeaking()) {
			ctxt._program->_loopCounter = 2;
			int event = _vm->_input->getLastButtonEvent();

			if (event == kMouseLeftUp) {
				ttsMan->stop();
				ctxt._program->_loopCounter = 0;
			}
		}
	}

	if (--ctxt._program->_loopCounter > 0) {
		ctxt._ip = ctxt._program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = ctxt._inst;
	int16 _si = inst->_opB.getValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (_si > 0 ? _si : -_si);
		if (ctxt._modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}

	int16 lvalue = inst->_opA.getValue();

	if (inst->_index == INST_INC) {
#ifdef USE_TTS
		if (_vm->_endCredits && ctxt._anim->_name[0] == 's' && _currentCredit < kNumberOfCredits) {
			const char **credits;

			switch (_vm->getInternLanguage()) {
			case kItalian:
				credits = endCreditsItalian;
				break;
			case kFrench:
				credits = endCreditsFrench;
				break;
			case kEnglish:
				credits = endCreditsEnglish;
				break;
			case kGerman:
				credits = endCreditsGerman;
				break;
			default:
				credits = endCreditsItalian;
				break;
			}

			_vm->sayText(credits[_currentCredit], Common::TextToSpeechManager::QUEUE);
			_currentCredit++;
		}
#endif

		lvalue += _si;
	} else {
		lvalue -= _si;
	}

	inst->_opA.setValue(lvalue);

}


DECLARE_INSTRUCTION_OPCODE(set) {
	ctxt._inst->_opA.setValue(ctxt._inst->_opB.getValue());
}


DECLARE_INSTRUCTION_OPCODE(put) {
	InstructionPtr inst = ctxt._inst;
	Common::Rect r;
	inst->_a->getFrameRect(r);

	Graphics::Surface v18;
	v18.init(r.width(), r.height(), r.width(), inst->_a->getFrameData(), Graphics::PixelFormat::createFormatCLUT8());

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();
	bool mask = (inst->_flags & kInstMaskedPut) == kInstMaskedPut;

	_vm->_gfx->patchBackground(v18, x, y, mask);
}

DECLARE_INSTRUCTION_OPCODE(show) {
	ctxt._suspend = true;
}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", ctxt._inst->_index);
}

DECLARE_INSTRUCTION_OPCODE(call) {
	_vm->callFunction(ctxt._inst->_immediate, nullptr);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (g_engineFlags & kEngineWalking) {
		ctxt._ip--;
		ctxt._suspend = true;
	}
}


DECLARE_INSTRUCTION_OPCODE(start) {
	ctxt._inst->_a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_vm->_activeZone = ctxt._inst->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	InstructionPtr inst = ctxt._inst;

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();

	_vm->scheduleWalk(x, y, false);
}

DECLARE_INSTRUCTION_OPCODE(endscript) {
	if ((ctxt._anim->_flags & kFlagsLooping) == 0) {
		ctxt._anim->_flags &= ~kFlagsActing;
		_vm->_cmdExec->run(ctxt._anim->_commands, ctxt._anim);
		ctxt._program->_status = kProgramDone;
	}

	ctxt._ip = 0;
	ctxt._suspend = true;
}




DECLARE_COMMAND_OPCODE(invalid) {
	error("Can't execute invalid command '%i'", ctxt._cmd->_id);
}

DECLARE_COMMAND_OPCODE(set) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags |= ctxt._cmd->_flags;
	} else {
		_vm->setLocationFlags(ctxt._cmd->_flags);
	}
}


DECLARE_COMMAND_OPCODE(clear) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags &= ~ctxt._cmd->_flags;
	} else {
		_vm->clearLocationFlags(ctxt._cmd->_flags);
	}
}


DECLARE_COMMAND_OPCODE(start) {
	ctxt._cmd->_zone->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	if (ACTIONTYPE(ctxt._cmd->_zone) == kZoneSpeak) {
		_vm->enterDialogueMode(ctxt._cmd->_zone);
	} else {
		_vm->_activeZone = ctxt._cmd->_zone;
	}
}


DECLARE_COMMAND_OPCODE(get) {
	ctxt._cmd->_zone->_flags &= ~kFlagsFixed;
	_vm->runZone(ctxt._cmd->_zone);
}


DECLARE_COMMAND_OPCODE(location) {
	_vm->scheduleLocationSwitch(ctxt._cmd->_string.c_str());
}


DECLARE_COMMAND_OPCODE(open) {
	_vm->updateDoor(ctxt._cmd->_zone, false);
}


DECLARE_COMMAND_OPCODE(close) {
	_vm->updateDoor(ctxt._cmd->_zone, true);
}

DECLARE_COMMAND_OPCODE(on) {
	_vm->showZone(ctxt._cmd->_zone, true);
}


DECLARE_COMMAND_OPCODE(off) {
	_vm->showZone(ctxt._cmd->_zone, false);
}


DECLARE_COMMAND_OPCODE(call) {
	_vm->callFunction(ctxt._cmd->_callable, &ctxt._z);
}


DECLARE_COMMAND_OPCODE(toggle) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags ^= ctxt._cmd->_flags;
	} else {
		_vm->toggleLocationFlags(ctxt._cmd->_flags);
	}
}


DECLARE_COMMAND_OPCODE(drop){
	_vm->dropItem( ctxt._cmd->_object );
}


DECLARE_COMMAND_OPCODE(quit) {
	_vm->quitGame();
}


DECLARE_COMMAND_OPCODE(move) {
	_vm->scheduleWalk(ctxt._cmd->_move.x, ctxt._cmd->_move.y, false);
}


DECLARE_COMMAND_OPCODE(stop) {
	ctxt._cmd->_zone->_flags &= ~kFlagsActing;
}

CommandExec_ns::CommandExec_ns(Parallaction_ns* vm) : CommandExec(vm), _vm(vm) {
	CommandOpcodeSet *table = nullptr;

	SetOpcodeTable(_opcodes);
	COMMAND_OPCODE(invalid);
	COMMAND_OPCODE(set);
	COMMAND_OPCODE(clear);
	COMMAND_OPCODE(start);
	COMMAND_OPCODE(speak);
	COMMAND_OPCODE(get);
	COMMAND_OPCODE(location);
	COMMAND_OPCODE(open);
	COMMAND_OPCODE(close);
	COMMAND_OPCODE(on);
	COMMAND_OPCODE(off);
	COMMAND_OPCODE(call);
	COMMAND_OPCODE(toggle);
	COMMAND_OPCODE(drop);
	COMMAND_OPCODE(quit);
	COMMAND_OPCODE(move);
	COMMAND_OPCODE(stop);
}

ProgramExec_ns::ProgramExec_ns(Parallaction_ns *vm) : _vm(vm) {
	_currentCredit = 0;

	_instructionNames = _instructionNamesRes_ns;

	ProgramOpcodeSet *table = nullptr;

	SetOpcodeTable(_opcodes);
	INSTRUCTION_OPCODE(invalid);
	INSTRUCTION_OPCODE(on);
	INSTRUCTION_OPCODE(off);
	INSTRUCTION_OPCODE(set);		// x
	INSTRUCTION_OPCODE(set);		// y
	INSTRUCTION_OPCODE(set);		// z
	INSTRUCTION_OPCODE(set);		// f
	INSTRUCTION_OPCODE(loop);
	INSTRUCTION_OPCODE(endloop);
	INSTRUCTION_OPCODE(show);
	INSTRUCTION_OPCODE(inc);
	INSTRUCTION_OPCODE(inc);		// dec
	INSTRUCTION_OPCODE(set);
	INSTRUCTION_OPCODE(put);
	INSTRUCTION_OPCODE(call);
	INSTRUCTION_OPCODE(wait);
	INSTRUCTION_OPCODE(start);
	INSTRUCTION_OPCODE(sound);
	INSTRUCTION_OPCODE(move);
	INSTRUCTION_OPCODE(endscript);
}

}	// namespace Parallaction
