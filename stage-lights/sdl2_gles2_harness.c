#ifdef STAGE_LIGHTS_USE_SDL2_GLES2_HARNESS

#include "gles2_harness.h"

#include <GLES2/gl2.h>
#include <SDL2/SDL.h>

#include <time.h>

#if defined STAGE_LIGHTS_LINUX
#elif defined STAGE_LIGHTS_OSX
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif


#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <stdbool.h>

#include <string.h>


#include "glus.h"


#include "stage_lights.h"


/* A simple function that prints a message, the error code returned by SDL,
 * and quits the application */
void sdldie(const char *msg)
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}
 
 
void checkSDLError(int line)
{
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
	}
#endif
}
 
 
/* Our program's entry point */
int main(int argc, char *argv[])
{
#ifdef STAGE_LIGHTS_LINUX
    struct timespec lastts;
#endif
#ifdef STAGE_LIGHTS_OSX
    uint64_t lasttime = mach_absolute_time();
    mach_timebase_info_data_t timebase;
#endif
    SDL_Window *mainwindow; /* Our window handle */
    SDL_GLContext maincontext; /* Our opengl context handle */
    SDL_Event event;
    bool quit = false;
    
    (void)argc;
    (void)argv;
 
    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
        sdldie("Unable to initialize SDL"); /* Or die on error */
 
    /* Request opengl es 2.0 context. */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
 
    /* Turn on double buffering with a 24bit Z buffer.
     * You may need to change this to 16 or 32 for your system */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
 
    /* Create our window centered at 512x512 resolution */
    mainwindow = SDL_CreateWindow("Stage Lights", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!mainwindow) /* Die if creation failed */
        sdldie("Unable to create window");
 
    checkSDLError(__LINE__);
 
    /* Create our opengl context and attach it to our window */
    maincontext = SDL_GL_CreateContext(mainwindow);
    checkSDLError(__LINE__);
 
 
    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);
 
 
#ifdef STAGE_LIGHTS_LINUX
    clock_gettime(CLOCK_MONOTONIC, &lastts);
#endif
#ifdef STAGE_LIGHTS_OSX
    mach_timebase_info(&timebase);
#endif
    gles2_harness_init(argc > 1 ? argv[1] : NULL);

    while (!quit)
    {
        int width = 0, height = 0;
#ifdef STAGE_LIGHTS_LINUX
        struct timespec ts;
#endif
#ifdef STAGE_LIGHTS_OSX
        uint64_t time;
#endif
        uint64_t time_ns;

		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				quit = true;
			}
		}

        SDL_GetWindowSize(mainwindow, &width, &height);
        gles2_harness_reshape(width, height);
        
#ifdef STAGE_LIGHTS_LINUX
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_nsec -= lastts.tv_nsec;
        if(ts.tv_nsec < 0) {
            ts.tv_nsec += 1000000000;
            --ts.tv_sec;
        }
        assert(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000);
        assert(ts.tv_sec >= 0);
        lastts = ts;
        time_ns = ts.tv_nsec + ts.tv_sec * 1000000000;
#endif
#ifdef STAGE_LIGHTS_OSX
        time = mach_absolute_time();
        time_ns = ((time - lasttime) * timebase.numer) / timebase.denom;
        lasttime = time;
#endif
        
        gles2_harness_update((float)(time_ns / 1.0e9));

		SDL_GL_SwapWindow(mainwindow);
    }
    
    gles2_harness_terminate();

    /* Delete our opengl context, destroy our window, and shutdown SDL */
    SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
    SDL_Quit();

    return 0;
}

#endif

