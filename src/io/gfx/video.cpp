
/**
 *
 * @file video.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed util.c to util.cpp
 * - 13th July 2009: Created graphics.cpp from parts of util.cpp
 * - 26th July 2009: Renamed graphics.cpp to video.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Contains graphics utility functions.
 *
 */


#include "paletteeffects.h"
#include "video.h"

#ifdef SCALE
	#include <scalebit.h>
#endif

#include "util.h"

#include <string.h>

#ifdef __vita__
#include <vitaGL.h>
#include <imgui_vita.h>

bool autohide = false;
bool visible = false;
bool fullscreen = false;
bool bilinear = true;
bool vflux_window = false;
bool credits_window = false;
bool vflux_enabled = false;
float vcolors[3];
uint16_t *vindices;
float *colors;
float *vertices;
uint64_t tick;
SDL_Shader shader = SDL_SHADER_NONE;

void LoadSettings() {
	FILE *fp = fopen("ux0:data/jazz/imgui.cfg", "rb");
	if (fp) {
		fseek(fp, 0, SEEK_SET);
		fread(&autohide, sizeof(bool), 1, fp);
		fread(&fullscreen, sizeof(bool), 1, fp);
		fread(&bilinear, sizeof(bool), 1, fp);
		fread(&vflux_enabled, sizeof(bool), 1, fp);
		fread(&vcolors, 3 * sizeof(float), 1, fp);
		fread(&shader, sizeof(SDL_Shader), 1, fp);
		fclose(fp);
	}
}

void SaveSettings() {
	FILE *fp = fopen("ux0:data/jazz/imgui.cfg", "wb");
	if (fp) {
		fseek(fp, 0, SEEK_SET);
		fwrite(&autohide, sizeof(bool), 1, fp);
		fwrite(&fullscreen, sizeof(bool), 1, fp);
		fwrite(&bilinear, sizeof(bool), 1, fp);
		fwrite(&vflux_enabled, sizeof(bool), 1, fp);
		fwrite(&vcolors, 3 * sizeof(float), 1, fp);
		fwrite(&shader, sizeof(SDL_Shader), 1, fp);
		fclose(fp);
	}
}

void SetSettings() {
	if (fullscreen) {
		SDL_SetVideoModeScaling(0, 0, 960, 544);
	} else {
		int sh = 544;
		int sw = (float)320 * ((float)sh / (float)240);
		int x = (960 - sw) / 2;
		SDL_SetVideoModeScaling(x, 0, sw, sh);
	}

	SDL_SetVideoModeBilinear(bilinear);
	SDL_SetVideoShader(shader);

	if (vflux_enabled) {
		memcpy(&colors[0], vcolors, sizeof(float) * 3);
		memcpy(&colors[4], vcolors, sizeof(float) * 3);
		memcpy(&colors[8], vcolors, sizeof(float) * 3);
		memcpy(&colors[12], vcolors, sizeof(float) * 3);

		float c;
		SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
		if (time.hour < 6)			// Night/Early Morning
			c = 0.25f;
		else if (time.hour < 10)	// Morning/Early Day
			c = 0.1f;
		else if (time.hour < 15)	// Mid day
			c = 0.05f;
		else if (time.hour < 19)	// Late day
			c = 0.15f;
		else						// Evening/Night
			c = 0.2f;
		colors[3] = colors[7] = colors[11] = colors[15] = c;
	}
}

void ImGui_callback() {
	ImGui_ImplVitaGL_NewFrame();

	if ((!autohide || visible) && ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save config", nullptr, nullptr)) {
				SaveSettings();
			}
			if (ImGui::MenuItem("Reload config", nullptr, nullptr)) {
				LoadSettings();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Graphics")) {
			if (ImGui::MenuItem("Fullscreen", nullptr, fullscreen)) {
				fullscreen = !fullscreen;
			}
			if (ImGui::MenuItem("Bilinear Filter", nullptr, bilinear)) {
				bilinear = !bilinear;
			}
			if (ImGui::MenuItem("vFlux Config", nullptr, vflux_window)) {
				vflux_window = !vflux_window;
			}
			if (ImGui::BeginMenu("Shaders")) {
				if (ImGui::MenuItem("None", nullptr, shader == SDL_SHADER_NONE)) {
					shader = SDL_SHADER_NONE;
				}
				if (ImGui::MenuItem("Sharp Bilinear", nullptr, shader == SDL_SHADER_SHARP_BILINEAR_SIMPLE)) {
					shader = SDL_SHADER_SHARP_BILINEAR_SIMPLE;
				}
				if (ImGui::MenuItem("Sharp Bilinear (Scanlines)", nullptr, shader == SDL_SHADER_SHARP_BILINEAR)) {
					shader = SDL_SHADER_SHARP_BILINEAR;
				}
				if (ImGui::MenuItem("LCD 3x", nullptr, shader == SDL_SHADER_LCD3X)) {
					shader = SDL_SHADER_LCD3X;
				}
				if (ImGui::MenuItem("xBR x2", nullptr, shader == SDL_SHADER_XBR_2X_FAST)) {
					shader = SDL_SHADER_XBR_2X_FAST;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options")) {
			if (ImGui::MenuItem("Auto-hide menu bar", nullptr, autohide)) {
				autohide = !autohide;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Info")) {
			if (ImGui::MenuItem("Credits", nullptr, credits_window)) {
				credits_window = !credits_window;
			}
			ImGui::EndMenu();
		}

		if (vflux_window) {
			ImGui::Begin("vFlux Configuration", &vflux_window);
			ImGui::ColorPicker3("Filter Color", vcolors);
			ImGui::Checkbox("Enable vFlux", &vflux_enabled);
			ImGui::End();
		}

		if (credits_window) {
			ImGui::Begin("Credits", &credits_window);
			ImGui::TextColored(ImVec4(255, 255, 0, 255), "OpenJazz");
			ImGui::Text("Game Creator: AlisterT");
			ImGui::Text("Port Author: usineur");
			ImGui::Separator();
			ImGui::TextColored(ImVec4(255, 255, 0, 255), "Special thanks to:");
			ImGui::Text("Rinnegatamante: SDL 1.2 and imgui Vita ports");
			ImGui::End();
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(870);

		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::EndMainMenuBar();
	}

	SetSettings();

	if (vflux_enabled) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		vglIndexPointerMapped(vindices);
		vglVertexPointerMapped(vertices);
		vglColorPointerMapped(GL_FLOAT, colors);
		vglDrawObjects(GL_TRIANGLE_FAN, 4, true);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());

	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
	uint64_t delta_touch = sceKernelGetProcessTimeWide() - tick;
	if (touch.reportNum > 0) {
		visible = true;
		ImGui::GetIO().MouseDrawCursor = true;
		tick = sceKernelGetProcessTimeWide();
	} else if (delta_touch > 3000000) {
		visible = false;
		ImGui::GetIO().MouseDrawCursor = false;
	}
}
#endif


/**
 * Creates a surface.
 *
 * @param pixels Pixel data to copy into the surface. Can be NULL.
 * @param width Width of the pixel data and of the surface to be created
 * @param height Height of the pixel data and of the surface to be created
 *
 * @return The completed surface
 */
SDL_Surface* createSurface (unsigned char * pixels, int width, int height) {

	SDL_Surface *ret;
	int y;

	// Create the surface
	ret = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);

	// Set the surface's palette
	video.restoreSurfacePalette(ret);

	if (pixels) {

		// Upload pixel data to the surface
		if (SDL_MUSTLOCK(ret)) SDL_LockSurface(ret);

		for (y = 0; y < height; y++)
			memcpy(((unsigned char *)(ret->pixels)) + (ret->pitch * y),
				pixels + (width * y), width);

		if (SDL_MUSTLOCK(ret)) SDL_UnlockSurface(ret);

	}

	return ret;

}


/**
 * Create the video output object.
 */
Video::Video () {

	int count;

	screen = NULL;

#ifdef SCALE
	scaleFactor = 1;
#endif

	// Generate the logical palette
	for (count = 0; count < 256; count++)
		logicalPalette[count].r = logicalPalette[count].g =
 			logicalPalette[count].b = count;

	currentPalette = logicalPalette;

	return;

}


/**
 * Find the maximum horizontal and vertical resolutions.
 */
void Video::findMaxResolution () {

#ifdef NO_RESIZE
	maxW = DEFAULT_SCREEN_WIDTH;
	maxH = DEFAULT_SCREEN_HEIGHT;
#else
	SDL_Rect **resolutions;
	int count;

	resolutions = SDL_ListModes(NULL, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);

	if (resolutions == (SDL_Rect **)(-1)) {

		maxW = MAX_SCREEN_WIDTH;
		maxH = MAX_SCREEN_HEIGHT;

	} else {

		maxW = SW;
		maxH = SH;

		for (count = 0; resolutions[count] != NULL; count++) {

			if (resolutions[count]->w > maxW) maxW = resolutions[count]->w;
			if (resolutions[count]->h > maxH) maxH = resolutions[count]->h;

		}

		if (maxW > MAX_SCREEN_WIDTH) maxW = MAX_SCREEN_WIDTH;
		if (maxH > MAX_SCREEN_HEIGHT) maxH = MAX_SCREEN_HEIGHT;
	}
#endif

	return;
}


/**
 * Initialise video output.
 *
 * @param width Width of the window or screen
 * @param height Height of the window or screen
 * @param startFullscreen Whether or not to start in full-screen mode
 *
 * @return Success
 */
bool Video::init (int width, int height, bool startFullscreen) {

	fullscreen = startFullscreen;

	if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

	if (!reset(width, height)) {

		logError("Could not set video mode", SDL_GetError());

		return false;

	}

	setTitle(NULL);

	findMaxResolution();

	return true;

}


/**
 * Sets the size of the video window or the resolution of the screen.
 *
 * @param width New width of the window or screen
 * @param height New height of the window or screen
 *
 * @return Success
 */
bool Video::reset (int width, int height) {

	screenW = width;
	screenH = height;

#ifdef SCALE
	if (canvas != screen) SDL_FreeSurface(canvas);
#endif

#ifdef __vita__
	screen = SDL_SetVideoMode(screenW, screenH, 8, FULLSCREEN_FLAGS);

	int sh = VITA_SCREEN_HEIGHT;
	int sw = (float)screen->w * ((float)sh / (float)screen->h);
	int x = (VITA_SCREEN_WIDTH - sw) / 2;
	SDL_SetVideoModeScaling(x, 0, sw, sh);

	vindices = (uint16_t*)malloc(sizeof(uint16_t) * 4);
	colors = (float*)malloc(sizeof(float) * 4 * 4);
	vertices = (float*)malloc(sizeof(float) * 3 * 4);

	LoadSettings();
	SetSettings();

	vertices[0]  =   0.0f;
	vertices[1]  =   0.0f;
	vertices[2]  =   0.0f;
	vertices[3]  = 960.0f;
	vertices[4]  =   0.0f;
	vertices[5]  =   0.0f;
	vertices[6]  = 960.0f;
	vertices[7]  = 544.0f;
	vertices[8]  =   0.0f;
	vertices[9]  =   0.0f;
	vertices[10] = 544.0f;
	vertices[11] =   0.0f;
	vindices[0]  = 0;
	vindices[1]  = 1;
	vindices[2]  = 2;
	vindices[3]  = 3;

	ImGui::CreateContext();
	ImGui_ImplVitaGL_Init();
	ImGui_ImplVitaGL_TouchUsage(true);
	ImGui_ImplVitaGL_MouseStickUsage(false);
	ImGui_ImplVitaGL_UseIndirectFrontTouch(true);
	ImGui::StyleColorsDark();
	ImGui::GetIO().MouseDrawCursor = false;
	ImGui::GetIO().IniFilename = "ux0:data/jazz/imgui.ini";

	SDL_SetVideoCallback(reinterpret_cast<void(*)()>(ImGui_callback));
#elif defined(NO_RESIZE)
	screen = SDL_SetVideoMode(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 8, FULLSCREEN_FLAGS);
#else
	screen = SDL_SetVideoMode(screenW, screenH, 8, fullscreen? FULLSCREEN_FLAGS: WINDOWED_FLAGS);
#endif

	if (!screen) return false;


#ifdef SCALE
	// Check that the scale will fit in the current resolution
	while ( ((screenW/SW < scaleFactor) || (screenH/SH < scaleFactor)) && (scaleFactor > 1) ) {

		scaleFactor--;

	}

	if (scaleFactor > 1) {

		canvasW = screenW / scaleFactor;
		canvasH = screenH / scaleFactor;
		canvas = createSurface(NULL, canvasW, canvasH);

	} else
#endif
    {

		canvasW = screenW;
		canvasH = screenH;
		canvas = screen;

	}

#if !defined(WIZ) && !defined(GP2X)
	expose();
#endif


	/* A real 8-bit display is quite likely if the user has the right video
	card, the right video drivers, the right version of DirectX/whatever, and
	the right version of SDL. In other words, it's not likely enough. If a real
	palette is assumed when
	a) there really is a real palette, there will be an extremely small speed
		gain.
	b) the palette is emulated, there will be a HUGE speed loss.
	Therefore, assume the palette is emulated. */
	/// @todo Find a better way to determine if palette is emulated
	fakePalette = true;

	return true;

}


/**
 * Sets the display palette.
 *
 * @param palette The new palette
 */
void Video::setPalette (SDL_Color *palette) {

	// Make palette changes invisible until the next draw. Hopefully.
	clearScreen(SDL_MapRGB(screen->format, 0, 0, 0));
	flip(0);

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
	currentPalette = palette;

	return;

}


/**
 * Returns the current display palette.
 *
 * @return The current display palette
 */
SDL_Color* Video::getPalette () {

	return currentPalette;

}


/**
 * Sets some colours of the display palette.
 *
 * @param palette The palette containing the new colours
 * @param first The index of the first colour in both the display palette and the specified palette
 * @param amount The number of colours
 */
void Video::changePalette (SDL_Color *palette, unsigned char first, unsigned int amount) {

	SDL_SetPalette(screen, SDL_PHYSPAL, palette, first, amount);

	return;

}


/**
 * Restores a surface's palette.
 *
 * @param surface Surface with a modified palette
 */
void Video::restoreSurfacePalette (SDL_Surface* surface) {

	SDL_SetPalette(surface, SDL_LOGPAL, logicalPalette, 0, 256);

	return;

}


/**
 * Returns the maximum possible screen width.
 *
 * @return The maximum width
 */
int Video::getMaxWidth () {

	return maxW;

}


/**
 * Returns the maximum possible screen height.
 *
 * @return The maximum height
 */
int Video::getMaxHeight () {

	return maxH;

}


/**
 * Returns the current width of the window or screen.
 *
 * @return The width
 */
int Video::getWidth () {

	return screenW;

}


/**
 * Returns the current height of the window or screen.
 *
 * @return The height
 */
int Video::getHeight () {

	return screenH;

}


/**
 * Sets the window title.
 *
 * @param the title or NULL, to use default
 */
void Video::setTitle (const char *title) {

	const char titleBase[] = "OpenJazz";
	char *windowTitle = NULL;
	int titleLen = strlen(titleBase) + 1;

	if (title != NULL) {

		titleLen = strlen(titleBase) + 3 + strlen(title) + 1;

	}

	windowTitle = new char[titleLen];

	strcpy(windowTitle, titleBase);

	if (title != NULL) {

		strcat(windowTitle, " - ");
		strcat(windowTitle, title);

	}

	SDL_WM_SetCaption(windowTitle, NULL);

	delete[] windowTitle;

}


#ifdef SCALE
/**
 * Returns the current scaling factor.
 *
 * @return The scaling factor
 */
int Video::getScaleFactor () {

	return scaleFactor;

}


/**
 * Sets the scaling factor.
 *
 * @param newScaleFactor The new scaling factor
 */
int Video::setScaleFactor (int newScaleFactor) {

	if ((SW * newScaleFactor <= screenW) && (SH * newScaleFactor <= screenH)) {

		scaleFactor = newScaleFactor;

		if (screen) reset(screenW, screenH);

	}

	return scaleFactor;

}
#endif

#ifndef FULLSCREEN_ONLY
/**
 * Determines whether or not full-screen mode is being used.
 *
 * @return Whether or not full-screen mode is being used
 */
bool Video::isFullscreen () {

	return fullscreen;

}
#endif


/**
 * Refresh display palette.
 */
void Video::expose () {

	SDL_SetPalette(screen, SDL_LOGPAL, logicalPalette, 0, 256);
	SDL_SetPalette(screen, SDL_PHYSPAL, currentPalette, 0, 256);

	return;

}


/**
 * Update video based on a system event.
 *
 * @param event The system event. Events not affecting video will be ignored
 */
void Video::update (SDL_Event *event) {

#if !defined(FULLSCREEN_ONLY) || !defined(NO_RESIZE)
	switch (event->type) {

	#ifndef FULLSCREEN_ONLY
		case SDL_KEYDOWN:

			// If Alt + Enter has been pressed, switch between windowed and full-screen mode.
			if ((event->key.keysym.sym == SDLK_RETURN) &&
				(event->key.keysym.mod & KMOD_ALT)) {

				fullscreen = !fullscreen;

				if (fullscreen) SDL_ShowCursor(SDL_DISABLE);

				reset(screenW, screenH);

				if (!fullscreen) SDL_ShowCursor(SDL_ENABLE);

				findMaxResolution();

			}

			break;
    #endif

    #ifndef NO_RESIZE
		case SDL_VIDEORESIZE:

			reset(event->resize.w, event->resize.h);

			break;
    #endif

		case SDL_VIDEOEXPOSE:

			expose();

			break;

	}
#endif

	return;

}


/**
 * Draw graphics to screen.
 *
 * @param mspf Ticks per frame
 * @param paletteEffects Palette effects to use
 * @param effectsStopped Whether the effects should be applied without advancing
 */
void Video::flip (int mspf, PaletteEffect* paletteEffects, bool effectsStopped) {

	SDL_Color shownPalette[256];

#ifdef SCALE
	if (canvas != screen) {

		// Copy everything that has been drawn so far
		scale(scaleFactor,
			screen->pixels, screen->pitch,
			canvas->pixels, canvas->pitch,
			screen->format->BytesPerPixel, canvas->w, canvas->h);

	}
#endif

	// Apply palette effects
	if (paletteEffects) {

		/* If the palette is being emulated, compile all palette changes and
		apply them all at once.
		If the palette is being used directly, apply all palette effects
		directly. */

		if (fakePalette) {

			memcpy(shownPalette, currentPalette, sizeof(SDL_Color) * 256);

			paletteEffects->apply(shownPalette, false, mspf, effectsStopped);

			SDL_SetPalette(screen, SDL_PHYSPAL, shownPalette, 0, 256);

		} else {

			paletteEffects->apply(shownPalette, true, mspf, effectsStopped);

		}

	}

	// Show what has been drawn
	SDL_Flip(screen);

	return;

}


/**
 * Fill the screen with a colour.
 *
 * @param index Index of the colour to use
 */
void Video::clearScreen (int index) {

#if defined(CAANOO) || defined(WIZ) || defined(GP2X) || defined(GAMESHELL)
	// always 240 lines cleared to black
	memset(video.screen->pixels, index, 320*240);
#elif defined (_3DS)
	memset(video.screen->pixels, index, 400*240);
#else
	SDL_FillRect(canvas, NULL, index);
#endif

	return;

}


/**
 * Fill a specified rectangle of the screen with a colour.
 *
 * @param x X-coordinate of the left side of the rectangle
 * @param y Y-coordinate of the top of the rectangle
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @param index Index of the colour to use
 */
void drawRect (int x, int y, int width, int height, int index) {

	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;

	SDL_FillRect(canvas, &dst, index);

	return;

}

