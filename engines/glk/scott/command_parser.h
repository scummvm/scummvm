#ifndef GLK_SCOTT_PARSER
#define GLK_SCOTT_PARSER

#include "glk/glk_types.h"
#include "common/str-array.h"

namespace Glk {
namespace Scott {

#define NUMBER_OF_DIRECTIONS 14
#define NUMBER_OF_SKIPPABLE_WORDS 18
#define NUMBER_OF_DELIMITERS 5
#define NUMBER_OF_EXTRA_COMMANDS 20
#define NUMBER_OF_EXTRA_NOUNS 16

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

char **splitIntoWords(glui32 *string, int length);
int getInput(int *vb, int *no);
void freeCommands();
glui32 *toUnicode(const char *string);
char *fromUnicode(glui32 *unicodeString, int origLength);
int whichWord(const char *word, Common::StringArray list, int wordLength);

} // End of namespace Scott
} // End of namespace Glk

#endif
