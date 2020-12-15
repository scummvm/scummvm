/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_SET_PC_H
#define ICB_SET_PC_H

#include "engines/icb/p4_generic.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/rlp_api.h"
#include "engines/icb/common/pc_props.h"
#include "engines/icb/common/px_staticlayers.h"

namespace ICB {

#define WEATHER_SCREEN_WIDTH SCREEN_WIDTH
#define WEATHER_HALF_SCREEN_WIDTH (WEATHER_SCREEN_WIDTH / 2)
#define WEATHER_SCREEN_HEIGHT SCREEN_DEPTH
#define WEATHER_HALF_SCREEN_HEIGHT (WEATHER_SCREEN_HEIGHT / 2)
#define WEATHER_SCREEN_DEPTH 512
#define WEATHER_HALF_SCREEN_DEPTH (WEATHER_SCREEN_DEPTH / 2)
#define WEATHER_MAX_PARTICLES 256

#define MAX_PROP_STATES 512
#define MAX_CAMERA_PROPS 16

#define _MAX_SHADOW_LAYERS_ 4
#define WEATHER_NONE 0
#define WEATHER_RAIN 1
#define WEATHER_SNOW 2

extern uint32 bg_buffer_id;
extern LRECT full_rect;

class _set {
	friend void sv_WeatherControls();

public:
	_set();
	~_set();

public:
	bool8 Init(const char *camera_name, const char *h_camera_name); // camera name
	void Init_construction_matrices();
	void Init_base_bitmap_buffers();
	void ReInit();

private:
	void Load_props();
	void Load_props_old();

public:
	inline PXcamera &GetCamera();
	inline rlp_API *GetPRig();
	inline pcStaticLayers *GetStaticLayers();
	inline pcPropFile *GetProps();
	inline uint8 *GetBackground();
	inline uint32 GetPropQty();
	inline const char *GetPropName(uint32 n);
	inline uint32 GetPropRange(uint32 n);
	bool8 DoesPropExist(const char *propName);

	int OK() { return m_setOk; };
	void Reset();
	bool8 DoesCameraExist(const char *camera_name, const char *camera_cluster_name);

	/* Save game compliance */
	void LoadGFXInfo(Common::SeekableReadStream *stream);
	void SaveGFXInfo(Common::WriteStream *stream);

private:
	void HackMakeCamera();

public: /* Drawing Functions */
	void RefreshBackdrop();
	void Set_draw(bool8 helper);

	const char *GetSetName() const { return set_name; }

private:                       /* Member variables .... Move this to private section */
	char set_name[MAXPATHLEN];    // the camera name (no url)
	char set_url[MAXPATHLEN];     // the full url of the camera
	uint32 set_url_hash;         // the hashed url of the camera
	char set_cluster[MAXPATHLEN]; // cluster file name
	uint32 set_cluster_hash;     // hashed cluster file name
	/* Waiting for removal */
	char h_set_name[MAXPATHLEN]; // just the hash camera name - not full url
	char rvpcm_file_name[MAXPATHLEN]; // Props & Construction Matrix
	uint32 rvpcm_file_hash;          // Props & Construction Matrix hash value
	char rvcam_file_name[MAXPATHLEN]; // Camera
	uint32 rvcam_file_hash;          // Camera hash value
	char rvsl_file_name[MAXPATHLEN];  // Static Layers
	uint32 rvsl_file_hash;           // Static Layers hash value
	char rvbg_file_name[MAXPATHLEN];  // Background
	uint32 rvbg_file_hash;           // Background hash value
	char rlx_file_name[MAXPATHLEN];   // Poly Lighting rig
	uint32 rlx_file_hash;            // Poly Lighting rig hash
	void SettleCamera();
	/* Waiting for removal */

public: /* _ WEATHER _ RAIN _ SNOW _ LIGHTNING _ SPARKLE */
	void DrawSparkles();
	void DrawWeather();
	void InitWeather(int type, int particleQty, int lightning, int windX, int windY, int windZ);

private:
	int16 m_wParticleX[WEATHER_MAX_PARTICLES]; // -320 - 320
	int16 m_wParticleY[WEATHER_MAX_PARTICLES]; // -240 - 240
	int16 m_wParticleZ[WEATHER_MAX_PARTICLES]; // -512 - 512
	int8 m_wParticleDX[WEATHER_MAX_PARTICLES];
	int8 m_wParticleDY[WEATHER_MAX_PARTICLES];
	int8 m_wParticleDZ[WEATHER_MAX_PARTICLES];
	int m_wLightningTimer;
	int m_wType;
	int m_wWindX;
	int m_wWindY;
	int m_wWindZ;
	int m_wParticleQty;
	int m_wLightning;

private:
	int m_setOk;                   // Is The Set OK flag
	PXcamera m_camera;             // The camera
	_pcSetHeader *m_currentCamera; // All the camera data

	pcPropFile *m_props;

public: /* Prop Surfaces */
	uint32 m_propSurfaceIds[MAX_CAMERA_PROPS];
	int m_propSurfaces[MAX_PROP_STATES];
	int m_propResolutions[MAX_CAMERA_PROPS];
	int m_TotalPropSurfaces;
};

inline rlp_API *_set::GetPRig() { return (rlp_API *)(((uint8 *)m_currentCamera) + m_currentCamera->lightOffset); }

inline pcStaticLayers *_set::GetStaticLayers() { return (pcStaticLayers *)(((uint8 *)m_currentCamera) + m_currentCamera->layerOffset); }

inline pcPropFile *_set::GetProps() { return m_props; }

inline uint8 *_set::GetBackground() { return (uint8 *)(((uint8 *)m_currentCamera) + m_currentCamera->backgroundOffset); }

inline PXcamera &_set::GetCamera() { return m_camera; }

inline uint32 _set::GetPropQty() {
	// Load the prop file
	pcPropFile *propFile = GetProps();
	return propFile->GetPropQty();
}

inline const char *_set::GetPropName(uint32 n) {
	// Load the prop file
	pcPropFile *propFile = GetProps();
	// Get the name of prop n
	return propFile->GetProp(n)->GetName();
}

inline uint32 _set::GetPropRange(uint32 n) {
	// Load the prop file
	pcPropFile *propFile = GetProps();
	// Get the name of prop n
	return propFile->GetProp(n)->GetStateQty();
}

} // End of namespace ICB

#endif
