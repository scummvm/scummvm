/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/emi/costume/emisprite_component.h"
#include "engines/grim/resource.h"
#include "engines/grim/costume.h"
#include "engines/grim/model.h"

namespace Grim {

EMISpriteComponent::EMISpriteComponent(Component *p, int parentID, const char *filename, Component *prevComponent, tag32 t) : Component(p, parentID, filename, t), _sprite(NULL) {
}

EMISpriteComponent::~EMISpriteComponent() {
	delete _sprite;
}

void EMISpriteComponent::init() {

	// FIXME: this code should probably go into a sprite class.
	Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(_name+"b");
	if (!stream)
		return;
	uint32 namelength = stream->readUint32LE();
	char *name = new char[namelength];
	stream->read(name, namelength);
	delete[] name;
	stream->seek(40, SEEK_CUR);
	uint32 texnamelength = stream->readUint32LE();
	char *texname = new char[texnamelength];
	stream->read(texname, texnamelength);
	/* unknown = */ stream->readUint32LE();
	float width, height;
	char data[8];
	stream->read(data, sizeof(data));
	width = get_float(data);
	height = get_float(data+4);


	_sprite = new Sprite();
	_sprite->_material = g_resourceloader->loadMaterial(texname, 0);
	_sprite->_width = width;
	_sprite->_height = height;
	_sprite->_next = NULL;
	_sprite->_visible = true;
	_sprite->_pos.set(0,0,0);

	delete[] texname;
	delete stream;

}

int EMISpriteComponent::update(uint time) {
	return 0;
}

void EMISpriteComponent::reset() {
}

void EMISpriteComponent::draw() {
	if (_sprite) {
		_sprite->draw();
	}
}

} // end of namespace Grim
