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

#ifndef MTROPOLIS_MODIFIERS_H
#define MTROPOLIS_MODIFIERS_H

#include "mtropolis/runtime.h"
#include "mtropolis/data.h"

namespace MTropolis {

struct ModifierLoaderContext;
class MiniscriptProgram;

class BehaviorModifier : public Modifier, public IModifierContainer {
public:
	bool load(ModifierLoaderContext &context, const Data::BehaviorModifier &data);

	const Common::Array<Common::SharedPtr<Modifier> > &getModifiers() const override;
	void appendModifier(const Common::SharedPtr<Modifier> &modifier) override;

private:
	Common::Array<Common::SharedPtr<Modifier> > _children;
	Event _enableWhen;
	Event _disableWhen;
};

class MiniscriptModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MiniscriptModifier &data);

private:
	Event _enableWhen;

	Common::SharedPtr<MiniscriptProgram> _program;
};

class MessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::MessengerModifier &data);

private:
	Event _when;
	Event _send;

	MessageFlags _messageFlags;
	MessageWithType _messageWithType;
	uint32 _messageDestination;	// May be a MessageDestination or GUID
};

class IfMessengerModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::IfMessengerModifier &data);

private:
	Event _when;
	Event _send;

	MessageFlags _messageFlags;
	MessageWithType _messageWithType;
	uint32 _messageDestination; // May be a MessageDestination or GUID

	Common::SharedPtr<MiniscriptProgram> _program;
};

class BooleanVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::BooleanVariableModifier &data);

private:
	bool _value;
};

class PointVariableModifier : public Modifier {
public:
	bool load(ModifierLoaderContext &context, const Data::PointVariableModifier &data);

private:
	Point16 _value;
};

}	// End of namespace MTropolis

#endif
