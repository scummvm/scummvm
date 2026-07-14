/* ScummVM - Graphic Adventure Engine */

#include "ripper/dialogue.h"

#include "common/debug.h"

#include "ripper/detection.h"
#include "ripper/script.h"

namespace Ripper {

bool DialogueManager::execute(const CompiledScript &script, const ScriptCommand &command) {
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
