#include "prince/script.h"
#include "prince/prince.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/stream.h"

namespace Prince {

static const uint16 NUM_OPCODES = 144;

Script::Script(PrinceEngine *vm) : 
	_code(NULL), _stacktop(0), _vm(vm), _random("GroovieScripts"), _opcodeNF(false) {
}

Script::~Script() {
	delete[] _code;
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
	_codeSize = stream.size();
	_code = new byte[_codeSize];

	if (!_code)
		return false;

	stream.read(_code, _codeSize);
	// Initialize the script
	_currentInstruction = READ_LE_UINT32(_code + 4);

	return true;
}

void Script::debugScript(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

    Common::String str = Common::String::format("@0x%04X: ", _lastInstruction);
	str += Common::String::format("op %02d: ", _lastOpcode);
    debug("%s %s", str.c_str(), buf);
}

void Script::step() {
    //while (!_opcodeNF)
    {
        _lastInstruction = _currentInstruction;
        // Prepare the base debug string
        Common::String dstr = Common::String::format("@0x%04X: ", _currentInstruction);

        // Get the current opcode
        _lastOpcode = readScript16bits();

        dstr += Common::String::format("op %02d: ", _lastOpcode);

        if (_lastOpcode > NUM_OPCODES)
            error("Trying to execute unknown opcode %s", dstr.c_str());


        debug("%s", dstr.c_str());

        // Execute the current opcode
        OpcodeFunc op = _opcodes[_lastOpcode];
        (this->*op)();
    }
}

uint8 Script::getCodeByte(uint32 address) {
	if (address >= _codeSize)
		error("Trying to read a script byte at address 0x%04X, while the "
			"script is just 0x%04X bytes long", address, _codeSize);
	return _code[address];
}

uint8 Script::readScript8bits() {
	uint8 data = getCodeByte(_currentInstruction);
	_currentInstruction++;
	return data;
}

uint16 Script::readScript16bits() {
	uint8 lower = readScript8bits();
	uint8 upper = readScript8bits();
	return lower | (upper << 8);
}

uint32 Script::readScript32bits() {
	uint16 lower = readScript16bits();
	uint16 upper = readScript16bits();
	return lower | (upper << 16);
}

void Script::O_WAITFOREVER() {
    debugScript("O_WAITFOREVER");
    _currentInstruction -= 2;
}

void Script::O_BLACKPALETTE() {
    debugScript("O_BLACKPALETTE");
}

void Script::O_SETUPPALETTE() {
    debugScript("O_SETUPPALETTE");
}

void Script::O_INITROOM() {
    uint16 roomId = readScript16bits();
    debugScript("O_INITROOM %d", roomId);
    _vm->loadLocation(roomId);
}

void Script::O_SETSAMPLE() {
    uint16 sampleId = readScript16bits();
    int32 sampleNameOffset = readScript32bits();
    const char * sampleName = (const char *)_code + _currentInstruction + sampleNameOffset - 4;
    debugScript("O_SETSAMPLE %d %s", sampleId, sampleName);
}

void Script::O_FREESAMPLE() {
    uint16 sample = readScript16bits();
    debugScript("O_FREESAMPLE %d", sample);
}

void Script::O_PLAYSAMPLE() {
    uint16 sampleId = readScript16bits();
    uint16 loopType = readScript16bits();
    debugScript("O_PLAYSAMPLE sampleId %d loopType %d", sampleId, loopType);
}

void Script::O_PUTOBJECT() {
    uint16 roomId = readScript16bits();
    uint16 slot = readScript16bits();
    uint16 objectId = readScript16bits();
    debugScript("O_PUTOBJECT roomId %d, slot %d, objectId %d", roomId, slot, objectId);
}

void Script::O_REMOBJECT() {
    uint16 roomId = readScript16bits();
    uint16 objectId = readScript16bits();

    debugScript("O_REMOBJECT roomId %d objectId %d", roomId, objectId);
}

void Script::O_SHOWANIM() {
    uint16 slot = readScript16bits();
    uint16 animId = readScript16bits();

    debugScript("O_SHOWANIM slot %d, animId %d", slot, animId);
}

void Script::O_CHECKANIMEND() {
    uint16 slot = readScript16bits();
    uint16 frameId = readScript16bits();

    debugScript("O_CHECKANIMEND slot %d, frameId %d", slot, frameId);
}

void Script::O_FREEANIM() {
    uint16 slot = readScript16bits();
    debugScript("O_FREEANIM slot %d", slot);
}

void Script::O_CHECKANIMFRAME() {
    uint16 slot = readScript16bits();
    uint16 frameId = readScript16bits();

    debugScript("O_CHECKANIMFRAME slot %d, frameId %d", slot, frameId);
}

void Script::O_PUTBACKANIM() {
    uint16 roomId = readScript16bits();
    uint16 slot = readScript16bits();
    uint32 animId = readScript32bits();
    debugScript("O_PUTBACKANIM roomId %d, slot %d, animId %d", roomId, slot, animId);
}

void Script::O_REMBACKANIM() {
    uint16 roomId = readScript16bits();
    uint16 slot = readScript16bits();

    debugScript("O_REMBACKANIM roomId %d, slot %d", roomId, slot);
}

void Script::O_CHECKBACKANIMFRAME() {}

void Script::O_FREEALLSAMPLES() {}

void Script::O_SETMUSIC() {}

void Script::O_STOPMUSIC() {}

void Script::O__WAIT() {}

void Script::O_UPDATEOFF() {}

void Script::O_UPDATEON() {}

void Script::O_UPDATE () {}

void Script::O_CLS() {}


void Script::O__CALL() {
    int32 address = readScript32bits();
    _stack[_stacktop] = _currentInstruction;
    _stacktop++;
    _currentInstruction += address - 4;
    debugScript("O__CALL 0x%04X", _currentInstruction);
}
void Script::O_RETURN() {
	// Get the return address
	if (_stacktop > 0) {
		_stacktop--;
		_currentInstruction = _stack[_stacktop];
        debugScript("O_RETURN 0x%04X", _currentInstruction);
	} else {
		error("Return: Stack is empty");
	}
}
void Script::O_GO() {
    int32 opPC = readScript32bits();
    debugScript("O_GO 0x%04X", opPC);
    _currentInstruction += opPC - 4;
}
void Script::O_BACKANIMUPDATEOFF() {}

void Script::O_BACKANIMUPDATEON() {
    uint16 slot = readScript16bits();
    debugScript("O_BACKANIMUPDATEON %d", slot);
}

void Script::O_CHANGECURSOR() {
    uint16 cursorId = readScript16bits();
    debugScript("O_CHANGECURSOR %x", cursorId);
}
void Script::O_CHANGEANIMTYPE() {}
void Script::O__SETFLAG() {
    uint16 flagId = readScript16bits();
    uint16 value = readScript16bits();
    debugScript("O__SETFLAG 0x%04X %d", flagId, value);
    _flags[flagId-0x8000] = value;
}

void Script::O_COMPARE() {
    uint16 flagId = readScript16bits();
    uint16 value = readScript16bits();
    debugScript("O_COMPARE flagId 0x%04X, value %d", flagId, value);
    _result = (_flags[flagId-0x8000] == value);
}

void Script::O_JUMPZ() {
    int32 offset = readScript32bits();
    debugScript("O_JUMPZ offset 0x%04X", offset);
    if (_result == 0)
    {
        _currentInstruction += offset - 4;
    }
}

void Script::O_JUMPNZ() {
    int32 offset = readScript32bits();
    debugScript("O_JUMPNZ offset 0x%04X", offset);
    if (_result)
    {
        _currentInstruction += offset - 4;
    }
}

void Script::O_EXIT() {}

void Script::O_ADDFLAG() {
    uint16 flagId = readScript16bits();
    uint16 value = readScript16bits();

    _flags[flagId-0x8000] += value;
    if (_flags[flagId-0x8000])
        _result = 1;
    else
        _result = 0;

    debugScript("O_ADDFLAG flagId %d, value %d", flagId, value);
}

void Script::O_TALKANIM() {
    uint16 animSlot = readScript16bits();
    uint16 slot = readScript16bits();

    debugScript("O_TALKANIM animSlot %d, slot %d", animSlot, slot);
}

void Script::O_SUBFLAG() {
    uint16 flagId = readScript16bits();
    uint16 value = readScript16bits();

    _flags[flagId-0x8000] -= value;
    if (_flags[flagId-0x8000])
        _result = 1;
    else
        _result = 0;

    debugScript("O_SUBFLAG flagId %d, value %d", flagId, value);
}

void Script::O_SETSTRING() {
    int32 offset = readScript32bits();

    debugScript("O_SETSTRING 0x%04X", offset);
}

void Script::O_ANDFLAG() {}

void Script::O_GETMOBDATA() {}

void Script::O_ORFLAG() {}

void Script::O_SETMOBDATA() {}

void Script::O_XORFLAG() {}

void Script::O_GETMOBTEXT() {}

void Script::O_MOVEHERO() {
    uint16 heroId = readScript16bits();
    uint16 x = readScript16bits();
    uint16 y = readScript16bits();
    uint16 dir = readScript16bits();
    
    debugScript("O_MOVEHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Script::O_WALKHERO() {
    uint16 heroId = readScript16bits();

    debugScript("O_WALKHERO %d", heroId);
}

void Script::O_SETHERO() {}
void Script::O_HEROOFF() {
    uint16 heroId = readScript16bits();
    debugScript("O_HEROOFF %d", heroId);
}

void Script::O_HEROON() {
    uint16 heroId = readScript16bits();
    debugScript("O_HEROON %d", heroId);
}

void Script::O_CLSTEXT() {}
void Script::O_CALLTABLE() {}
void Script::O_CHANGEMOB() {}
void Script::O_ADDINV() {}
void Script::O_REMINV() {}
void Script::O_REPINV() {}
void Script::O_OBSOLETE_GETACTION() {}
void Script::O_ADDWALKAREA() {}
void Script::O_REMWALKAREA() {}
void Script::O_RESTOREWALKAREA() {}

void Script::O_WAITFRAME() {
    debugScript("O_WAITFRAME");
    _opcodeNF = true;
}

void Script::O_SETFRAME() {}
void Script::O_RUNACTION() {}
void Script::O_COMPAREHI() {}
void Script::O_COMPARELO() {}
void Script::O_PRELOADSET() {}
void Script::O_FREEPRELOAD() {}
void Script::O_CHECKINV() {}
void Script::O_TALKHERO() {}

void Script::O_WAITTEXT() {
    uint16 slot = readScript16bits();
    debugScript("O_WAITTEXT slot %d", slot);
}

void Script::O_SETHEROANIM() {}
void Script::O_WAITHEROANIM() {}
void Script::O_GETHERODATA() {}
void Script::O_GETMOUSEBUTTON() {}
void Script::O_CHANGEFRAMES() {}
void Script::O_CHANGEBACKFRAMES() {}
void Script::O_GETBACKANIMDATA() {}
void Script::O_GETANIMDATA() {}
void Script::O_SETBGCODE() {
    int32 bgcode = readScript32bits();
    debugScript("O_SETBGCODE %d", bgcode);
}
void Script::O_SETBACKFRAME() {}
void Script::O_GETRND() {}
void Script::O_TALKBACKANIM() {}
void Script::O_LOADPATH() {}

void Script::O_GETCHAR() {
    uint16 flagId = readScript16bits();
    debugScript("O_GETCHAR %d", flagId);
}

void Script::O_SETDFLAG() {}
void Script::O_CALLDFLAG() {}
void Script::O_PRINTAT() {}
void Script::O_ZOOMIN() {}
void Script::O_ZOOMOUT() {}
void Script::O_SETSTRINGOFFSET() {}
void Script::O_GETOBJDATA() {}
void Script::O_SETOBJDATA() {}
void Script::O_SWAPOBJECTS() {}
void Script::O_CHANGEHEROSET() {}
void Script::O_ADDSTRING() {}
void Script::O_SUBSTRING() {}
void Script::O_INITDIALOG() {}
void Script::O_ENABLEDIALOGOPT() {}
void Script::O_DISABLEDIALOGOPT() {}
void Script::O_SHOWDIALOGBOX() {}

void Script::O_STOPSAMPLE() {
    uint16 slot = readScript16bits();
    debugScript("O_STOPSAMPLE slot %d", slot);
}

void Script::O_BACKANIMRANGE() {}
void Script::O_CLEARPATH() {}
void Script::O_SETPATH() {}
void Script::O_GETHEROX() {}
void Script::O_GETHEROY() {}
void Script::O_GETHEROD() {}
void Script::O_PUSHSTRING() {}
void Script::O_POPSTRING() {}
void Script::O_SETFGCODE() {}
void Script::O_STOPHERO() {}
void Script::O_ANIMUPDATEOFF() {}
void Script::O_ANIMUPDATEON() {}

void Script::O_FREECURSOR() {
    debugScript("O_FREECURSOR");
}

void Script::O_ADDINVQUIET() {}
void Script::O_RUNHERO() {}
void Script::O_SETBACKANIMDATA() {}

void Script::O_VIEWFLC() {
    uint16 animNr = readScript16bits();
    debugScript("O_VIEWFLC animNr %d", animNr);
}

void Script::O_CHECKFLCFRAME() {}

void Script::O_CHECKFLCEND() {}

void Script::O_FREEFLC() {}
void Script::O_TALKHEROSTOP() {}
void Script::O_HEROCOLOR() {}
void Script::O_GRABMAPA() {}
void Script::O_ENABLENAK() {}
void Script::O_DISABLENAK() {}
void Script::O_GETMOBNAME() {}
void Script::O_SWAPINVENTORY() {}
void Script::O_CLEARINVENTORY() {}

void Script::O_SKIPTEXT() {
    debugScript("O_SKIPTEXT");
}

void Script::O_SETVOICEH() {}

void Script::O_SETVOICEA() {}

void Script::O_SETVOICEB() {}

void Script::O_SETVOICEC() {}

void Script::O_VIEWFLCLOOP() {}

void Script::O_FLCSPEED() {}

void Script::O_OPENINVENTORY() {
    debugScript("O_OPENINVENTORY");
}

void Script::O_KRZYWA() {
    debugScript("O_KRZYWA");
}

void Script::O_GETKRZYWA() {
    debugScript("O_GETKRZYWA");
}

void Script::O_GETMOB() {}

void Script::O_INPUTLINE() {
    debugScript("O_INPUTLINE");
}

void Script::O_SETVOICED() {}

void Script::O_BREAK_POINT() {
    debugScript("O_BREAK_POINT");
}

Script::OpcodeFunc Script::_opcodes[NUM_OPCODES] = {
    &Script::O_WAITFOREVER,
    &Script::O_BLACKPALETTE,
    &Script::O_SETUPPALETTE,
    &Script::O_INITROOM,
    &Script::O_SETSAMPLE,
    &Script::O_FREESAMPLE,
    &Script::O_PLAYSAMPLE,
    &Script::O_PUTOBJECT,
    &Script::O_REMOBJECT,
    &Script::O_SHOWANIM,
    &Script::O_CHECKANIMEND,
    &Script::O_FREEANIM,
    &Script::O_CHECKANIMFRAME,
    &Script::O_PUTBACKANIM,
    &Script::O_REMBACKANIM,
    &Script::O_CHECKBACKANIMFRAME,
    &Script::O_FREEALLSAMPLES,
    &Script::O_SETMUSIC,
    &Script::O_STOPMUSIC,
    &Script::O__WAIT,
    &Script::O_UPDATEOFF,
    &Script::O_UPDATEON,
    &Script::O_UPDATE ,
    &Script::O_CLS,
    &Script::O__CALL,
    &Script::O_RETURN,
    &Script::O_GO,
    &Script::O_BACKANIMUPDATEOFF,
    &Script::O_BACKANIMUPDATEON,
    &Script::O_CHANGECURSOR,
    &Script::O_CHANGEANIMTYPE,
    &Script::O__SETFLAG,
    &Script::O_COMPARE,
    &Script::O_JUMPZ,
    &Script::O_JUMPNZ,
    &Script::O_EXIT,
    &Script::O_ADDFLAG,
    &Script::O_TALKANIM,
    &Script::O_SUBFLAG,
    &Script::O_SETSTRING,
    &Script::O_ANDFLAG,
    &Script::O_GETMOBDATA,
    &Script::O_ORFLAG,
    &Script::O_SETMOBDATA,
    &Script::O_XORFLAG,
    &Script::O_GETMOBTEXT,
    &Script::O_MOVEHERO,
    &Script::O_WALKHERO,
    &Script::O_SETHERO,
    &Script::O_HEROOFF,
    &Script::O_HEROON,
    &Script::O_CLSTEXT,
    &Script::O_CALLTABLE,
    &Script::O_CHANGEMOB,
    &Script::O_ADDINV,
    &Script::O_REMINV,
    &Script::O_REPINV,
    &Script::O_OBSOLETE_GETACTION,
    &Script::O_ADDWALKAREA,
    &Script::O_REMWALKAREA,
    &Script::O_RESTOREWALKAREA,
    &Script::O_WAITFRAME,
    &Script::O_SETFRAME,
    &Script::O_RUNACTION,
    &Script::O_COMPAREHI,
    &Script::O_COMPARELO,
    &Script::O_PRELOADSET,
    &Script::O_FREEPRELOAD,
    &Script::O_CHECKINV,
    &Script::O_TALKHERO,
    &Script::O_WAITTEXT,
    &Script::O_SETHEROANIM,
    &Script::O_WAITHEROANIM,
    &Script::O_GETHERODATA,
    &Script::O_GETMOUSEBUTTON,
    &Script::O_CHANGEFRAMES,
    &Script::O_CHANGEBACKFRAMES,
    &Script::O_GETBACKANIMDATA,
    &Script::O_GETANIMDATA,
    &Script::O_SETBGCODE,
    &Script::O_SETBACKFRAME,
    &Script::O_GETRND,
    &Script::O_TALKBACKANIM,
    &Script::O_LOADPATH,
    &Script::O_GETCHAR,
    &Script::O_SETDFLAG,
    &Script::O_CALLDFLAG,
    &Script::O_PRINTAT,
    &Script::O_ZOOMIN,
    &Script::O_ZOOMOUT,
    &Script::O_SETSTRINGOFFSET,
    &Script::O_GETOBJDATA,
    &Script::O_SETOBJDATA,
    &Script::O_SWAPOBJECTS,
    &Script::O_CHANGEHEROSET,
    &Script::O_ADDSTRING,
    &Script::O_SUBSTRING,
    &Script::O_INITDIALOG,
    &Script::O_ENABLEDIALOGOPT,
    &Script::O_DISABLEDIALOGOPT,
    &Script::O_SHOWDIALOGBOX,
    &Script::O_STOPSAMPLE,
    &Script::O_BACKANIMRANGE,
    &Script::O_CLEARPATH,
    &Script::O_SETPATH,
    &Script::O_GETHEROX,
    &Script::O_GETHEROY,
    &Script::O_GETHEROD,
    &Script::O_PUSHSTRING,
    &Script::O_POPSTRING,
    &Script::O_SETFGCODE,
    &Script::O_STOPHERO,
    &Script::O_ANIMUPDATEOFF,
    &Script::O_ANIMUPDATEON,
    &Script::O_FREECURSOR,
    &Script::O_ADDINVQUIET,
    &Script::O_RUNHERO,
    &Script::O_SETBACKANIMDATA,
    &Script::O_VIEWFLC,
    &Script::O_CHECKFLCFRAME,
    &Script::O_CHECKFLCEND,
    &Script::O_FREEFLC,
    &Script::O_TALKHEROSTOP,
    &Script::O_HEROCOLOR,
    &Script::O_GRABMAPA,
    &Script::O_ENABLENAK,
    &Script::O_DISABLENAK,
    &Script::O_GETMOBNAME,
    &Script::O_SWAPINVENTORY,
    &Script::O_CLEARINVENTORY,
    &Script::O_SKIPTEXT,
    &Script::O_SETVOICEH,
    &Script::O_SETVOICEA,
    &Script::O_SETVOICEB,
    &Script::O_SETVOICEC,
    &Script::O_VIEWFLCLOOP,
    &Script::O_FLCSPEED,
    &Script::O_OPENINVENTORY,
    &Script::O_KRZYWA,
    &Script::O_GETKRZYWA,
    &Script::O_GETMOB,
    &Script::O_INPUTLINE,
    &Script::O_SETVOICED,
    &Script::O_BREAK_POINT,
};

}

