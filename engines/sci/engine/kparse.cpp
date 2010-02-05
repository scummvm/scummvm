/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* String and parser handling */

#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/message.h"
#include "sci/engine/kernel.h"

namespace Sci {

/*************************************************************/
/* Parser */
/**********/


reg_t kSaid(EngineState *s, int argc, reg_t *argv) {
	reg_t heap_said_block = argv[0];
	byte *said_block;
	int new_lastmatch;
#ifdef DEBUG_PARSER
	const int debug_parser = 1;
#else
	const int debug_parser = 0;
#endif

	if (!heap_said_block.segment)
		return NULL_REG;

	said_block = (byte *)s->_segMan->derefBulkPtr(heap_said_block, 0);

	if (!said_block) {
		warning("Said on non-string, pointer %04x:%04x", PRINT_REG(heap_said_block));
		return NULL_REG;
	}

#ifdef DEBUG_PARSER
		debugC(2, kDebugLevelParser, "Said block:", 0);
		s->_voc->decipherSaidBlock(said_block);
#endif

	if (s->_voc->parser_event.isNull() || (GET_SEL32V(s->_segMan, s->_voc->parser_event, SELECTOR(claimed)))) {
		return NULL_REG;
	}

	new_lastmatch = said(s, said_block, debug_parser);
	if (new_lastmatch  != SAID_NO_MATCH) { /* Build and possibly display a parse tree */

#ifdef DEBUG_PARSER
		printf("kSaid: Match.\n");
#endif

		s->r_acc = make_reg(0, 1);

		if (new_lastmatch != SAID_PARTIAL_MATCH)
			PUT_SEL32V(s->_segMan, s->_voc->parser_event, SELECTOR(claimed), 1);

	} else {
		return NULL_REG;
	}
	return s->r_acc;
}

reg_t kParse(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t stringpos = argv[0];
	Common::String string = s->_segMan->getString(stringpos);
	char *error;
	ResultWordList words;
	reg_t event = argv[1];
	Vocabulary *voc = s->_voc;

	s->_voc->parser_event = event;

	bool res = voc->tokenizeString(words, string.c_str(), &error);
	s->_voc->parserIsValid = false; /* not valid */

	if (res && !words.empty()) {
		s->_voc->synonymizeTokens(words);

		s->r_acc = make_reg(0, 1);

#ifdef DEBUG_PARSER
			debugC(2, kDebugLevelParser, "Parsed to the following blocks:", 0);

			for (ResultWordList::const_iterator i = words.begin(); i != words.end(); ++i)
				debugC(2, kDebugLevelParser, "   Type[%04x] Group[%04x]", i->_class, i->_group);
#endif

		int syntax_fail = voc->parseGNF(words);

		if (syntax_fail) {
			s->r_acc = make_reg(0, 1);
			PUT_SEL32V(segMan, event, SELECTOR(claimed), 1);

			invoke_selector(INV_SEL(s->_gameObj, syntaxFail, kStopOnInvalidSelector), 2, s->_voc->parser_base, stringpos);
			/* Issue warning */

			debugC(2, kDebugLevelParser, "Tree building failed");

		} else {
			s->_voc->parserIsValid = true;
			PUT_SEL32V(segMan, event, SELECTOR(claimed), 0);

#ifdef DEBUG_PARSER
			s->_voc->dumpParseTree();
#endif
		}

	} else {

		s->r_acc = make_reg(0, 0);
		PUT_SEL32V(segMan, event, SELECTOR(claimed), 1);
		if (error) {
			s->_segMan->strcpy(s->_voc->parser_base, error);
			debugC(2, kDebugLevelParser, "Word unknown: %s", error);
			/* Issue warning: */

			invoke_selector(INV_SEL(s->_gameObj, wordFail, kStopOnInvalidSelector), 2, s->_voc->parser_base, stringpos);
			free(error);
			return make_reg(0, 1); /* Tell them that it didn't work */
		}
	}

	return s->r_acc;
}

reg_t kSetSynonyms(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t object = argv[0];
	List *list;
	Node *node;
	int script;
	int numSynonyms = 0;

	// Only SCI0-SCI1 EGA games had a parser. In newer versions, this is a stub
	if (getSciVersion() > SCI_VERSION_1_EGA)
		return s->r_acc;

	s->_voc->clearSynonyms();

	list = s->_segMan->lookupList(GET_SEL32(segMan, object, SELECTOR(elements)));
	node = s->_segMan->lookupNode(list->first);

	while (node) {
		reg_t objpos = node->value;
		int seg;

		script = GET_SEL32V(segMan, objpos, SELECTOR(number));
		seg = s->_segMan->getScriptSegment(script);

		if (seg > 0)
			numSynonyms = s->_segMan->getScript(seg)->getSynonymsNr();

		if (numSynonyms) {
			byte *synonyms = s->_segMan->getScript(seg)->getSynonyms();

			if (synonyms) {
				debugC(2, kDebugLevelParser, "Setting %d synonyms for script.%d",
				          numSynonyms, script);

				if (numSynonyms > 16384) {
					error("Segtable corruption: script.%03d has %d synonyms",
					         script, numSynonyms);
					/* We used to reset the corrupted value here. I really don't think it's appropriate.
					 * Lars */
				} else
					for (int i = 0; i < numSynonyms; i++) {
						synonym_t tmp;
						tmp.replaceant = (int16)READ_LE_UINT16(synonyms + i * 4);
						tmp.replacement = (int16)READ_LE_UINT16(synonyms + i * 4 + 2);
						s->_voc->addSynonym(tmp);
					}
			} else
				warning("Synonyms of script.%03d were requested, but script is not available", script);

		}

		node = s->_segMan->lookupNode(node->succ);
	}

	debugC(2, kDebugLevelParser, "A total of %d synonyms are active now.", numSynonyms);

	return s->r_acc;
}

} // End of namespace Sci
