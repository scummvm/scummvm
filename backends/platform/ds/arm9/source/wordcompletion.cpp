#include "wordcompletion.h"
#include "osystem_ds.h"
#include "engines/agi/agi.h"	// Caution for #define for NUM_CHANNELS, causes problems in mixer_intern.h

#ifdef ENABLE_AGI

namespace DS {
// Default dictionary is about 64Kb, so 128Kb should be enough for future expansion
#define WORD_BUFFER_SIZE (128 * 1024)

// Default dictionary has ~8000 words
#define MAX_WORD_COUNT 16000

char wordBuffer[WORD_BUFFER_SIZE];
int wordBufferPos = 0;

char* wordBufferPtr[MAX_WORD_COUNT];
int wordBufferPtrPos = 0;

void addAutoCompleteLine(const char *line) {

	while (*line != 0) {
		char word[32];
		int length;

		// Skip the T9-style numbers
		while (*line != ' ') {
			line++;
		}
		line++;

		do {
			length = 0;

			if (*line == ' ') line++;


			// Copy the new word
			do {
				word[length++] = *line++;
			} while ((*line != '\0') && (*line != ' ') && (*line != '\n'));

			word[length] = '\0';


			// Store a pointer to the start of the word
			wordBufferPtr[wordBufferPtrPos++] = &wordBuffer[wordBufferPos];

			// copy the new word into the buffer
			strcpy(&wordBuffer[wordBufferPos], word);
			wordBufferPos += strlen(word) + 1;
		} while (*line == ' ');
	}
}

int stringCompare(const void* a, const void* b) {
	const char** as = (const char **) a;
	const char** bs = (const char **) b;

	return scumm_stricmp(*as, *bs);
}

void clearAutoCompleteWordList() {
	wordBufferPtrPos = 0;
	wordBufferPos = 0;
}

void sortAutoCompleteWordList() {
	// Sort the whole word list into alphabetical order
	qsort((void *)wordBufferPtr, wordBufferPtrPos, 4, stringCompare);
}

// Sends the current available words to the virtual keyboard code for display
bool findWordCompletions(const char* input) {
	int min = 0;
	int max = wordBufferPtrPos - 1;
	char *word;
	int position;
	char partialWord[32];

	// Early out if dictionary not loaded
	if (wordBufferPtrPos == 0)
		return false;

	OSystem_DS* system = (OSystem_DS *) g_system;
	system->clearAutoComplete();

	int start = 0;
	for (int r = strlen(input) - 1; r>0; r--) {
		if (input[r] == ' ') {
			start = r + 1;
			break;
		}
	}
	strcpy(partialWord, &input[start]);

	if (*partialWord == 0) {
		return false;
	}

	do {
		position = min + ((max - min) / 2);

		// Get the word from the dictonary line
		word = wordBufferPtr[position];

		// Now check to see if the word is before or after the stub we're after
		int result = scumm_stricmp(partialWord, word);

		if (result == 0) {
			// We've found the whole word.  Aren't we good.
			break;
		} else if (result > 0) {
			// We're too early, so change the minimum position
			min = position + 1;
		} else if (result < 0) {
			// We're too early, so change the maximum position
			max = position - 1;
		}

//		consolePrintf("Word: %s, (%d, %d) result: %d\n", word, min, max, result);

	} while (max - min > 0);

	position = min;
	word = wordBufferPtr[position];
	//consolePrintf("Final word: %s\n", word);



	system->setCharactersEntered(strlen(partialWord));


	bool match = true;


	for (int r = 0; partialWord[r] != 0; r++) {
		if (word[r] != partialWord[r]) {
			match = false;
			break;
		}
	}

	if (!match) {
		position++;
		if (position == wordBufferPtrPos)
			return false;
		word = wordBufferPtr[position];
//		consolePrintf("Final word: %s\n", word);
	}


	match = true;

	do {

		for (int r = 0; partialWord[r] != 0; r++) {
			if (word[r] != partialWord[r]) {
				match = false;
				break;
			}
		}

		if (match) {
			system->addAutoComplete(word);
		}

		position++;
		if (position < wordBufferPtrPos) {
			word = wordBufferPtr[position];
		}

	} while ((match) && (position < wordBufferPtrPos));

	return true;

}

}
#endif
