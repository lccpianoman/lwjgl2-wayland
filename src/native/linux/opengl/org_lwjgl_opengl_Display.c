/*
 * Copyright (c) 2002-2008 LWJGL Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of 'LWJGL' nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * $Id$
 *
 * Linux specific display functions.
 *
 * @author elias_naur <elias_naur@users.sourceforge.net>
 * @version $Revision$
 */

#include <epoxy/gl.h>
#include <epoxy/egl.h>
#define NO_SDL_GLEXT
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <jni.h>
#include "common_tools.h"
#include "context.h"
#include "org_lwjgl_opengl_LinuxDisplay.h"
#include "org_lwjgl_opengl_LinuxDisplayPeerInfo.h"
#include "org_lwjgl_LinuxSysImplementation.h"

int is_running = 0;
SDL_Window *context_window;
SDL_GLContext context;
int window_singleton_flag = 0;

JNIEXPORT jint JNICALL Java_org_lwjgl_DefaultSysImplementation_getJNIVersion
  (JNIEnv *env, jobject ignored) {
	return org_lwjgl_LinuxSysImplementation_JNI_VERSION;
}

JNIEXPORT jstring JNICALL Java_org_lwjgl_opengl_LinuxDisplay_getErrorText(JNIEnv *env, jclass unused, jlong display_ptr, jlong error_code) {
	const char *str = "Unimplemented see " __FILE__;
	return NewStringNativeWithLength(env, str, strlen(str));
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_callErrorHandler(JNIEnv *env, jclass unused, jlong handler_ptr, jlong display_ptr, jlong event_ptr) {
	return 0;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_setErrorHandler(JNIEnv *env, jclass unused) {
	return 1;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_resetErrorHandler(JNIEnv *env, jclass unused, jlong handler_ptr) {
	return 1;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSync(JNIEnv *env, jclass unused, jlong display_ptr, jboolean throw_away_events) {
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_sync(JNIEnv *env, jclass unused, jlong display_ptr, jboolean throw_away_events) {
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetDefaultScreen(JNIEnv *env, jclass unused, jlong display_ptr) {
	return 1; // aka initialized in OpenDisplay
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nInternAtom(JNIEnv *env, jclass unused, jlong display_ptr, jstring atom_name_obj, jboolean only_if_exists) {
	static int i = 0;
	i++;
	return i;
}

// suppress -Wimplicit-function-declaration
extern int setenv(const char *name, const char *value, int overwrite);

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_openDisplay(JNIEnv *env, jclass clazz) {
	static int initialized = 0;
	if (initialized) {
		printfDebugJava(env, "SDL already initialized.");
		return initialized;
	}

	const char *sdl_videodriver = getenv("SDL_VIDEODRIVER");

	if (sdl_videodriver == NULL || strcmp(sdl_videodriver, "wayland") != 0) {
		if (setenv("SDL_VIDEODRIVER", "wayland", 1) != 0) {
			throwException(env, "Failed to set SDL video driver to wayland.");
			return 0;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		throwFormattedException(env, "Failed to initialize SDL: %s", SDL_GetError());
		return initialized;
	}

	initialized = 1;

	return initialized;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_closeDisplay(JNIEnv *env, jclass clazz, jlong display) {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(context_window);
	SDL_Quit();
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplayPeerInfo_initDrawable(JNIEnv *env, jclass clazz, jlong window, jobject peer_info_handle) {
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplayPeerInfo_initDefaultPeerInfo(JNIEnv *env, jclass clazz, jlong display, jint screen, jobject peer_info_handle, jobject pixel_format) {
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSetTitle(JNIEnv * env, jclass clazz, jlong title) {
	const char *str = (const char *)title;
	SDL_SetWindowTitle(context_window, str);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nReshape(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr, jint x, jint y, jint width, jint height) {
	SDL_Window *window = (SDL_Window *)window_ptr;
	SDL_SetWindowSize(window, width, height);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_synchronize(JNIEnv *env, jclass clazz, jlong display, jboolean synchronize) {
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetX(JNIEnv *env, jclass unused) {
	int x;
	int y;
	SDL_GetWindowPosition(context_window, &x, &y);
	return x;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetY(JNIEnv *env, jclass unused) {
	int x;
	int y;
	SDL_GetWindowPosition(context_window, &x, &y);
	return y;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetWidth(JNIEnv *env, jclass unused) {
	int w;
	int h;
	SDL_GL_GetDrawableSize(context_window, &w, &h);
	return w;
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetHeight(JNIEnv *env, jclass unused) {
	int w;
	int h;
	SDL_GL_GetDrawableSize(context_window, &w, &h);
	return h;
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGetInputFocus(JNIEnv *env, jclass unused, jlong display_ptr) {
	return (intptr_t)context_window;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSetInputFocus(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr, jlong time) {
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nCreateWindow(JNIEnv *env, jclass clazz, jboolean fullscreen, jint x, jint y, jint width, jint height) {
	if (window_singleton_flag) {
		fprintf(stderr, "=============================================\n");
		fprintf(stderr, "Window has already been created.\n");
		fprintf(stderr, "The wayland patch doesn't delete and recreate\n");
		fprintf(stderr, "the window upon setting fullscreen\n");
		fprintf(stderr, "=============================================\n");
		fflush(stderr);
		return (intptr_t)context_window;
	}
	
	int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		| SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_ALWAYS_ON_TOP;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	context_window = SDL_CreateWindow("lwjgl2-sdl-wayland",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height,
			flags);

	if (!context_window) {
		throwFormattedException(env, "Failed to create SDL window: %s", SDL_GetError());
		SDL_Quit();
		return 0;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
	context = SDL_GL_CreateContext(context_window);
	if (context == NULL) {
		throwFormattedException(env, "Failed to create SDL GL context: %s", SDL_GetError());
		SDL_DestroyWindow(context_window);
		SDL_Quit();
		return 0;
	}

	SDL_GL_MakeCurrent(context_window, context);

	SDL_GL_SetSwapInterval(1);

	is_running = 1;
	window_singleton_flag = 1;
	return (intptr_t)context_window;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSetWindowSize(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr, jint width, jint height, jboolean resizable) {
	SDL_SetWindowSize(context_window, width, height);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nDestroyWindow(JNIEnv *env, jclass clazz, jlong display, jlong window_ptr) {
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSetWindowIcon
  (JNIEnv *env, jclass clazz, jlong display, jlong window_ptr, jobject icons_buffer, jint icons_buffer_size)
{
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nUngrabKeyboard(JNIEnv *env, jclass unused) {
	SDL_SetWindowKeyboardGrab(context_window, SDL_FALSE);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGrabKeyboard(JNIEnv *env, jclass unused) {
	SDL_SetWindowKeyboardGrab(context_window, SDL_TRUE);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nGrabPointer(JNIEnv *env, jclass unused) {
	return SDL_SetRelativeMouseMode(SDL_TRUE);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nSetViewPort(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jint screen) {
	int w;
	int h;
	SDL_GL_GetDrawableSize(context_window, &w, &h);
	glViewport(0, 0, w, h);
}

JNIEXPORT jint JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nUngrabPointer(JNIEnv *env, jclass unused) {
	return SDL_SetRelativeMouseMode(SDL_FALSE);
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nDefineCursor(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jlong cursor_ptr) {
	/*Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	Cursor cursor = (Cursor)cursor_ptr;
	XDefineCursor(disp, win, cursor);*/
}

JNIEXPORT jlong JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nCreateBlankCursor(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr) {
	/*Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	unsigned int best_width, best_height;
	if (XQueryBestCursor(disp, win, 1, 1, &best_width, &best_height) == 0) {
		throwException(env, "Could not query best cursor size");
		return false;
	}
	Pixmap mask = XCreatePixmap(disp, win, best_width, best_height, 1);
	XGCValues gc_values;
	gc_values.foreground = 0;
	GC gc = XCreateGC(disp, mask, GCForeground, &gc_values);
	XFillRectangle(disp, mask, gc, 0, 0, best_width, best_height);
	XFreeGC(disp, gc);
	XColor dummy_color;
	Cursor cursor = XCreatePixmapCursor(disp, mask, mask, &dummy_color, &dummy_color, 0, 0);
	XFreePixmap(disp, mask);
	return cursor;*/
	return 0;
}

JNIEXPORT void JNICALL Java_org_lwjgl_opengl_LinuxDisplay_nIconifyWindow(JNIEnv *env, jclass unused, jlong display_ptr, jlong window_ptr, jint screen) {
	/*Display *disp = (Display *)(intptr_t)display_ptr;
	Window win = (Window)window_ptr;
	XIconifyWindow(disp, win, screen);*/
}
