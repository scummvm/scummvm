
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

#ifndef BAGEL_BAGLIB_EXPRESSION_OBJECT_H
#define BAGEL_BAGLIB_EXPRESSION_OBJECT_H

#include "bagel/baglib/object.h"
#include "bagel/baglib/expression.h"

namespace Bagel {

/**
 * CBagExpressionObject is an object that can be place within the slide window.
 */
class CBagExpressionObject : public CBagObject {
private:
	CBagExpression *_expression;
	bool _conditionalFl;

public:
	CBagExpressionObject();
	virtual ~CBagExpressionObject();

	bool runObject() override;
	ParseCodes setInfo(CBagIfstream &istr) override;

	bool isConditional() const {
		return _conditionalFl;
	}
	void setConditional(bool b = true) {
		_conditionalFl = b;
	}
};

} // namespace Bagel

#endif
