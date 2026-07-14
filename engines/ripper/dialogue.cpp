/* ScummVM - Graphic Adventure Engine */

#include "ripper/dialogue.h"

#include "common/debug.h"

#include "ripper/detection.h"
#include "ripper/script.h"

namespace Ripper {

bool DialogueManager::execute(const CompiledScript &script, const ScriptCommand &command) {
	if (command.opcode == 0x16) {
		if (command.arguments.size() < 2 || command.arguments[0].data.empty())
			return false;
		Choice choice;
		for (uint i = 0; i < command.arguments[0].data.size() &&
				command.arguments[0].data[i] != 0; ++i)
			choice.text += (char)command.arguments[0].data[i];
		choice.result = command.arguments[1].value & 0xffff;
		_choices.push_back(choice);
		debugC(1, kDebugScripts,
			"Ripper: dialogue choice appended index=%u result=%u text='%s'",
			_choices.size() - 1, choice.result, choice.text.c_str());
		return true;
	}

	if (command.opcode == 0x0a) {
		debugC(1, kDebugScripts,
			"Ripper: dialogue choice list completed script='%s' offset=0x%x "
				"selector=%u choices=%u",
			script.getMemberName().c_str(), command.offset, command.selector, _choices.size());
		for (uint i = 0; i < _choices.size(); ++i)
			debugC(2, kDebugScripts, "Ripper: dialogue choice index=%u result=%u text='%s'",
				i, _choices[i].result, _choices[i].text.c_str());
		_choices.clear();
		return true;
	}

	debugC(1, kDebugScripts,
		"Ripper: dialogue command script='%s' offset=0x%x selector=%u arguments=%u",
		script.getMemberName().c_str(), command.offset, command.selector,
		command.arguments.size());
	for (uint i = 0; i < command.arguments.size(); ++i) {
		const ScriptArgument &argument = command.arguments[i];
		Common::String text;
		for (uint j = 0; j < argument.data.size() && argument.data[j] != 0; ++j)
			text += (char)argument.data[j];
		debugC(2, kDebugScripts,
			"Ripper: dialogue argument=%u type=%u value=0x%x bytes=%u text='%s'",
			i, argument.type, argument.value, argument.data.size(), text.c_str());
	}
	// The first slice records the decoded payload. Rendering and modal input
	// will be added once the selector-specific Ghidra handlers are mapped.
	return true;
}

} // End of namespace Ripper
