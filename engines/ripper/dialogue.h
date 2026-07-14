/* ScummVM - Graphic Adventure Engine */

#ifndef RIPPER_DIALOGUE_H
#define RIPPER_DIALOGUE_H

#include "common/str.h"

namespace Ripper {

struct ScriptCommand;
class CompiledScript;

class DialogueManager {
public:
	bool execute(const CompiledScript &script, const ScriptCommand &command);
};

} // End of namespace Ripper

#endif // RIPPER_DIALOGUE_H
