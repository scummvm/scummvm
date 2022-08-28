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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/zlib.h"

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/xactive_animation.h"
#include "engines/wintermute/base/gfx/xanimation_channel.h"
#include "engines/wintermute/base/gfx/xanimation_set.h"
#include "engines/wintermute/base/gfx/xframe_node.h"
#include "engines/wintermute/base/gfx/xmaterial.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/gfx/xloader.h"
#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/wintermute.h"

namespace Wintermute {

static const int kCabBlockSize = 0x8000;
static const int kCabInputmax = kCabBlockSize + 12;

static byte *DecompressMsZipData(byte *buffer, uint32 inputSize, uint32 &decompressedSize) {
#ifdef USE_ZLIB
	bool error = false;

	Common::MemoryReadStream data(buffer, inputSize);
	byte *compressedBlock = new byte[kCabInputmax];
	byte *decompressedBlock = new byte[kCabBlockSize];

	// Read decompressed size of compressed data and minus 16 bytes of xof header
	decompressedSize = data.readUint32LE() - 16;

	uint32 remainingData = inputSize;
	uint32 decompressedPos = 0;
	byte *decompressedData = new byte[decompressedSize];
	if (!decompressedData)
		error = true;

	while (!error && remainingData) {
		uint16 uncompressedLen = data.readUint16LE();
		uint16 compressedLen = data.readUint16LE();
		remainingData -= 4;

		if (data.err()) {
			error = true;
			break;
		}

		if (remainingData == 0) {
			break;
		}

		if (compressedLen > kCabInputmax || uncompressedLen > kCabBlockSize) {
			error = true;
			break;
		}

		// Read the compressed block
		if (data.read(compressedBlock, compressedLen) != compressedLen) {
			error = true;
			break;
		}
		remainingData -= compressedLen;

		// Check the CK header
		if (compressedBlock[0] != 'C' || compressedBlock[1] != 'K') {
			error = true;
			break;
		}

		// Decompress the block. If it isn't the first, provide the previous block as dictonary
		byte *dict = decompressedPos ? decompressedBlock : nullptr;
		bool decRes = Common::inflateZlibHeaderless(decompressedBlock, uncompressedLen, compressedBlock + 2, compressedLen - 2, dict, kCabBlockSize);
		if (!decRes) {
			error = true;
			break;
		}

		// Copy the decompressed data
		memcpy(decompressedData + decompressedPos, decompressedBlock, uncompressedLen);
		decompressedPos += uncompressedLen;
	}
	if (decompressedSize != decompressedPos)
		error = true;

	delete[] compressedBlock;
	delete[] decompressedBlock;

	if (!error)
		return decompressedData;
#endif
	warning("DecompressMsZipData: Error decompressing data!");
	decompressedSize = 0;
	return nullptr;
}

static XFileLexer createXFileLexer(byte *&buffer, uint32 fileSize) {
	// xof header of an .X file consists of 16 bytes
	// bytes 9 to 12 contain a string which can be 'txt ', 'bin ', 'bzip, 'tzip', depending on the format
	byte dataFormatBlock[5];
	Common::copy(buffer + 8, buffer + 12, dataFormatBlock);
	dataFormatBlock[4] = '\0';

	bool textMode = (strcmp((char *)dataFormatBlock, "txt ") == 0 || strcmp((char *)dataFormatBlock, "tzip") == 0);

	if (strcmp((char *)dataFormatBlock, "bzip") == 0 || strcmp((char *)dataFormatBlock, "tzip") == 0) {
		uint32 decompressedSize;
		// we skip the 16 bytes xof header of the file
		byte *buf = DecompressMsZipData(buffer + 16, fileSize - 16, decompressedSize);
		delete[] buffer;
		buffer = buf;
		return XFileLexer(buffer, decompressedSize, textMode);
	} else {
		// we skip the 16 bytes xof header of the file
		return XFileLexer(buffer + 16, fileSize - 16, textMode);
	}
}

IMPLEMENT_PERSISTENT(XModel, false)

//////////////////////////////////////////////////////////////////////////
XModel::XModel(BaseGame *inGame, BaseObject *owner) : BaseObject(inGame) {
	_owner = owner;

	_rootFrame = nullptr;

	_drawingViewport.setEmpty();
	_lastWorldMat.setToIdentity();
	_lastViewMat.setToIdentity();
	_lastProjMat.setToIdentity();
	_lastOffsetX = _lastOffsetY = 0;

	_BBoxStart = Math::Vector3d(0.0f, 0.0f, 0.0f);
	_BBoxEnd = Math::Vector3d(0.0f, 0.0f, 0.0f);
	_boundingRect.setEmpty();

	for (int i = 0; i < X_NUM_ANIMATION_CHANNELS; i++) {
		_channels[i] = nullptr;
	}

	_ticksPerSecond = kDefaultTicksPerSecond;
}

//////////////////////////////////////////////////////////////////////////
XModel::~XModel() {
	cleanup();
}

//////////////////////////////////////////////////////////////////////////
void XModel::cleanup(bool complete) {
	// empty animation channels
	for (int i = 0; i < X_NUM_ANIMATION_CHANNELS; i++) {
		delete _channels[i];
		_channels[i] = nullptr;
	}

	// remove animation sets
	for (uint32 i = 0; i < _animationSets.size(); i++) {
		delete _animationSets[i];
	}
	_animationSets.clear();

	if (complete) {
		for (uint i = 0; i < _mergedModels.size(); ++i) {
			delete[] _mergedModels[i];
		}
		_mergedModels.clear();
	}

	for (uint32 i = 0; i < _matSprites.size(); i++) {
		delete _matSprites[i];
		_matSprites[i] = nullptr;
	}
	_matSprites.clear();

	for (uint i = 0; i < _materialReferences.size(); ++i) {
		delete _materialReferences[i]._material;
	}
	_materialReferences.clear();

	// remove root frame
	delete _rootFrame;
	_rootFrame = nullptr;

	_parentModel = nullptr;

	_ticksPerSecond = kDefaultTicksPerSecond;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::loadFromFile(const Common::String &filename, XModel *parentModel) {
	cleanup(false);

	_parentModel = parentModel;

	uint32 fileSize = 0;
	byte *buffer = BaseFileManager::getEngineInstance()->getEngineInstance()->readWholeFile(filename, &fileSize);
	XFileLexer lexer = createXFileLexer(buffer, fileSize);

	_rootFrame = new FrameNode(_gameRef);

	bool res = _rootFrame->loadFromXAsRoot(filename, lexer, this, _materialReferences);
	if (res) {
		findBones(false, parentModel);
	}

	for (int i = 0; i < X_NUM_ANIMATION_CHANNELS; ++i) {
		_channels[i] = new AnimationChannel(_gameRef, this);
	}

	setFilename(filename.c_str());

	delete[] buffer;

	return res;
}

bool XModel::mergeFromFile(const Common::String &filename) {
	if (!_rootFrame) {
		return false;
	}

	uint32 fileSize = 0;
	byte *buffer = BaseFileManager::getEngineInstance()->getEngineInstance()->readWholeFile(filename, &fileSize);
	XFileLexer lexer = createXFileLexer(buffer, fileSize);

	lexer.advanceToNextToken();
	parseFrameDuringMerge(lexer, filename);

	findBones(false, nullptr);

	bool found = false;

	for (uint i = 0; i < _mergedModels.size(); ++i) {
		if (scumm_stricmp(_mergedModels[i], filename.c_str()) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		char *path = new char[filename.size() + 1];
		strcpy(path, filename.c_str());
		_mergedModels.add(path);
	}

	delete[] buffer;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::loadAnimationSet(XFileLexer &lexer, const Common::String &filename) {
	bool res = true;

	AnimationSet *animSet = new AnimationSet(_gameRef, this);

	if (animSet->loadFromX(lexer, filename)) {
		_animationSets.add(animSet);
	} else {
		delete animSet;
		res = false;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::loadAnimation(const Common::String &filename, AnimationSet *parentAnimSet) {
	// not sure if we need this here (not completely implemented anyways and also not called)
	// are there animation objects in .X outside of an animation set?

	// if no parent anim set is specified, create one
	bool newAnimSet = false;
	if (parentAnimSet == nullptr) {
		parentAnimSet = new AnimationSet(_gameRef, this);

		parentAnimSet->setName(PathUtil::getFileName(filename).c_str());
		newAnimSet = true;
	}

	// create the new object
	Animation *Anim = new Animation(_gameRef);

	parentAnimSet->addAnimation(Anim);

	if (newAnimSet) {
		_animationSets.add(parentAnimSet);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::findBones(bool animOnly, XModel *parentModel) {
	FrameNode *rootFrame;
	if (parentModel == nullptr)
		rootFrame = _rootFrame;
	else
		rootFrame = parentModel->getRootFrame();

	if (rootFrame && !animOnly) {
		_rootFrame->findBones(rootFrame);
	}

	for (uint32 i = 0; i < _animationSets.size(); i++) {
		_animationSets[i]->findBones(rootFrame);
	}

	return true;
}

void XModel::parseFrameDuringMerge(XFileLexer &lexer, const Common::String &filename) {
	while (!lexer.eof()) {
		if (lexer.tokenIsIdentifier("Frame")) {
			lexer.advanceToNextToken();
			parseFrameDuringMerge(lexer, filename);
		} else if (lexer.tokenIsIdentifier("AnimationSet")) {
			lexer.advanceToNextToken();
			loadAnimationSet(lexer, filename);
		} else if (lexer.tokenIsOfType(IDENTIFIER)) {
			lexer.skipObject();
		} else {
			lexer.advanceToNextToken(); // we ignore anything else here
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool XModel::update() {
	// reset all bones to default position
	reset();

	// update all animation channels
	for (int i = 0; i < X_NUM_ANIMATION_CHANNELS; i++) {
		_channels[i]->update(i == 1);
	}

	// update matrices
	if (_rootFrame) {
		Math::Matrix4 tempMat;
		tempMat.setToIdentity();
		_rootFrame->updateMatrices(tempMat);

		return _rootFrame->updateMeshes();
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool XModel::playAnim(int channel, const Common::String &name, uint32 transitionTime, bool forceReset, uint32 stopTransitionTime) {
	if (channel < 0 || channel >= X_NUM_ANIMATION_CHANNELS) {
		return false;
	}

	// are we already playing this animation?
	if (!forceReset) {
		if (_channels[channel]->getName() && name.equalsIgnoreCase(_channels[channel]->getName())) {
			return true;
		}
	}

	// find animation set by name
	AnimationSet *anim = getAnimationSetByName(name);

	if (anim) {
		char *currentAnim = _channels[channel]->getName();
		if (_owner && currentAnim) {
			// clean this up later
			transitionTime = _owner->getAnimTransitionTime(currentAnim, const_cast<char *>(name.c_str()));
		}

		return _channels[channel]->playAnim(anim, transitionTime, stopTransitionTime);
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool XModel::stopAnim(int channel, uint32 transitionTime) {
	if (channel < 0 || channel >= X_NUM_ANIMATION_CHANNELS) {
		return false;
	}

	return _channels[channel]->stopAnim(transitionTime);
}

//////////////////////////////////////////////////////////////////////////
bool XModel::stopAnim(uint32 transitionTime) {
	const int NUM_SKEL_ANI_CHANNELS = 10;

	for (int channel = 0; channel < NUM_SKEL_ANI_CHANNELS; channel++) {
		stopAnim(channel, transitionTime);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::reset() {
	if (_rootFrame) {
		_rootFrame->resetMatrices();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::isAnimPending(int channel, const char *animName) {
	if (!animName) {
		if (_channels[channel]->isPlaying()) {
			return true;
		}
	} else {
		if (_channels[channel]->isPlaying() && _channels[channel]->getName() && scumm_stricmp(animName, _channels[channel]->getName()) == 0) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::isAnimPending(char *animName) {
	for (int channel = 0; channel < X_NUM_ANIMATION_CHANNELS; channel++) {
		if (isAnimPending(channel, animName)) {
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::updateShadowVol(ShadowVolume *shadow, Math::Matrix4 &modelMat, const Math::Vector3d &light, float extrusionDepth) {
	if (_rootFrame) {
		return _rootFrame->updateShadowVol(shadow, modelMat, light, extrusionDepth);
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool XModel::render() {
	if (_rootFrame) {
		// set culling
		if(_owner && !_owner->_drawBackfaces) {
			_gameRef->_renderer3D->enableCulling();
		} else {
			_gameRef->_renderer3D->disableCulling();
		}

		// render everything
		bool res = _rootFrame->render(this);

		// remember scene offset
		Rect32 rc;
		_gameRef->getCurrentViewportRect(&rc);
		float width = (float)rc.right - (float)rc.left;
		float height = (float)rc.bottom - (float)rc.top;

		// margins
		int mleft = rc.left;
		int mright = _gameRef->_renderer->getWidth() - width - rc.left;
		int mtop = rc.top;
		int mbottom = _gameRef->_renderer->getHeight() - height - rc.top;

		_lastOffsetX = _gameRef->_offsetX + (mleft - mright) / 2;
		_lastOffsetY = _gameRef->_offsetY + (mtop - mbottom) / 2;

		// update bounding box and 2D bounding rectangle
		updateBoundingRect();

		return res;
	} else {
		return false;
	}
}

bool XModel::renderFlatShadowModel() {
	if (_rootFrame) {
		if(_owner && !_owner->_drawBackfaces) {
			_gameRef->_renderer3D->enableCulling();
		} else {
			_gameRef->_renderer3D->disableCulling();
		}

		return _rootFrame->renderFlatShadowModel();
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
Math::Matrix4 *XModel::getBoneMatrix(const char *boneName) {
	FrameNode *bone = _rootFrame->findFrame(boneName);

	if (bone) {
		return bone->getCombinedMatrix();
	} else {
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
FrameNode *XModel::getRootFrame() {
	return _rootFrame;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::isTransparentAt(int x, int y) {
	if (!_rootFrame) {
		return false;
	}

	Math::Ray ray = _gameRef->_renderer3D->rayIntoScene(x, y);

	// transform to model space
	Math::Vector3d end = ray.getOrigin() + ray.getDirection();
	Math::Matrix4 m = _lastWorldMat;
	m.inverse();
	m.transform(&ray.getOrigin(), true);
	m.transform(&end, true);
	Math::Vector3d pickRayDirection = end - ray.getOrigin();

	return !_rootFrame->pickPoly(&ray.getOrigin(), &pickRayDirection);
}

//////////////////////////////////////////////////////////////////////////
void XModel::updateBoundingRect() {
	_BBoxStart = Math::Vector3d(0, 0, 0);
	_BBoxStart = Math::Vector3d(0, 0, 0);

	if (_rootFrame) {
		_rootFrame->getBoundingBox(&_BBoxStart, &_BBoxEnd);
	}

	_boundingRect.left = INT_MAX_VALUE;
	_boundingRect.top = INT_MAX_VALUE;
	_boundingRect.right = INT_MIN_VALUE;
	_boundingRect.bottom = INT_MIN_VALUE;

	Math::Vector3d vec2d(0, 0, 0);

	float x1 = _BBoxStart.x();
	float x2 = _BBoxEnd.x();
	float y1 = _BBoxStart.y();
	float y2 = _BBoxEnd.y();
	float z1 = _BBoxStart.z();
	float z2 = _BBoxEnd.z();

	int32 screenX = 0;
	int32 screenY = 0;

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x1, y1, z1), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x1, y1, z2), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x1, y2, z1), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x1, y2, z2), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x2, y1, z1), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x2, y1, z2), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x2, y2, z1), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);

	_gameRef->_renderer3D->project(_lastWorldMat, Math::Vector3d(x2, y2, z2), screenX, screenY);
	updateRect(&_boundingRect, screenX, screenY);
}

//////////////////////////////////////////////////////////////////////////
void XModel::updateRect(Rect32 *rc, int32 x, int32 y) {
	rc->left   = MIN(rc->left, x);
	rc->right  = MAX(rc->right, x);
	rc->top    = MIN(rc->top, y);
	rc->bottom = MAX(rc->bottom, y);
}

//////////////////////////////////////////////////////////////////////////
AnimationSet *XModel::getAnimationSetByName(const Common::String &name) {
	for (uint32 i = 0; i < _animationSets.size(); i++) {
		if (name.equalsIgnoreCase(_animationSets[i]->_name)) {
			return _animationSets[i];
		}
	}

	return nullptr;
}

TOKEN_DEF_START
	TOKEN_DEF(NAME)
	TOKEN_DEF(LOOPING)
	TOKEN_DEF(EVENT)
	TOKEN_DEF(FRAME)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool XModel::parseAnim(byte *buffer) {
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(NAME)
		TOKEN_TABLE(LOOPING)
		TOKEN_TABLE(EVENT)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	char *name = nullptr;
	bool looping = false;
	bool loopingSet = false;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME: {
			BaseUtils::setString(&name, (char *)params);

			AnimationSet *anim = getAnimationSetByName(name);
			if (!anim) {
				_gameRef->LOG(0, "Error: Animation '%s' cannot be found in the model.", name);
			}
			break;
		}

		case TOKEN_LOOPING:
			parser.scanStr((char *)params, "%b", &looping);
			loopingSet = true;
			break;

		case TOKEN_EVENT:
			if (!name) {
				_gameRef->LOG(0, "Error: NAME filed must precede any EVENT fields in actor definition files.");
			} else {
				AnimationSet *anim = getAnimationSetByName(name);
				if (anim)
					parseEvent(anim, params);
			}
			break;
		}
	}

	if (cmd != PARSERR_EOF) {
		return false;
	}

	bool ret = true;

	if (name) {
		AnimationSet *anim = getAnimationSetByName(name);
		if (anim) {
			if (loopingSet)
				anim->_looping = looping;
		}
	}

	delete[] name;

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::parseEvent(AnimationSet *anim, byte *buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(FRAME)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	AnimationSet::AnimationEvent *event = new AnimationSet::AnimationEvent();
	if (!event) {
		return false;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			BaseUtils::setString(&event->_eventName, (char *)params);
			break;

		case TOKEN_FRAME:
			parser.scanStr((char *)params, "%d", &event->_frame);
			break;
		}
	}

	if (cmd != PARSERR_EOF) {
		delete event;
		return false;
	}

	if (event->_eventName) {
		anim->addEvent(event);
	} else {
		delete event;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::setMaterialSprite(const char *materialName, const char *spriteFilename) {
	if (!materialName || !spriteFilename) {
		return false;
	}

	if (!_rootFrame) {
		return false;
	}

	BaseSprite *sprite = new BaseSprite(_gameRef);
	if (!sprite || !sprite->loadFile(spriteFilename)) {
		delete sprite;
		return false;
	}

	XModelMatSprite *matSprite = nullptr;
	for (uint32 i = 0; i < _matSprites.size(); i++) {
		if (scumm_stricmp(_matSprites[i]->_matName, materialName) == 0) {
			matSprite = _matSprites[i];
			break;
		}
	}
	if (matSprite) {
		matSprite->setSprite(sprite);
	} else {
		matSprite = new XModelMatSprite(materialName, sprite);
		_matSprites.add(matSprite);
	}

	_rootFrame->setMaterialSprite(matSprite->_matName, matSprite->_sprite);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::setMaterialTheora(const char *materialName, const char *theoraFilename) {
	if (!materialName || !theoraFilename) {
		return false;
	}

	if (!_rootFrame) {
		return false;
	}

	VideoTheoraPlayer *theora = new VideoTheoraPlayer(_gameRef);

	if (!theora || theora->initialize(theoraFilename)) {
		delete theora;
		return false;
	}

	theora->play(VID_PLAY_POS, 0, 0, false, false, true);

	XModelMatSprite *matSprite = nullptr;
	for (uint32 i = 0; i < _matSprites.size(); i++) {
		if (scumm_stricmp(_matSprites[i]->_matName, materialName) == 0) {
			matSprite = _matSprites[i];
			break;
		}
	}

	if (matSprite) {
		matSprite->setTheora(theora);
	} else {
		matSprite = new XModelMatSprite(materialName, theora);
		_matSprites.add(matSprite);
	}
	_rootFrame->setMaterialTheora(matSprite->_matName, matSprite->_theora);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::initializeSimple() {
	if (!_rootFrame) {
		return false;
	}

	// init after load
	for (uint32 i = 0; i < _matSprites.size(); i++) {
		if (_matSprites[i]->_theora) {
			_rootFrame->setMaterialTheora(_matSprites[i]->_matName, _matSprites[i]->_theora);
		} else if (_matSprites[i]->_sprite) {
			_rootFrame->setMaterialSprite(_matSprites[i]->_matName, _matSprites[i]->_sprite);
		}
	}
	// TODO: Effects

	if (_parentModel) {
		findBones(false, _parentModel);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transferVector3d(TMEMBER(_BBoxStart));
	persistMgr->transferVector3d(TMEMBER(_BBoxEnd));
	persistMgr->transferRect32(TMEMBER(_boundingRect));

	if (!persistMgr->getIsSaving()) {
		_drawingViewport.setEmpty();
	}

	persistMgr->transferSint32(TMEMBER(_lastOffsetX));
	persistMgr->transferSint32(TMEMBER(_lastOffsetY));

	persistMgr->transferMatrix4(TMEMBER(_lastProjMat));
	persistMgr->transferMatrix4(TMEMBER(_lastViewMat));
	persistMgr->transferMatrix4(TMEMBER(_lastWorldMat));

	persistMgr->transferPtr(TMEMBER(_owner));
	_mergedModels.persist(persistMgr);

	// load model
	if (!persistMgr->getIsSaving()) {
		for (int i = 0; i < X_NUM_ANIMATION_CHANNELS; i++) {
			_channels[i] = nullptr;
		}

		_rootFrame = nullptr;

		if (getFilename()) {
			loadFromFile(getFilename());
		}

		for (uint i = 0; i < _mergedModels.size(); ++i) {
			mergeFromFile(_mergedModels[i]);
		}
	}

	persistMgr->transferPtr(TMEMBER(_parentModel));

	// animation properties
	int32 numAnims;
	if (persistMgr->getIsSaving()) {
		numAnims = _animationSets.size();
	}

	persistMgr->transferSint32(TMEMBER(numAnims));

	if (persistMgr->getIsSaving()) {
		for (uint32 i = 0; i < _animationSets.size(); i++) {
			persistMgr->transferCharPtr(TMEMBER(_animationSets[i]->_name));
			_animationSets[i]->persist(persistMgr);
		}
	} else {
		for (int i = 0; i < numAnims; i++) {
			bool needsDelete = false;
			char *animName;
			persistMgr->transferCharPtr(TMEMBER(animName));
			AnimationSet *animSet = getAnimationSetByName(animName);
			if (!animSet) {
				animSet = new AnimationSet(_gameRef, this);
				needsDelete = true;
			}

			animSet->persist(persistMgr);
			if (needsDelete) {
				delete animSet;
			}

			delete[] animName;
		}
	}

	// persist channels
	for (int i = 0; i < X_NUM_ANIMATION_CHANNELS; i++) {
		_channels[i]->persist(persistMgr);
	}

	// persist material sprites
	int32 numMatSprites;
	if (persistMgr->getIsSaving()) {
		numMatSprites = _matSprites.size();
	}

	persistMgr->transferSint32(TMEMBER(numMatSprites));

	for (int i = 0; i < numMatSprites; i++) {
		if (persistMgr->getIsSaving()) {
			_matSprites[i]->persist(persistMgr);
		} else {
			XModelMatSprite *MatSprite = new XModelMatSprite();
			MatSprite->persist(persistMgr);
			_matSprites.add(MatSprite);
		}
	}

	if (!persistMgr->getIsSaving())
		initializeSimple();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool XModel::invalidateDeviceObjects() {
	if (_rootFrame) {
		return _rootFrame->invalidateDeviceObjects();
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool XModel::restoreDeviceObjects() {
	if (_rootFrame) {
		return _rootFrame->restoreDeviceObjects();
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool XModel::unloadAnimation(const char *animName) {
	bool found = false;
	for (uint32 i = 0; i < _animationSets.size(); i++) {
		if (scumm_stricmp(animName, _animationSets[i]->_name) == 0) {
			for (int j = 0; j < X_NUM_ANIMATION_CHANNELS; j++) {
				if (_channels[j])
					_channels[j]->unloadAnim(_animationSets[i]);
			}

			found = true;
			delete _animationSets[i];
			_animationSets.remove_at(i);
			i++;
		}
	}
	return found;
}

} // namespace Wintermute
