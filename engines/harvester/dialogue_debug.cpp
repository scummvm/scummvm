/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harvester/dialogue_debug.h"

#include "harvester/text.h"

namespace Harvester {

namespace {

static Common::String buildSubtitleSummary(const Text &text, int wavId) {
	Common::String subtitle;
	if (!text.resolveDialogueSubtitle(wavId, subtitle))
		subtitle = "<missing subtitle>";
	return subtitle;
}

static Common::String buildResponseSummary(const Text &text, int responseLineIndex) {
	Common::String responseLine = text.getDialogueResponseLine(responseLineIndex);
	if (responseLine.empty())
		responseLine = "<missing response line>";
	return responseLine;
}

static void appendLine(Common::Array<Common::String> &lines, const Common::String &line) {
	lines.push_back(line);
}

static void appendDialogueLine(Common::Array<Common::String> &lines, const Text &text,
		const char *indent, const char *speakerId, int wavId, int headVariant) {
	appendLine(lines, Common::String::format(
		"%sLINE speaker=%s wav=0x%x (%d) head=%d: %s",
		indent, speakerId, wavId, wavId, headVariant, buildSubtitleSummary(text, wavId).c_str()));
}

static void appendResponseMenu(Common::Array<Common::String> &lines, const Text &text,
		const char *indent, int responseLineIndex) {
	appendLine(lines, Common::String::format(
		"%sMENU rsp=0x%x (%d): %s",
		indent, responseLineIndex, responseLineIndex + 1,
		buildResponseSummary(text, responseLineIndex).c_str()));
}

static void appendSergeantDay5Exit(Common::Array<Common::String> &lines, const Text &text,
		const char *indent) {
	appendLine(lines, Common::String::format(
		"%sIF FLAG DAY_5 && !STATE sergeant.day5ReminderShown", indent));
	appendDialogueLine(lines, text, Common::String::format("%s  ", indent).c_str(),
		"SERGEANT", 0x4324, 1);
}

static void buildSergeantDialogueDebugDump(const Text &text, Common::Array<Common::String> &lines) {
	appendLine(lines, "NPC SERGEANT");
	appendLine(lines, "STATE sergeant.introPending, sergeant.dialogueStateD2d50, sergeant.dialogueStateD2d54, sergeant.day5ReminderShown");
	appendLine(lines, "BRANCH no_item");
	appendLine(lines, "  IF FLAG GOT_REMAINS_FOR_LODGE");
	appendLine(lines, "    ACTION restore item REMAINS to owner RAH");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x4333, 0);
	appendLine(lines, "  IF FLAG STEPHANIE_IS_DEAD && (INVENTORY INVITE || FLAG TAKEN_INVITE_TO_SERGEANT)");
	appendLine(lines, "    ACTION show ST_BEDRM/STEFSKULL");
	appendLine(lines, "    ACTION restore item INVITE to owner RAH");
	appendLine(lines, "    ACTION set flags TAKEN_INVITE_TO_SERGEANT, NEED_REMAINS_FOR_LODGE");
	appendLine(lines, "    ACTION hide NPCs DWAYNE_ST_BEDRM, MRS_POTTS_ST_BEDRM; show NPC MRS_POTTS");
	appendLine(lines, "    ACTION play FST GRAPHIC/FST/C149.FST");
	appendDialogueLine(lines, text, "    ", "PC", 0x4305, 0);
	appendLine(lines, "  IF FLAG DINER_BURNED");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x42b5, 1);
	appendLine(lines, "  IF FLAG BARBER_POLE_STOLEN");
	appendLine(lines, "    ACTION restore item BARBER_POLE to owner RAH");
	appendLine(lines, "    ACTION set flag ASSIGNED_DNA_TASK");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x426f, 0);
	appendLine(lines, "  IF FLAG BOLT_OF_CLOTH_TAKEN");
	appendLine(lines, "    ACTION restore item BOLTCLTH to owner RAH");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x4241, 0);
	appendLine(lines, "  IF FLAG SCRATCHED_TUCKER");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x420d, 0);
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x421e, 0);
	appendResponseMenu(lines, text, "    ", 0x2b4);
	appendLine(lines, "      CHOICE 1");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x422a, 0);
	appendLine(lines, "      CHOICE 2");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x422f, 0);
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4239, 0);
	appendLine(lines, "  IF FLAG HAVE_COMPLETED_LODGE_APP");
	appendLine(lines, "    ACTION restore item COMPLETED_LODGE_APPLICATION to owner RAH");
	appendLine(lines, "    IF !FLAG QUEST_1");
	appendDialogueLine(lines, text, "      ", "SERGEANT", 0x41cd, 1);
	appendDialogueLine(lines, text, "      ", "SERGEANT", 0x4292, 0);
	appendLine(lines, "      ACTION set flag QUEST_1");
	appendLine(lines, "      ACTION execute GO_DAY_2");
	appendLine(lines, "    ELSE IF !STATE sergeant.dialogueStateD2d54");
	appendDialogueLine(lines, text, "      ", "SERGEANT", 0x41d2, 1);
	appendResponseMenu(lines, text, "      ", 0x2b6);
	appendLine(lines, "        CHOICE 1");
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x41db, 0);
	appendDialogueLine(lines, text, "          ", "PC", 0x41e2, 0);
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x41e6, 0);
	appendDialogueLine(lines, text, "          ", "PC", 0x41ec, 2);
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x41f0, 0);
	appendDialogueLine(lines, text, "          ", "PC", 0x41f6, 0);
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x41fa, 0);
	appendLine(lines, "          ACTION set STATE sergeant.dialogueStateD2d54");
	appendLine(lines, "        CHOICE 2");
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x4201, 3);
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x4202, 3);
	appendDialogueLine(lines, text, "          ", "SERGEANT", 0x4203, 2);
	appendLine(lines, "          ACTION clear STATE sergeant.dialogueStateD2d54");
	appendSergeantDay5Exit(lines, text, "      ");
	appendLine(lines, "    ELSE");
	appendDialogueLine(lines, text, "      ", "SERGEANT", 0x41f0, 0);
	appendDialogueLine(lines, text, "      ", "PC", 0x41f6, 0);
	appendDialogueLine(lines, text, "      ", "SERGEANT", 0x41fa, 0);
	appendSergeantDay5Exit(lines, text, "      ");
	appendLine(lines, "  IF STATE sergeant.introPending");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x413f, 2);
	appendDialogueLine(lines, text, "    ", "PC", 0x4144, 0);
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x4145, 0);
	appendDialogueLine(lines, text, "    ", "PC", 0x4146, 0);
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x4148, 2);
	appendResponseMenu(lines, text, "    ", 0x2b7);
	appendLine(lines, "      CHOICE 1");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4153, 0);
	appendLine(lines, "      CHOICE 2");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4157, 1);
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x415d, 0);
	appendResponseMenu(lines, text, "    ", 0x2b8);
	appendLine(lines, "      CHOICE 1");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4168, 1);
	appendDialogueLine(lines, text, "        ", "PC", 0x416c, 0);
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4170, 1);
	appendResponseMenu(lines, text, "        ", 0x2b9);
	appendLine(lines, "          CHOICE 1");
	appendDialogueLine(lines, text, "            ", "SERGEANT", 0x417c, 0);
	appendLine(lines, "          CHOICE 2");
	appendDialogueLine(lines, text, "            ", "SERGEANT", 0x4182, 2);
	appendLine(lines, "      ELSE");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4188, 0);
	appendDialogueLine(lines, text, "        ", "PC", 0x418e, 0);
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x4192, 0);
	appendSergeantDay5Exit(lines, text, "    ");
	appendLine(lines, "  ELSE");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x41ac, 0);
	appendLine(lines, "    IF !FLAG HAVE_LODGE_APP");
	appendDialogueLine(lines, text, "      ", "SERGEANT", 0x41b0, 0);
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x41b4, 3);
	appendResponseMenu(lines, text, "    ", 0x2bb);
	appendLine(lines, "      CHOICE 1");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x41bf, 2);
	appendLine(lines, "        ACTION set STATE sergeant.dialogueStateD2d50");
	appendLine(lines, "      CHOICE 2");
	appendDialogueLine(lines, text, "        ", "SERGEANT", 0x41c4, 0);
	appendSergeantDay5Exit(lines, text, "    ");
	appendLine(lines, "BRANCH item_use");
	appendLine(lines, "  ITEM BARBER_POLE");
	appendLine(lines, "    ACTION restore item BARBER_POLE to owner RAH");
	appendLine(lines, "    ACTION set flag ASSIGNED_DNA_TASK");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x426f, 0);
	appendLine(lines, "  ITEM BOLTCLTH");
	appendLine(lines, "    ACTION restore item BOLTCLTH to owner RAH");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x4241, 0);
	appendLine(lines, "  ITEM COMPLETED_LODGE_APPLICATION");
	appendLine(lines, "    SAME AS no_item / FLAG HAVE_COMPLETED_LODGE_APP branch");
	appendLine(lines, "  ITEM INVITE");
	appendLine(lines, "    SAME AS no_item / STEPHANIE_IS_DEAD invite branch");
	appendLine(lines, "  ITEM REMAINS");
	appendLine(lines, "    SAME AS no_item / GOT_REMAINS_FOR_LODGE branch");
	appendLine(lines, "  ELSE");
	appendDialogueLine(lines, text, "    ", "SERGEANT", 0x42fa, 0);
}

} // End of anonymous namespace

bool buildDialogueDebugDump(const Common::String &npcName, const Text &text,
		Common::Array<Common::String> &lines) {
	lines.clear();

	if (npcName.equalsIgnoreCase("SERGEANT")) {
		buildSergeantDialogueDebugDump(text, lines);
		return true;
	}

	return false;
}

} // End of namespace Harvester
