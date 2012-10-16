#include "Graphics_Imp.h"
#include "Graphics.h"

namespace Graphics
{
	/// @brief Renders a text image
	/// @param textImage Handle to a text image object
	/// @param x Screen x coordinate
	/// @param y Screen y coordinate
	/// @param w Screen width
	/// @param h Screen height
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int DrawTextImage (TextImage * textImage, int x, int y, int w, int h)
	{
		if (0 == textImage) return 0;

		Main & g = Main::Get();

		g.MapRect(x, y, w, h);
		g.EnableTexture(true);

		int sx = x, ex = x + w - 1;
		int sy = y, ey = y + h - 1;

		glBindTexture(GL_TEXTURE_2D, textImage->mTexture);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, textImage->mT);			glVertex2i(sx, sy);
			glTexCoord2f(textImage->mS, textImage->mT);	glVertex2i(ex, sy);
			glTexCoord2f(textImage->mS, 0.0f);			glVertex2i(ex, ey);
			glTexCoord2f(0.0f, 0.0f);					glVertex2i(sx, ey);
		glEnd();

		return 1;
	}

	/// @brief Unloads a text image object from the renderer
	/// @param textImage Handle to a text image object
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int UnloadTextImage (TextImage * textImage)
	{
		delete textImage;

		return 1;
	}

	/// @brief Generates a renderable text image from a font
	/// @param font Handle to a font object, used to generate the text image
	/// @param text The text to render in the text image
	/// @param color The color of the rendered text image
	/// @return 0 on failure; text image on success
	/// @note Tested
	TextImage * LoadTextImage (Font_h font, std::string const & text, SDL_Color color)
	{
		if (0 == font) return 0;

		TextImage * textImage = 0;

		try {
			textImage = new TextImage(static_cast<FT_Size>(font), text, color);

			Main::Get().mTextImages.insert(textImage);
		} catch (std::bad_alloc &) {}

		return textImage;
	}
}