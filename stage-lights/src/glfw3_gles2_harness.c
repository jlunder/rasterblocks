//========================================================================
// Simple GLFW example
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#ifdef STAGE_LIGHTS_USE_GLFW3_GLES2_HARNESS

#include "gles2_harness.h"


#include <GLFW/glfw3.h>

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


static void error_callback(int error, const char* description)
{
    GLUS_UNUSED(error);
    
    fputs(description, stderr);
}


static void key_callback(
	GLFWwindow* window,
	int key,
	int scancode, int action,
	int mods)
{
	GLUS_UNUSED(window);
	GLUS_UNUSED(key);
    GLUS_UNUSED(scancode);
    GLUS_UNUSED(action);
    GLUS_UNUSED(mods);
    
/*
    if(action == GLFW_PRESS) {
        switch(key) {
        case 'Q': ddh_mode = 0; break;
        case 'W': ddh_mode = 1; break;
        case 'E': ddh_mode = 2; break;
        case 'R': ddh_mode = 3; break;
        case 'T': ddh_mode = 4; break;
        case 'Y': ddh_mode = 5; break;
        case 'U': ddh_mode = 6; break;
        case 'I': ddh_mode = 7; break;
        case 'O': ddh_mode = 8; break;
        case 'P': ddh_mode = 9; break;
        
        case 'A': ddh_submode = 0; break;
        case 'S': ddh_submode = 1; break;
        case 'D': ddh_submode = 2; break;
        case 'F': ddh_submode = 3; break;
        case 'G': ddh_submode = 4; break;
        case 'H': ddh_submode = 5; break;
        case 'J': ddh_submode = 6; break;
        case 'K': ddh_submode = 7; break;
        case 'L': ddh_submode = 8; break;
        case ';': ddh_submode = 9; break;
        
        case '[': ddh_button_a = true; break;
        case ']': ddh_button_b = true; break;
        
        case GLFW_KEY_LEFT: gles2_harness_input_left = true; break;
        case GLFW_KEY_RIGHT: gles2_harness_input_right = true; break;
        case GLFW_KEY_UP: gles2_harness_input_up = true; break;
        case GLFW_KEY_DOWN: gles2_harness_input_down = true; break;
        
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
    }
    else if(action == GLFW_RELEASE) {
        switch(key) {
        case '[': ddh_button_a = false; break;
        case ']': ddh_button_b = false; break;
        
        case GLFW_KEY_LEFT: gles2_harness_input_left = false;
        case GLFW_KEY_RIGHT: gles2_harness_input_right = false;
        case GLFW_KEY_UP: gles2_harness_input_up = false;
        case GLFW_KEY_DOWN: gles2_harness_input_down = false;
        }
    }
*/
}


int main(int argc, char * argv[])
{
    GLFWwindow* window;
#ifdef STAGE_LIGHTS_LINUX
    struct timespec lastts;
    
    clock_gettime(CLOCK_MONOTONIC, &lastts);
#endif
#ifdef STAGE_LIGHTS_OSX
    uint64_t lasttime = mach_absolute_time();
    mach_timebase_info_data_t timebase;
    
    mach_timebase_info(&timebase);
#endif

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);


    window = glfwCreateWindow(640, 480, "Stage Lights", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    
#if defined STAGE_LIGHTS_LINUX
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error %s\n", glewGetErrorString(err));
        exit(1);
    }
#endif
    
    gles2_harness_init(argc, argv);

    while (!glfwWindowShouldClose(window))
    {
        int width = 0, height = 0;
#ifdef STAGE_LIGHTS_LINUX
        struct timespec ts;
#endif
#ifdef STAGE_LIGHTS_OSX
        uint64_t time;
#endif
        uint64_t time_ns;

        glfwGetFramebufferSize(window, &width, &height);

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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    gles2_harness_terminate();

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


#endif // STAGE_LIGHTS_USE_GLES2_HARNESS


