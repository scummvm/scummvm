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

#include "engines/metaengine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_displacement.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXDisplacement, false)

BaseScriptable *makeSXDisplacement(BaseGame *inGame, ScStack *stack) {
	return new SXDisplacement(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXDisplacement::SXDisplacement(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(5);

	_subFrameSrc = static_cast<BaseSubFrame *>(stack->pop()->getNative());
	_subFrameDst = static_cast<BaseSubFrame *>(stack->pop()->getNative());
	_forceX = stack->pop()->getInt();
	_forceY = stack->pop()->getInt();
	_speed = stack->pop()->getInt();
	_animIndex = 0;

	for (int32 i = 0; i < 1024; i++) {
		_tableX[i] = sin(i * 6.283 / 128.0);
		_tableY[i] = cos(i * 6.283 / 128.0);
	}

	int32 width = _subFrameSrc->getWidth();
	int32 height = _subFrameSrc->getHeight();

	_subFrameSrc->startPixelOperations();
	_subFrameDst->startPixelOperations();

	for (int32 y = 0; y < height; y++) {
		for (int32 x = 0; x < width; x++) {
			uint32 pixel = _subFrameSrc->getPixel(x, y);
			_subFrameDst->putPixel(x, y, pixel);
		}
	}

	_subFrameSrc->endPixelOperations();
	_subFrameDst->endPixelOperations();
}

//////////////////////////////////////////////////////////////////////////
SXDisplacement::~SXDisplacement() {
}

//////////////////////////////////////////////////////////////////////////
const char *SXDisplacement::scToString() {
	return "[displacement object]";
}

//////////////////////////////////////////////////////////////////////////
bool SXDisplacement::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetMe()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetMe") == 0) {
		stack->correctParams(2);

		_subFrameSrc = static_cast<BaseSubFrame *>(stack->pop()->getNative());
		_subFrameDst = static_cast<BaseSubFrame *>(stack->pop()->getNative());

		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Setforces()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Setforces") == 0) {
		stack->correctParams(3);

		_forceX = stack->pop()->getInt();
		_forceY = stack->pop()->getInt();
		_speed = stack->pop()->getInt();

		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Animate()
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Animate") == 0) {
		stack->correctParams(0);

		if (_subFrameSrc) {
			int32 width = _subFrameSrc->getWidth();
			int32 height = _subFrameSrc->getHeight();

			_subFrameSrc->startPixelOperations();
			_subFrameDst->startPixelOperations();

			for (int32 y = 0; y < height; y++) {
				for (int32 x = 0; x < width; x++) {
					uint32 pixel = _subFrameSrc->getPixel(x, y);
					_subFrameDst->putPixel(x, y, pixel);
				}
			}

			_animIndex += _speed;
			if (_animIndex > 128)
				_animIndex = 0;

			for (int32 y = 0; y < height; y++) {
				for (int32 x = 0; x < width; x++) {
					uint32 pixel = _subFrameSrc->getPixel(x, y);
					int32 offsetX = (int32)(_forceX * _tableX[_animIndex + x]);
					int32 offsetY = (int32)(_forceY * _tableY[_animIndex + y]);
					if (width > (x + offsetX) &&
					    height > (y + offsetY) &&
					    (x + offsetX) >= 0 &&
					    (y + offsetY) >= 0) {
						_subFrameDst->putPixel(x + offsetX, y + offsetY, pixel);
					}
				}
			}

			_subFrameSrc->endPixelOperations();
			_subFrameDst->endPixelOperations();

			stack->pushBool(true);
			return STATUS_OK;
		} else {
			stack->pushBool(false);
		}
	}

	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
ScValue *SXDisplacement::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// ForceX
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ForceX") == 0) {
		_scValue->setInt(_forceX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ForceY
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ForceY") == 0) {
		_scValue->setInt(_forceY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Speed
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Speed") == 0) {
		_scValue->setInt(_speed);
		return _scValue;
	}

	else {
		return _scValue;
	}
}

//////////////////////////////////////////////////////////////////////////
bool SXDisplacement::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// ForceX
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ForceX") == 0) {
		_forceX = _scValue->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ForceY
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ForceY") == 0) {
		_forceY = _scValue->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Speed
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Speed") == 0) {
		_speed = _scValue->getInt();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool SXDisplacement::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_subFrameSrc));
	persistMgr->transferPtr(TMEMBER_PTR(_subFrameDst));
	persistMgr->transferSint32(TMEMBER(_forceX));
	persistMgr->transferSint32(TMEMBER(_forceY));
	persistMgr->transferSint32(TMEMBER(_speed));
	persistMgr->transferSint32(TMEMBER(_animIndex));

	if (!persistMgr->getIsSaving()) {
		for (int32 i = 0; i < 1024; i++) {
			_tableX[i] = sin(i * 6.283 / 128.0);
			_tableY[i] = cos(i * 6.283 / 128.0);
		}
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
