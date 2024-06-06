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

#ifndef __QD_FILE_OWNER__
#define __QD_FILE_OWNER__



namespace QDEngine {

class qdFileOwner {
public:
	qdFileOwner() : files_size_(0), cd_info_(0) { }

	unsigned files_size() const {
		return files_size_;
	}
	virtual bool get_files_list(qdFileNameList &files_to_copy, qdFileNameList &files_to_pack) const = 0;

	void add_to_CD(int cd_number) {
		cd_info_ |= 1 << cd_number;
	}
	void remove_from_CD(int cd_number) {
		cd_info_ &= ~(1 << cd_number);
	}
	bool is_on_CD(int cd_number = 1) const {
		int inf = (cd_info_) ? cd_info_ : 1;
		return (inf & (1 << cd_number));
	}
	void clear_cd_info() {
		cd_info_ = 0;
	}
	void calc_files_size();

	unsigned CD_info() const {
		return cd_info_;
	}
	void set_CD_info(unsigned inf) {
		cd_info_ = inf;
	}

private:

	unsigned cd_info_;
	unsigned files_size_;
};

} // namespace QDEngine

#endif // __QD_FILE_OWNER__
