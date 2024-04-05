#ifndef __UI_TEXT_PARSER_H__
#define __UI_TEXT_PARSER_H__

#include "gr_font.h"

struct OutNode{
	enum{
		NEW_LINE,
		TEXT,
		COLOR
	} type;
	int width;
	union{
		struct {
			const char* begin;
			const char* end;
		};
		struct {
			int style;
		};
		int color;
	};
	OutNode() : type(NEW_LINE), width(0), begin(0), end(0) {}
	OutNode(const char* b, const char* e, int wd) : type(TEXT), width(wd), begin(b), end(e) {}
	OutNode(int clr) : type(COLOR), width(0), color(clr) {}
};

typedef std::vector<OutNode> OutNodes;

class UI_TextParser
{
public:
	UI_TextParser(const grFont* font = 0);
	UI_TextParser(const UI_TextParser& src);
	
	void operator= (const UI_TextParser& src);
	
	void setFont(const grFont* font);

	void parseString(const char* text, int color = 0, int fitIn = -1);

	const OutNodes& outNodes() const { return outNodes_; }
	
	int fontHeight() const { return font_ ? font_->size_y() : 1; }
	const Vect2i& size() const { return size_; }

	int lineCount() const { return lineCount_; }
	OutNodes::const_iterator getLineBegin(int lineNum) const;

private:
	void init();

	__forceinline int fromHex(char a)
	{
		if(a>='0' && a<='9')
			return a-'0';
		if(a>='A' && a<='F')
			return a-'A'+10;
		if(a>='a' && a<='f')
			return a-'a'+10;
		return -1;
	}

	__forceinline void addChar(BYTE cc)
	{
		int width = font_->char_width(cc);
		if(testWidth(width) || cc != ' ')
			tagWidth_ += width;
		++pstr_;
	}

	__forceinline void skipNode()
	{
		lineBegin_ = pstr_;
		lastSpace_ = lineBegin_;
		lastTagWidth_ = 0;
		tagWidth_ = 0;
	}

	__forceinline void putNode(OutNode& node)
	{
		outNodes_.push_back(node);
		skipNode();
	}

	void putText()
	{
		if(pstr_ == lineBegin_)
			return;
		lineWidth_ += tagWidth_;
		putNode(OutNode(lineBegin_, pstr_, tagWidth_));
	}

	void endLine()
	{
		size_.x = max(size_.x, lineWidth_);

		outNodes_[prevLineIndex_].width = lineWidth_;
		lineWidth_ = 0;

		outNodes_.push_back(OutNode());
		prevLineIndex_ = outNodes_.size() - 1;

		++lineCount_;
	}

	void getColor(int defColor);
	int getStyle(const char* styleptr, const char* end);
	int getToken();
	bool testWidth(int width);

	OutNodes outNodes_;

	int prevLineIndex_;
	const char* lastSpace_;
	int lastTagWidth_;

	const char* lineBegin_;
	const char* pstr_;
	int tagWidth_;
	int lineWidth_;

	int fitIn_;

	Vect2i size_;
	int lineCount_;

	const grFont* font_;

};

#endif //__UI_TEXT_PARSER_H__
