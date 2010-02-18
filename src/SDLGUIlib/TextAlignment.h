#pragma once
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>


namespace TextAlignment
{
	enum Enum
	{
		TopLeft, Top, TopRight, Left, Centre, Right, BottomLeft, Bottom, BottomRight
	};
}

struct WidgetText
{
private:
	TextAlignment::Enum alignment_;
	std::string text_;
	std::vector<std::string> text_lines_;
	bool autowrap_;
	int line_break_length_;

	void LayoutText()
	{
		if(!autowrap_)
		{
			boost::split(text_lines_, text_, boost::is_any_of("\n"));
		} else
		{
			std::vector<std::string> lines;
			boost::split(lines, text_, boost::is_any_of("\n"));
			text_lines_.clear();
			for(std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
			{
				std::vector<std::string> words;
				std::string builder;
				boost::split(words, *it, boost::is_any_of(" "));
				for(std::vector<std::string>::iterator word = words.begin(); word != words.end(); ++word)
				{
					if(builder.length() + word->length() > static_cast<size_t>(line_break_length_))
					{
						text_lines_.push_back(builder);
						builder = *word;
					} else
					{
						if(builder.length() > 0)
							builder = builder + " " + *word;
						else
							builder = *word;
					}
				}
				text_lines_.push_back(builder);
			}
		}
	}

public:
	WidgetText()
	{
		text_=""; 
		alignment_ = TextAlignment::Centre;
		autowrap_ = false; 
		line_break_length_ = 1000;
	}

	std::string GetText(){return text_;}
	std::vector<std::string> GetTextLines(){return text_lines_;}
	void SetText(std::string _text)
	{
		text_ = _text;
		LayoutText();
	}

	TextAlignment::Enum GetAlignment(){return alignment_;}
	void SetAlignment(TextAlignment::Enum _alignment){alignment_ = _alignment;}

	bool GetAutowrap(){return autowrap_;}
	void SetAutowrap(bool _autowrap, int _line_break_length)
	{
		autowrap_ = _autowrap;
		line_break_length_ = _line_break_length;
		LayoutText();
	}
};