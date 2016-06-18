#include "champion.h"


namespace DM {

ChampionMan::ChampionMan(DMEngine *vm): _vm(vm) {}

ChampionIndex ChampionMan::getIndexInCell(int16 mapX, int16 mapY, ViewCell cell) {
	return kChampionNone;
}

}