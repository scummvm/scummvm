#include "dm.h"
#include "gfx.h"
#include "champion.h"



namespace DM {

#define kChampionStatusBoxSpacing 69 // @ C69_CHAMPION_STATUS_BOX_SPACING


enum PanelContent {
	kPanelContentFoodWaterPoisoned = 0, // @ C00_PANEL_FOOD_WATER_POISONED
	kPanelContentScroll = 2, // @ C02_PANEL_SCROLL
	kPanelContentChest = 4, // @ C04_PANEL_CHEST
	kPanelContentResurrectReincarnate = 5 // @ C05_PANEL_RESURRECT_REINCARNATE
};



class InventoryMan {
	DMEngine *_vm;
public:
	int16 _inventoryChampionOrdinal; // @ G0423_i_InventoryChampionOrdinal
	PanelContent _panelContent; // @ G0424_i_PanelContent 
	InventoryMan(DMEngine *vm);
	void toggleInventory(ChampionIndex championIndex); // @ F0355_INVENTORY_Toggle_CPSE
	void drawStatusBoxPortrait(ChampionIndex championIndex); // @ F0354_INVENTORY_DrawStatusBoxPortrait
	void drawPanelHorizontalBar(int16 x, int16 y, int16 pixelWidth, Color color); // @ F0343_INVENTORY_DrawPanel_HorizontalBar
	void drawPanelFoodOrWaterBar(int16 amount, int16 y, Color color); // @ F0344_INVENTORY_DrawPanel_FoodOrWaterBar 
	void drawPanelFoodWaterPoisoned(); // @ F0345_INVENTORY_DrawPanel_FoodWaterPoisoned
	void drawPanelResurrectReincarnate(); // @ F0346_INVENTORY_DrawPanel_ResurrectReincarnate
	void drawPanel(); // @ F0347_INVENTORY_DrawPanel

};

}
