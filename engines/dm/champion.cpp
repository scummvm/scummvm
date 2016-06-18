#include "champion.h"


namespace DM {

ChampionMan::ChampionMan(DMEngine *vm) : _vm(vm) {
	_leaderIndex = kChampionNone;
}

uint16 ChampionMan::getChampionPortraitX(uint16 index) {
	return ((index) & 0x7) << 5;
}

uint16 ChampionMan::getChampionPortraitY(uint16 index) {
	return ((index) >> 3) * 29;
}

int16 ChampionMan::getDecodedValue(char *string, uint16 characterCount) {
	int val = 0;
	for (uint16 i = 0; i < characterCount; ++i) {
		val = (val << 4) + (string[i] - 'A');
	}
	return val;
}

ChampionIndex ChampionMan::getIndexInCell(ViewCell cell) {
	for (uint16 i = 0; i < _partyChampionCount; ++i) {
		if ((_champions[i]._cell == cell) && _champions[i]._currHealth)
			return (ChampionIndex)i;
	}

	return kChampionNone;
}

}