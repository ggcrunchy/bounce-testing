#include "Graphics_Imp.h"
#include "Graphics.h"

namespace Graphics
{
	/// @brief Renders a picture
	/// @param picture Handle to the picture object
	/// @param x Screen x coordinate
	/// @param y Screen y coordinate
	/// @param w Screen width
	/// @param h Screen height
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int DrawPicture (Picture * picture, int x, int y, int w, int h)
	{
		if (0 == picture) return 0;

		Main & g = Main::Get();

		g.MapRect(x, y, w, h);
		g.EnableTexture(true);

		int sx = x, ex = x + w - 1;
		int sy = y, ey = y + h - 1;

		SetPicture(picture);

		glBegin(GL_QUADS);
			glTexCoord2f(picture->mS0, picture->mT1);	glVertex2i(sx, sy);
			glTexCoord2f(picture->mS1, picture->mT1);	glVertex2i(ex, sy);
			glTexCoord2f(picture->mS1, picture->mT0);	glVertex2i(ex, ey);
			glTexCoord2f(picture->mS0, picture->mT0);	glVertex2i(sx, ey);
		glEnd();

		return 1;
	}

	/// @brief Acquires a picture's texels
	/// @param picture Handle to the picture object
	/// @param fS0 [out] On success, initial texture s-coordinate
	/// @param fT0 [out] On success, initial texture t-coordinate
	/// @param fS1 [out] On success, terminal texture s-coordinate
	/// @param fT1 [out] On success, terminal texture t-coordinate
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int GetPictureTexels (Picture * picture, float & fS0, float & fT0, float & fS1, float & fT1)
	{
		if (0 == picture) return 0;

		fS0 = picture->mS0;
		fT0 = picture->mT0;
		fS1 = picture->mS1;
		fT1 = picture->mT1;

		return 1;
	}

	/// @brief Assigns a picture's texels
	/// @param picture Handle to the picture object
	/// @param fS0 Initial texture s-coordinate
	/// @param fT0 Initial texture t-coordinate
	/// @param fS1 Terminal texture s-coordinate
	/// @param fT1 Terminal texture t-coordinate
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int SetPictureTexels (Picture * picture, float fS0, float fT0, float fS1, float fT1)
	{
		if (0 == picture) return 0;

		picture->mS0 = fS0;
		picture->mT0 = fT0;
		picture->mS1 = fS1;
		picture->mT1 = fT1;

		return 1;
	}

	/// @brief Unloads a picture object from the renderer
	/// @param picture Handle to the picture object
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int UnloadPicture (Picture * picture)
	{
		delete picture;

		return 1;
	}

	/// @brief Instantiates a picture object
	/// @param name Name of file used to load picture's image data
	/// @param fS0 Initial texture s-coordinate
	/// @param fT0 Initial texture t-coordinate
	/// @param fS1 Terminal texture s-coordinate
	/// @param fT1 Terminal texture t-coordinate
	/// @param picture [out] On success, handle to a picture object
	/// @return 0 on failure; picture on success
	/// @note Tested
	Picture * LoadPicture (std::string const & name, float fS0, float fT0, float fS1, float fT1)
	{
		Main & g = Main::Get();

		// Look for the image associated with the file name. If it is not found, make a new
		// image and load it into the image map. Attempt to load data into it.
		if (g.mImages.find(name) == g.mImages.end())
		{
			try {
				g.mImages[name] = new Image(name);
			} catch (std::bad_alloc &) {
				return 0;
			}
		}

		// Create a new picture bound to the image, assign its texture coordinates, and add it
		// to the core set.
		Picture * picture = new Picture(g.mImages[name]);

		picture->mS0 = fS0;
		picture->mT0 = fT0;
		picture->mS1 = fS1;
		picture->mT1 = fT1;

		g.mPictures.insert(picture);

		return picture;
	}
}