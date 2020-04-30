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
#include "ultima/ultima4/conversation/dialogueloader_hw.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

Response *hawkwindGetAdvice(const DynamicResponse *kw);
Response *hawkwindGetIntro(const DynamicResponse *dynResp);

/* Hawkwind text indexes */
#define HW_SPEAKONLYWITH 40
#define HW_RETURNWHEN 41
#define HW_ISREVIVED 42
#define HW_WELCOME 43
#define HW_GREETING1 44
#define HW_GREETING2 45
#define HW_PROMPT 46
#define HW_DEFAULT 49
#define HW_ALREADYAVATAR 50
#define HW_GOTOSHRINE 51
#define HW_BYE 52

/**
 * A special case dialogue loader for Hawkwind.
 */
Dialogue *U4HWDialogueLoader::load(void *source) {
	Std::vector<Common::String> &hawkwindText = g_ultima->_hawkwindText;
	hawkwindText = u4read_stringtable("hawkwind");

	Dialogue *dlg = new Dialogue();
	dlg->setTurnAwayProb(0);

	dlg->setName("Hawkwind");
	dlg->setPronoun("He");
	dlg->setPrompt(hawkwindText[HW_PROMPT]);
	Response *intro = new DynamicResponse(&hawkwindGetIntro);
	dlg->setIntro(intro);
	dlg->setLongIntro(intro);
	dlg->setDefaultAnswer(new Response(Common::String("\n" + hawkwindText[HW_DEFAULT])));

	for (int v = 0; v < VIRT_MAX; v++) {
		Common::String virtue(getVirtueName((Virtue) v));
		lowercase(virtue);
		virtue = virtue.substr(0, 4);
		dlg->addKeyword(virtue, new DynamicResponse(&hawkwindGetAdvice, virtue));
	}

	Response *bye = new Response(hawkwindText[HW_BYE]);
	bye->add(g_responseParts->STOPMUSIC);
	bye->add(g_responseParts->END);
	dlg->addKeyword("bye", bye);
	dlg->addKeyword("", bye);

	return dlg;
}

/**
 * Generate the appropriate response when the player asks Lord British
 * for help.  The help text depends on the current party status; when
 * one quest item is complete, Lord British provides some direction to
 * the next one.
 */
Response *hawkwindGetAdvice(const DynamicResponse *dynResp) {
	Common::String text;
	int virtue = -1, virtueLevel = -1;
	Std::vector<Common::String> &hawkwindText = g_ultima->_hawkwindText;

	/* check if asking about a virtue */
	for (int v = 0; v < VIRT_MAX; v++) {
		if (scumm_strnicmp(dynResp->getParam().c_str(), getVirtueName((Virtue) v), 4) == 0) {
			virtue = v;
			virtueLevel = g_ultima->_saveGame->_karma[v];
			break;
		}
	}
	if (virtue != -1) {
		text = "\n\n";
		if (virtueLevel == 0)
			text += hawkwindText[HW_ALREADYAVATAR] + "\n";
		else if (virtueLevel < 80)
			text += hawkwindText[(virtueLevel / 20) * 8 + virtue];
		else if (virtueLevel < 99)
			text += hawkwindText[3 * 8 + virtue];
		else /* virtueLevel >= 99 */
			text = hawkwindText[4 * 8 + virtue] + hawkwindText[HW_GOTOSHRINE];
	} else {
		text = Common::String("\n") + hawkwindText[HW_DEFAULT];
	}

	return new Response(text);
}

Response *hawkwindGetIntro(const DynamicResponse *dynResp) {
	Response *intro = new Response("");
	Std::vector<Common::String> &hawkwindText = g_ultima->_hawkwindText;

	if (g_context->_party->member(0)->getStatus() == STAT_SLEEPING ||
	        g_context->_party->member(0)->getStatus() == STAT_DEAD) {
		intro->add(hawkwindText[HW_SPEAKONLYWITH] + g_context->_party->member(0)->getName() +
		    hawkwindText[HW_RETURNWHEN] + g_context->_party->member(0)->getName() +
		    hawkwindText[HW_ISREVIVED]);
		intro->add(g_responseParts->END);
	} else {
		intro->add(g_responseParts->STARTMUSIC_HW);
		intro->add(g_responseParts->HAWKWIND);

		intro->add(hawkwindText[HW_WELCOME] + g_context->_party->member(0)->getName() +
			hawkwindText[HW_GREETING1] + hawkwindText[HW_GREETING2]);
	}

	return intro;
}

} // End of namespace Ultima4
} // End of namespace Ultima
