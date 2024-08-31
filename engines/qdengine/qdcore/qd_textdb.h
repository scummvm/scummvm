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

#ifndef QDENGINE_QDCORE_TEXTDB_H
#define QDENGINE_QDCORE_TEXTDB_H

namespace Common {
class SeekableReadStream;
}

namespace QDEngine {

/// База данных с текстами.
class qdTextDB {
public:
	qdTextDB();
	~qdTextDB();

	/// Очистка базы.
	void clear() {
		_texts.clear();
	}

	/// Возвращает текст с идентификатором text_id.
	/**
	Если текст не найден - вернет пустую строку.
	*/
	const char *getText(const char *text_id) const;

	/// Возвращает звук к тексту с идентификатором text_id.
	const char *getSound(const char *text_id) const;

	/// Возвращает комментарий текста с идентификатором text_id.
	const char *getComment(const char *text_id) const;

	/// Загрузка базы.
	/**
	Если clear_old_texts == true, то загруженная в данный момент база очищается.
	В финальной версии база комментариев игнорируется.
	*/
	bool load(Common::SeekableReadStream *fh, const char *comments_file_name = NULL, bool clear_old_texts = true);

	static qdTextDB &instance();

private:

	struct qdText {
		qdText(const char *text, const char *snd) : _text(text), _sound(snd) { }
		qdText() {}

		Common::String _text;
		Common::String _sound;
		Common::String _comment;
	};

	typedef Common::HashMap<Common::String, qdText> qdTextMap;
	qdTextMap _texts;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_TEXTDB_H
