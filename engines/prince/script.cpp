#include "prince/script.h"

#include "common/debug-channels.h"
#include "common/stream.h"

namespace Prince {

Script::Script(PrinceEngine *vm) : 
	_code(NULL), _stacktop(0), _vm(vm), _random("GroovieScripts") {
}

Script::~Script() {
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
	_codeSize = stream.size();
	_code = new byte[_codeSize];

	if (!_code)
		return false;

	stream.read(_code, _codeSize);
	// Initialize the script
	_currentInstruction = 0;

	return true;
}

void Script::step() {
}

uint8 Script::getCodeByte(uint16 address) {
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

}

