#include "dm.h"
#include "gfx.h"
#include "champion.h"



namespace DM {

class InventoryMan {
	DMEngine *_vm;
public:
	int16 _inventoryChampionOrdinal; // @ G0423_i_InventoryChampionOrdinal
	InventoryMan(DMEngine *vm);
	void toggleInventory(ChampionIndex championIndex); // @ F0355_INVENTORY_Toggle_CPSE
};

}