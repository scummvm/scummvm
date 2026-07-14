/* ScummVM - Graphic Adventure Engine */

#ifndef RIPPER_DIALOGUE_H
#define RIPPER_DIALOGUE_H

#include "common/str.h"
#include "common/array.h"

namespace Ripper {

struct ScriptCommand;
class CompiledScript;

class DialogueManager {
public:
	bool execute(const CompiledScript &script, const ScriptCommand &command);
	bool isPending() const { return _pending; }
	void clearPending() { _pending = false; _choices.clear(); }

private:
	struct Choice {
		Common::String text;
		uint16 result;
	};
	Common::Array<Choice> _choices;
	bool _pending = false;
};

} // End of namespace Ripper

#endif // RIPPER_DIALOGUE_H
