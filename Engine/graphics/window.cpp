/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2017 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"
#include <iostream>

#include <common_features/graphics_funcs.h>
#include <common_features/logger.h>
#include <common_features/tr.h>
#include <common_features/fmt_format_ne.h>

#include <settings/global_settings.h>
#include <settings/debugger.h>
#include <gui/pge_msgbox.h>
#include "gl_renderer.h"


Scene  *PGE_Window::m_currentScene      = nullptr;

int     PGE_Window::Width               = 800;
int     PGE_Window::Height              = 600;

double  PGE_Window::frameRate           = 1000.0 / 15.0;
int     PGE_Window::frameDelay          = 15;
bool    PGE_Window::vsync               = true;
bool    PGE_Window::vsyncIsSupported    = true;

bool    PGE_Window::showDebugInfo       = false;
bool    PGE_Window::showPhysicsDebug    = false;

SDL_Window      *PGE_Window::window     = NULL;
SDL_GLContext    PGE_Window::glcontext  = NULL;

//! Is graphical sub-system initialized?
static  bool g_isRenderInit     = false;
//! Is mouse cursor must be shown?
static  bool g_showMouseCursor  = true;

static SDL_bool IsFullScreen(SDL_Window *win)
{
    Uint32 flags = SDL_GetWindowFlags(win);
    return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? SDL_TRUE : SDL_FALSE;
}

bool PGE_Window::isSdlError()
{
    const char *error = SDL_GetError();
    return (*error != '\0');
}

bool PGE_Window::checkSDLError(const char *fn, int line, const char *func)
{
    const char *error = SDL_GetError();

    if(*error != '\0')
    {
        PGE_MsgBox::warn(fmt::format_ne("SDL Error: {0}\nFile: {1}\nFunction: {2}\nLine: {3}",
                                     error,
                                     fn,
                                     func,
                                     line));
        SDL_ClearError();
        return true;
    }

    return false;
}

void PGE_Window::printSDLWarn(std::string info)
{
    PGE_MsgBox::warn(fmt::format_ne("{0}\nSDL Error: {1}",
                                 info,
                                 SDL_GetError())
                    );
}

void PGE_Window::printSDLError(std::string info)
{
    PGE_MsgBox::error(fmt::format_ne("{0}\nSDL Error: {1}",
                                  info,
                                  SDL_GetError()));
}

int PGE_Window::msgBoxInfo(std::string title, std::string text)
{
#ifdef PGE_ENGINE_DEBUG
    if(PGE_Debugger::isDebuggerPresent())
    {
        pLogDebug("MESSAGEBOX: %s: %s", title.c_str(), text.c_str());
        return 0;
    }
#endif
    return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                    title.c_str(), text.c_str(), window);
}

int PGE_Window::msgBoxWarning(std::string title, std::string text)
{
#ifdef PGE_ENGINE_DEBUG
    if(PGE_Debugger::isDebuggerPresent())
    {
        pLogWarning("MESSAGEBOX: %s: %s", title.c_str(), text.c_str());
        return 0;
    }
#endif
    return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
                                    title.c_str(), text.c_str(), window);
}

int PGE_Window::msgBoxCritical(std::string title, std::string text)
{
#ifdef PGE_ENGINE_DEBUG
    if(PGE_Debugger::isDebuggerPresent())
    {
        pLogCritical("MESSAGEBOX: %s: %s", title.c_str(), text.c_str());
        return 0;
    }
#endif
    return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                    title.c_str(), text.c_str(), window);
}

bool PGE_Window::init(std::string WindowTitle, int renderType)
{
#if 0 //For testing! Change 0 to 1 and unommend one of GL Renderers to debug one specific renderer!
    GlRenderer::setup_SW_SDL();
    //GlRenderer::setup_OpenGL21();
    //GlRenderer::setup_OpenGL31();
#else
    //Detect renderer
    GlRenderer::RenderEngineType rtype = GlRenderer::setRenderer(static_cast<GlRenderer::RenderEngineType>(renderType));

    if(rtype == GlRenderer::RENDER_INVALID)
    {
        //% "Unable to find OpenGL support!\nSoftware renderer will be started.\n"
        printSDLWarn(qtTrId("RENDERER_NO_OPENGL_ERROR"));
        SDL_ClearError();
        rtype = GlRenderer::RENDER_SW_SDL;
    }

    switch(rtype)
    {
    case GlRenderer::RENDER_OPENGL_3_1:
        GlRenderer::setup_OpenGL31();
        break;

    case GlRenderer::RENDER_OPENGL_2_1:
        GlRenderer::setup_OpenGL21();
        break;

    case GlRenderer::RENDER_SW_SDL:
        GlRenderer::setup_SW_SDL();
        break;

    case GlRenderer::RENDER_AUTO:
    case GlRenderer::RENDER_INVALID:
        //% "Renderer is not selected!"
        printSDLError(qtTrId("NO_RENDERER_ERROR"));
        return false;
    }
#endif //0

    GlRenderer::setVirtualSurfaceSize(Width, Height);
    GlRenderer::setViewportSize(Width, Height);
    window = SDL_CreateWindow(WindowTitle.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                          #ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
                              Width / 2, Height / 2,
                          #else
                              Width, Height,
                          #endif //__EMSCRIPTEN__
                              SDL_WINDOW_RESIZABLE |
                              SDL_WINDOW_HIDDEN |
                              SDL_WINDOW_ALLOW_HIGHDPI |
                              GlRenderer::SDL_InitFlags());

    if(window == NULL)
    {
        //% "Unable to create window!"
        printSDLError(qtTrId("WINDOW_CREATE_ERROR"));
        SDL_ClearError();
        return false;
    }

    if(isSdlError())
    {
        //% "Unable to create window!"
        printSDLError(qtTrId("WINDOW_CREATE_ERROR"));
        SDL_ClearError();
        return false;
    }

#ifdef __EMSCRIPTEN__ //Set canvas be 1/2 size for a faster rendering
    SDL_SetWindowMinimumSize(window, Width / 2, Height / 2);
#else
    SDL_SetWindowMinimumSize(window, Width, Height);
#endif //__EMSCRIPTEN__

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    GraphicsHelps::initFreeImage();

#ifdef _WIN32
    FIBITMAP *img[2];
    img[0] = GraphicsHelps::loadImageRC("cat_16.png");
    img[1] = GraphicsHelps::loadImageRC("cat_32.png");

    if(!GraphicsHelps::setWindowIcon(window, img[0], 16))
    {
        //% "Unable to setup window icon!"
        printSDLWarn(qtTrId("WINDOW_ICON_INIT_ERROR"));
        SDL_ClearError();
    }

    if(!GraphicsHelps::setWindowIcon(window, img[1], 32))
    {
        //% "Unable to setup window icon!"
        printSDLWarn(qtTrId("WINDOW_ICON_INIT_ERROR"));
        SDL_ClearError();
    }

    GraphicsHelps::closeImage(img[0]);
    GraphicsHelps::closeImage(img[1]);
#else//IF _WIN32

    FIBITMAP *img;
#   ifdef __APPLE__
    img = GraphicsHelps::loadImageRC("cat_256.png");
#   else
    img = GraphicsHelps::loadImageRC("cat_32.png");
#   endif //__APPLE__

    if(img)
    {
        SDL_Surface *sicon = GraphicsHelps::fi2sdl(img);
        SDL_SetWindowIcon(window, sicon);
        GraphicsHelps::closeImage(img);
        SDL_FreeSurface(sicon);

        if(isSdlError())
        {
            //% "Unable to setup window icon!"
            printSDLWarn(qtTrId("WINDOW_ICON_INIT_ERROR"));
            SDL_ClearError();
        }
    }
#endif//IF _WIN32 #else

    g_isRenderInit = true;
    //Init OpenGL (to work with textures, OpenGL should be load)
    pLogDebug("Init OpenGL settings...");

    if(!GlRenderer::init())
    {
        //% "Unable to initialize renderer context!"
        printSDLError(qtTrId("RENDERER_CONTEXT_INIT_ERROR"));
        SDL_ClearError();
        g_isRenderInit = false;
        return false;
    }

    pLogDebug("Toggle vsync...");
    vsyncIsSupported = (SDL_GL_SetSwapInterval(1) == 0);
    toggleVSync(vsync);
    pLogDebug(fmt::format_ne("V-Sync supported: {0}", vsyncIsSupported).c_str());
    return true;
}

void PGE_Window::setWindowTitle(std::string title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

void PGE_Window::toggleVSync(bool vsync)
{
    if(vsync)
    {
        int display_count = 0, display_index = 0;
        SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

        if((display_count = SDL_GetNumVideoDisplays()) < 1)
        {
            pLogWarning("SDL_GetNumVideoDisplays returned: %d", display_count);
            return;
        }

        if(SDL_GetCurrentDisplayMode(display_index, &mode) != 0)
        {
            pLogWarning("SDL_GetDisplayMode failed: %s", SDL_GetError());
            return;
        }

        //SDL_Log("SDL_GetDisplayMode(0, 0, &mode):\t\t%i bpp\t%i x %i",
        //SDL_BITSPERPIXEL(mode.format), mode.w, mode.h);
        //const char *error = SDL_GetError();
        if(SDL_GL_SetSwapInterval(1) == 0)
        {
            //Vertical syncronization is supported
            vsyncIsSupported = true;
            if(mode.refresh_rate > 0)
                frameDelay = static_cast<int>(std::ceil(1000.0 / static_cast<double>(mode.refresh_rate)));
            frameRate  = 1000.0 / static_cast<double>(frameDelay);
            g_AppSettings.timeOfFrame = frameDelay;
            g_AppSettings.frameRate = frameRate;
            SDL_ClearError();
        }
        else
        {
            //Vertical syncronization is NOT supported
            vsyncIsSupported = false;
            frameDelay = g_AppSettings.timeOfFrame;
            frameRate = g_AppSettings.frameRate;
            //Disable vertical syncronization because unsupported
            g_AppSettings.vsync = false;
            PGE_Window::vsync = false;
            SDL_GL_SetSwapInterval(0);
            SDL_ClearError();
        }
    }
    else
        SDL_GL_SetSwapInterval(0);
}


bool PGE_Window::uninit()
{
    if(!g_isRenderInit)
        return false;

    // Swith to WINDOWED mode
    if(SDL_SetWindowFullscreen(window, SDL_FALSE) < 0)
    {
        pLogWarning("Setting windowed failed: %s", SDL_GetError());
        return false;
    }

    SDL_HideWindow(window);
    GlRenderer::uninit();
    GraphicsHelps::closeFreeImage();
    SDL_DestroyWindow(window);
    window = NULL;
    g_isRenderInit = false;
    return true;
}

bool PGE_Window::isReady()
{
    return g_isRenderInit;
}

void PGE_Window::setCursorVisibly(bool viz)
{
    g_showMouseCursor = viz;

    if(window != NULL)
    {
        if(!IsFullScreen(window))
        {
            if(g_showMouseCursor)
                SDL_ShowCursor(SDL_ENABLE);
            else
                SDL_ShowCursor(SDL_DISABLE);
        }
    }
}

void PGE_Window::clean()
{
    if(window == NULL) return;

    GlRenderer::setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    GlRenderer::clearScreen();
    GlRenderer::flush();
    GlRenderer::repaint();
}

int PGE_Window::setFullScreen(bool fs)
{
    if(window == NULL) return -1;

    if(fs != IsFullScreen(window))
    {
        if(fs)
        {
            // Swith to FULLSCREEN mode
            if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
            {
                //Hide mouse cursor in full screen mdoe
                pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
                return -1;
            }

            SDL_ShowCursor(SDL_DISABLE);
            return 1;
        }
        else
        {
            //Show mouse cursor
            if(g_showMouseCursor)
                SDL_ShowCursor(SDL_ENABLE);

            // Swith to WINDOWED mode
            if(SDL_SetWindowFullscreen(window, SDL_FALSE) < 0)
            {
                pLogWarning("Setting windowed failed: %s", SDL_GetError());
                return -1;
            }

            return 0;
        }
    }

    return 0;
}


/// Toggles On/Off FullScreen
/// @returns -1 on error, 1 on Set fullscreen successful, 0 on Set Windowed successful
int PGE_Window::SDL_ToggleFS(SDL_Window *win)
{
    if(!win)
        win = window;

    if(IsFullScreen(win))
    {
        //Show mouse cursor
        if(g_showMouseCursor)
            SDL_ShowCursor(SDL_ENABLE);

        // Swith to WINDOWED mode
        if(SDL_SetWindowFullscreen(win, SDL_FALSE) < 0)
        {
            pLogWarning("Setting windowed failed: %s", SDL_GetError());
            return -1;
        }

        return 0;
    }

    // Swith to FULLSCREEN mode
    if(SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP) < 0)
    {
        //Hide mouse cursor in full screen mdoe
        pLogWarning("Setting fullscreen failed: %s", SDL_GetError());
        return -1;
    }

    SDL_ShowCursor(SDL_DISABLE);
    return 1;
}


int PGE_Window::processEvents(SDL_Event &event)
{
    switch(event.type)
    {
    case SDL_WINDOWEVENT:
    {
        if((event.window.event == SDL_WINDOWEVENT_RESIZED)||
                (event.window.event == SDL_WINDOWEVENT_MOVED))
            GlRenderer::resetViewport();

        return 1;
    }

    case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
        case SDLK_f:
            if((event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) != 0)
            {
                g_AppSettings.fullScreen = (PGE_Window::SDL_ToggleFS(PGE_Window::window) == 1);
                return 2;
            }

            break;
            #ifdef PANIC_KEY //Panic! (If you wanna have able to quickly close game

        //        from employer - add "DEFINES+=PANIC_KEY" into qmake args
        //        and then you can press NumPad + to instantly close game)
        case SDLK_KP_PLUS:
        {
            SDL_DestroyWindow(window);
            SDL_CloseAudio();
            exit(EXIT_FAILURE);
            return 2;
        }

        #endif

        case SDLK_F2:
        {
            PGE_Window::showPhysicsDebug = !PGE_Window::showPhysicsDebug;
            return 2;
        }

        case SDLK_F3:
        {
            PGE_Window::showDebugInfo = !PGE_Window::showDebugInfo;
            return 2;
        }

        case SDLK_F11:
        {
            GlRenderer::toggleRecorder();
            return 2;
        }

        case SDLK_F12:
        {
            GlRenderer::makeShot();
            return 2;
        }
        }

        break;
    }

    return 0;
}
