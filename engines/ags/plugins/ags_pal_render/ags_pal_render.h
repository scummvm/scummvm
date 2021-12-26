/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_PAL_RENDER_AGS_PAL_RENDER_H
#define AGS_PLUGINS_AGS_PAL_RENDER_AGS_PAL_RENDER_H

#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/serializer.h"

namespace AGS3 {
namespace Plugins {
namespace AGSPalRender {

class AGSPalRender : public PluginBase {
	SCRIPT_HASH(AGSPalRender)
private:
	void syncGame(Serializer &s);
public:
	AGSPalRender() : PluginBase() {}
	virtual ~AGSPalRender() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *lpEngine) override;
	void AGS_EngineShutdown() override;
	int64 AGS_EngineOnEvent(int event, NumberPtr data) override;

	/**
	 * @defgroup PALInternal
	 * @{
	 */

	void LoadCLUT(ScriptMethodParams &params);
	void CycleRemap(ScriptMethodParams &params);
	void GetColor565(ScriptMethodParams &params);
	void GetLuminosityFromPalette(ScriptMethodParams &params);
	void AGSFastSin(ScriptMethodParams &params);
	void AGSFastCos(ScriptMethodParams &params);
	void AGSFastRoot(ScriptMethodParams &params);
	void GetRemappedSlot(ScriptMethodParams &params);
	void ResetRemapping(ScriptMethodParams &params);
	void GetModifiedBackgroundImage(ScriptMethodParams &params);
	void WriteObjectivePalette(ScriptMethodParams &params);
	void ReadObjectivePaletteR(ScriptMethodParams &params);
	void ReadObjectivePaletteB(ScriptMethodParams &params);
	void ReadObjectivePaletteG(ScriptMethodParams &params);

	/**@}*/

	/**
	 * @defgroup LensDistort
	 * @{
	 */

	void SetLensPos(ScriptMethodParams &params);
	void GetLensX(ScriptMethodParams &params);
	void GetLensY(ScriptMethodParams &params);
	void SetLensDrawn(ScriptMethodParams &params);
	void GetLensDrawn(ScriptMethodParams &params);
	void SetLensOffsetClamp(ScriptMethodParams &params);
	void GetLensOffsetClamp(ScriptMethodParams &params);
	void GetLensLevel(ScriptMethodParams &params);
	void SetLensLevel(ScriptMethodParams &params);
	void LensInitialize(ScriptMethodParams &params);

	/**@}*/

	/**
	 * @defgroup Translucence
	 * @{
	 */

	void CreateTranslucentOverlay(ScriptMethodParams &params);
	void DeleteTranslucentOverlay(ScriptMethodParams &params);
	void MoveTranslucentOverlay(ScriptMethodParams &params);
	void GetTranslucentOverlayX(ScriptMethodParams &params);
	void GetTranslucentOverlayY(ScriptMethodParams &params);
	void GetTranslucentOverlaySprite(ScriptMethodParams &params);
	void GetTranslucentOverlayLevel(ScriptMethodParams &params);
	void GetTranslucentOverlayEnabled(ScriptMethodParams &params);
	void GetTranslucentOverlayAlpha(ScriptMethodParams &params);
	void SetTranslucentOverlayAlpha(ScriptMethodParams &params);
	void SetTranslucentOverlayEnabled(ScriptMethodParams &params);
	void DrawTransSprite(ScriptMethodParams &params);

	 /**@}*/

	/**
	 * @defgroup Starfield
	 * @{
	 */

	void GetStarfieldOverscan(ScriptMethodParams &params);
	void SetStarfieldOverscan(ScriptMethodParams &params);
	void GetStarfieldOriginX(ScriptMethodParams &params);
	void GetStarfieldOriginY(ScriptMethodParams &params);
	void SetStarfieldDepthMultiplier(ScriptMethodParams &params);
	void GetStarfieldDepthMultiplier(ScriptMethodParams &params);
	void GetStarfieldMaxStars(ScriptMethodParams &params);
	void SetStarSpriteScaleBoost(ScriptMethodParams &params);
	void GetStarSpriteScaleBoost(ScriptMethodParams &params);
	void SetStarMaxRadius(ScriptMethodParams &params);
	void GetStarMaxRadius(ScriptMethodParams &params);
	void GetStarX(ScriptMethodParams &params);
	void GetStarY(ScriptMethodParams &params);
	void GetStarZ(ScriptMethodParams &params);
	void SetStarPosition(ScriptMethodParams &params);
	void RotateStar(ScriptMethodParams &params);
	void SetStarColor(ScriptMethodParams &params);
	void GetStarColor(ScriptMethodParams &params);
	void SetStarSprite(ScriptMethodParams &params);
	void GetStarSprite(ScriptMethodParams &params);
	void SetStarSpriteRange(ScriptMethodParams &params);
	void InitializeStars(ScriptMethodParams &params);
	void IterateStars(ScriptMethodParams &params);
	void DrawStars(ScriptMethodParams &params);
	void SetStarsOriginPoint(ScriptMethodParams &params);

	 /**@}*/

	/**
	 * @defgroup Plasma
	 * @{
	 */

	void DoFire(ScriptMethodParams &params);
	void SetPlasmaType(ScriptMethodParams &params);
	void ResetPlasmaSettings(ScriptMethodParams &params);
	void DrawPlasma(ScriptMethodParams &params);
	void SetPlasmaRootType(ScriptMethodParams &params);
	void GetPlasmaRootType(ScriptMethodParams &params);

	 /**@}*/

	/**
	 * @defgroup Reflections
	 * @{
	 */

	void SetReflections(ScriptMethodParams &params);
	void IsReflectionsOn(ScriptMethodParams &params);
	void SetCharacterReflected(ScriptMethodParams &params);
	void GetCharacterReflected(ScriptMethodParams &params);
	void SetObjectReflected(ScriptMethodParams &params);
	void GetObjectReflected(ScriptMethodParams &params);
	void ReplaceCharacterReflectionView(ScriptMethodParams &params);
	void SetObjectReflectionIgnoreScaling(ScriptMethodParams &params);

	 /**@}*/

	/**
	 * @defgroup raycast
	 * @{
	 */

	void MakeTextures(ScriptMethodParams &params);
	void Raycast_Render(ScriptMethodParams &params);
	void MoveForward(ScriptMethodParams &params);
	void MoveBackward(ScriptMethodParams &params);
	void RotateLeft(ScriptMethodParams &params);
	void RotateRight(ScriptMethodParams &params);
	void Init_Raycaster(ScriptMethodParams &params);
	void QuitCleanup(ScriptMethodParams &params);
	void LoadMap(ScriptMethodParams &params);
	void Ray_InitSprite(ScriptMethodParams &params);
	void Ray_SetPlayerPosition(ScriptMethodParams &params);
	void Ray_GetPlayerX(ScriptMethodParams &params);
	void Ray_GetPlayerY(ScriptMethodParams &params);
	void Ray_GetPlayerAngle(ScriptMethodParams &params);
	void Ray_SetPlayerAngle(ScriptMethodParams &params);

	void Ray_GetWallHotspot(ScriptMethodParams &params);
	void Ray_GetWallTexture(ScriptMethodParams &params);
	void Ray_GetWallSolid(ScriptMethodParams &params);
	void Ray_GetWallIgnoreLighting(ScriptMethodParams &params);
	void Ray_GetWallAlpha(ScriptMethodParams &params);
	void Ray_GetWallBlendType(ScriptMethodParams &params);

	void Ray_SelectTile(ScriptMethodParams &params);

	void Ray_GetHotspotAt(ScriptMethodParams &params);
	void Ray_GetObjectAt(ScriptMethodParams &params);

	void Ray_DrawTile(ScriptMethodParams &params);
	void Ray_DrawOntoTile(ScriptMethodParams &params);
	void Ray_SetNoClip(ScriptMethodParams &params);
	void Ray_GetNoClip(ScriptMethodParams &params);
	void Ray_SetSpriteInteractObj(ScriptMethodParams &params);
	void  Ray_GetSpriteInteractObj(ScriptMethodParams &params);
	void Ray_SetSpritePosition(ScriptMethodParams &params);
	void Ray_SetSpriteVertOffset(ScriptMethodParams &params);
	void Ray_GetSpriteVertOffset(ScriptMethodParams &params);
	void Ray_GetSpriteX(ScriptMethodParams &params);
	void Ray_GetSpriteY(ScriptMethodParams &params);

	void Ray_SetWallHotspot(ScriptMethodParams &params);
	void Ray_SetWallTextures(ScriptMethodParams &params);
	void Ray_SetWallSolid(ScriptMethodParams &params);
	void Ray_SetWallIgnoreLighting(ScriptMethodParams &params);
	void Ray_SetWallAlpha(ScriptMethodParams &params);
	void Ray_SetWallBlendType(ScriptMethodParams &params);

	void Ray_GetMoveSpeed(ScriptMethodParams &params);
	void Ray_SetMoveSpeed(ScriptMethodParams &params);
	void Ray_GetRotSpeed(ScriptMethodParams &params);
	void Ray_SetRotSpeed(ScriptMethodParams &params);
	void Ray_GetWallAt(ScriptMethodParams &params);
	void Ray_GetLightAt(ScriptMethodParams &params);
	void Ray_SetLightAt(ScriptMethodParams &params);
	void Ray_SetWallAt(ScriptMethodParams &params);
	void Ray_GetPlaneY(ScriptMethodParams &params);
	void Ray_SetPlaneY(ScriptMethodParams &params);
	void Ray_GetDistanceAt(ScriptMethodParams &params);
	void Ray_GetSpriteAngle(ScriptMethodParams &params);
	void Ray_SetSpriteAngle(ScriptMethodParams &params);
	void Ray_SetSpriteView(ScriptMethodParams &params);
	void Ray_GetSpriteView(ScriptMethodParams &params);
	void Ray_SetSpriteFrame(ScriptMethodParams &params);
	void Ray_GetSpriteFrame(ScriptMethodParams &params);

	void Ray_GetTileX_At(ScriptMethodParams &params);
	void Ray_GetTileY_At(ScriptMethodParams &params);

	void Ray_SetSkyBox(ScriptMethodParams &params);
	void Ray_GetSkyBox(ScriptMethodParams &params);

	void Ray_SetAmbientLight(ScriptMethodParams &params);
	void Ray_GetAmbientLight(ScriptMethodParams &params);
	void Ray_SetAmbientColor(ScriptMethodParams &params);


	void Ray_GetSpriteAlpha(ScriptMethodParams &params);
	void Ray_SetSpriteAlpha(ScriptMethodParams &params);
	void Ray_GetSpritePic(ScriptMethodParams &params);
	void Ray_SetSpritePic(ScriptMethodParams &params);

	void Ray_GetSpriteScaleX(ScriptMethodParams &params);
	void Ray_SetSpriteScaleX(ScriptMethodParams &params);
	void Ray_GetSpriteScaleY(ScriptMethodParams &params);
	void Ray_SetSpriteScaleY(ScriptMethodParams &params);

	void Ray_SetSpriteBlendType(ScriptMethodParams &params);
	void Ray_GetSpriteBlendType(ScriptMethodParams &params);

	void Ray_SetFloorAt(ScriptMethodParams &params);
	void Ray_SetCeilingAt(ScriptMethodParams &params);
	void Ray_GetCeilingAt(ScriptMethodParams &params);
	void Ray_GetFloorAt(ScriptMethodParams &params);
	void Ray_GetLightingAt(ScriptMethodParams &params);
	void Ray_SetLightingAt(ScriptMethodParams &params);
	void Ray_GetAmbientWeight(ScriptMethodParams &params);

	void Ray_HasSeenTile(ScriptMethodParams &params);

	/**@}*/
};

} // namespace AGSPalRender
} // namespace Plugins
} // namespace AGS3

#endif
