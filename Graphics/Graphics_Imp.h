#ifndef GRAPHICS_IMP_H
#define GRAPHICS_IMP_H

#include <SDL/SDL_error.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_types.h>
#include <SDL/SDL_video.h>
extern "C"
{
	#include <ft2build.h>
	#include FT_FREETYPE_H
	#include FT_GLYPH_H
	#include FT_SIZES_H
}
#include <bitset>
#include <map>
#include <set>
#include <string>

namespace Graphics
{
	typedef GLfloat GLfV[3];
	typedef GLdouble GLdV[3];

	/// @brief Internal image representation
	struct Image {
	// Members
		GLuint mTexture;///< Texture used by image
		Uint32 mCount;	///< Reference count for image sprites
	// Methods
		Image (std::string const & name);
		~Image (void);
	};

	/// @brief Internal picture representation
	struct Picture {
	// Members
		Image * mImage;	///< Image used by picture
		GLfloat mS0;///< Initial s-coordinate
		GLfloat mS1;///< Terminal s-coordinate
		GLfloat mT0;///< Initial t-coordinate
		GLfloat mT1;///< Terminal t-coordinate
	// Methods
		Picture (Image * image);
		~Picture (void);
	};

	// @brief Internal face representation
	struct Face {
	// Members
		std::map<int, FT_Size> mSizes;	///< Sizes bound to face
		FT_Face mFace;	///< Face data used by FreeType
	// Methods
		Face (std::string const & name);
		~Face (void);

		FT_Size GetSize (std::string const & name, int size);
	};

	/// @brief Internal text image representation
	struct TextImage {
	// Members
		GLuint mTexture;///< Texture used by text image
		FT_Glyph mGlyph;///< Glyph information pertinent to character
		GLfloat mS;	///< Texture s-extent
		GLfloat mT;	///< Texture t-extent
	// Methods
		TextImage (FT_Size pSize, std::string const & text, SDL_Color color);
		~TextImage (void);
	};

	/// @brief Structure used to represent the graphics renderer
	struct Main {
	// Members
		std::bitset<32> mStatus;///< Current renderer status
		std::map<std::string, Image*> mImages;	///< Image storage
		std::map<std::string, Face*> mFaces;///< Face storage
		std::set<Picture*> mPictures;	///< Picture storage
		std::set<TextImage*> mTextImages;	///< Text image storage
		FT_Library mFreeType;	///< Library used to maintain text
		GLUquadric * mQuadric;	///< Object used to render quadrics
		GLdV mEye;	///< Position of camera eye
		GLdV mTarget;	///< Position of camera target
		GLdV mUp;	///< Up vector
		GLdouble mFOV;	///< Field of view, in degrees
		GLdouble mNearZ;///< Near clipping plane distance
		GLdouble mFarZ;	///< Far clipping plane distance
		GLsizei mResW;	///< Resolution width
		GLsizei mResH;	///< Resolution height
	// Enumerations
		enum {
			ePerspective,	///< If true, use perspective projection
			eTexture	///< If true, texturing is enabled
		};
	// Methods
		Main (void);
		~Main (void);

		static Main & Get (void);

		void Commit (bool b3D);
		void EnableTexture (bool bEnable);
		void MapRect (int & x, int & y, int & w, int & h);
	};
}

#endif // GRAPHICS_IMP_H