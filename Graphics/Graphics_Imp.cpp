/// @file
/// Internal graphics system for renderer

#include "Graphics_Imp.h"
#include "Graphics.h"
#include <cassert>
#include <cmath>

/// @brief 26.6 fixed-point grid-fitting routines
#define Round(x)((x) & -64)
#define Floor(x) Round(x + 32)
#define Ceiling(x) Round(x + 63)

namespace Graphics
{
	/// @var c_Rmask
	/// @brief Red mask constant
	static Uint32 const c_Rmask = SDL_BYTEORDER == SDL_BIG_ENDIAN ? 0xFF000000 : 0x000000FF;
	/// @var c_Gmask
	/// @brief Green mask constant
	static Uint32 const c_Gmask = SDL_BYTEORDER == SDL_BIG_ENDIAN ? 0x00FF0000 : 0x0000FF00;
	/// @var c_Bmask
	/// @brief Blue mask constant
	static Uint32 const c_Bmask = SDL_BYTEORDER == SDL_BIG_ENDIAN ? 0x0000FF00 : 0x00FF0000;
	/// @var c_Amask
	/// @brief Alpha mask constant
	static Uint32 const c_Amask = SDL_BYTEORDER == SDL_BIG_ENDIAN ? 0x000000FF : 0xFF000000;

	/// @brief Gets the next power-of-2 value
	/// @param num Number to increase
	/// @return Power-of-2 value greater than or equal to num
	/// @note Tested
	static int PowerOf2 (int num)
	{
		int comp = 1;
		
		while (comp < num) comp += comp;

		return comp;
	}

	/// @brief Loads data into a texture
	/// @param pImage Image data used to build texture
	/// @param bLinear If true, use a linear filter
	/// @return Generated texture
	/// @note Tested
	static GLuint LoadTexture (SDL_Surface * pImage, bool bLinear)
	{
		// OpenGL will be used in RGBA mode; thus, power-of-2, RGBA textures are needed.
		// Convert the image to the nearest fit if necessary, freeing the old surface.
		int w = PowerOf2(pImage->w), h = PowerOf2(pImage->h);

		if (w != pImage->w || h != pImage->h || pImage->format->BytesPerPixel != 4)
		{
			SDL_Surface * pOld = pImage;

			pImage = SDL_CreateRGBSurface(0, w, h, 32, c_Rmask, c_Gmask, c_Bmask, c_Amask);

			SDL_BlitSurface(pOld, 0, pImage, 0);
			SDL_FreeSurface(pOld);
		}

		// Load the image data into the texture. Assign some properties and return it.
		GLuint texture;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bLinear ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bLinear ? GL_LINEAR : GL_NEAREST);

		SDL_FreeSurface(pImage);

		return texture;
	}

	/// @var G_Main
	/// @brief Graphics manager singleton
	Main G_Main;

	/// @brief Constructs an Image object
	/// @param name Name of file used to load image
	/// @note Tested
	Image::Image (std::string const & name) : mCount(0)
	{
		// Attempt to load the image data, given the filename.
		SDL_Surface * pImage = IMG_Load(name.c_str());

		if (0 == pImage) throw std::bad_alloc();

		mTexture = LoadTexture(pImage, true);
	}

	/// @brief Destructs an Image object
	/// @note Tested
	Image::~Image (void)
	{
		assert(0 == mCount);

		glDeleteTextures(1, &mTexture);

		// Remove the image from the graphics core.
		for (std::map<std::string, Image*>::iterator iter = G_Main.mImages.begin(); iter != G_Main.mImages.end(); ++iter)
		{
			if (iter->second != this) continue;

			G_Main.mImages.erase(iter);

			return;
		}
	}

	/// @brief Constructs a Picture object
	/// @param image Image to which picture belongs
	/// @note Tested
	Picture::Picture (Image * image) : mImage(image)
	{
		assert(image != 0);

		++mImage->mCount;
	}

	/// @brief Destructs a Picture object
	/// @note Tested
	Picture::~Picture (void)
	{
		assert(mImage != 0);
		assert(mImage->mCount > 0);

		// Destroy the picture's image if it becomes unreferenced.
		if (0 == --mImage->mCount) delete mImage;

		// Remove the picture from the core.
		G_Main.mPictures.erase(this);
	}

	/// @brief Constructs a Face object
	/// @param name Name of file used to load face
	/// @note Tested
	Face::Face (std::string const & name)
	{
		FT_New_Face(G_Main.mFreeType, name.c_str(), 0, &mFace);
	}

	/// @brief Destructs a Face object
	/// @note Tested
	Face::~Face (void)
	{
		FT_Done_Face(mFace);
	}

	/// @brief Acquires a size from the face
	/// @param name Name of file used to load face
	/// @param size Size to obtain
	/// @note Tested
	FT_Size Face::GetSize (std::string const & name, int size)
	{
		if (mSizes.find(size) == mSizes.end())
		{
			FT_Size Size;	FT_New_Size(mFace, &Size);

			FT_Activate_Size(Size);
			FT_Set_Pixel_Sizes(mFace, 0, size);

			mSizes[size] = Size;
		}

		return mSizes[size];
	}

	/// @brief Constructs a TextImage object
	/// @param pSize
	/// @param text
	/// @param color
	/// @note Tested
	TextImage::TextImage (FT_Size pSize, std::string const & text, SDL_Color color)
	{
		FT_Activate_Size(pSize);

		// OpenGL will be used in RGBA mode; thus, power-of-2, RGBA textures are needed. In
		// addition, properly alpha-blended text images are needed. Convert the image to the
		// nearest fit and free the old surface.
		int textW, textH;	GetTextSize(pSize, text.c_str(), textW, textH);

		SDL_Surface * pImage = SDL_CreateRGBSurface(0, PowerOf2(textW), PowerOf2(textH), 32, c_Rmask, c_Gmask, c_Bmask, c_Amask);

		SDL_SetAlpha(pImage, 0, 0);

		//
		FT_GlyphSlot pSlot = pSize->face->glyph;

		SDL_LockSurface(pImage);

		Uint8 * pCorner = static_cast<Uint8*>(pImage->pixels);

		for (Uint32 index = 0; index < text.size(); ++index)
		{
			FT_Load_Char(pSize->face, text[index], FT_LOAD_DEFAULT);

			FT_Glyph Char;	FT_Get_Glyph(pSlot, &Char);

			FT_Vector vec = { -pSlot->metrics.horiBearingX, -pSlot->metrics.horiBearingY };

			FT_Glyph_Transform(Char, 0, &vec);
			FT_Glyph_To_Bitmap(&Char, FT_RENDER_MODE_NORMAL, 0, 1);

			//
			FT_BitmapGlyph bit = (FT_BitmapGlyph)Char;

			Uint32 diff = Ceiling(pSize->metrics.ascender + vec.y);

			Uint8 * pLine = pCorner + ((diff - 1) * pImage->pitch - vec.x * 4) / 64;
			Uint8 * pData = bit->bitmap.buffer;

			for (int row = 0; row < bit->bitmap.rows; ++row)
			{
				for (int col = 0; col < bit->bitmap.width; ++col)
				{
					memset(&pLine[col * 4], pData[col], 4);
				}

				pLine += pImage->pitch;
				pData += bit->bitmap.pitch;
			}

			pCorner += FT_MulDiv(bit->root.advance.x, 4, 64);
		}
		
		SDL_UnlockSurface(pImage);

		// Load the texture and set its extents.
		mTexture = LoadTexture(pImage, true);

		mS = GLfloat(textW) / PowerOf2(textW);
		mT = GLfloat(textH) / PowerOf2(textH);
	}

	/// @brief Destructs a TextImage object
	/// @note Tested
	TextImage::~TextImage (void)
	{
		glDeleteTextures(1, &mTexture);

		// Remove the text image from the core.
		G_Main.mTextImages.erase(this);
	}

	/// @brief Constructs the graphics manager
	/// @note Tested
	Main::Main (void) : mResW(0), mResH(0)
	{
		mQuadric = gluNewQuadric();

		gluQuadricNormals(mQuadric, GLU_FLAT);
		gluQuadricTexture(mQuadric, GL_TRUE);
	}

	/// @brief Destructs the graphics manager
	/// @note Tested
	Main::~Main (void)
	{
		gluDeleteQuadric(mQuadric);
	}

	/// @brief Accesses the graphics manager singleton
	/// @return Reference to the graphics manager singleton
	/// @note Tested
	Main & Main::Get (void)
	{
		return G_Main;
	}

	/// @brief Commits a mode switch
	/// @param bool b3D If true, switch is to 3D
	/// @note Tested
	void Main::Commit (bool b3D)
	{
		/* Modelview */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (b3D)
		{
			gluLookAt(mEye[0], mEye[1], mEye[2], mTarget[0], mTarget[1], mTarget[2], mUp[0], mUp[1], mUp[2]);
		}

		/* Projection */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		if (b3D)
		{
			if (mStatus[ePerspective]) gluPerspective(mFOV, double(mResW) / double(mResH), mNearZ, mFarZ);

			else glOrtho(-1.0f, +1.0f, -1.0f, +1.0f, mNearZ, mFarZ);
		}

		else gluOrtho2D(0.0, double(mResW), 0.0, double(mResH));
	}

	/// @brief Enables or disables texturing
	/// @param bEnable If true, enable texturing
	/// @note Tested
	void Main::EnableTexture (bool bEnable)
	{
		if (mStatus[eTexture] == bEnable) return;

		(mStatus[eTexture] ? glDisable : glEnable)(GL_TEXTURE_2D);

		mStatus.flip(eTexture);
	}

	/// @brief Maps a rectangle into the current resolution
	/// @param x [in-out] Input x-coordinate; mapped to output x-coordinate
	/// @param y [in-out] Input y-coordinate; mapped to output y-coordinate
	/// @param w [in-out] Input width; mapped to output width
	/// @param h [in-out] Input height; mapped to output height
	/// @note Tested
	void Main::MapRect (int & x, int & y, int & w, int & h)
	{
		y = mResH - (y + h + 1);
	}
}