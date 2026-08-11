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
#include "tetraedge/te/te_frustum.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_ray_intersection.h"

namespace Tetraedge {

/*static*/
bool TeWarp::debug = false;

TeWarp::TeWarp() : _visible1(false), _loaded(false), _preloaded(false),
	_someXVal(0), _someYVal(0), _someMeshX(0), _someMeshY(0),
	_renderWarpBlocs(true), _xCount(0), _yCount(0), _clickedPickMesh(nullptr),
	_clickedAnimData(nullptr), _markersActive(true) {
}

TeWarp::~TeWarp() {
	_markerValidatedSignal.clear();
	unload();
	_file.reset();
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
	const Common::Point lastMouse = g_engine->getInputMgr()->lastMousePos();
	Math::Ray mouseRay = _camera.getRay(lastMouse);
	for (auto &animData : _loadedAnimData) {
		if (_clickedAnimData == &animData) {
			TePickMesh &pickMesh = animData._frameDatas[animData._curFrameNo]._pickMesh;
			TeVector3f32 intersectPt;
			float intersectLen;
			if (pickMesh.enabled() && pickMesh.intersect(mouseRay, intersectPt, intersectLen)) {
				_markerValidatedSignal.call(pickMesh.name());
				break;
			}
		}
	}
	TePickMesh *mesh = TeRayIntersection::getMesh(mouseRay, _pickMeshes2, FLT_MAX, 0, nullptr);
	if (mesh && mesh == _clickedPickMesh)
		_markerValidatedSignal.call(mesh->name());
	_clickedAnimData = nullptr;
	_clickedPickMesh = nullptr;
}

void TeWarp::clear() {
	_putAnimData.clear();

	for (auto &data : _loadedAnimData)
		data._enabled = false;

	for (auto *marker : _warpMarkers)
		marker->marker()->visible(false);
}

void TeWarp::configMarker(const Common::String &objname, int markerImgNo, long markerId) {
	Exit *exit = findExit(objname, false);
	long foundId = -1;
	if (exit) {
		foundId = exit->_markerId;
	} else {
		AnimData *anim = findAnimation(objname);
		if (!anim || anim->_markerIds.empty()) {
			warning("configMarker: Didn't find marker %s", objname.c_str());
			return;
		}
		foundId = anim->_markerIds[0];
	}
	assert(foundId >= 0 && foundId < (long)_warpMarkers.size());

	TeWarpMarker *warpMarker = _warpMarkers[foundId];
	// The game uses TeSprite, but we use the layout system instead.
	TeLayout &frontLayout = g_engine->getApplication()->frontLayout();
	if (markerImgNo == -1) {
		warpMarker->marker()->visible(false);
		frontLayout.removeChild(&warpMarker->marker()->button());
	} else {
		Common::Path markerPath(Common::String::format("2D/Menus/InGame/Marker_%d.png#anim", markerImgNo));
		Common::Path markerPathDown(Common::String::format("2D/Menus/InGame/Marker_%d_over.png", markerImgNo));
		if (!exit)
			warpMarker->setName(objname);
		else
			warpMarker->setName(Common::String("3D\\") + objname);

		warpMarker->marker()->button().load(markerPath, markerPathDown, "");
		TeSpriteLayout *btnUp = dynamic_cast<TeSpriteLayout*>(warpMarker->marker()->button().upLayout());
		if (!btnUp)
			error("Loading button image %s failed", markerPath.toString(Common::Path::kNativeSeparator).c_str());
		//warning("TeWarp::configMarker: set anim values and something else here?");
		btnUp->_tiledSurfacePtr->_frameAnim.setLoopCount(-1);
		btnUp->play();
		warpMarker->marker()->visible(true);
		// Ensure markers appear below menus and videos.
		frontLayout.removeChild(&warpMarker->marker()->button());
		frontLayout.addChildBefore(&warpMarker->marker()->button(), frontLayout.child(0));
	}
}

TeWarp::AnimData *TeWarp::findAnimation(const Common::String &objname) {
	for (auto &anim : _loadedAnimData) {
		if (anim._name == objname)
			return &anim;
	}
	return nullptr;
}

TeWarp::Exit *TeWarp::findExit(const Common::String &objname, bool flag) {
	Common::String fullName;
	if (flag)
		fullName = objname;
	else
		fullName = Common::String("3D\\") + objname;

	for (auto &e : _exitList) {
		if (e._linkedWarpPath.contains(fullName))
			return &e;
	}
	return nullptr;
}

bool TeWarp::hasObjectOrAnim(const Common::String &objname) const {
	for (const auto &anim : _loadedAnimData) {
		if (anim._name == objname)
			return true;
	}
	return false;
}

void TeWarp::init() {
	TeVector3f32 winSize = g_engine->getApplication()->getMainWindow().size();
	_camera.setProjMatrixType(1);
	_camera.viewport(0, 0, (int)winSize.x(), (int)winSize.y());
	_camera.setOrthoPlanes(1, 4096);
	_camera.setAspectRatio(winSize.x() / winSize.y());
	// update proj matrix
	_camera.projectionMatrix();
	warning("TODO: Finish TeWarp::init?");
}

void TeWarp::load(const Common::Path &path, bool flag) {
	if (_warpPath == path && _loaded)
		return;
	_warpPath = path;

	if (path.empty())
		error("Empty TeWarp path!");

	TeCore *core = g_engine->getCore();
	TetraedgeFSNode node = core->findFile(_warpPath);
	if (!node.isReadable()) {
		error("Couldn't find TeWarp path data '%s'", _warpPath.toString(Common::Path::kNativeSeparator).c_str());
	}

	if (_preloaded)
		error("TODO: Support preloading in TeWarp::load");
	_file.reset(node.createReadStream());
	char header[7];
	header[6] = '\0';
	_file->read(header, 6);
	if (Common::String(header) != "TeWarp")
		error("Invalid header in warp data %s", _warpPath.toString().c_str());
	uint32 globalTexDataOffset = _file->readUint32LE();
	_texEncodingType = _file->readPascalString();
	_xCount = _file->readUint32LE();
	_yCount = _file->readUint32LE();
	uint32 numAnims = _file->readUint32LE();
	_someXVal = _file->readUint32LE();
	_someYVal = _file->readUint32LE();
	_someMeshX = _file->readUint32LE();
	_someMeshY = _file->readUint32LE();
	if (_xCount > 1000 || _yCount > 1000 || numAnims > 1000)
		error("Improbable values in TeWarp data xCount %d yCount %d numAnims %d", _xCount, _yCount, numAnims);
	_warpBlocs.resize(_xCount * _yCount * 6);
	for (uint i = 0; i < _xCount * _yCount * 6; i++) {
		TeWarpBloc::CubeFace face = static_cast<TeWarpBloc::CubeFace>(_file->readByte());
		// TODO: This is strange, surely we only need to set the offset and create the bloc
		// once but the code seems to do it xCount * yCount times..
		for (uint j = 0; j < _xCount * _yCount; j++) {
			uint xoff = _file->readUint16LE();
			uint yoff = _file->readUint16LE();
			if (xoff > 1000 || yoff > 1000)
				error("TeWarp::load: Improbable offsets %d, %d", xoff, yoff);
			uint32 blocTexOffset = _file->readUint32LE();
			_warpBlocs[i].setTextureFileOffset(globalTexDataOffset + blocTexOffset);
			_warpBlocs[i].create(face, _xCount, _yCount, TeVector2s32(xoff, yoff));
		}
	}
	_loadedAnimData.resize(numAnims);
	_putAnimData.reserve(numAnims);
	for (uint i = 0; i < numAnims; i++) {
		char aname[5];
		_file->read(aname, 4);
		aname[4] = '\0';
		_loadedAnimData[i]._name = aname;
		uint numFrames = _file->readUint32LE();
		if (numFrames > 1000)
			error("TeWarp::load: Improbable frame count %d", numFrames);
		byte numSomething = _file->readByte();
		_loadedAnimData[i]._frameDatas.resize(numFrames);
		for (uint j = 0; j < numFrames; j++) {
			FrameData &frameData = _loadedAnimData[i]._frameDatas[j];
			frameData._loadedTexCount = 0;
			Common::Array<TeWarpBloc> warpBlocs;
			for (uint k = 0; k < numSomething; k++) {
				uint blocCount = _file->readUint32LE();
				if (blocCount > 1000)
					error("TeWarp::load: Improbable bloc count %d", blocCount);
				if (blocCount) {
					TeWarpBloc::CubeFace face = static_cast<TeWarpBloc::CubeFace>(_file->readByte());
					warpBlocs.resize(blocCount);
					for (auto &warpBloc : warpBlocs) {
						uint xoff = _file->readUint16LE();
						uint yoff = _file->readUint16LE();
						if (xoff > 10000 || yoff > 10000)
							error("TeWarp::load: Improbable offsets %d, %d", xoff, yoff);
						uint32 texDataOff = _file->readUint32LE();
						warpBloc.setTextureFileOffset(globalTexDataOffset + texDataOff);
						warpBloc.create(face, _someXVal, _someYVal, TeVector2s32(xoff, yoff));
						if (flag)
							warpBloc.color(TeColor(255, 0, 0, 255));
					}
					uint meshSize = _file->readUint32LE();
					if (meshSize > 1000)
						error("TeWarp::load: Improbable meshSize %d", meshSize);
					TePickMesh tmpMesh;
					tmpMesh.setName(aname);
					tmpMesh.nbTriangles(meshSize * 2);
					for (uint m = 0; m < meshSize; m++) {
						uint xoff = _file->readUint16LE();
						uint yoff = _file->readUint16LE();
						if (xoff > 10000 || yoff > 10000)
							error("TeWarp::load: Improbable offsets %d, %d", xoff, yoff);
						addQuadToPickMesh(tmpMesh, m * 2, face, TeVector2s32(xoff, yoff), _someMeshX, _someMeshY);
					}
					tmpMesh.setEnabled(true);
					if (frameData._pickMesh.name().empty()) {
						frameData._pickMesh = tmpMesh;
					} else {
						frameData._pickMesh += tmpMesh;
					}
				}
			}
			frameData._warpBlocs.resize(warpBlocs.size());
			for (uint k = 0; k < frameData._warpBlocs.size(); k++) {
				frameData._warpBlocs[k] = warpBlocs[k];
			}
		}

	}

	_loaded = true;
}

bool TeWarp::onMarkerValidated(const Common::String &name) {
	_markerValidatedSignal.call(name);
	return false;
}

bool TeWarp::onMouseLeftDown(const Common::Point &pt) {
	const Math::Ray mouseRay = _camera.getRay(pt);
	_clickedPickMesh = nullptr;
	_clickedAnimData = nullptr;

	bool hitAnimData = false;
	const FrameData *frameData = nullptr;
	for (const auto &animData : _loadedAnimData) {
		frameData = &(animData._frameDatas[animData._curFrameNo]);
		TeVector3f32 interesctPt;
		float intersectDist;
		if (frameData->_pickMesh.enabled() && frameData->_pickMesh.intersect(mouseRay, interesctPt, intersectDist)) {
			_clickedAnimData = &animData;
			hitAnimData = true;
			break;
		}
	}

	if (!hitAnimData) {
		_clickedPickMesh = TeRayIntersection::getMesh(mouseRay, _pickMeshes2, FLT_MAX, 0, nullptr);
		if (_clickedPickMesh) {
			const Exit *exit = findExit(_clickedPickMesh->name(), true);
			_warpMarkers[exit->_markerId]->marker()->button().setEnable(false);
		}
		return false;
	}

	const AnimData *data = findAnimation(frameData->_pickMesh.name());
	for (auto &markerId : data->_markerIds) {
		_warpMarkers[markerId]->marker()->button().setEnable(false);
	}
	return false;
}

void TeWarp::putObject(const Common::String &name, bool enable) {
	bool found = false;
	for (auto &animData : _loadedAnimData) {
		if (animData._name != name || animData._frameDatas.size() != 1
				|| animData._curFrameNo != 0)
			continue;
		bool alreadyAdded = false;
		for (auto putAnim : _putAnimData) {
			if (putAnim == &animData) {
				alreadyAdded = true;
				break;
			}
		}
		animData._enabled = true;
		if (!alreadyAdded)
			_putAnimData.push_back(&animData);
		for (auto &frameData : animData._frameDatas) {
			frameData._pickMesh.setEnabled(enable);
		}
		found = true;
	}
	if (!found)
		warning("putObject: Impossible de trouver l\'objet %s dans le Warp", name.c_str());
}

void TeWarp::update() {
	if (!_visible1 || !_file)
		return;
	//Application *app = g_engine->getApplication();
	_frustum.update(_camera);
	for (auto &bloc : _warpBlocs) {
		bloc.loadTexture(*_file, _texEncodingType);
	}

	for (auto &anim : _loadedAnimData) {
		if (anim._repCount && anim._frameDatas.size() > 1) {
			uint64 elapsed = anim._timer.getTimeFromStart();
			int frameNow = elapsed * anim._fps / 1000000.0;
			int lastFrame = anim._curFrameNo;
			if (anim._repCount != -1) {
				anim._repCount = anim._repCount - frameNow / (anim._endFrameNo - anim._firstFrameNo);
				if (anim._repCount < 1) {
					anim._repCount = 0;
					frameNow = anim._endFrameNo - 1;
					_animFinishedSignal.call(anim._name);
				}
			}
			
			anim._curFrameNo = anim._firstFrameNo + ((frameNow - anim._firstFrameNo) % (anim._endFrameNo - anim._firstFrameNo));
			if (anim._curFrameNo != lastFrame) {
				anim._frameDatas[lastFrame].unloadTextures();
				anim._frameDatas[lastFrame]._loadedTexCount = 0;
			}
		}
		anim._frameDatas[anim._curFrameNo].loadTextures(_frustum, *_file, _texEncodingType);
	}
}

void TeWarp::sendExit(TeWarp::Exit &exit) {
	_paths.push_back(exit._linkedWarpPath);
	TePickMesh *mesh = new TePickMesh();
	mesh->setName(exit._linkedWarpPath);
	mesh->nbTriangles(exit._warpBlockList.size() * 2);
	uint trinum = 0;
	for (const auto &block : exit._warpBlockList) {
		addQuadToPickMesh(*mesh, trinum, block._face, block._offset, block._x, block._y);
		trinum += 2;
	}
	exit._warpBlockList.clear();
	TeMarker *marker = _warpMarkers[exit._markerId]->marker();
	assert(marker);
	marker->button().load("2D/Menus/InGame/Marker_0.png#anim", "2D/Menus/InGame/Marker_0_over.png", "");
	marker->visible(false);
	marker->setZLoc(200.0f);
	_exitList.push_back(exit);
}

void TeWarp::sendMarker(const Common::String &name, unsigned long markerId) {
	AnimData *anim = findAnimation(name);
	if (anim)
		anim->_markerIds.push_back(markerId);
}

void TeWarp::startAnimationPart(const Common::String &name, int x, int startFrame, int endFrame, bool flag) {
	bool started = false;
	for (auto &animData : _loadedAnimData) {
		if (animData._name != name)
			continue;
		animData._enabled = true;
		bool alreadyPut = false;
		for (auto *putAnim : _putAnimData) {
			if (putAnim == &animData)
				alreadyPut = true;
		}
		if (!alreadyPut)
			_putAnimData.push_back(&animData);

		animData._repCount = x;
		animData._timer.stop();
		animData._firstFrameNo = startFrame;
		if (endFrame < 0)
			endFrame += animData._frameDatas.size();
		animData._endFrameNo = endFrame;
		for (auto &frameData : animData._frameDatas) {
			// TODO: Is this setting the right thing?
			frameData._pickMesh.setEnabled(flag);
		}
		
		animData._timer.start();
		started = true;
	}
	if (!started)
		warning("startAnimationPartImpossible de trouver l\'animation %s dans le Warp.", name.c_str());
}

void TeWarp::setColor(const TeColor &col) {
	Te3DObject2::setColor(col);
	for (auto &warpMarker : _warpMarkers) {
		warpMarker->marker()->button().setColor(col);
	}
}

void TeWarp::setMouseLeftUpForMakers() {
	for (auto &warpMarker : _warpMarkers) {
		warpMarker->marker()->button().setEnable(true);
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
			for (auto *marker : _warpMarkers) {
				TeMarker *m = marker->marker();
				delete marker;
				// May be still handling the button click
				if (m)
					m->deleteLater();
			}
			_warpMarkers.clear();
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

	if (_renderWarpBlocs) {
		for (auto &bloc : _warpBlocs) {
			bloc.render();
		}
	}

	for (AnimData *animData : _putAnimData) {
		if (!animData->_enabled)
			continue;
		for (FrameData &frameData : animData->_frameDatas) {
			for (TeWarpBloc &b : frameData._warpBlocs) {
				if (_frustum.isTriangleInside(b.vertex(0), b.vertex(1), b.vertex(3))
						|| _frustum.isTriangleInside(b.vertex(1), b.vertex(2), b.vertex(3))) {
					b.render();
				}
			}
		}
	}

	for (auto &warpMarker : _warpMarkers) {
		warpMarker->marker()->update(_camera);
	}

	renderer->setCurrentColor(TeColor(255, 255, 255, 255));
	renderer->popMatrix();
	renderer->enableZBuffer();
	TeCamera::restore();
	renderer->setMatrixMode(TeRenderer::MM_GL_PROJECTION);
	renderer->popMatrix();
	renderer->setMatrixMode(TeRenderer::MM_GL_MODELVIEW);
	renderer->popMatrix();
}

void TeWarp::rotateCamera(const TeQuaternion &rot) {
	TeQuaternion normRot = rot;
	normRot.normalize();
	_camera.setRotation(normRot);
}

void TeWarp::setFov(float fov) {
	_camera.setFov(fov);
	// update proj matrix
	_camera.projectionMatrix();
}

void TeWarp::takeObject(const Common::String &name) {
	bool found = false;
	for (auto &animData : _loadedAnimData) {
		if (animData._name != name)
			continue;
		animData._curFrameNo = 0;
		animData._enabled = false;
		for (uint i = 0; i < _putAnimData.size(); i++) {
			if (_putAnimData[i] == &animData) {
				_putAnimData.remove_at(i);
				break;
			}
		}
		for (auto &frame : animData._frameDatas) {
			frame._pickMesh.setEnabled(false);
		}
		found = true;
	}
	if (!found)
		warning("takeObject: Impossible de trouver l\'objet %s dans le Warp", name.c_str());
}

void TeWarp::unload() {
	// Not done in original but can happen if user clicks really fast.
	g_engine->getInputMgr()->_mouseLDownSignal.remove(this, &TeWarp::onMouseLeftDown);
	unloadTextures();
	_xCount = 0;
	_yCount = 0;
	_loadedAnimData.clear();
	_putAnimData.clear();
	_paths.clear();
	_pickMeshes2.clear();
	_exitList.clear();
	for (auto *marker : _warpMarkers) {
		TeMarker *m = marker->marker();
		delete marker;
		// May be still handling the button click
		if (m)
			m->deleteLater();
	}
	_warpMarkers.clear();
	_preloaded = false;
	_loaded = false;
}

void TeWarp::unloadTextures() {
	for (auto &bloc : _warpBlocs) {
		bloc.unloadTexture();
	}

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
	_camera.setOrthoPlanes(1, 4096);
	_camera.setAspectRatio(screen.x() / screen.y());
	// update proj matrix
	_camera.projectionMatrix();
}

void TeWarp::FrameData::loadTextures(const TeFrustum &frustum, Common::SeekableReadStream &file, const Common::String &fileType) {
	for (auto &b : _warpBlocs) {
		if (!b.isLoaded() && (frustum.isTriangleInside(b.vertex(0), b.vertex(1), b.vertex(3))
				|| frustum.isTriangleInside(b.vertex(1), b.vertex(2), b.vertex(3)))) {
			b.loadTexture(file, fileType);
		}
	}
}

void TeWarp::FrameData::unloadTextures() {
	for (auto &bloc : _warpBlocs)
		bloc.unloadTexture();
}

} // end namespace Tetraedge
