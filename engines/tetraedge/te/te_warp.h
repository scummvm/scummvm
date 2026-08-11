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

#ifndef TETRAEDGE_TE_TE_WARP_H
#define TETRAEDGE_TE_TE_WARP_H

#include "common/file.h"

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_frustum.h"
#include "tetraedge/te/te_pick_mesh.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_warp_marker.h"
#include "tetraedge/te/te_warp_bloc.h"

namespace Tetraedge {

// Note: Only used in Amerzone
class TeWarp : public Te3DObject2 {
public:
	struct FrameData {
		Common::Array<TeWarpBloc> _warpBlocs;
		TePickMesh _pickMesh;
		int _loadedTexCount;
		// Note: dropped "minblock" param from original as it's only
		// ever set to 0
		void loadTextures(const TeFrustum &frustum, Common::SeekableReadStream &file, const Common::String &fileType);
		void unloadTextures();
	};

	class AnimData {
	public:
		AnimData() : _fps(15.0f), _enabled(false), _curFrameNo(0), _repCount(0), _firstFrameNo(0), _endFrameNo(0) {}
		Common::Array<unsigned long> _markerIds;
		float _fps;
		int _repCount;
		int _firstFrameNo;
		int _endFrameNo;
		int _curFrameNo;
		bool _enabled;
		TeTimer _timer;
		Common::String _name;
		Common::Array<FrameData> _frameDatas;
	};

	struct Block {
		TeWarpBloc::CubeFace _face;
		int _x;
		int _y;
		TeVector2s32 _offset;
	};

	struct Exit {
		Exit() : _camAngleX(0), _camAngleY(0), _markerId(0) {}
		Common::String _name;
		Common::String _linkedWarpPath;
		float _camAngleX;
		float _camAngleY;
		long _markerId;
		Common::List<Block> _warpBlockList;
	};

	TeWarp();
	~TeWarp();

	void activeMarkers(bool active);
	uint addQuadToPickMesh(TePickMesh &pickmesh, uint trinum, TeWarpBloc::CubeFace face,
		const TeVector2s32 &param_4, uint param_5, uint param_6);
	TeMarker *allocMarker(unsigned long *nMarkers);
	void configMarker(const Common::String &objname, int markerImgNo, long markerId);
	void checkObjectEvents();
	void clear();
	//void entry(); // unused
	AnimData *findAnimation(const Common::String &name);
	Exit *findExit(const Common::String &name, bool flag);
	//int getAnimationFrame(const Common::String &name); // unused
	bool hasObjectOrAnim(const Common::String &name) const;
	void init();
	void load(const Common::Path &path, bool flag);
	//void loadTextures(); // unused
	//void preload(const Common::Path &path); // unused
	void putObject(const Common::String &name, bool enable);
	void update();
	void render();
	void rotateCamera(const TeQuaternion &rot);
	void sendExit(Exit &exit);
	void sendMarker(const Common::String &name, unsigned long markerId);
	void setColor(const TeColor &col) override;
	void setMarkersOpacity(float opacity);
	void setMouseLeftUpForMakers();
	void setFov(float fov);
	void setVisible(bool v1, bool v2);
	void startAnimationPart(const Common::String &name, int x, int y, int z, bool flag);
	void takeObject(const Common::String &name);
	void unload();
	void unloadTextures();
	void updateCamera(const TeVector3f32 &screen);

	static bool debug;

	TeSignal1Param<const Common::String &> &markerValidatedSignal() { return _markerValidatedSignal; }
	TeSignal1Param<const Common::String &> &animFinishedSignal() { return _animFinishedSignal; }
	bool markersActive() const { return _markersActive; }

private:
	bool onMouseLeftDown(const Common::Point &pt);
	bool onMarkerValidated(const Common::String &name);

	Common::ScopedPtr<Common::SeekableReadStream> _file;
	Common::Path _warpPath;
	TeCamera _camera;
	bool _markersActive;
	bool _visible1;
	bool _loaded;
	bool _preloaded;
	bool _renderWarpBlocs;

	const TePickMesh *_clickedPickMesh;
	const AnimData *_clickedAnimData;

	TeFrustum _frustum;

	Common::Array<TeWarpMarker *> _warpMarkers;
	// FIXME: Seems useless
	Common::List<Common::String> _paths;
	Common::Array<AnimData *> _putAnimData;
	Common::List<TeWarp::Exit> _exitList;
	uint _someXVal;
	uint _someYVal;
	uint _someMeshX;
	uint _someMeshY;
	uint _xCount;
	uint _yCount;
	Common::Array<TeWarpBloc> _warpBlocs;
	Common::Array<TePickMesh *> _pickMeshes2;
	Common::Array<AnimData> _loadedAnimData;
	TeSignal1Param<const Common::String &> _markerValidatedSignal;
	TeSignal1Param<const Common::String &> _animFinishedSignal;
	Common::String _texEncodingType;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_WARP_H
