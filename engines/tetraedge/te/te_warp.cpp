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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/te/te_warp.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

/*static*/
bool TeWarp::debug = false;

TeWarp::TeWarp() : _visible1(false), _loaded(false), _preloaded(false),
	_numAnims(0), _someXVal(0), _someYVal(0), _someMeshX(0), _someMeshY(0) {
}

TeWarp::~TeWarp() {
	unload();
	_file.close();
}

void TeWarp::activeMarkers(bool active) {
	_markersActive = active;
	for (auto &warpMarker : _warpMarkers)
		warpMarker->marker()->active(active);
}

uint TeWarp::addQuadToPickMesh(TePickMesh &pickmesh, uint trinum,
		TeWarpBloc::CubeFace face, const TeVector2s32 &size, uint xscale, uint yscale) {
	TeVector3f32 pt1;
	TeVector3f32 pt2;
	TeVector3f32 pt3;
	TeVector3f32 pt4;

    float x1 = size._x * (1000.0f / xscale) - 500.0f;
    float x2 = 1000.0f / xscale + x1;
	float y1 = size._y * (1000.0f / yscale) - 500.0f;
    float y2 = 1000.0f / yscale + y1;

	switch (face) {
	case TeWarpBloc::Face0:
		pt1 = TeVector3f32(-x1, 500, -y1);
		pt2 = TeVector3f32(-x2, 500, -y1);
		pt3 = TeVector3f32(-x2, 500, -y2);
		pt4 = TeVector3f32(-x1, 500, -y2);
		break;
	case TeWarpBloc::Face1:
		pt1 = TeVector3f32(-x1, -500, y1);
		pt2 = TeVector3f32(-x2, -500, y1);
		pt3 = TeVector3f32(-x2, -500, y2);
		pt4 = TeVector3f32(-x1, -500, y2);
		break;
    case TeWarpBloc::Face2:
		pt1 = TeVector3f32(-x1, y1, 500);
		pt2 = TeVector3f32(-x2, y1, 500);
		pt3 = TeVector3f32(-x2, y2, 500);
		pt4 = TeVector3f32(-x1, y2, 500);
		break;
    case TeWarpBloc::Face3:
		pt1 = TeVector3f32(x1, y1, -500);
		pt2 = TeVector3f32(x2, y1, -500);
		pt3 = TeVector3f32(x2, y2, -500);
		pt4 = TeVector3f32(x1, y2, -500);
		break;
    case TeWarpBloc::Face4:
		pt1 = TeVector3f32(500, y1, x1);
		pt2 = TeVector3f32(500, y1, x2);
		pt3 = TeVector3f32(500, y2, x2);
		pt4 = TeVector3f32(500, y2, x1);
		break;
    case TeWarpBloc::Face5:
		pt1 = TeVector3f32(-500, y1, -x1);
		pt2 = TeVector3f32(-500, y1, -x2);
		pt3 = TeVector3f32(-500, y2, -x2);
		pt4 = TeVector3f32(-500, y2, -x1);
	default:
		break;
	}

	pickmesh.setTriangle(trinum, pt1, pt2, pt4);
	pickmesh.setTriangle(trinum + 1, pt2, pt3, pt4);
	return trinum + 1;
}


TeMarker *TeWarp::allocMarker(unsigned long *nMarkers) {
	TeMarker *newMarker = new TeMarker();
	TeWarpMarker *newWarpMarker = new TeWarpMarker();
	newWarpMarker->marker(newMarker);
	newWarpMarker->markerButtonSignal().add(this, &TeWarp::onMarkerValidated);
	*nMarkers = _warpMarkers.size();
	_warpMarkers.push_back(newWarpMarker);
	return newMarker;
}

void TeWarp::checkObjectEvents() {
	//const Common::Point lastMouse = g_engine->getInputMgr()->lastMousePos();
	//Math::Ray mouseRay = _camera.getRay(lastMouse);
	//for (uint i = 0; i < _numAnims; i++) {
	//
	//}
	error("TODO: Implement TeWarp::checkObjectEvents");
}

void TeWarp::clear() {
	_animDatas.clear();

	error("TODO: Implement TeWarp::clear");
}

TeWarp::AnimData *TeWarp::findAnimation(const Common::String &name) {
	for (uint i = 0; i < _numAnims; i++) {
		if (_loadedAnimData[i]._name == name)
			return _loadedAnimData.data() + i;
	}
	return nullptr;
}

bool TeWarp::hasObjectOrAnim(const Common::String &name) {
	for (uint i = 0; i < _numAnims; i++) {
		if (_loadedAnimData[i]._name == name)
			return true;
	}
	return false;
}

void TeWarp::init() {
	// This mostly sets up the camera.. maybe nothing to do?
	warning("TODO: Implement TeWarp::init");
}

void TeWarp::load(const Common::String &path, bool flag) {
	if (_warpPath == path && _loaded)
		return;
	_warpPath = path;
	TeCore *core = g_engine->getCore();
	Common::FSNode node = core->findFile(_warpPath);
	if (node.isReadable())
		error("Couldn't find TeWarp path '%s'", _warpPath.c_str());

	if (_preloaded)
		error("TODO: Support preloading in TeWarp::load");
	Common::File file;
	file.open(node);
	char header[7];
	header[6] = '\0';
	file.read(header, 6);
	if (Common::String(header) != "TeWarp")
		error("Invalid header in warp data %s", _warpPath.c_str());
	uint32 globalTexDataOffset = file.readUint32LE();
	Common::String encodingType = file.readPascalString();
	_xCount = file.readUint32LE();
	_yCount = file.readUint32LE();
	_numAnims = file.readUint32LE();
	_someXVal = file.readUint32LE();
	_someYVal = file.readUint32LE();
	warning("TeWarp::load: TODO: Identify these ints..");
	/*int someInt3 = */file.readUint32LE();
	/*int someInt4 = */file.readUint32LE();
	_warpBlocs.resize(_xCount * _yCount * 6);
	for (uint i = 0; i < _xCount * _yCount * 6; i++) {
		TeWarpBloc::CubeFace face = static_cast<TeWarpBloc::CubeFace>(file.readByte());
		for (uint j = 0; j < _xCount * _yCount; j++) {
			unsigned short offx = file.readUint16LE();
			unsigned short offy = file.readUint16LE();
			unsigned int blocTexOffset = file.readUint32LE();
			_warpBlocs[j].setTextureFileOffset(globalTexDataOffset + blocTexOffset);
			TeVector2s32 offset(offx, offy);
			_warpBlocs[j].create(face, _xCount, _yCount, offset);
		}
	}
	_loadedAnimData.resize(_numAnims);
	_animDatas.reserve(_numAnims);
	for (uint i = 0; i < _numAnims; i++) {
		char aname[5];
		file.read(aname, 4);
		aname[4] = '\0';
		_loadedAnimData[i]._name = aname;
		uint numFrames = file.readUint32LE();
		byte numSomething = file.readByte();
		_loadedAnimData[i]._frameDatas.resize(numFrames);
		for (uint j = 0; j < numFrames; j++) {
			FrameData &frameData = _loadedAnimData[i]._frameDatas[j];
			frameData._loadedTexCount = 0;
			frameData._numWarpBlocs = 0;
			Common::Array<TeWarpBloc> warpBlocs;
			for (uint k = 0; k < numSomething; k++) {
				uint blocCount = file.readUint32LE();
				if (blocCount) {
					TeWarpBloc::CubeFace face = static_cast<TeWarpBloc::CubeFace>(file.readByte());
					warpBlocs.resize(frameData._numWarpBlocs + blocCount);
					for (auto &warpBloc : warpBlocs) {
						uint xoff = file.readUint16LE();
						uint yoff = file.readUint16LE();
						uint32 texDataOff = file.readUint32LE();
						warpBloc.setTextureFileOffset(globalTexDataOffset + texDataOff);
						warpBloc.create(face, _someXVal, _someYVal, TeVector2s32(xoff, yoff));
						if (flag)
							warpBloc.color(TeColor(255, 0, 0, 255));
					}
					uint meshSize = file.readUint32LE();
					TePickMesh tmpMesh;
					tmpMesh.setName(aname);
					tmpMesh.nbTriangles(meshSize * 2);
					for (uint m = 0; m < meshSize; m++) {
						uint xoff = file.readUint16LE();
						uint yoff = file.readUint16LE();
						addQuadToPickMesh(tmpMesh, m * 2, face, TeVector2s32(xoff, yoff), _someMeshX, _someMeshY);
					}
					tmpMesh.setFlag(true);
					if (frameData._pickMesh.name().empty()) {
						frameData._pickMesh = tmpMesh;
					} else {
						frameData._pickMesh += tmpMesh;
					}
				}
			}
			frameData._warpBlocs.resize(frameData._numWarpBlocs);

			error("TODO: Finish line 323~343");
		}

	}

	_loaded = true;
	error("TODO: Finish TeWarp::load");
}

bool TeWarp::onMarkerValidated(const Common::String &name) {
	_markerValidatedSignal.call(name);
	return false;
}

bool TeWarp::onMouseLeftDown(const Common::Point &pt) {
	error("TODO: Implement TeWarp::onMouseLeftDown");
}

void TeWarp::update() {
	if (!_visible1 || !_file.isOpen())
		return;
	Application *app = g_engine->getApplication();
	_frustum.update(app->mainWindowCamera());
	error("TODO: Implement TeWarp::update");
}

void TeWarp::sendExit(TeWarp::Exit &exit) {
	_paths.push_back(exit._linkedWarpPath);
	TePickMesh *mesh = new TePickMesh();
	mesh->setName(exit._linkedWarpPath);
	mesh->nbTriangles(exit._warpBlockList.size() * 2);
	uint trinum = 0;
	for (auto &block : exit._warpBlockList) {
		addQuadToPickMesh(*mesh, trinum, block._face, block._offset, block._x, block._y);
		trinum += 2;
	}
	exit._warpBlockList.clear();
	TeMarker *marker = _warpMarkers[exit._markerId]->marker();
	assert(marker);
	marker->button().load("2D/Menus/InGame/Marker_0.png", "2D/Menus/InGame/Marker_0_over.png", "");
	marker->visible(false);
	marker->setSomeFloat(-999.0f);
	_exitList.push_back(exit);
}

void TeWarp::sendMarker(const Common::String &name, unsigned long markerId) {
	AnimData *anim = findAnimation(name);
	if (anim)
		anim->_markerIds.push_back(markerId);
}

void TeWarp::setColor(const TeColor &col) {
	Te3DObject2::setColor(col);
	for (auto &warpMarker : _warpMarkers) {
		warpMarker->marker()->button().setColor(col);
	}
}

void TeWarp::setMouseLeftUpForMakers() {
	for (auto &marker : _warpMarkers) {
		marker->marker()->button().setEnable(true);
	}
}

void TeWarp::setVisible(bool v1, bool v2) {
	if (_visible1 == v1)
		return;

	_visible1 = v1;
	TeInputMgr *inputMgr = g_engine->getInputMgr();
	if (v1) {
		inputMgr->_mouseLDownSignal.add(this, &TeWarp::onMouseLeftDown);
	} else {
		if (v2) {
			error("TODO: Implement TeWarp::setVisible for v2==true");
		}
		inputMgr->_mouseLDownSignal.remove(this, &TeWarp::onMouseLeftDown);
	}
}

void TeWarp::render() {
	if (!_visible1 || _scale.x() == 0.0f || _scale.y() == 0.0f ||
			_scale.z() == 0.0f || !worldVisible() || color().a() == 0)
		return;

	TeRenderer *renderer = g_engine->getRenderer();
	renderer->setMatrixMode(TeRenderer::MM_GL_PROJECTION);
	renderer->pushMatrix();
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->pushMatrix();
	_camera.apply();
	renderer->disableZBuffer();
	renderer->pushMatrix();

	// TODO: Render the WarpBlocs here.

	for (auto &warpMarker : _warpMarkers) {
		warpMarker->marker()->update(&_camera);
	}

	renderer->setCurrentColor(TeColor(255, 255, 255, 255));
	renderer->popMatrix();
	renderer->enableZBuffer();
	TeCamera::restore();
	renderer->setMatrixMode(TeRenderer::MM_GL_PROJECTION);
	renderer->popMatrix();
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->popMatrix();

	error("TODO: Finish TeWarp::render");
}

void TeWarp::rotateCamera(const TeQuaternion &rot) {
	TeQuaternion normRot = rot;
	normRot.normalize();
	_camera.setRotation(normRot);
}

void TeWarp::setFov(float fov) {
	_camera.setFov(fov);
}

void TeWarp::unload() {
	error("TODO: Implement TeWarp::unload");
}

void TeWarp::unloadTextures() {
	for (uint i = 0; i < _xCount * _yCount * 6; i++) {
		_warpBlocs[i].unloadTexture();
	}

	if (_numAnims)
		error("TODO: Finish TeWarp::unloadTextures");

	for (auto &animData : _loadedAnimData) {
		for (auto &frameData : animData._frameDatas) {
			for (auto &warpBloc : frameData._warpBlocs) {
				warpBloc.unloadTexture();
			}
		}
	}
}

void TeWarp::updateCamera(const TeVector3f32 &screen) {
	_camera.viewport(0, 0, screen.x(), screen.y());
	_camera.setOrthoPlanes(1, 4086);
	_camera.setAspectRatio(screen.x() / screen.y());
}

} // end namespace Tetraedge
