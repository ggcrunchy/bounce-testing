#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL/SDL_types.h>
#include <SDL/SDL_video.h>
#include <string>

namespace Graphics
{
	struct Picture;
	struct TextImage;

	typedef void * Font_h;

	typedef float Color[3];
	typedef float Vector[3];

	int GetTextSize (Font_h font, std::string const & text, int & width, int & height);
	int UnloadFont (Font_h font);

	int DrawPicture (Picture * picture, int x, int y, int w, int h);
	int GetPictureTexels (Picture * picture, float & fS0, float & fT0, float & fS1, float & fT1);
	int SetPictureTexels (Picture * picture, float fS0, float fT0, float fS1, float fT1);
	int UnloadPicture (Picture * picture);

	int DrawTextImage (TextImage * textImage, int x, int y, int w, int h);
	int UnloadTextImage (TextImage * textImage);

	Font_h LoadFont (std::string const & name, int size);
	Picture * LoadPicture (std::string const & name, float fS0, float fT0, float fS1, float fT1);
	TextImage * LoadTextImage (Font_h font, std::string const & text, SDL_Color color);

	void Close (void);
	void DrawBox (int x, int y, int w, int h, bool bFilled);
	void DrawDisk (Vector center, float angle, float rOuter, float rInner, int slices, int loops);
	void DrawGrid (int x, int y, int w, int h, Uint32 xCuts, Uint32 yCuts);
	void DrawLine (int sx, int sy, int ex, int ey);
	void DrawLine3D (Vector P, Vector Q);
	void DrawQuad (Vector UL, Vector UR, Vector LL, Vector LR, Vector normal);
	void DrawSphere (Vector center, float radius, int slices, int stacks);
	void Enter2D (void);
	void Enter3D (void);
	void GetColor (Color color);
	void GetVideoSize (Uint32 & width, Uint32 & height);
	void Prepare (void);
	void Render (void);
	void SetBounds (int x, int y, int w, int h);
	void SetClipPlanes (float fNearZ, float fFarZ);
	void SetColor (Color color);
	void SetEye (Vector eye);
	void SetFOV (float fFOV);
	void SetPicture (Picture * picture);
	void SetTarget (Vector target);
	void Setup (int width, int height, int bpp, bool bFullscreen);
	void SetUpVector (Vector up);
	void SetVideoMode (int width, int height, int bpp, bool bFullscreen);
	void Set3DProjection (bool bPerspective);
}

#endif // GRAPHICS_H