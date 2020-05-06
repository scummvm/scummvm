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

#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/conversation/dialogueloader_lb.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/filesys/u4file.h"

namespace Ultima {
namespace Ultima4 {

Response *lordBritishGetHelp(const DynamicResponse *resp);
Response *lordBritishGetIntro(const DynamicResponse *resp);

/**
 * A special case dialogue loader for Lord British.  Loads most of the
 * keyword/responses from a string table originally extracted from the
 * game executable.  The  "help" response is a special case that changes
 * based on the current party status.
 */
Dialogue *U4LBDialogueLoader::load(void *source) {
	Std::vector<Common::String> lbKeywords = u4read_stringtable("lb_keywords");
	Std::vector<Common::String> lbText = u4read_stringtable("lb_text");

	Dialogue *dlg = new Dialogue();
	dlg->setTurnAwayProb(0);

	dlg->setName("Lord British");
	dlg->setPronoun("He");
	dlg->setPrompt("What else?\n");
	Response *intro = new DynamicResponse(&lordBritishGetIntro);
	dlg->setIntro(intro);
	dlg->setLongIntro(intro);
	dlg->setDefaultAnswer(new Response("\nHe says: I\ncannot help thee\nwith that.\n"));

	for (unsigned i = 0; i < lbKeywords.size(); i++) {
		dlg->addKeyword(lbKeywords[i], new Response(lbText[i]));
	}

	Response *heal = new Response("\n\n\n\n\n\nHe says: I am\nwell, thank ye.");
	heal->add(g_responseParts->HEALCONFIRM);
	dlg->addKeyword("heal", heal);

	Response *bye;
	if (g_context->_party->size() > 1)
		bye = new Response("Lord British says: Fare thee well my friends!");
	else
		bye = new Response("Lord British says: Fare thee well my friend!");
	bye->add(g_responseParts->STOPMUSIC);
	bye->add(g_responseParts->END);
	dlg->addKeyword("bye", bye);
	dlg->addKeyword("", bye);

	dlg->addKeyword("help", new DynamicResponse(&lordBritishGetHelp));

	return dlg;
}

/**
 * Generate the appropriate response when the player asks Lord British
 * for help.  The help text depends on the current party status; when
 * one quest item is complete, Lord British provides some direction to
 * the next one.
 */
Response *lordBritishGetHelp(const DynamicResponse *resp) {
	int v;
	bool fullAvatar, partialAvatar;
	Common::String text;

	/*
	 * check whether player is full avatar (in all virtues) or partial
	 * avatar (in at least one virtue)
	 */
	fullAvatar = true;
	partialAvatar = false;
	for (v = 0; v < VIRT_MAX; v++) {
		fullAvatar &= (g_ultima->_saveGame->_karma[v] == 0);
		partialAvatar |= (g_ultima->_saveGame->_karma[v] == 0);
	}

	if (g_ultima->_saveGame->_moves <= 1000) {
		text = "To survive in this hostile land thou must first know thyself! Seek ye to master thy weapons and thy magical ability!\n"
		       "\nTake great care in these thy first travels in Britannia.\n"
		       "\nUntil thou dost well know thyself, travel not far from the safety of the townes!\n";
	}

	else if (g_ultima->_saveGame->_members == 1) {
		text = "Travel not the open lands alone. There are many worthy people in the diverse townes whom it would be wise to ask to Join thee!\n"
		       "\nBuild thy party unto eight travellers, for only a true leader can win the Quest!\n";
	}

	else if (g_ultima->_saveGame->_runes == 0) {
		text = "Learn ye the paths of virtue. Seek to gain entry unto the eight shrines!\n"
		       "\nFind ye the Runes, needed for entry into each shrine, and learn each chant or \"Mantra\" used to focus thy meditations.\n"
		       "\nWithin the Shrines thou shalt learn of the deeds which show thy inner virtue or vice!\n"
		       "\nChoose thy path wisely for all thy deeds of good and evil are remembered and can return to hinder thee!\n";
	}

	else if (!partialAvatar) {
		text = "Visit the Seer Hawkwind often and use his wisdom to help thee prove thy virtue.\n"
		       "\nWhen thou art ready, Hawkwind will advise thee to seek the Elevation unto partial Avatarhood in a virtue.\n"
		       "\nSeek ye to become a partial Avatar in all eight virtues, for only then shalt thou be ready to seek the codex!\n";
	}

	else if (g_ultima->_saveGame->_stones == 0) {
		text = "Go ye now into the depths of the dungeons. Therein recover the 8 colored stones from the altar pedestals in the halls of the dungeons.\n"
		       "\nFind the uses of these stones for they can help thee in the Abyss!\n";
	}

	else if (!fullAvatar) {
		text = "Thou art doing very well indeed on the path to Avatarhood! Strive ye to achieve the Elevation in all eight virtues!\n";
	}

	else if ((g_ultima->_saveGame->_items & ITEM_BELL) == 0 ||
	         (g_ultima->_saveGame->_items & ITEM_BOOK) == 0 ||
	         (g_ultima->_saveGame->_items & ITEM_CANDLE) == 0) {
		text = "Find ye the Bell, Book and Candle!  With these three things, one may enter the Great Stygian Abyss!\n";
	}

	else if ((g_ultima->_saveGame->_items & ITEM_KEY_C) == 0 ||
	         (g_ultima->_saveGame->_items & ITEM_KEY_L) == 0 ||
	         (g_ultima->_saveGame->_items & ITEM_KEY_T) == 0) {
		text = "Before thou dost enter the Abyss thou shalt need the Key of Three Parts, and the Word of Passage.\n"
		       "\nThen might thou enter the Chamber of the Codex of Ultimate Wisdom!\n";
	}

	else {
		text = "Thou dost now seem ready to make the final journey into the dark Abyss! Go only with a party of eight!\n"
		       "\nGood Luck, and may the powers of good watch over thee on this thy most perilous endeavor!\n"
		       "\nThe hearts and souls of all Britannia go with thee now. Take care, my friend.\n";
	}

	return new Response(Common::String("He says: ") + text);
}

Response *lordBritishGetIntro(const DynamicResponse *resp) {
	Response *intro = new Response("");
	intro->add(g_responseParts->STARTMUSIC_LB);

	if (g_ultima->_saveGame->_lbIntro) {
		if (g_ultima->_saveGame->_members == 1) {
			intro->add(Common::String("\n\n\nLord British\nsays:  Welcome\n") +
			           g_context->_party->member(0)->getName() + "!\n\n");
		} else if (g_ultima->_saveGame->_members == 2) {
			intro->add(Common::String("\n\nLord British\nsays:  Welcome\n") +
			           g_context->_party->member(0)->getName() +
			           " and thee also " +
			           g_context->_party->member(1)->getName() +
			           "!\n\n");
		} else {
			intro->add(Common::String("\n\n\nLord British\nsays:  Welcome\n") +
			           g_context->_party->member(0)->getName() +
			           " and thy\nworthy\nAdventurers!\n\n");
		}

		// Lord British automatically adds "What would thou ask of me?"

		// Check levels here, just like the original!
		intro->add(g_responseParts->ADVANCELEVELS);
	}

	else {
		intro->add(Common::String("\n\n\nLord British rises and says: At long last!\n") +
		           g_context->_party->member(0)->getName() +
		           " thou hast come!  We have waited such a long, long time...\n"
		           "\n\nLord British sits and says: A new age is upon Britannia. The great evil Lords are gone but our people lack direction and purpose in their lives...\n\n\n"
		           "A champion of virtue is called for. Thou may be this champion, but only time shall tell.  I will aid thee any way that I can!\n\n"
		           "How may I help thee?\n");
		g_ultima->_saveGame->_lbIntro = 1;
	}

	return intro;
}

} // End of namespace Ultima4
} // End of namespace Ultima
