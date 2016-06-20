#include "dm.h"
#include "gfx.h"
#include "champion.h"



namespace DM {

#define kChampionStatusBoxSpacing 69 // @ C69_CHAMPION_STATUS_BOX_SPACING

class InventoryMan {
	DMEngine *_vm;
public:
	int16 _inventoryChampionOrdinal; // @ G0423_i_InventoryChampionOrdinal
	InventoryMan(DMEngine *vm);
	void toggleInventory(ChampionIndex championIndex); // @ F0355_INVENTORY_Toggle_CPSE
	void drawStatusBoxPortrait(ChampionIndex championIndex); // @ F0354_INVENTORY_DrawStatusBoxPortrait
	void drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color); // @ F0343_INVENTORY_DrawPanel_HorizontalBar
	void drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color); // @ F0344_INVENTORY_DrawPanel_FoodOrWaterBar 
};

}