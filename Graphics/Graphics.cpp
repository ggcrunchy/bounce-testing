#include "Graphics_Imp.h"
#include "Graphics.h"
#include <cmath>
#include <iostream>

namespace Graphics
{
	/// @brief Closes the renderer
	/// @note Tested
	void Close (void)
	{
		Main & g = Main::Get();

		// Unload all pictures; doing so unloads images, as well.
		while (!g.mPictures.empty())
		{
			UnloadPicture(*g.mPictures.begin());
		}

		// Unload all text images and fonts.
		while (!g.mTextImages.empty())
		{
			UnloadTextImage(*g.mTextImages.begin());
		}

		while (!g.mFaces.empty())
		{
			Face * pFace = g.mFaces.begin()->second;

			while (!pFace->mSizes.empty())
			{
				UnloadFont(pFace->mSizes.begin()->second);
			}
		}

		// Close TrueType font support.
		FT_Done_FreeType(g.mFreeType);

		// Close the video subsystem.
		if (SDL_WasInit(SDL_INIT_VIDEO)) SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}

	/// @brief Renders a square, unfilled box in the given color
	/// @param x Screen x coordinate
	/// @param y Screen y coordinate
	/// @param w Screen width
	/// @param h Screen height
	/// @param bFilled If true, draw a filled box
	/// @note Tested
	void DrawBox (int x, int y, int w, int h, bool bFilled)
	{
		Main & g = Main::Get();

		g.MapRect(x, y, w, h);
		g.EnableTexture(false);

		int sx = x, ex = x + w - 1;
		int sy = y, ey = y + h - 1;

		glBegin(bFilled ? GL_QUADS : GL_LINE_LOOP);
			glVertex2i(sx, sy);
			glVertex2i(ex, sy);
			glVertex2i(ex, ey);
			glVertex2i(sx, ey);
		glEnd();
	}

	/// @brief Renders a disk
	/// @param center Disk center
	/// @param angle Orientation about y-axis
	/// @param rOuter Outer radius
	/// @param rInner Inner radius
	/// @param slices Slices to break disk into
	/// @param loops Loops to break disk into
	/// @note Tested
	void DrawDisk (Vector center, float angle, float rOuter, float rInner, int slices, int loops)
	{
		Main::Get().EnableTexture(true);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(center[0], center[1], center[2]);
		glRotatef(angle, 0.0f, 1.0f, 0.0f);
		gluDisk(Main::Get().mQuadric, rInner, rOuter, slices, loops);
		glPopMatrix();
	}

	struct Acc {
		int mAcc;
		int mBase;
		int mCount;
		int mDim;

		int Value (void)
		{
			return mBase + mAcc / mCount;
		}

		bool Valid (void)
		{
			return mAcc != mCount * mDim;
		}

		void Update (void)
		{
			mAcc += mDim;
		}

		Acc (int base, int dim, int count) : mAcc(0), mBase(base), mDim(dim), mCount(count + 1)
		{
		}
	};

	/// @brief Renders a square, unfilled grid in the current color
	/// @param x Screen x coordinate
	/// @param y Screen y coordinate
	/// @param w Screen width
	/// @param h Screen height
	/// @param xCuts Number of horizontal partitions
	/// @param yCuts Number of vertical partitions
	/// @note Tested
	void DrawGrid (int x, int y, int w, int h, Uint32 xCuts, Uint32 yCuts)
	{
		Main & g = Main::Get();

		g.MapRect(x, y, w, h);
		g.EnableTexture(false);

		int sx = x, ex = x + w - 1;
		int sy = y, ey = y + h - 1;

		glBegin(GL_LINES);
			// Draw the right side.
			glVertex2i(ex, sy);
			glVertex2i(ex, ey);

			// Draw the top side.
			glVertex2i(ex, ey);
			glVertex2i(sx, ey);

			// Draw the vertical lines.
			for (Acc X(x, w, int(xCuts)); X.Valid(); X.Update())
			{
				glVertex2i(X.Value(), sy);
				glVertex2i(X.Value(), ey);
			}

			// Draw the horizontal lines.
			for (Acc Y(y, h, int(yCuts)); Y.Valid(); Y.Update())
			{
				glVertex2i(sx, Y.Value());
				glVertex2i(ex, Y.Value());
			}
		glEnd();
	}

	/// @brief Renders a line
	/// @param sx Screen start x coordinate
	/// @param sy Screen start y coordinate
	/// @param fEX Screen end x coordinate
	/// @param fEY Screen end y coordinate
	/// @note Tested
	void DrawLine (int sx, int sy, int ex, int ey)
	{
		Main & g = Main::Get();

		g.EnableTexture(false);

		glBegin(GL_LINES);
			glVertex2i(sx, g.mResH - sy - 1);
			glVertex2i(ex, g.mResH - ey - 1);
		glEnd();
	}

	/// @brief Renders a line between two points
	/// @param P Initial point of line
	/// @param Q Terminal point of line
	/// @note Tested
	void DrawLine3D (Vector P, Vector Q)
	{
		Main::Get().EnableTexture(false);

		glBegin(GL_LINES);
			glVertex3fv(P);
			glVertex3fv(Q);
		glEnd();
	}

	/// @brief Renders a quad
	/// @param UL Upper-left vertex
	/// @param UR Upper-right vertex
	/// @param LL Lower-left vertex
	/// @param LR Lower-right vertex
	/// @param normal Quad normal
	/// @note Tested
	void DrawQuad (Vector UL, Vector UR, Vector LL, Vector LR, Vector normal)
	{
		Main::Get().EnableTexture(true);

		glBegin(GL_QUADS);
			glNormal3fv(normal);

			float * points[] = { UL, UR, LL, LR };			

			for (Uint32 index = 0; index < 4; ++index)
			{
				glVertex3fv(points[index]);

				glTexCoord2f((index + 1) & 2 ? 0.0f : 1.0f, index < 2 ? 1.0f : 0.0f);
			}
		glEnd();
	}

	/// @brief Renders a sphere
	/// @param center Sphere center
	/// @param radius Sphere radius
	/// @param slices Slices to break sphere into
	/// @param stacks Stacks to break sphere into
	/// @note Tested
	void DrawSphere (Vector center, float radius, int slices, int stacks)
	{
		Main::Get().EnableTexture(true);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(center[0], center[1], center[2]);
		gluSphere(Main::Get().mQuadric, radius, slices, stacks);
		glPopMatrix();
	}

	/// @brief Sets up the 2D rendering mode
	/// @note Tested
	void Enter2D (void)
	{
		// Disable 3D features.
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

		// Enable 2D features.
		glEnable(GL_BLEND);
		glEnable(GL_SCISSOR_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Commit changes.
		Main::Get().Commit(false);
	}

	/// @brief Sets up the 3D rendering mode
	/// @note Tested
	void Enter3D (void)
	{
		// Disable 2D features.
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);

		// Enable 3D features.
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glMaterialf(GL_FRONT, GL_AMBIENT, 1.0f);
		glMaterialf(GL_FRONT, GL_DIFFUSE, 1.0f);
		glMaterialf(GL_BACK, GL_AMBIENT, 0.0f);
		glMaterialf(GL_BACK, GL_DIFFUSE, 0.5f);

		GLfloat a[] = { 0.5f, 0.5f, 0.5f, 1.0f };	glLightfv(GL_LIGHT0, GL_AMBIENT, a);
		GLfloat d[] = { 1.0f, 1.0f, 1.0f, 1.0f };	glLightfv(GL_LIGHT0, GL_DIFFUSE, d);
		GLfloat p[] = { 0.0f, 0.0f, 2.0f, 1.0f };	glLightfv(GL_LIGHT0, GL_POSITION, p);

		// Commit changes.
		Main::Get().Commit(true);
	}

	/// @brief Gets the current color
	/// @param color [out] On success, the color
	/// @note Tested
	void GetColor (Color color)
	{
		GLfloat colors[4];	glGetFloatv(GL_CURRENT_COLOR, colors);

		memcpy(color, colors, sizeof(Color));
	}

	/// @brief Gets the video dimensions
	/// @param width [out] On success, the video width
	/// @param height [out] On success, the video height
	/// @note Tested
	void GetVideoSize (Uint32 & width, Uint32 & height)
	{
		Main & g = Main::Get();

		width = g.mResW;
		height = g.mResH;
	}

	/// @brief Prepares a render frame
	/// @return 0 on failure, non-0 for success
	/// @note Tested
	void Prepare (void)
	{
		Main & g = Main::Get();

		glScissor(0, 0, g.mResW, g.mResH);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	/// @brief Performs rendering
	/// @note Tested
	void Render (void)
	{
		SDL_GL_SwapBuffers();
	}

	/// @brief Sets the render bounds
	/// @param x Screen x coordinate
	/// @param y Screen y coordinate
	/// @param w Screen width
	/// @param h Screen height
	/// @note Tested
	void SetBounds (int x, int y, int w, int h)
	{
		Main & g = Main::Get();

		g.MapRect(x, y, w, h);
		glScissor(x, y, w, h);
	}

	/// @brief Sets the clipping planes
	/// @param fNearZ Near clipping plane z
	/// @param fFarZ Far clipping plane z
	/// @note Tested
	void SetClipPlanes (float fNearZ, float fFarZ)
	{
		Main::Get().mNearZ = fNearZ;
		Main::Get().mFarZ = fFarZ;
	}

	/// @brief Sets the current draw color
	/// @param color Color to assign
	/// @note Tested
	void SetColor (Color color)
	{
		glColor3fv(color);
	}

	/// @brief Sets the eye position
	/// @param eye Eye vector
	/// @note Tested
	void SetEye (Vector eye)
	{
		Main::Get().mEye[0] = eye[0];
		Main::Get().mEye[1] = eye[1];
		Main::Get().mEye[2] = eye[2];
	}

	/// @brief Sets the field of view
	/// @param fFOV Field of view, in degrees
	/// @note Tested
	void SetFOV (float fFOV)
	{
		Main::Get().mFOV = fFOV;
	}

	/// @brief Sets the current picture
	/// @param picture Picture to assign
	/// @note Tested
	void SetPicture (Picture * picture)
	{
		if (0 == picture) return;
		if (0 == picture->mImage) return;
		
		glBindTexture(GL_TEXTURE_2D, picture->mImage->mTexture);
	}

	/// @brief Sets the view target
	/// @param target Target vector
	/// @note Tested
	void SetTarget (Vector target)
	{
		Main::Get().mTarget[0] = target[0];
		Main::Get().mTarget[1] = target[1];
		Main::Get().mTarget[2] = target[2];
	}

	/// @brief Sets up the renderer
	/// @param width Screen width of mode
	/// @param height Screen height of mode
	/// @param bpp Bits per pixel of mode
	/// @param bFullscreen If true, this is a full-screen video mode
	/// @note Tested
	void Setup (int width, int height, int bpp, bool bFullscreen)
	{
		// Set up the video subsystem, enabling rendering.
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "Unable to initialize SDL video: " << SDL_GetError() << std::endl;

			return;
		}

		// Set up TrueType font support.
		if (FT_Init_FreeType(&Main::Get().mFreeType) != 0)
		{
			std::cerr << "Unable to initialize FreeType2." << std::endl;

			return;
		}

		// Set some OpenGL configuration properties. Set the requested video mode.
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SetVideoMode(width, height, bpp, bFullscreen);

		// Assign default field values.
		Color color = { 1.0f, 1.0f, 1.0f };
		Vector vector = { 0.0f, 0.0f, 0.0f };

		SetColor(color);
		SetClipPlanes(1.0f, 1000.0f);
		SetEye(vector);
		SetFOV(45.0f);
		SetTarget(vector);

		vector[1] = 1.0f;

		SetUpVector(vector);
		Set3DProjection(true);

		// Begin in 2D mode.
		Enter2D();
	}

	/// @brief Sets the up vector
	/// @param up Up vector
	/// @note Tested
	void SetUpVector (Vector up)
	{
		Main::Get().mUp[0] = up[0];
		Main::Get().mUp[1] = up[1];
		Main::Get().mUp[2] = up[2];
	}

	/// @brief Sets the renderer's video mode
	/// @param width Screen width of mode
	/// @param height Screen height of mode
	/// @param bpp Bits per pixel of mode
	/// @param bFullscreen If true, this is a full-screen video mode
	/// @note Tested
	void SetVideoMode (int width, int height, int bpp, bool bFullscreen)
	{
		Main & g = Main::Get();

		// Attempt to set an OpenGL video mode, either windowed or full-screen.
		Uint32 flags = SDL_OPENGL | (bFullscreen ? SDL_FULLSCREEN : 0);

		if (SDL_SetVideoMode(width, height, bpp, flags) == 0)
		{
			std::cerr << "Unable to set video mode: " << SDL_GetError() << std::endl;

			return;
		}

		// Record the resolution.
		g.mResW = width;
		g.mResH = height;
	}

	/// @brief Sets the 3D projection mode
	/// @param bPerspective If true, use perspective projection
	/// @note Tested
	void Set3DProjection (bool bPerspective)
	{
		Main::Get().mStatus[Main::ePerspective] = bPerspective;
	}
}