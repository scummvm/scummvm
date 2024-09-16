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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"

#include "qdengine/system/graphics/gr_dispatcher.h"

#include "qdengine/qdcore/qd_game_dispatcher.h"

#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_interface_screen.h"
#include "qdengine/qdcore/qd_interface_text_window.h"
#include "qdengine/qdcore/qd_interface_slider.h"


namespace QDEngine {

const float qdInterfaceCaretPeriod = 0.3f;

bool qdInterfaceTextWindow::_caretVisible = false;
float qdInterfaceTextWindow::_caretTimer = 0.f;

qdInterfaceTextWindow::qdInterfaceTextWindow() : _text_size(0, 0),
	_scrolling_speed(0),
	_text_set(NULL),
	_text_set_id(qdGameDispatcher::TEXT_SET_DIALOGS),
	_slider(NULL),
	_windowType(WINDOW_DIALOGS),
	_infoType(INFO_NONE) {
	_textFormat.toggle_global_depend(false);

	_inputStringLimit = 0;
	_scrolling_position = 0;
	_text_set_position = 0;

	_textVAlign = VALIGN_BOTTOM;

	_background_color = 0;
	_has_background_color = false;
	_background_alpha = 0;

	_isEditing = false;
	_caretPose = -1;

	_playerID = 0;

	_border_background.set_owner(this);
}

qdInterfaceTextWindow::qdInterfaceTextWindow(const qdInterfaceTextWindow &wnd) : qdInterfaceElement(wnd),
	_text_size(wnd._text_size),
	_scrolling_speed(wnd._scrolling_speed),
	_text_set(wnd._text_set),
	_text_set_id(wnd._text_set_id),
	_slider_name(wnd._slider_name),
	_slider(wnd._slider),
	_windowType(wnd._windowType),
	_infoType(wnd._infoType),
	_playerID(wnd._playerID),
	_inputString(wnd._inputString),
	_inputStringBackup(wnd._inputStringBackup),
	_inputStringLimit(wnd._inputStringLimit),
	_textFormat(wnd._textFormat),
	_textVAlign(wnd._textVAlign) {
	_scrolling_position = 0;
	_text_set_position = 0;

	_background_color = wnd._background_color;
	_has_background_color = wnd._has_background_color;
	_background_alpha = wnd._background_alpha;

	_isEditing = wnd._isEditing;
	_caretPose = wnd._caretPose;

	_border_background.set_owner(this);
	_border_background = wnd._border_background;
}

qdInterfaceTextWindow &qdInterfaceTextWindow::operator = (const qdInterfaceTextWindow &wnd) {
	if (this == &wnd) return *this;

	*static_cast<qdInterfaceElement *>(this) = wnd;

	_text_size = wnd._text_size;
	_scrolling_speed = wnd._scrolling_speed;
	_text_set = wnd._text_set;
	_text_set_id = wnd._text_set_id;
	_slider_name = wnd._slider_name;
	_slider = wnd._slider;

	_windowType = wnd._windowType;
	_infoType = wnd._infoType;
	_playerID = wnd._playerID;

	_inputString = wnd._inputString;
	_inputStringBackup = wnd._inputStringBackup;
	_inputStringLimit = wnd._inputStringLimit;
	_textFormat = wnd._textFormat;

	_textVAlign = wnd._textVAlign;

	_isEditing = wnd._isEditing;
	_caretPose = wnd._caretPose;

	_border_background = wnd._border_background;
	_background_color = wnd._background_color;

	return *this;
}

qdInterfaceTextWindow::~qdInterfaceTextWindow() {
	_border_background.unregister_resources();
}

bool qdInterfaceTextWindow::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	if (!_text_set) return false;

	if (_windowType == WINDOW_DIALOGS) {
		if (ev == mouseDispatcher::EV_LEFT_DOWN) {
			if (qdScreenText * p = _text_set->get_text(x, y)) {
				if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
					dp->set_flag(qdGameDispatcher::DIALOG_CLICK_FLAG);
					dp->set_mouse_click_state(p->owner());
				}
				return true;
			}
		} else if (ev == mouseDispatcher::EV_MOUSE_MOVE) {
			_text_set->clear_hover_mode();
			if (qdScreenText *p = _text_set->get_text(x, y))
				p->set_hover_mode(true);
		}
	}

	return false;
}

bool qdInterfaceTextWindow::keyboard_handler(Common::KeyCode vkey) {
	if (_windowType == WINDOW_EDIT && _isEditing)
		return edit_input(vkey);

	return false;
}

bool qdInterfaceTextWindow::char_input_handler(int input) {
	warning("STUB: qdInterfaceTextWindow::char_input_handler");
	bool ret = false;
#if 0
	bool ret = __super::char_input_handler(input);
#endif
	if (_windowType == WINDOW_EDIT && _isEditing) {
		if (!_inputStringLimit || (int)_inputString.size() < _inputStringLimit) {
			if (Common::isPrint(input) || input == '_' || input == '-' || input == ' ') {
				_inputString.insertChar(input, _caretPose++);
				return true;
			}
		}
	}

	return ret;
}

void qdInterfaceTextWindow::hover_clear() {
	if (_windowType == WINDOW_DIALOGS) {
		if (_text_set)
			_text_set->clear_hover_mode();
	}
}

bool qdInterfaceTextWindow::init(bool is_game_active) {
	set_state(&_border_background);

	if (_windowType == WINDOW_DIALOGS) {
		if (!_text_set)
			_text_set = qdGameDispatcher::get_dispatcher()->screen_texts_dispatcher().get_text_set(_text_set_id);

		if (!_slider) {
			if (qdInterfaceScreen * p = static_cast<qdInterfaceScreen * >(owner()))
				_slider = dynamic_cast<qdInterfaceSlider * >(p->get_element(slider_name()));
		}

		if (_text_set) {
			_text_set->set_max_text_width(_text_size.x);
			update_text_position();
		}
	} else {
		qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();

		switch (_infoType) {
		case INFO_PLAYER_NAME:
			_inputString = dp->hall_of_fame_player_name(_playerID);
			if (dp->is_hall_of_fame_updated(_playerID) && !_isEditing)
				edit_start();
			break;
		case INFO_PLAYER_SCORE:
			if (dp->hall_of_fame_player_score(_playerID)) {
				Common::String buf;
				buf += Common::String::format("%d", dp->hall_of_fame_player_score(_playerID));
				_inputString = buf.c_str();
			} else
				_inputString = "";
			break;
		default:
			break;
		}
		if (_windowType == WINDOW_TEXT)
			set_input_string(input_string());
	}
	return true;
}

bool qdInterfaceTextWindow::save_script_body(Common::WriteStream &fh, int indent) const {
	if (_border_background.has_animation()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<border_back>%s</border_back>\r\n", qdscr_XML_string(_border_background.animation_file().toString('\\'))));
	}

	if (!_slider_name.empty()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<window_slider>%s</window_slider>\r\n", qdscr_XML_string(_slider_name.c_str())));
	}

	if (_text_size.x || _text_size.y) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<screen_size>%d %d</screen_size>\r\n", _text_size.x, _text_size.y));
	}

	if (_background_color) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<color>%u</color>\r\n", _background_color));
	}

	if (_has_background_color) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString("<enable_background>1</enable_background>\r\n");
	}

	if (_background_alpha) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<alpha>%d</alpha>\r\n", _background_alpha));
	}

	if (_windowType != WINDOW_DIALOGS) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<window_type>%d</window_type>\r\n", (int)_windowType));
	}

	if (_infoType != INFO_NONE) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<info_type>%d</info_type>\r\n", (int)_infoType));
	}

	if (_playerID) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<player_id>%d</player_id>\r\n", _playerID));
	}

	if (!_inputString.empty()) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<text>%s</text>\r\n", qdscr_XML_string(_inputString.c_str())));
	}

	if (_inputStringLimit) {
		for (int i = 0; i <= indent; i++) {
			fh.writeString("\t");
		}
		fh.writeString(Common::String::format("<string_length>%d</string_length>\r\n", _inputStringLimit));
	}

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<valign>%d</valign>\r\n", _textVAlign));

	if (_textFormat != qdScreenTextFormat::default_format()) {
		_textFormat.save_script(fh, indent + 1);
	}

	return true;
}

bool qdInterfaceTextWindow::load_script_body(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_TEXT_WINDOW_BORDER_BACK:
			set_border_background_file(Common::Path(it->data(), '\\'));
			break;
		case QDSCR_TEXT_WINDOW_SLIDER:
			set_slider_name(it->data());
			break;
		case QDSCR_SCREEN_SIZE:
			xml::tag_buffer(*it) > _text_size.x > _text_size.y;
			break;
		case QDSCR_COLOR:
			xml::tag_buffer(*it) > _background_color;
			break;
		case QDSCR_ALPHA:
			xml::tag_buffer(*it) > _background_alpha;
			break;
		case QDSCR_ENABLE_BACKGROUND:
			_has_background_color = xml::tag_buffer(*it).get_int() != 0;
			break;
		case QDSCR_TEXT_WINDOW_TYPE:
			_windowType = WindowType(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_WINDOW_INFO_TYPE:
			_infoType = InfoType(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_WINDOW_PLAYER_ID:
			xml::tag_buffer(*it) > _playerID;
			break;
		case QDSCR_SCREEN_TEXT_FORMAT:
			_textFormat.load_script(&*it);
			_textFormat.toggle_global_depend(false);
			break;
		case QDSCR_TEXT_WINDOW_MAX_STRING_LENGTH:
			xml::tag_buffer(*it) > _inputStringLimit;
			break;
		case QDSCR_TEXT:
			_inputString = it->data();
			break;
		case QDSCR_VALIGN:
			_textVAlign = TextVAlign(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdInterfaceTextWindow::redraw() const {
	qdInterfaceElement::redraw();

	if (_windowType == WINDOW_DIALOGS) {
		if (_text_set) {
			int l_clip, t_clip, r_clip, b_clip;
			grDispatcher::instance()->getClip(l_clip, t_clip, r_clip, b_clip);

			Vect2i ar = r();
			grDispatcher::instance()->limitClip(ar.x - _text_size.x / 2, ar.y - _text_size.y / 2, ar.x + _text_size.x / 2, ar.y + _text_size.y / 2);

			if (_has_background_color) {
				Vect2i text_r = _text_set->screen_pos();
				Vect2i text_sz = _text_set->screen_size();

				if (!_background_alpha)
					grDispatcher::instance()->rectangle(ar.x - _text_size.x / 2, text_r.y - text_sz.y / 2, _text_size.x, text_sz.y, _background_color, _background_color, GR_FILLED);
				else
					grDispatcher::instance()->rectangleAlpha(ar.x - _text_size.x / 2, text_r.y - text_sz.y / 2, _text_size.x, text_sz.y, _background_color, _background_alpha);
			}

			_text_set->redraw();

			grDispatcher::instance()->setClip(l_clip, t_clip, r_clip, b_clip);

			if (g_engine->_debugDraw)
				grDispatcher::instance()->rectangle(ar.x - _text_size.x / 2, ar.y - _text_size.y / 2, _text_size.x, _text_size.y, 0xFFFFFF, 0, GR_OUTLINED, 3);
		}
	} else if (_windowType == WINDOW_EDIT || _windowType == WINDOW_TEXT) {
		if (_has_background_color) {
			Vect2i ar = r();
			if (!_background_alpha)
				grDispatcher::instance()->rectangle(ar.x - _text_size.x / 2, ar.y - _text_size.y / 2, _text_size.x, _text_size.y, _background_color, _background_color, GR_FILLED);
			else
				grDispatcher::instance()->rectangleAlpha(ar.x - _text_size.x / 2, ar.y - _text_size.y / 2, _text_size.x, _text_size.y, _background_color, _background_alpha);
		}

		text_redraw();
	}

	return true;
}

bool qdInterfaceTextWindow::need_redraw() const {
	if (qdInterfaceElement::need_redraw())
		return true;

	if (_windowType == WINDOW_DIALOGS) {
		if (_text_set && _text_set->need_redraw())
			return true;

		if (_slider && _slider->need_redraw())
			return true;
	} else if (_windowType == WINDOW_EDIT || _windowType == WINDOW_TEXT)
		return true;

	return false;
}

bool qdInterfaceTextWindow::quant(float dt) {
	qdInterfaceElement::quant(dt);

	if (_isEditing) {
		_caretTimer -= dt;
		if (_caretTimer < 0.f) {
			_caretVisible = !_caretVisible;
			_caretTimer = qdInterfaceCaretPeriod;
		}
	}

	if (_windowType == WINDOW_DIALOGS) {
		if (is_visible() && _text_set && _text_set->was_changed())
			update_text_position();

		if (_text_set) {
			if (fabs(_scrolling_position) > FLT_EPS) {
				float delta = _scrolling_speed * dt;
				if (fabs(_scrolling_position) > delta)
					_scrolling_position += (_scrolling_position > 0) ? -delta : delta;
				else
					_scrolling_position = 0;

				Vect2i pos = _text_set->screen_pos();
				pos.y = _text_set_position + round(_scrolling_position);
				_text_set->set_screen_pos(pos);
			}

			if (_text_set->is_empty()) {
				if (is_visible()) {
					hide();
					if (qdInterfaceScreen * sp = dynamic_cast<qdInterfaceScreen * >(owner()))
						sp->build_visible_elements_list();

					qdGameDispatcher::get_dispatcher()->toggle_full_redraw();
				}
			} else {
				if (!is_visible()) {
					show();
					if (qdInterfaceScreen * sp = dynamic_cast<qdInterfaceScreen * >(owner()))
						sp->build_visible_elements_list();
				}
			}

			if (_slider) {
				if (!is_visible() || _text_size.y > _text_set->screen_size().y) {
					if (_slider->is_visible()) {
						_slider->hide();

						if (qdInterfaceScreen * sp = dynamic_cast<qdInterfaceScreen * >(owner()))
							sp->build_visible_elements_list();

						qdGameDispatcher::get_dispatcher()->toggle_full_redraw();
					}
				} else {
					if (!_slider->is_visible()) {
						_slider->show();

						if (qdInterfaceScreen * sp = dynamic_cast<qdInterfaceScreen * >(owner()))
							sp->build_visible_elements_list();
					}
				}
			}
		}
	}

	return true;
}

void qdInterfaceTextWindow::update_text_position() {
	if (_text_set) {
		Vect2f rr = r() - text_size() / 2 + _text_set->screen_size() / 2;
		switch (_textVAlign) {
		case VALIGN_CENTER:
			rr.y += (float)(text_size().y - _text_set->screen_size().y) / 2.0;
			break;
		case VALIGN_BOTTOM:
			rr.y += text_size().y - _text_set->screen_size().y;
			break;
		default:
			break;
		}
		_text_set->set_screen_pos(rr);
		_text_set_position = rr.y;

		set_scrolling(_text_set->new_texts_height());
		_text_set->clear_new_texts_height();

		_text_set->toggle_changed(false);
	}
}

bool qdInterfaceTextWindow::hit_test(int x, int y) const {
	x -= r().x;
	y -= r().y;

	if (x >= -size_x() / 2 && x < size_x() / 2 && y >= -size_y() / 2 && y < size_y() / 2)
		return true;

	return false;
}

grScreenRegion qdInterfaceTextWindow::screen_region() const {
	return grScreenRegion(r().x, r().y, size_x(), size_y());
}

int qdInterfaceTextWindow::size_x() const {
	return (qdInterfaceElement::size_x() > _text_size.x) ? qdInterfaceElement::size_x() : _text_size.x;
}

int qdInterfaceTextWindow::size_y() const {
	return (qdInterfaceElement::size_y() > _text_size.y) ? qdInterfaceElement::size_y() : _text_size.y;
}

void qdInterfaceTextWindow::set_scrolling(int y_delta) {
	if (_scrolling_speed > FLT_EPS)
		_scrolling_position = y_delta;
	else
		_scrolling_position = 0;
}

void qdInterfaceTextWindow::text_redraw() const {
	Vect2i ar = r() - _text_size / 2;

	uint32 col = _textFormat.color();

	const grFont *font = qdGameDispatcher::get_dispatcher()->
	                     find_font(_textFormat.font_type());

	if (_windowType == WINDOW_EDIT && _isEditing) {
		int sz0 = grDispatcher::instance()->textWidth("|", 0, font);
		int sz1 = grDispatcher::instance()->textWidth(_inputString.c_str(), 0, font);
		int x0 = ar.x;
		switch (_textFormat.alignment()) {
		case qdScreenTextFormat::ALIGN_LEFT:
			break;
		case qdScreenTextFormat::ALIGN_CENTER:
			x0 += (_text_size.x - sz0 - sz1) / 2;
			break;
		case qdScreenTextFormat::ALIGN_RIGHT:
			x0 += _text_size.x - sz0 - sz1;
			break;
		}

		Common::String str = _inputString.substr(0, _caretPose);
		if (!str.empty()) {
			grDispatcher::instance()->drawAlignedText(x0, ar.y, _text_size.x, _text_size.y, col, str.c_str(), GR_ALIGN_LEFT, 0, 0, font);
			x0 += grDispatcher::instance()->textWidth(str.c_str(), 0, font);
		}
		if (_caretVisible)
			grDispatcher::instance()->drawAlignedText(x0, ar.y, _text_size.x, _text_size.y, col, "|", GR_ALIGN_LEFT, 0, 0, font);
		x0 += grDispatcher::instance()->textWidth("|", 0, font);

		str = _inputString.substr(_caretPose, Common::String::npos);
		if (!str.empty())
			grDispatcher::instance()->drawAlignedText(x0, ar.y, _text_size.x, _text_size.y, col, str.c_str(), GR_ALIGN_LEFT, 0, 0, font);
	} else {
		if (_windowType == WINDOW_TEXT)
			grDispatcher::instance()->drawParsedText(ar.x, ar.y, _text_size.x, _text_size.y, col, &_parser, grTextAlign(_textFormat.alignment()), font);
		else
			grDispatcher::instance()->drawAlignedText(ar.x, ar.y, _text_size.x, _text_size.y, col, _inputString.c_str(), grTextAlign(_textFormat.alignment()), 0, 0, font);
	}
}

bool qdInterfaceTextWindow::edit_input(Common::KeyCode vkey) {
	if (_isEditing) {
		switch (vkey) {
		case Common::KEYCODE_ESCAPE:
			if (!edit_done(true))
				return false;
			return true;

		case Common::KEYCODE_RETURN:
			if (!edit_done(false))
				return false;
			return true;

		case Common::KEYCODE_LEFT:
			if (_caretPose > 0)
				--_caretPose;
			return true;

		case Common::KEYCODE_HOME:
			_caretPose = 0;
			return true;

		case Common::KEYCODE_END:
			_caretPose = _inputString.size();
			return true;

		case Common::KEYCODE_RIGHT:
			if (_caretPose < (int)_inputString.size())
				++_caretPose;
			return true;

		case Common::KEYCODE_BACKSPACE:
			if (_caretPose > 0 && _caretPose <= (int)_inputString.size())
				_inputString.erase(--_caretPose, 1);
			return true;

		case Common::KEYCODE_DELETE:
			if (_caretPose >= 0 && _caretPose < (int)_inputString.size())
				_inputString.erase(_caretPose, 1);

			return true;

		default:
			break;
		}

		return true;
	}

	return false;
}

void qdInterfaceTextWindow::set_input_string(const char *str) {
	_inputString = str;

	if (_windowType == WINDOW_TEXT) {
		_parser.setFont(qdGameDispatcher::get_dispatcher()->find_font(_textFormat.font_type()));
		_parser.parseString(_inputString.c_str(), _textFormat.color());
	}
}

bool qdInterfaceTextWindow::edit_start() {
	if (!_isEditing) {
		_isEditing = true;
		_inputStringBackup = _inputString;
		_caretPose = _inputString.size();
		return true;
	}

	return false;
}

bool qdInterfaceTextWindow::edit_done(bool cancel) {
	if (_isEditing) {
		bool end_edit = true;

		switch (_infoType) {
		case INFO_NONE:
			if (qdInterfaceDispatcher * dp = qdInterfaceDispatcher::get_dispatcher())
				dp->handle_event(!cancel ? qdInterfaceEvent::EVENT_MODAL_OK : qdInterfaceEvent::EVENT_MODAL_CANCEL, 0, this);
			break;
		case INFO_PLAYER_NAME:
			if (cancel || _inputString.empty()) {
				end_edit = false;
			} else {
				if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
					dp->set_hall_of_fame_player_name(_playerID, _inputString.c_str());
					dp->save_hall_of_fame();
				}
			}
			break;
		default:
			break;
		}

		if (end_edit) {
			_isEditing = false;

			if (cancel)
				_inputString = _inputStringBackup;
		}

		return true;
	}

	return false;
}
} // namespace QDEngine
