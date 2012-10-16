#include "Graphics_Imp.h"
#include "Graphics.h"

namespace Graphics
{
	/// @brief Gets the dimensions of a text string constructed from a given font object
	/// @param font Handle to a font object
	/// @param text Text string whose size is being determined
	/// @param width [out] On success, the string's width
	/// @param height [out] On success, the string's height
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int GetTextSize (Font_h font, std::string const & text, int & width, int & height)
	{
		if (0 == font) return 0;

		FT_Size pSize = static_cast<FT_Size>(font);

		FT_Activate_Size(pSize);

		FT_GlyphSlot pSlot = pSize->face->glyph;

		width = 0;

		for (Uint32 index = 0; text[index] != '\0'; ++index)
		{
			FT_Load_Char(pSize->face, text[index], FT_LOAD_DEFAULT);

			width += pSlot->advance.x;
		}

		width /= 64;
		height = pSize->metrics.height / 64 + 1;

		return 1;
	}

	/// @brief Unloads a font object from the renderer
	/// @param font Handle to a font object
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	int UnloadFont (Font_h font)
	{
		if (0 == font) return 0;

		Main & g = Main::Get();

		// Remove the size from the graphics core.
		FT_Size pSize = static_cast<FT_Size>(font);

		std::map<std::string, Face*>::iterator iter = g.mFaces.begin();

		while (iter != g.mFaces.end() && iter->second->mFace != pSize->face) ++iter;

		if (iter != g.mFaces.end())
		{
			for (std::map<int, FT_Size>::iterator sIter = iter->second->mSizes.begin(); sIter != iter->second->mSizes.end(); ++sIter)
			{
				if (sIter->second != pSize) continue;
				
				iter->second->mSizes.erase(sIter);

				// If the last size is removed, unload the font itself.
				if (iter->second->mSizes.empty()) g.mFaces.erase(iter);

				return 1;
			}
		}

		return 0;
	}

	/// @brief Instantiates a font object
	/// @param name Name of file used to load font data
	/// @param size Size in which to load font
	/// @return 0 on failure; font on success
	/// @note Tested
	Font_h LoadFont (std::string const & name, int size)
	{
		if (size <= 0) return 0;

		Main & g = Main::Get();

		// Load the face if it is not loaded. Acquire the size.
		if (g.mFaces.find(name) == g.mFaces.end())
		{
			g.mFaces[name] = new Face(name);
		}

		return g.mFaces[name]->GetSize(name, size);
	}
}