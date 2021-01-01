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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/conversation/dialogueloader_tlk.h"
#include "ultima/shared/std/containers.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A dialogue loader for standard u4dos .tlk files.
 */
Dialogue *U4TlkDialogueLoader::load(void *source) {
	Common::ReadStream *file = static_cast<Common::ReadStream *>(source);

	enum QTrigger {
		NONE = 0,
		JOB = 3,
		HEALTH = 4,
		KEYWORD1 = 5,
		KEYWORD2 = 6
	};

	/* there's no dialogues left in the file */
	char tlk_buffer[288];
	if (file->read(tlk_buffer, sizeof(tlk_buffer)) != sizeof(tlk_buffer))
		return nullptr;

	char *ptr = &tlk_buffer[3];
	Std::vector<Common::String> strings;
	for (int i = 0; i < 12; i++) {
		strings.push_back(ptr);
		ptr += strlen(ptr) + 1;
	}

	Dialogue *dlg = new Dialogue();
	byte prob = tlk_buffer[2];
	QTrigger qtrigger = QTrigger(tlk_buffer[0]);
	bool humilityTestQuestion = tlk_buffer[1] == 1;

	dlg->setTurnAwayProb(prob);

	dlg->setName(strings[0]);
	dlg->setPronoun(strings[1]);
	dlg->setPrompt("\nYour Interest:\n");

	// Fix the actor description Common::String, converting the first character
	// to lower-case.
	strings[2].setChar(tolower(strings[2][0]), 0);

	// ... then replace any newlines in the Common::String with spaces
	size_t index = strings[2].find("\n");
	while (index != Common::String::npos) {
		strings[2].setChar(' ', index);
		index = strings[2].find("\n");
	}

	// ... then append a period to the end of the Common::String if one does
	// not already exist
	if (!Common::isPunct(strings[2][strings[2].size() - 1]))
		strings[2] = strings[2] + Common::String(".");

	// ... and finally, a few characters in the game have descriptions
	// that do not begin with a definite (the) or indefinite (a/an)
	// article.  On those characters, insert the appropriate article.
	if ((strings[0] == "Iolo")
	        || (strings[0] == "Tracie")
	        || (strings[0] == "Dupre")
	        || (strings[0] == "Traveling Dan"))
		strings[2] = Common::String("a ") + strings[2];

	Common::String introBase = Common::String("\nYou meet ") + strings[2] + "\n";

	dlg->setIntro(new Response(introBase + dlg->getPrompt()));
	dlg->setLongIntro(new Response(introBase +
	                               "\n" + dlg->getPronoun() + " says: I am " + dlg->getName() + "\n"
	                               + dlg->getPrompt()));
	dlg->setDefaultAnswer(new Response("That I cannot\nhelp thee with."));

	Response *yes = new Response(strings[8]);
	Response *no = new Response(strings[9]);
	if (humilityTestQuestion) {
		yes->add(g_responseParts->BRAGGED);
		no->add(g_responseParts->HUMBLE);
	}
	dlg->setQuestion(new Dialogue::Question(strings[7], yes, no));

	// one of the following four keywords triggers the speaker's question
	Response *job = new Response(Common::String("\n") + strings[3]);
	Response *health = new Response(Common::String("\n") + strings[4]);
	Response *kw1 = new Response(Common::String("\n") + strings[5]);
	Response *kw2 = new Response(Common::String("\n") + strings[6]);

	switch (qtrigger) {
	case JOB:
		job->add(g_responseParts->ASK);
		break;
	case HEALTH:
		health->add(g_responseParts->ASK);
		break;
	case KEYWORD1:
		kw1->add(g_responseParts->ASK);
		break;
	case KEYWORD2:
		kw2->add(g_responseParts->ASK);
		break;
	case NONE:
	default:
		break;
	}
	dlg->addKeyword("job", job);
	dlg->addKeyword("heal", health);
	dlg->addKeyword(strings[10], kw1);
	dlg->addKeyword(strings[11], kw2);

	// NOTE: We let the talker's custom keywords override the standard
	// keywords like HEAL and LOOK.  This behavior differs from u4dos,
	// but fixes a couple conversation files which have keywords that
	// conflict with the standard ones (e.g. Calabrini in Moonglow has
	// HEAL for healer, which is unreachable in u4dos, but clearly
	// more useful than "Fine." for health).
	Common::String look = Common::String("\nYou see ") + strings[2];
	dlg->addKeyword("look", new Response(look));
	dlg->addKeyword("name", new Response(Common::String("\n") + dlg->getPronoun() + " says: I am " + dlg->getName()));
	dlg->addKeyword("give", new Response(Common::String("\n") + dlg->getPronoun() + " says: I do not need thy gold.  Keep it!"));
	dlg->addKeyword("join", new Response(Common::String("\n") + dlg->getPronoun() + " says: I cannot join thee."));

	Response *bye = new Response("\nBye.");
	bye->add(g_responseParts->END);
	dlg->addKeyword("bye", bye);
	dlg->addKeyword("", bye);

	/*
	 * This little easter egg appeared in the Amiga version of Ultima IV.
	 * I've never figured out what the number means.
	 * "Banjo" Bob Hardy was the programmer for the Amiga version.
	 */
	dlg->addKeyword("ojna", new Response("\nHi Banjo Bob!\nYour secret\nnumber is\n4F4A4E0A"));

	return dlg;
}

} // End of namespace Ultima4
} // End of namespace Ultima
