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

#ifndef QDENGINE_QDCORE_QD_RESOURCE_H
#define QDENGINE_QDCORE_QD_RESOURCE_H

#include "common/path.h"


//! Базовый класс для игровых ресурсов.
/**
Анимации, звуки и т.д.
*/
namespace QDEngine {

class qdResource {
public:
	//! Форматы файлов.
	enum file_format_t {
		//! анимация - .qda
		RES_ANIMATION,
		//! спрайт - .tga
		RES_SPRITE,
		//! звук - .wav
		RES_SOUND,
		//! неопознанный формат
		RES_UNKNOWN
	};

	qdResource();
	qdResource(const qdResource &res);
	virtual ~qdResource();

	qdResource &operator = (const qdResource &res);

	//! Загружает в память данные ресурса.
	virtual bool load_resource() = 0;
	//! Выгружает из памяти данные ресурса.
	virtual bool free_resource() = 0;

	//! Устанавливает имя файла, в котором хранятся данные ресурса.
	virtual void set_resource_file(const Common::Path file_name) = 0;
	//! Возвращает имя файла, в котором хранятся данные ресурса.
	/**
	Если оно не задано, должна возвращаеть NULL.
	*/
	virtual const Common::Path resource_file() const = 0;

	//! Возвращает true, если данные ресурса загружены в память.
	bool is_resource_loaded() const {
		return _is_loaded;
	}

	static file_format_t file_format(const Common::Path file_name);

#ifdef __QD_DEBUG_ENABLE__
	virtual uint32 resource_data_size() const = 0;
#endif

protected:

	//! Устанавливает или скидывает флаг, показывающий что данные ресурса загружены в память.
	void toggle_resource_status(bool st = true) {
		_is_loaded = st;
	}

private:

	//! Равно true, если данные ресурса загружены в память.
	bool _is_loaded;
};

#ifdef __QD_DEBUG_ENABLE__
class qdResourceInfo {
public:
	qdResourceInfo(const qdResource *res = NULL, const qdNamedObject *owner = NULL);
	qdResourceInfo(const qdResourceInfo &inf) : _resource(inf._resource), _data_size(inf._data_size), _resource_owner(inf._resource_owner) { }
	~qdResourceInfo();

	qdResourceInfo &operator = (const qdResourceInfo &inf) {
		if (this == &inf) return *this;

		_resource = inf._resource;
		_resource_owner = inf._resource_owner;
		_data_size = inf._data_size;

		return *this;
	}

	bool operator < (const qdResourceInfo &inf) const {
		return _data_size < inf._data_size;
	}

	uint32 data_size() const {
		return _data_size;
	}

	qdResource::file_format_t file_format() const;

private:
	uint32 _data_size;
	const qdResource *_resource;
	const qdNamedObject *_resource_owner;
};

typedef Std::vector<qdResourceInfo> qdResourceInfoContainer;

#endif

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_RESOURCE_H
