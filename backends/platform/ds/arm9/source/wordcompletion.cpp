#include "wordcompletion.h"
#include "engines/agi/agi.h"
#include "osystem_ds.h"

#ifndef DISABLE_AGI

namespace DS {

void findWordCompletions(char* input) {
	int start = 0;
	for (int r = strlen(input) - 1; r>0; r--) {
		if (input[r] == ' ') {
			start = r + 1;
			break;
		}
	}

	char word[32];
	strcpy(word, &input[start]);

	int fchr = word[0] - 'a';
	int len = strlen(word);

	OSystem_DS* system = (OSystem_DS *) g_system;
	system->clearAutoComplete();
	system->setCharactersEntered(strlen(word));

	if (strlen(word) == 0) {
		return;
	}		

	uint8 *wordList = Agi::AgiEngine::getWordsData();
	uint8 *wordListEnd = Agi::AgiEngine::getWordsData() + Agi::AgiEngine::getWordsDataSize();

	/* Get the offset to the first word beginning with the
	 * right character
	 */
	wordList += READ_BE_UINT16(wordList + 2 * fchr);

	char currentWord[32];

	
	while (wordList < wordListEnd) {
		int pos = *wordList++;		// Number of chars to keep from previous word

		if (wordList == wordListEnd)
			break;

		char c;
		do {
			c = *wordList++;
			currentWord[pos++] =  (~c) & 0x7F;
		} while ((c & 0x80) == 0);		// Top bit indicates end of word
		currentWord[pos++] = '\0';

		if (!strncmp(currentWord, word, strlen(word))) {
			system->addAutoComplete(currentWord);
		}

		wordList += 2;	// Skip the two byte word id.

	}

}

}
#endif
