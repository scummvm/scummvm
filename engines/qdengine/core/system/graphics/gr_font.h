#ifndef __GR_FONT_H__
#define __GR_FONT_H__

#include "gr_screen_region.h"

class XZipStream;

class grFont 
{
public:
	grFont();
	~grFont();

	bool load(const char* fname);

	bool load_index(XStream& fh);
	bool load_alpha(XStream& fh);

	bool load_index(XZipStream& fh);
	bool load_alpha(XZipStream& fh);

	int size_x() const { return size_x_; }
	int size_y() const { return size_y_; }

	int alpha_buffer_size_x() const { return alpha_buffer_sx_; }
	int alpha_buffer_size_y() const { return alpha_buffer_sy_; }

	const unsigned char* alpha_buffer() const { return alpha_buffer_; }

	const grScreenRegion& find_char(int code) const
	{
		grFontCharVector::const_iterator it = std::find(chars_.begin(),chars_.end(),code);
		if(it != chars_.end()) return it -> region_;

		return grScreenRegion::EMPTY;
	}

	int char_width(int code) const { return code == ' ' ? size_x()/2 : find_char(code).size_x(); }

private:

	int size_x_;
	int size_y_;

	int alpha_buffer_sx_;
	int alpha_buffer_sy_;
	unsigned char* alpha_buffer_;

	struct grFontChar
	{
		grFontChar() : code_(-1) { }

		int code_;
		grScreenRegion region_;

		bool operator == (int code) const { return (code_ == code); }
	};

	typedef std::vector<grFontChar> grFontCharVector;
	grFontCharVector chars_;
};

#endif /* __GR_FONT_H__ */