/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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

#ifndef STARK_RESOURCES_DIALOG_H
#define STARK_RESOURCES_DIALOG_H

#include "common/str.h"

#include "engines/stark/resources/resource.h"
#include "engines/stark/resourcereference.h"

namespace Stark {

class XRCReadStream;

class Dialog : public Resource {
public:
	static const ResourceType::Type TYPE = ResourceType::kDialog;

	Dialog(Resource *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~Dialog();

	// Resource API
	void readData(XRCReadStream *stream) override;

protected:
	struct Line {
		ResourceReference _field_0;
		ResourceReference _field_30;
	};

	struct Reply {
		Common::Array<Line> _lines;
		uint32 _conditionType;
		ResourceReference _conditionReference;
		ResourceReference _conditionScriptReference;
		uint32 _conditionReversed;
		uint32 _field_88;
		uint32 _minChapter;
		uint32 _maxChapter;
		uint32 _field_84;
		uint32 _nextDialogIndex;
		ResourceReference _nextScriptReference;
	};

	struct Topic {
		Common::Array<Reply> _replies;
		uint32 _field_14;
	};

	void printData() override;

	Common::Array<Topic> _topics;
	uint32 _globalDialogIndex;
	uint32 _hasAskAbout;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_DIALOG_H
