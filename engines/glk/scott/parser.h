#ifndef PARSER_H
#define PARSER_H

namespace Glk {
namespace Scott {

struct Command {
	int _verb;
	int _noun;
	int _item;
	int _verbWordIndex;
	int _nounWordIndex;
	int _allFlag;
	struct Command *_previous;
	struct Command *_next;
};

enum ExtraCommand {
	NO_COMMAND,
	RESTART,
	SAVE,
	RESTORE,
	SCRIPT,
	ON,
	OFF,
	UNDO,
	RAM,
	RAMSAVE,
	RAMLOAD,
	GAME,
	COMMAND,
	ALL,
	IT,
	EXCEPT
};

} // End of namespace Scott
} // End of namespace Glk

#endif
