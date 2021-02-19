#include "common/scummsys.h"
#include "common/str.h"

namespace StarTrek {

struct TypoFix {
	Common::String origText;
	Common::String newText;
};

Common::String patchRoomMessage(const char *text) {
	Common::String txt = text;
	int i = 0;

	TypoFix typoFixes[] = {
	    { "#LOV2\\LOV2_012#", "#LOV1\\LOV1_010#" }, // LOV2_012: Audio file missing
	    { "#LOV3\\LOV3_#", "#LOV3\\LOV3_000#" },       // LOV3_000: Message index missing
		{ "They are in no condition to talk right now", "#LOV4\\LOV4N007#They are in no condition to talk right now." },	// LOV4N007: add missing audio
		{ "#FEA3\\FEA3_030#", "#LOVA\\LOVA_100#" }, // FEA3_030: Wrong voice actor when McCoy says "He's dead, Jim"
		{ "#FEA4\\TX_FEA4N008", "#FEA4\\FEA4_005" },	// FEA4N008: Fix wrong audio
		{ "#TRI3\\TRI3U080", "#TRI1\\TRI1U080" },	// Fix audio with missing filter
	    { "spock", "Spock" },	// LOVA_F08
	    { "sysnthesize", "synthesize" },	// LOVA_F55
	    { "gullability", "gullibility" },	// MUD0_023
	    { "Well, now! I think", "Well, now I think" },	// MUD2_002
	    { "I don't understand enough of the alien's thinking", "I don't understand enough of how the aliens thought," },	// MUD2_014
	    { "to think after all the stunts that Harry has pulled", "to think that after all the stunts that Harry has pulled," },	// MUD3_011
	    { "and they were certain", "and they are certain" },	// MUD3_022
	    { "DId you know", "Did you know" },	// MUD4_008
	    { "before it retreats Captain", "before it retreats, Captain" },	// FEA1_035
	    { "it must have a nasty bite", "it may have a nasty bite" },	// FEA1_041
	    { "he'll be up in about an hour", "he'll be up in about a half hour" },	// FEA3_012
	    { "sHe's dead, Jim!", "He's dead, Jim!" },	// FEA3_030
	    { "those thorns.You might", "those thorns. You might" },	// FEA5_009
	    { "with our phaser not working", "with our phasers not working" },	// FEA5_018
	    { "in a previous life", "in your previous life" },	// FEA5_020
	    { "isn't that just great", "isn't this just great" },	// FEA6_017
	    { "that action, Captain It may", "that action, Captain. It may" },	// FEA6_019
	    { "that attack you", "that attacked you" },	// FEA6N016
	    { "I'm a surgeon not a alien", "I'm a surgeon, not an alien" },	// SIN2_012
	    { "to bypass it's lock system", "to bypass its lock system" },	// SIN4_023
	    { "Sparks explode and", "Sparks fly and" },	// SIN5N012
	    { "the Enterprise!We've", "the Enterprise! We've" },	// TRI0_036
	    { "Male Human-Vulcan", "One male Human-Vulcan" },	// TRI1_025
	    { "with a phaser", "with your phaser" },	// TRI1_048
	    { "Male Human,", "He's a male Human," },	// TRI2_015
	    { "Male Human-Vulcan", "One male Human-Vulcan" },	// TRI2_017
	    { "He's a Male Human", "One male Human" },	// TRI3_013
	    { "Male Human,", "He's a male Human," },	// TRI3_014
	    { "Male Human-Vulcan", "One male Human-Vulcan" },	// TRI3_016
	    { "Captain, come in please!", "Captain, please come in!" },	// TRI3U084
	    { "I didn't want it", "I don't want it" },	// TRI4_003
	    { "a fair trail", "a fair trial" },	// TRI4_024
	    { "what an enemy does not expect", "what the enemy does not expect" },	// TRI4_039
	    { "will believe you", "to believe you" },	// TRI4_057
	    { "at which to transport you", "to which to transport you" },	// TRI5_045
	    { "a beam light", "a beam of light" },	// TRI5N002
	    { "saphire", "sapphire" },	// TRI5N016, TRI5N017, TRI5N018
	    { "a emerald", "an emerald" },	// TRI5N019, TRI5N020, TRI5N021
	    { "torpedo is loaded", "torpedoes are loaded" },	// VEN2_050
	    { "><upon", "upon" },	// VEN6_005
	    { "Its not", "It's not" },	// VEN8_037
	    { "", "" }
	};

	// Fix typos where some messages contain a hyphen instead of an underscore
	// (e.g in LOV2)
	if (txt[10] == '-')
		txt.replace(10, 1, "_");

	// Fix typos where some messages contain double spacing (e.g. in FEA3_020)
	int32 spacePos = txt.find("  ");
	if (spacePos > 0)
		txt.deleteChar(spacePos);

	// Fix typos
	do {
		const Common::String origText = typoFixes[i].origText;
		const Common::String newText = typoFixes[i].newText;

		int32 pos = txt.find(origText);
		if (pos > 0)
			txt.replace(pos, origText.size(), newText, pos, newText.size());

		i++;
	} while (typoFixes[i].origText != "");

	return txt;
}

} // End of namespace StarTrek
