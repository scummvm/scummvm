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

#include "common/config-manager.h"
#include "common/translation.h"

#include "scumm/he/moonbase/map_main.h"
#include "scumm/he/moonbase/dialog-mapgenerator.h"

namespace Scumm {

enum {
	kAlgorCmd = 'ALGR',
	kAlgorRandCmd = 'ALRM',

	kSizeCmd = 'SIZE',
	kSizeRandCmd = 'SZRM',

	kTileCmd = 'TILE',
	kTileRandCmd = 'TLRM',

	kEnergyCmd = 'ENGY',
	kEnergyRandCmd = 'EGRM',

	kTerrainCmd = 'TRIN',
	kTerrainRandCmd = 'TRRM',

	kWaterCmd = 'WTER',
	kWaterRandCmd = 'WTRM',

	kCancelCmd = 'CNCL',
	kGenerateCmd = 'GNRT'
};

MapGeneratorDialog::MapGeneratorDialog(bool demo) : Dialog("MapGenerator"), _refreshing(false) {
	// I18N: Random map generator for Moonbase Commander
	_dialogTitle = new GUI::StaticTextWidget(this, "MapGenerator.Title", _("Random Map Options"));
	_dialogTitle->setAlign(Graphics::kTextAlignCenter);

	// I18N: Map generator algorithms
	_algorDesc = new GUI::StaticTextWidget(this, "MapGenerator.Algorithm", _("Algorithm"));
	_algorDesc->setAlign(Graphics::kTextAlignLeft);
	_algorGroup = new GUI::RadiobuttonGroup(this, kAlgorCmd);
	// I18N: Spiff algorithm
	_algorSpiff = new GUI::RadiobuttonWidget(this, "MapGenerator.AlgorithmSpiff", _algorGroup, SPIFF_GEN, _("Spiff"));
	// I18N: Katton algorithm
	_algorSpiff = new GUI::RadiobuttonWidget(this, "MapGenerator.AlgorithmKatton", _algorGroup, KATTON_GEN, _("Katton"));
	// I18N: Random algorithm
	_algorRandom = new GUI::CheckboxWidget(this, "MapGenerator.AlgorithmRandom", _("Random"), _("Picks the map algorithm randomly."), kAlgorRandCmd);

	// I18N: Map sizes
	_sizeDesc = new GUI::StaticTextWidget(this, "MapGenerator.Size", _("Size"));
	_sizeDesc->setAlign(Graphics::kTextAlignCenter);

	_sizeGroup = new GUI::RadiobuttonGroup(this, kSizeCmd);
	_sizeSmall = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeSmall", _sizeGroup, 4, _("Small"));
	_sizeMedium = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeMedium", _sizeGroup, 5, _("Medium"));
	_sizeLarge = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeLarge", _sizeGroup, 6, _("Large"));
	_sizeHuge = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeHuge", _sizeGroup, 7, _("Huge"));
	_sizeSAI = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeSAI", _sizeGroup, 8, _("SAI"));
	_sizeRidiculous = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeRidiculous", _sizeGroup, 9, _("Ridiculous"));
	_sizeMax = new GUI::RadiobuttonWidget(this, "MapGenerator.SizeMax", _sizeGroup, 10, _("Max"));
	// I18N: Random map size
	_sizeRandom = new GUI::CheckboxWidget(this, "MapGenerator.SizeRandom", _("Random"), _("Picks the map size randomly."), kSizeRandCmd);

	// I18N: Map tilesets
	_tileDesc = new GUI::StaticTextWidget(this, "MapGenerator.Tileset", _("Tileset"));
	_tileDesc->setAlign(Graphics::kTextAlignCenter);
	_tileGroup = new GUI::RadiobuttonGroup(this, kTileCmd);
	_tileTerrandra = new GUI::RadiobuttonWidget(this, "MapGenerator.TilesetTerrandra", _tileGroup, 1, Common::U32String("Terrandra"));
	_tileZanateros = new GUI::RadiobuttonWidget(this, "MapGenerator.TilesetZanateros", _tileGroup, 2, Common::U32String("Zanateros"));
	// Demo version of the game only has tilesets 1, 2, 4 and 6.  Don't create buttons for
	// missing tiles.
	if (!demo)
		_tileDrijim = new GUI::RadiobuttonWidget(this, "MapGenerator.TilesetDaijim", _tileGroup, 3, Common::U32String("Daijim 3"));
	_tileKyanite = new GUI::RadiobuttonWidget(this, "MapGenerator.TilesetKyanite", _tileGroup, 4, Common::U32String("Kyanite"));
	if (!demo)
		_tileEmerau = new GUI::RadiobuttonWidget(this, "MapGenerator.TilesetEmerau", _tileGroup, 5, Common::U32String("Emerau Glyph"));
	_tileAblation = new GUI::RadiobuttonWidget(this, "MapGenerator.TilesetAblation", _tileGroup, 6, Common::U32String("Ablation Land"));
	// I18N: Random tileset
	_tileRandom = new GUI::CheckboxWidget(this, "MapGenerator.TilesetRandom", _("Random"), _("Picks the map tileset randomly."), kTileRandCmd);

	// I18N: Percentage of energy pools
	_energyDesc = new GUI::StaticTextWidget(this, "MapGenerator.Energy", _("Energy"));
	_energyDesc->setAlign(Graphics::kTextAlignCenter);
	_energySlider = new GUI::SliderWidget(this, "MapGenerator.EnergySlider", Common::U32String(), kEnergyCmd);
	_energySlider->setMinValue(0); _energySlider->setMaxValue(6);
	// I18N: Energy slider label
	_energyLabel = new GUI::StaticTextWidget(this, "MapGenerator.EnergySliderLabel", _("Scarce - Lots"), Common::U32String());
	_energyLabel->setAlign(Graphics::kTextAlignCenter);


	// I18N: Random percentage of energy pools
	_energyRandom = new GUI::CheckboxWidget(this, "MapGenerator.EnergyRandom", _("Random"), _("Picks the random amount of energy pools."), kEnergyRandCmd);

	// I18N: Percentage of terrain
	_terrainDesc = new GUI::StaticTextWidget(this, "MapGenerator.Terrain", _("Terrain"));
	_terrainDesc->setAlign(Graphics::kTextAlignCenter);
	_terrainSlider = new GUI::SliderWidget(this, "MapGenerator.TerrainSlider", Common::U32String(), kTerrainCmd);
	_terrainSlider->setMinValue(0); _terrainSlider->setMaxValue(6);
	// I18N: Terrain slider label
	_terrainLabel = new GUI::StaticTextWidget(this, "MapGenerator.TerrainSliderLabel", _("Barren - Rough"), Common::U32String());
	_terrainLabel->setAlign(Graphics::kTextAlignCenter);

	// I18N: Random percentage of terrain
	_terrainRandom = new GUI::CheckboxWidget(this, "MapGenerator.TerrainRandom", _("Random"), _("Picks the random amount of terrain level."), kTerrainRandCmd);

	// I18N: Percentage of water
	_waterDesc = new GUI::StaticTextWidget(this, "MapGenerator.Water", _("Water"));
	_waterDesc->setAlign(Graphics::kTextAlignCenter);
	_waterSlider = new GUI::SliderWidget(this, "MapGenerator.WaterSlider", Common::U32String(), kWaterCmd);
	_waterSlider->setMinValue(0); _waterSlider->setMaxValue(6);
	// I18N: Water slider label
	_waterLabel = new GUI::StaticTextWidget(this, "MapGenerator.WaterSliderLabel", _("Driest - Wettest"), Common::U32String());
	_waterLabel->setAlign(Graphics::kTextAlignCenter);

	// I18N: Random percentage of water
	_waterRandom = new GUI::CheckboxWidget(this, "MapGenerator.WaterRandom", _("Random"), _("Picks the random amount of water."), kWaterRandCmd);

	_cancelButton = new GUI::ButtonWidget(this, "MapGenerator.Cancel", _("Cancel"), Common::U32String(), kCancelCmd);
	// I18N: Generate new map
	_generateButton = new GUI::ButtonWidget(this, "MapGenerator.Generate", _("Generate"), Common::U32String(), kGenerateCmd);
	refresh();
}

void MapGeneratorDialog::refresh() {
	_refreshing = true;

	//  ALGORITHM
	bool randomAlgorithm = true;
	if (ConfMan.hasKey("map_algorithm"))
		randomAlgorithm = ConfMan.getInt("map_algorithm") == 0;

	_algorGroup->setEnabled(!randomAlgorithm);
	_algorRandom->setState(randomAlgorithm);

	if (!randomAlgorithm)
		_algorGroup->setValue(ConfMan.getInt("map_algorithm"));

	// SIZE
	bool randomSize = true;
	if (ConfMan.hasKey("map_size"))
		randomSize = ConfMan.getInt("map_size") == 0;

	_sizeGroup->setEnabled(!randomSize);
	_sizeRandom->setState(randomSize);

	if (!randomSize)
		_sizeGroup->setValue(ConfMan.getInt("map_size"));

	// TILESET
	bool randomTileset = true;
	if (ConfMan.hasKey("map_tileset"))
		randomTileset = ConfMan.getInt("map_tileset") == 0;

	_tileGroup->setEnabled(!randomTileset);
	_tileRandom->setState(randomTileset);

	if (!randomTileset)
		_tileGroup->setValue(ConfMan.getInt("map_tileset"));

	// ENERGY
	bool randomEnergy = true;
	if (ConfMan.hasKey("map_energy"))
		randomEnergy = ConfMan.getInt("map_energy") == -1;

	_energySlider->setEnabled(!randomEnergy);
	_energyRandom->setState(randomEnergy);

	if (!randomEnergy)
		_energySlider->setValue(ConfMan.getInt("map_energy"));
	else if (ConfMan.hasKey("prev_map_energy"))
		_energySlider->setValue(ConfMan.getInt("prev_map_energy"));
	else
		_energySlider->setValue(3);

	// TERRAIN
	bool randomTerrain = true;
	if (ConfMan.hasKey("map_terrain"))
		randomTerrain = ConfMan.getInt("map_terrain") == -1;

	_terrainSlider->setEnabled(!randomTerrain);
	_terrainRandom->setState(randomTerrain);

	if (!randomTerrain)
		_terrainSlider->setValue(ConfMan.getInt("map_terrain"));
	else if (ConfMan.hasKey("prev_map_terrain"))
		_terrainSlider->setValue(ConfMan.getInt("prev_map_terrain"));
	else
		_terrainSlider->setValue(3);


	// WATER
	bool randomWater = true;
	if (ConfMan.hasKey("map_water"))
		randomWater = ConfMan.getInt("map_water") == -1;

	_waterSlider->setEnabled(!randomWater);
	_waterRandom->setState(randomWater);

	if (!randomWater)
		_waterSlider->setValue(ConfMan.getInt("map_water"));
	else if (ConfMan.hasKey("prev_map_water"))
		_waterSlider->setValue(ConfMan.getInt("prev_map_water"));
	else
		_waterSlider->setValue(3);

	drawDialog(GUI::kDrawLayerForeground);

	_refreshing = false;
}

void MapGeneratorDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (_refreshing)
		return;

	switch(cmd) {
	case kAlgorCmd:
		ConfMan.setInt("map_algorithm", data);
		break;
	case kAlgorRandCmd:
		if (data == 1) {
			if (ConfMan.hasKey("map_algorithm") && ConfMan.getInt("map_algorithm") != 0)
				// Store previous value
				ConfMan.setInt("prev_map_algorithm", ConfMan.getInt("map_algorithm"));
			ConfMan.setInt("map_algorithm", 0);
		} else {
			// Restore previous value
			int previousValue = SPIFF_GEN;
			if (ConfMan.hasKey("prev_map_algorithm"))
				previousValue = ConfMan.getInt("prev_map_algorithm");
			ConfMan.setInt("map_algorithm", previousValue);
		}

		refresh();
		break;
	case kSizeCmd:
		ConfMan.setInt("map_size", data);
		break;
	case kSizeRandCmd:
		if (data == 1) {
			if (ConfMan.hasKey("map_size") && ConfMan.getInt("map_size") != 0)
				// Store previous value
				ConfMan.setInt("prev_map_size", ConfMan.getInt("map_size"));
			ConfMan.setInt("map_size", 0);
		} else {
			// Restore previous value
			int previousValue = 4;
			if (ConfMan.hasKey("prev_map_size"))
				previousValue = ConfMan.getInt("prev_map_size");
			ConfMan.setInt("map_size", previousValue);
		}

		refresh();
		break;
	case kTileCmd:
		ConfMan.setInt("map_tileset", data);
		break;
	case kTileRandCmd:
		if (data == 1) {
			if (ConfMan.hasKey("map_tileset") && ConfMan.getInt("map_tileset") != 0)
				// Store previous value
				ConfMan.setInt("prev_map_tileset", ConfMan.getInt("map_tileset"));
			ConfMan.setInt("map_tileset", 0);
		} else {
			// Restore previous value
			int previousValue = 5;
			if (ConfMan.hasKey("prev_map_tileset"))
				previousValue = ConfMan.getInt("prev_map_tileset");
			ConfMan.setInt("map_tileset", previousValue);
		}

		refresh();
		break;
	case kEnergyCmd:
		ConfMan.setInt("map_energy", data);
		break;
	case kEnergyRandCmd:
		if (data == 1) {
			if (ConfMan.hasKey("map_energy") && ConfMan.getInt("map_energy") != -1)
				// Store previous value
				ConfMan.setInt("prev_map_energy", ConfMan.getInt("map_energy"));
			ConfMan.setInt("map_energy", -1);
		} else {
			// Restore previous value
			int previousValue = 3;
			if (ConfMan.hasKey("prev_map_energy"))
				previousValue = ConfMan.getInt("prev_map_energy");
			ConfMan.setInt("map_energy", previousValue);
		}

		refresh();
		break;
	case kTerrainCmd:
		ConfMan.setInt("map_terrain", data);
		break;
	case kTerrainRandCmd:
		if (data == 1) {
			if (ConfMan.hasKey("map_terrain") && ConfMan.getInt("map_terrain") != -1)
				// Store previous value
				ConfMan.setInt("prev_map_terrain", ConfMan.getInt("map_terrain"));
			ConfMan.setInt("map_terrain", -1);
		} else {
			// Restore previous value
			int previousValue = 3;
			if (ConfMan.hasKey("prev_map_terrain"))
				previousValue = ConfMan.getInt("prev_map_terrain");
			ConfMan.setInt("map_terrain", previousValue);
		}

		refresh();
		break;
	case kWaterCmd:
		ConfMan.setInt("map_water", data);
		break;
	case kWaterRandCmd:
		if (data == 1) {
			if (ConfMan.hasKey("map_water") && ConfMan.getInt("map_water") != -1)
				// Store previous value
				ConfMan.setInt("prev_map_water", ConfMan.getInt("map_water"));
			ConfMan.setInt("map_water", -1);
		} else {
			// Restore previous value
			int previousValue = 3;
			if (ConfMan.hasKey("prev_map_water"))
				previousValue = ConfMan.getInt("prev_map_water");
			ConfMan.setInt("map_water", previousValue);
		}

		refresh();
		break;
	case kCancelCmd:
		setResult(0);
		close();
		break;
	case kGenerateCmd:
		ConfMan.flushToDisk();
		setResult(1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void MapGeneratorDialog::handleKeyDown(Common::KeyState state) {
	switch (state.keycode) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		ConfMan.flushToDisk();
		setResult(1);
		close();
		break;
	default:
		Dialog::handleKeyDown(state);
	}
}

} // End of namespace Scumm
