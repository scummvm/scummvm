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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/castmember.h"
#include "director/cursor.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/util.h"

#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"


namespace Director {

enum MCITokenType {
    MCI_PLAY,
    MCI_OPEN,
    MCI_CLOSE,
    MCI_STATUS,
};

enum MCIDataType {
    MCI_COMMAND_HEAD,
    MCI_END_COMMAND,
    MCI_END_COMMAND_LIST,
    MCI_CONSTANT,
    MCI_END_CONSTANT,
    MCI_RETURN, 

    MCI_FLAG, 
    MCI_INTEGER,
    MCI_STRING,
    MCI_RECT,
    MCI_DWORD_PTR,
};

typedef struct MCITokenData {
    MCIDataType type;
    Common::String string;
    int integer = 0;
} MCITokenData;

typedef struct MCICommand {
    MCITokenType id;
    uint flags = 0;
    Common::String device; /* MCI device name */
    Common::HashMap<Common::String, MCITokenData> parameters;
} MCICommand;

struct CmdTableRow {
    const char *keystr;
    int flag;
    int num;
    MCIDataType data_type;
};

enum MCIError {
    MCIERR_NO_ERROR,
    MCIERR_UNRECOGNISED_COMMAND,
    MCIERR_STRING_PARSE,
};

static CmdTableRow table[] = {
  {"open"            ,MCI_OPEN     ,0          ,MCI_COMMAND_HEAD   },
  {""                ,MCI_INTEGER  ,0          ,MCI_RETURN         },
  {"notify"          ,0x00000001L  ,-1         ,MCI_FLAG           },
  {"wait"            ,0x00000002L  ,-1         ,MCI_FLAG           },
  {"type"            ,0x00002000L  ,-1         ,MCI_STRING         },
  {"element"         ,0x00000200L  ,-1         ,MCI_STRING         },
  {"alias"           ,0x00000400L  ,-1         ,MCI_STRING         },
  {"shareable"       ,0x00000100L  ,-1         ,MCI_FLAG           },
  {""                ,0x00000000L  ,-1         ,MCI_END_COMMAND    },

  {"close"           ,MCI_CLOSE    ,0          ,MCI_COMMAND_HEAD   },
  {"notify"          ,0x00000001L  ,-1         ,MCI_FLAG           },
  {"wait"            ,0x00000002L  ,-1         ,MCI_FLAG           },
  {""                ,0x00000000L  ,-1         ,MCI_END_COMMAND    },

  {"play"            ,MCI_PLAY     ,0          ,MCI_COMMAND_HEAD   },
  {"notify"          ,0x00000001L  ,-1         ,MCI_FLAG           },
  {"wait"            ,0x00000002L  ,-1         ,MCI_FLAG           },
  {"from"            ,0x00000004L  ,-1         ,MCI_INTEGER        },
  {"to"              ,0x00000008L  ,-1         ,MCI_INTEGER        },
  {""                ,0x00000000L  ,-1         ,MCI_END_COMMAND    },

  {"status"          ,MCI_STATUS   ,0          ,MCI_COMMAND_HEAD }  ,
  {""                ,MCI_DWORD_PTR,0          ,MCI_RETURN }        ,
  {"notify"          ,0x00000001L  ,-1         ,MCI_FLAG }          ,
  {"wait"            ,0x00000002L  ,-1         ,MCI_FLAG }          ,
  {""                ,0x00000100L  ,-1         ,MCI_CONSTANT }      ,
  {"position"        ,0x00000002L  ,-1         ,MCI_INTEGER }       ,
  {"length"          ,0x00000001L  ,-1         ,MCI_INTEGER }       ,
  {"number of tracks",-1           ,0x00000003L,MCI_INTEGER }       ,
  {"ready"           ,0x00000007L  ,-1         ,MCI_INTEGER }       ,
  {"mode"            ,0x00000004L  ,-1         ,MCI_INTEGER }       ,
  {"time format"     ,-1           ,0x00000006L,MCI_INTEGER }       ,
  {"current track"   ,-1           ,0x00000008L,MCI_INTEGER }       ,
  {""                ,0x00000000L  ,-1         ,MCI_END_CONSTANT }  ,
  { "track"          ,0x00000010L  ,-1         ,MCI_INTEGER }       ,
  { "start"          ,0x00000200L  ,-1         ,MCI_FLAG }          ,
  { ""               ,0x00000000L  ,-1         ,MCI_END_COMMAND }   ,
};

static MCIError get_string(const Common::String &name, uint &idx, Common::String &str) {
    uint i_end; 
    if (name[int(idx)] == '"' || name[int(idx)] == '\'') { /* Quoted string */
        char quote = name[int(idx)];
        i_end = name.findFirstOf(quote, idx + 1);
        if (i_end == Common::String::npos) {
            return MCIERR_STRING_PARSE; /* Unterminated string */
        }
        str = name.substr(idx + 1, i_end - idx - 1);
        idx = int(i_end + 2);
    }
    else { /* No quotes, so just find the next space */
        i_end = name.findFirstOf(' ', idx);
        if (i_end == Common::String::npos) {
            i_end = name.size();
        }

        str = name.substr(idx, i_end - idx);
        idx = int(i_end + 1);
    }
    debugC(5, kDebugLingoExec, "get_string: \"%s\"", str.c_str());
    return MCIERR_NO_ERROR;
}

        

static void create_token_list(Common::Array<Common::String> &token_list, const Common::String &name)
{
    uint idx = 0; 

    while (idx < name.size()) {
        Common::String str;
        MCIError err = get_string(name, idx, str);
        if (err != MCIERR_NO_ERROR) {
            break;
        }
        token_list.push_back(str);
    }
}

MCIError parse_mci_command(const Common::String &name, MCICommand &parsed_cmd) 
{
    Common::Array<Common::String> token_list;
    create_token_list(token_list, name);

    uint i_token = 0;
    int i_table = 0;
    int tableStart = -1, tableEnd = -1;

    Common::String &verb = token_list[0];

    /* Find the table section corresponding to the command's verb. */
    for (auto& cmd : table) {
        if (cmd.data_type == MCI_COMMAND_HEAD && cmd.keystr == verb) {
            tableStart = i_table;
        }
        else if (tableStart >= 0 && cmd.data_type == MCI_END_COMMAND) {
            tableEnd = i_table;
        }
        i_table++;
    }

    assert(tableStart >= 0 && tableEnd > 0);

    auto cmd = table[tableStart];
    parsed_cmd.id = (MCITokenType)cmd.flag;
    parsed_cmd.flags = cmd.num;

    /* The MCI device will ALWAYS be the second token. */
    parsed_cmd.device = token_list[1];

    /* Parse the rest of the arguments */
    i_token = 2;

    while (i_token < token_list.size()) {

        bool found = false;
        bool inConst = false;
        int flag, cflag;
        CmdTableRow *cmdtable, *c_cmdtable;
        auto& token = token_list[i_token];

        for (i_table = tableStart; i_table < tableEnd; i_table++) {

            MCIDataType cmd_type = table[i_table].data_type;
            flag = table[i_table].flag;
            cmdtable = &table[i_table];

            switch (cmd_type) {
                case MCI_CONSTANT:
                    c_cmdtable = cmdtable; inConst = true; cflag = flag;
                    break;
                case MCI_END_CONSTANT:
                    c_cmdtable = nullptr; inConst = false; cflag = 0;
                    break;
                default: break;
            }

            if (token != table[i_table].keystr) continue;

            found = true;

            switch (cmd_type) {
                case MCI_COMMAND_HEAD:
                case MCI_RETURN:
                case MCI_END_COMMAND:
                case MCI_END_COMMAND_LIST:
                case MCI_CONSTANT:
                case MCI_END_CONSTANT:
                    break;
                case MCI_FLAG:
                    parsed_cmd.flags |= flag;
                    i_token++;
                    break;
                case MCI_INTEGER:
                    {
                    if (inConst) { /* Handle the case where we've hit a MCI_INTEGER which is inside a MCI_CONSTANT block. */
                        MCITokenData token_data;
                        if (parsed_cmd.parameters.tryGetVal(c_cmdtable->keystr, token_data)) {
                            token_data.integer |= flag;
                        }
                        else {
                            token_data.type = MCI_CONSTANT;
                            token_data.integer |= flag;
                            parsed_cmd.parameters[c_cmdtable->keystr] = token_data;
                        }
                        parsed_cmd.flags |= cflag;
                        inConst = false;
                    }
                    else {
                        parsed_cmd.flags |= flag;
                        MCITokenData token_data; token_data.type = MCI_INTEGER; token_data.integer = atoi(token_list[++i_token].c_str());
                        parsed_cmd.parameters[token] = token_data;
                    }
                    i_token++;
                    break;
                    }
                case MCI_STRING:
                    {
                    parsed_cmd.flags |= flag;
                    MCITokenData token_data; token_data.type = MCI_STRING; token_data.string = token_list[++i_token];
                    parsed_cmd.parameters[token] = token_data;
                    i_token++;
                    break;
                    }
                default: 
                    warning("Unhandled command type.");
                    return MCIERR_UNRECOGNISED_COMMAND;
            }
        }

        if (!found) {
            warning("Parameter %s not found in table", token_list[i_token].c_str());
            return MCIERR_UNRECOGNISED_COMMAND;
        }
    }

    return MCIERR_NO_ERROR;
}

void Lingo::func_mci(const Common::String &name) {

    MCICommand parsed_cmd;
    parse_mci_command(name, parsed_cmd);

	switch (parsed_cmd.id) {
	case MCI_OPEN:
		{

			Common::File *file = new Common::File();

			if (!file->open(parsed_cmd.device)) {
				warning("Failed to open %s", parsed_cmd.device.c_str());
				delete file;
				return;
			}

            parsed_cmd.parameters["type"].string.toLowercase(); /* In the case the open command type has something like WaveAudio instead of waveaudio */

			if (parsed_cmd.parameters["type"].string == "waveaudio") {
				Audio::AudioStream *sound = Audio::makeWAVStream(file, DisposeAfterUse::YES);
                if (parsed_cmd.parameters.contains("alias"))
                {
                    _audioAliases[parsed_cmd.parameters["alias"].string] = sound;
                }
			} else {
				warning("Unhandled audio type %s", parsed_cmd.parameters["type"].string.c_str());
			}
		}
		break;
	case MCI_PLAY:
		{
			warning("MCI play file: %s, from: %d, to: %d", parsed_cmd.device.c_str(), parsed_cmd.parameters["from"].integer, parsed_cmd.parameters["to"].integer);

			if (!_audioAliases.contains(parsed_cmd.device)) {
				warning("Unknown alias %s", parsed_cmd.device.c_str());
				return;
			}

			uint32 from = parsed_cmd.parameters["from"].integer;
			uint32 to = parsed_cmd.parameters["to"].integer;

			_vm->getCurrentWindow()->getSoundManager()->playMCI(*_audioAliases[parsed_cmd.device], from, to);
		}
		break;
	default:
		warning("Unhandled MCI command: %d", parsed_cmd.id); /* TODO: Convert MCITokenType into string */
	}
}

void Lingo::func_mciwait(const Common::String &name) {
	warning("STUB: MCI wait file: %s", name.c_str());
}

void Lingo::func_goto(Datum &frame, Datum &movie, bool calledfromgo) {
	_vm->_playbackPaused = false;

	if (!_vm->getCurrentMovie())
		return;

	if (movie.type == VOID && frame.type == VOID)
		return;

	Window *stage = _vm->getCurrentWindow();
	Score *score = _vm->getCurrentMovie()->getScore();

	_vm->_skipFrameAdvance = true;

	// If there isn't already frozen Lingo (e.g. from a previous func_goto we haven't yet unfrozen),
	// freeze this script context. We'll return to it after entering the next frame.
	g_lingo->_freezeState = true;

	if (movie.type != VOID) {
		Common::String movieFilenameRaw = movie.asString();

		if (!stage->setNextMovie(movieFilenameRaw))
			return;

		// If we reached here from b_go, and the movie is getting swapped out,
		// reset all of the custom event handlers.
		if (calledfromgo)
			g_lingo->resetLingoGo();

		if (g_lingo->_updateMovieEnabled) {
			// Save the movie when branching to another movie.
			LB::b_saveMovie(0);
		}

		score->_playState = kPlayStopped;

		stage->_nextMovie.frameS.clear();
		stage->_nextMovie.frameI = -1;

		if (frame.type == STRING) {
			debugC(3, kDebugLingoExec, "Lingo::func_goto(): going to movie \"%s\", frame \"%s\"", movieFilenameRaw.c_str(), frame.u.s->c_str());
			stage->_nextMovie.frameS = *frame.u.s;
		} else if (frame.type != VOID) {
			debugC(3, kDebugLingoExec, "Lingo::func_goto(): going to movie \"%s\", frame %d", movieFilenameRaw.c_str(), frame.asInt());
			stage->_nextMovie.frameI = frame.asInt();
		} else {
			debugC(3, kDebugLingoExec, "Lingo::func_goto(): going to start of movie \"%s\"", movieFilenameRaw.c_str());
		}

		// Set cursor to watch.
		score->_defaultCursor.readFromResource(4);
		score->renderCursor(stage->getMousePos());

		return;
	}

	if (frame.type == STRING) {
		debugC(3, kDebugLingoExec, "Lingo::func_goto(): going to frame \"%s\"", frame.u.s->c_str());
		score->setStartToLabel(*frame.u.s);
	} else {
		debugC(3, kDebugLingoExec, "Lingo::func_goto(): going to frame %d", frame.asInt());
		score->setCurrentFrame(frame.asInt());
	}
}

void Lingo::func_gotoloop() {
	if (!_vm->getCurrentMovie())
		return;
	Score *score = _vm->getCurrentMovie()->getScore();
	debugC(3, kDebugLingoExec, "Lingo::func_gotoloop(): looping frame %d", score->getCurrentFrame());

	score->gotoLoop();

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_gotonext() {
	if (!_vm->getCurrentMovie())
		return;

	Score *score = _vm->getCurrentMovie()->getScore();
	score->gotoNext();
	debugC(3, kDebugLingoExec, "Lingo::func_gotonext(): going to next frame %d", score->getNextFrame());

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_gotoprevious() {
	if (!_vm->getCurrentMovie())
		return;

	Score *score = _vm->getCurrentMovie()->getScore();
	score->gotoPrevious();
	debugC(3, kDebugLingoExec, "Lingo::func_gotoprevious(): going to previous frame %d", score->getNextFrame());

	_vm->_skipFrameAdvance = true;
}

void Lingo::func_play(Datum &frame, Datum &movie) {
	MovieReference ref;
	Window *stage = _vm->getCurrentWindow();


	// play #done
	if (frame.type == SYMBOL) {
		if (!frame.u.s->equals("done")) {
			warning("Lingo::func_play: unknown symbol: #%s", frame.u.s->c_str());
			return;
		}
		if (stage->_movieStack.empty()) {	// No op if no nested movies
			return;
		}
		ref = stage->_movieStack.back();

		stage->_movieStack.pop_back();

		Datum m, f;

		if (ref.movie.empty()) {
			m.type = VOID;
		} else {
			m.type = STRING;
			m.u.s = new Common::String(ref.movie);
		}

		f.type = INT;
		f.u.i = ref.frameI;

		func_goto(f, m);

		return;
	}

	if (!_vm->getCurrentMovie()) {
		warning("Lingo::func_play(): no movie");
		return;
	}

	if (movie.type != VOID) {
		ref.movie = _vm->getCurrentMovie()->_movieArchive->getPathName();
	}
	ref.frameI = _vm->getCurrentMovie()->getScore()->getCurrentFrame();

	// if we are issuing play command from script channel script. then play done should return to next frame
	if (g_lingo->_currentChannelId == 0)
		ref.frameI++;

	stage->_movieStack.push_back(ref);

	func_goto(frame, movie);
}

void Lingo::func_cursor(Datum cursorDatum) {
	Score *score = _vm->getCurrentMovie()->getScore();
	if (cursorDatum.type == ARRAY){
		score->_defaultCursor.readFromCast(cursorDatum);
	} else {
		score->_defaultCursor.readFromResource(cursorDatum);
	}
	score->_cursorDirty = true;
}

void Lingo::func_beep(int repeats) {
	for (int r = 1; r <= repeats; r++) {
		_vm->getCurrentWindow()->getSoundManager()->systemBeep();
		if (r < repeats)
			g_director->delayMillis(400);
	}
}

int Lingo::func_marker(int m) 	{
	if (!_vm->getCurrentMovie())
		return 0;

	int labelNumber = _vm->getCurrentMovie()->getScore()->getCurrentLabelNumber();
	if (m != 0) {
		if (m < 0) {
			for (int marker = 0; marker > m; marker--)
				labelNumber = _vm->getCurrentMovie()->getScore()->getPreviousLabelNumber(labelNumber);
		} else {
			for (int marker = 0; marker < m; marker++)
				labelNumber = _vm->getCurrentMovie()->getScore()->getNextLabelNumber(labelNumber);
		}
	}

	return labelNumber;
}

uint16 Lingo::func_label(Datum &label) {
	Score *score = _vm->getCurrentMovie()->getScore();

	if (!score->_labels)
		return 0;

	if (label.type == STRING)
		return score->getLabel(*label.u.s);

	int num = CLIP<int>(label.asInt() - 1, 0, score->_labels->size() - 1);

	uint16 res = score->getNextLabelNumber(0);

	while (--num > 0)
		res = score->getNextLabelNumber(res);

	return res;
}

} // End of namespace Director
