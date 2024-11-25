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


/*
 *  The MCI documentation is: https://learn.microsoft.com/en-us/windows/win32/multimedia/mci
 *  Based on sources from Wine: https://github.com/wine-mirror/wine/blob/master/dlls/winmm/mci.c
 *  Table structure and algorithms also described in: https://patentimages.storage.googleapis.com/ad/06/7a/48766ca9df6fbc/US6397263.pdf
 */

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "common/file.h"

#include "director/director.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/window.h"

namespace Director {

enum MCITokenType {
    MCI_PLAY,
    MCI_OPEN,
    MCI_CLOSE,
    MCI_STATUS,
    MCI_RECORD,
    MCI_SEEK,
    MCI_STOP,
    MCI_PAUSE,
    MCI_GETDEVCAPS,
    MCI_SYSINFO,
    MCI_BREAK,
    MCI_SOUND,
    MCI_SAVE,
    MCI_LOAD,
    MCI_RESUME,
    MCI_SET,
    MCI_INFO,
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

enum MCIError {
    MCIERR_NO_ERROR,
    MCIERR_UNRECOGNISED_COMMAND,
    MCIERR_STRING_PARSE,
};

struct CmdTableRow {
    const char *keystr;
    int flag;
    int num;
    MCIDataType data_type;
};

static const CmdTableRow table[] = {
  {"open"            ,MCI_OPEN      ,0          ,MCI_COMMAND_HEAD },
  {""                ,MCI_INTEGER   ,0          ,MCI_RETURN }      ,
  {"notify"          ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  {"wait"            ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  {"type"            ,0x00002000L   ,-1         ,MCI_STRING }      ,
  {"element"         ,0x00000200L   ,-1         ,MCI_STRING }      ,
  {"alias"           ,0x00000400L   ,-1         ,MCI_STRING }      ,
  {"shareable"       ,0x00000100L   ,-1         ,MCI_FLAG }        ,
  {""                ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  {"close"           ,MCI_CLOSE     ,0          ,MCI_COMMAND_HEAD },
  {"notify"          ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  {"wait"            ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  {""                ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  {"play"            ,MCI_PLAY      ,0          ,MCI_COMMAND_HEAD },
  {"notify"          ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  {"wait"            ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  {"from"            ,0x00000004L   ,-1         ,MCI_INTEGER }     ,
  {"to"              ,0x00000008L   ,-1         ,MCI_INTEGER }     ,
  {""                ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  {"status"          ,MCI_STATUS    ,0          ,MCI_COMMAND_HEAD },
  {""                ,MCI_DWORD_PTR ,0          ,MCI_RETURN }      ,
  {"notify"          ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  {"wait"            ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  {""                ,0x00000100L   ,-1         ,MCI_CONSTANT }    ,
  {"position"        ,0x00000002L   ,-1         ,MCI_INTEGER }     ,
  {"length"          ,0x00000001L   ,-1         ,MCI_INTEGER }     ,
  {"number of tracks",-1            ,0x00000003L,MCI_INTEGER }     ,
  {"ready"           ,0x00000007L   ,-1         ,MCI_INTEGER }     ,
  {"mode"            ,0x00000004L   ,-1         ,MCI_INTEGER }     ,
  {"time format"     ,-1            ,0x00000006L,MCI_INTEGER }     ,
  {"current track"   ,-1            ,0x00000008L,MCI_INTEGER }     ,
  {""                ,0x00000000L   ,-1         ,MCI_END_CONSTANT },
  { "track"          ,0x00000010L   ,-1         ,MCI_INTEGER }     ,
  { "start"          ,0x00000200L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "record"         ,MCI_RECORD    ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { "from"           ,0x00000004L   ,-1         ,MCI_INTEGER }     ,
  { "to"             ,0x00000008L   ,-1         ,MCI_INTEGER }     ,
  { "insert"         ,0x00000100L   ,-1         ,MCI_FLAG }        ,
  { "overwrite"      ,0x00000200L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "seek"           ,MCI_SEEK      ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { "to start"       ,0x00000100L   ,-1         ,MCI_FLAG }        ,
  { "to end"         ,0x00000200L   ,-1         ,MCI_FLAG }        ,
  { "to"             ,0x00000008L   ,-1         ,MCI_INTEGER }     ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "stop"           ,MCI_STOP      ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "pause"          ,MCI_PAUSE     ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "capability"     ,MCI_GETDEVCAPS,0          ,MCI_COMMAND_HEAD },
  { ""               ,MCI_INTEGER   ,0          ,MCI_RETURN }      ,
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000100L   ,-1         ,MCI_CONSTANT }    ,
  { "can record"     ,0x00000001L   ,-1         ,MCI_INTEGER }     ,
  { "has audio"      ,0x00000002L   ,-1         ,MCI_INTEGER }     ,
  { "has video"      ,0x00000003L   ,-1         ,MCI_INTEGER }     ,
  { "uses files"     ,0x00000005L   ,-1         ,MCI_INTEGER }     ,
  { "compound device",0x00000006L   ,-1         ,MCI_INTEGER }     ,
  { "device type"    ,0x00000004L   ,-1         ,MCI_INTEGER }     ,
  { "can eject"      ,0x00000007L   ,-1         ,MCI_INTEGER }     ,
  { "can play"       ,0x00000008L   ,-1         ,MCI_INTEGER }     ,
  { "can save"       ,0x00000009L   ,-1         ,MCI_INTEGER }     ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_CONSTANT },
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "info"           ,MCI_INFO      ,0          ,MCI_COMMAND_HEAD },
  { ""               ,MCI_STRING    ,0          ,MCI_RETURN }      ,
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { "product"        ,0x00000100L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "set"            ,MCI_SET       ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { "time format"    ,0x00000400L   ,-1         ,MCI_CONSTANT }    ,
  { "milliseconds"   ,0x00000000L   ,-1         ,MCI_INTEGER }     ,
  { "ms"             ,0x00000000L   ,-1         ,MCI_INTEGER }     ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_CONSTANT },
  { "door open"      ,0x00000100L   ,-1         ,MCI_FLAG }        ,
  { "door closed"    ,0x00000200L   ,-1         ,MCI_FLAG }        ,
  { "audio"          ,0x00000800L   ,-1         ,MCI_CONSTANT }    ,
  { "all"            ,0x00000000L   ,-1         ,MCI_INTEGER }     ,
  { "left"           ,0x00000001L   ,-1         ,MCI_INTEGER }     ,
  { "right"          ,0x00000002L   ,-1         ,MCI_INTEGER }     ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_CONSTANT },
  { "video"          ,0x00001000L   ,-1         ,MCI_FLAG }        ,
  { "on"             ,0x00002000L   ,-1         ,MCI_FLAG }        ,
  { "off"            ,0x00004000L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "sysinfo"        ,MCI_SYSINFO   ,0          ,MCI_COMMAND_HEAD },
  { ""               ,MCI_STRING    ,0          ,MCI_RETURN }      ,
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { "quantity"       ,0x00000100L   ,-1         ,MCI_FLAG }        ,
  { "open"           ,0x00000200L   ,-1         ,MCI_FLAG }        ,
  { "installname"    ,0x00000800L   ,-1         ,MCI_FLAG }        ,
  { "name"           ,0x00000400L   ,-1         ,MCI_INTEGER }     ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "break"          ,MCI_BREAK     ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { "on"             ,0x00000100L   ,-1         ,MCI_INTEGER }     ,
  { "off"            ,0x00000400L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "sound"          ,MCI_SOUND     ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "save"           ,MCI_SAVE      ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000100L   ,-1         ,MCI_STRING }      ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "load"           ,MCI_LOAD      ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000100L   ,-1         ,MCI_STRING }      ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,

  { "resume"         ,MCI_RESUME    ,0          ,MCI_COMMAND_HEAD },
  { "notify"         ,0x00000001L   ,-1         ,MCI_FLAG }        ,
  { "wait"           ,0x00000002L   ,-1         ,MCI_FLAG }        ,
  { ""               ,0x00000000L   ,-1         ,MCI_END_COMMAND } ,
  };

static MCIError getString(const Common::String &name, uint &idx, Common::String &str) {
    uint i_end;

    if (name[idx] == '"' || name[idx] == '\'') { /* Quoted string */
        char quote = name[idx];
        i_end = name.findFirstOf(quote, idx + 1);
        if (i_end == Common::String::npos) {
            return MCIERR_STRING_PARSE; /* Unterminated string */
        }
        str = name.substr(idx + 1, i_end - idx - 1);
        idx = i_end + 2;
    } else { /* No quotes, so just find the next space */
        i_end = name.findFirstOf(' ', idx);
        if (i_end == Common::String::npos) {
            i_end = name.size();
        }

        str = name.substr(idx, i_end - idx);
        idx = i_end + 1;
    }

    debugC(5, kDebugLingoExec, "get_string(): Got string \"%s\"", str.c_str());
    return MCIERR_NO_ERROR;
}

static void createTokenList(Common::StringArray &tokenList, const Common::String &name) {
    uint idx = 0;

    while (idx < name.size()) {
        Common::String str;
        MCIError err = getString(name, idx, str);

        if (err != MCIERR_NO_ERROR) {
            break;
        }
        tokenList.push_back(str);
    }
}

static MCIError parseMCICommand(const Common::String &name, MCICommand &parsedCmd) {
    Common::StringArray token_list;
    createTokenList(token_list, name);

    uint i_token = 0;
    int i_table = 0;
    int tableStart = -1, tableEnd = -1;

    Common::String &verb = token_list[0];

    /* Find the table section corresponding to the command's verb. */
    for (auto& cmd : table) {
        if ((tableStart < 0) && (cmd.data_type == MCI_COMMAND_HEAD) && (cmd.keystr == verb)) {
            tableStart = i_table;
        } else if ((tableStart >= 0) && (cmd.data_type == MCI_END_COMMAND)) {
            tableEnd = i_table;
            break;
        }
        i_table++;
    }

    debugC(5, kDebugLingoExec, "parseMCICommand(): tableStart: %d, tableEnd: %d", tableStart, tableEnd);
    if (tableStart == -1 || tableEnd == -1) {
        warning("parseMCICommand(): Verb %s not found in table", verb.c_str());
		return MCIERR_UNRECOGNISED_COMMAND;
	}

    auto cmd = table[tableStart];
    parsedCmd.id = (MCITokenType)cmd.flag;
    parsedCmd.flags = cmd.num;

    /* The MCI device will ALWAYS be the second token. */
    parsedCmd.device = token_list[1];

    /* Parse the rest of the arguments */
    i_token = 2;

    while (i_token < token_list.size()) {
        bool found = false;
        bool inConst = false;
        int flag, cflag = 0;
        const CmdTableRow *cmdtable, *c_cmdtable = nullptr;
        auto& token = token_list[i_token];

        for (i_table = tableStart; i_table < tableEnd; i_table++) {
            MCIDataType cmd_type = table[i_table].data_type;
            flag = table[i_table].flag;
            cmdtable = &table[i_table];

            switch (cmd_type) {
            case MCI_CONSTANT:
                c_cmdtable = cmdtable;
                inConst = true;
                cflag = flag;
                break;

            case MCI_END_CONSTANT:
                c_cmdtable = nullptr;
                inConst = false;
                cflag = 0;
                break;

            default:
                break;
            }

            if (token != table[i_table].keystr)
                continue;

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
                parsedCmd.flags |= flag;
                i_token++;
                break;

            case MCI_INTEGER: {
                if (inConst) { /* Handle the case where we've hit a MCI_INTEGER which is inside a MCI_CONSTANT block. */
                    MCITokenData token_data;
                    if (parsedCmd.parameters.tryGetVal(c_cmdtable->keystr, token_data)) {
                        token_data.integer |= flag;
                    } else {
                        token_data.type = MCI_CONSTANT;
                        token_data.integer |= flag;
                        parsedCmd.parameters[c_cmdtable->keystr] = token_data;
                    }
                    parsedCmd.flags |= cflag;
                    inConst = false;
                } else {
                    parsedCmd.flags |= flag;

                    MCITokenData token_data;
                    token_data.type = MCI_INTEGER;
                    token_data.integer = atoi(token_list[++i_token].c_str());
                    parsedCmd.parameters[token] = token_data;
                }
                i_token++;
                break;
                }

            case MCI_STRING: {
                parsedCmd.flags |= flag;

                MCITokenData token_data;
                token_data.type = MCI_STRING;
                token_data.string = token_list[++i_token];
                parsedCmd.parameters[token] = token_data;
                i_token++;
                break;
                }

            default:
                warning("parseMCICommand(): Unhandled command type %d", cmd_type);
                return MCIERR_UNRECOGNISED_COMMAND;
            }
        }

        if (!found) {
            warning("parseMCICommand(): Parameter %s not found in table", token_list[i_token].c_str());
            return MCIERR_UNRECOGNISED_COMMAND;
        }
    }

    return MCIERR_NO_ERROR;
}

void Lingo::func_mci(const Common::String &name) {

    MCICommand parsedCmd;
    parseMCICommand(name, parsedCmd);

    switch (parsedCmd.id) {
    case MCI_OPEN: {
        Common::File *file = new Common::File();

        if (!file->open(Common::Path(parsedCmd.device, g_director->_dirSeparator))) {
            warning("func_mci(): Failed to open %s", parsedCmd.device.c_str());
            delete file;
            return;
        }

        parsedCmd.parameters["type"].string.toLowercase(); /* In the case the open command type has something like WaveAudio instead of waveaudio */

        if (parsedCmd.parameters["type"].string == "waveaudio") {
            Audio::AudioStream *sound = Audio::makeWAVStream(file, DisposeAfterUse::YES);
            if (parsedCmd.parameters.contains("alias")) {
                _audioAliases[parsedCmd.parameters["alias"].string] = sound;
            } else {
                delete sound;
            }
        } else {
            warning("func_mci(): Unhandled audio type %s", parsedCmd.parameters["type"].string.c_str());
            delete file;
        }
        }
        break;

    case MCI_PLAY: {
        warning("func_mci(): MCI play file: %s, from: %d, to: %d", parsedCmd.device.c_str(), parsedCmd.parameters["from"].integer, parsedCmd.parameters["to"].integer);

        if (!_audioAliases.contains(parsedCmd.device)) {
            warning("func_mci(): Unknown alias %s", parsedCmd.device.c_str());
            return;
        }

        uint32 from = parsedCmd.parameters["from"].integer;
        uint32 to = parsedCmd.parameters.contains("to") ? parsedCmd.parameters["to"].integer : -1;

        _vm->getCurrentWindow()->getSoundManager()->playMCI(*_audioAliases[parsedCmd.device], from, to);
        }
        break;

    default:
        warning("func_mci: Unhandled MCI command: %d", parsedCmd.id); /* TODO: Convert MCITokenType into string */
    }
}

void Lingo::func_mciwait(const Common::String &name) {
    warning("STUB: MCI wait file: %s", name.c_str());
}

} // End of namespace Director
