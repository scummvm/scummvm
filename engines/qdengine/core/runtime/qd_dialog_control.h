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

#ifndef __QD_DIALOG_CONTROL_H__
#define __QD_DIALOG_CONTROL_H__



namespace QDEngine {

class qdlgOption {
public:
	qdlgOption() : control_ID_(-1), control_parent_(NULL) { }

	void set_control(HWND ctl_parent, int ctl_id) {
		control_parent_ = ctl_parent;
		control_ID_ = ctl_id;
	}
	void set_ini_key(const char *ini_file, const char *ini_section, const char *ini_key) {
		ini_file_ = ini_file;
		ini_section_ = ini_section;
		ini_key_ = ini_key;
	}

	bool enable_control(bool status = true) const;

	int control_ID() const {
		return control_ID_;
	}
	HWND control_parent() const {
		return control_parent_;
	}
	bool has_control() const {
		if (control_ID_ == -1 || !control_parent_) return false;
		return true;
	}

	virtual int value() const = 0;
	virtual void set_value(int val) const = 0;

	bool load_value() const;
	bool save_value() const;

private:

	int control_ID_;
	HWND control_parent_;

	std::string ini_file_;
	std::string ini_section_;
	std::string ini_key_;
};

class qdlgOptionSlider : public qdlgOption {
public:
	int value() const;
	void set_value(int val) const;

	void set_value_range(int min, int max, bool invert_value = false);

private:

	bool invert_value_;
};

class qdlgOptionCheckbox : public qdlgOption {
public:
	int value() const;
	void set_value(int val) const;
};

class qdlgOptionDroplist : public qdlgOption {
public:
	qdlgOptionDroplist() { }

	void init_control() const;

	int value() const;
	void set_value(int val) const;

	void add_item(const char *title, int value);
	bool enable_item(int value);
	bool disable_item(int value);
	bool is_value_enabled(int value) const;

private:
	class qdlgDroplistItem {
	public:
		qdlgDroplistItem(const char *title, int value) : title_(title), value_(value), is_enabled_(true) { }

		bool operator == (int val) const {
			return (value_ == val);
		}

		const char *title() const {
			return title_.c_str();
		}
		int value() const {
			return value_;
		}

		bool is_enabled() const {
			return is_enabled_;
		}
		void enable() {
			is_enabled_ = true;
		}
		void disable() {
			is_enabled_ = false;
		}

	private:

		std::string title_;
		int value_;

		bool is_enabled_;
	};

	typedef std::vector<qdlgDroplistItem> item_container_t;
	item_container_t items_;

	int default_value() const;
};

} // namespace QDEngine

#endif /* __QD_DIALOG_CONTROL_H__ */
