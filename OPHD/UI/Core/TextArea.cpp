#include "TextArea.h"

#include "../../Common.h"
#include "../../Constants.h"
#include "../../Cache.h"

#include <NAS2D/Utility.h>
#include <NAS2D/Renderer/Renderer.h>


using namespace NAS2D;


void TextArea::font(const std::string& filePath, unsigned int pointSize)
{
	mFont = &fontCache.load(filePath, pointSize);
}


void TextArea::processString()
{
	mFormattedList.clear();

	if (mRect.width < 10 || !mFont || text().empty()) { return; }

	StringList tokenList = split_string(text().c_str(), ' ');

	std::size_t i = 0;
	int w = 0;
	while (i < tokenList.size())
	{
		std::string line;
		while (w < mRect.width && i < tokenList.size())
		{
			int tokenWidth = mFont->width(tokenList[i] + " ");
			w += tokenWidth;
			if (w >= mRect.width)
			{
				/**
				 * \todo	In some edge cases where the width of the TextArea is too
				 *			narrow for a single word/token, this will result in an infinite
				 *			loop. This edge case will need to be resolved either by splitting
				 *			the token that's too wide or by simply rendering it as is.
				 */
				//++i;
				break;
			}

			if (tokenList[i] == "\n")
			{
				++i;
				break;
			}
			line += (tokenList[i] + " ");
			++i;
		}
		w = 0;
		mFormattedList.push_back(line);
	}

	mNumLines = static_cast<std::size_t>(mRect.height / mFont->height());
}


void TextArea::onSizeChanged()
{
	Control::onSizeChanged();
	processString();
}


void TextArea::onTextChanged()
{
	processString();
}


void TextArea::onFontChanged()
{
	processString();
}


void TextArea::update()
{
	draw();
}


void TextArea::draw()
{
	auto& renderer = Utility<Renderer>::get();

	if (highlight()) { renderer.drawBox(mRect, NAS2D::Color::White); }

	if (!mFont) { return; }

	auto textPosition = mRect.startPoint();
	for (std::size_t i = 0; i < mFormattedList.size() && i < mNumLines; ++i)
	{
		renderer.drawText(*mFont, mFormattedList[i], textPosition, mTextColor);
		textPosition.y += mFont->height();
	}
}
