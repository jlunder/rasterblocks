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

#ifdef RB_USE_GLFW3_GLES2_HARNESS

#include "gles2_harness.h"


#include <GLFW/glfw3.h>

#include <time.h>

#if defined RB_LINUX
#elif defined RB_OSX
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


#include "rasterblocks.h"

#include "control_input.h"


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
    
    if(action == GLFW_PRESS) {
        switch(key) {
        case 'Q': gles2_harness_set_trigger(0); break;
        case 'W': gles2_harness_set_trigger(1); break;
        case 'E': gles2_harness_set_trigger(2); break;
        case 'R': gles2_harness_set_trigger(3); break;
        case 'T': gles2_harness_set_trigger(4); break;
        case 'Y': gles2_harness_set_trigger(5); break;
        case 'U': gles2_harness_set_trigger(6); break;
        case 'I': gles2_harness_set_trigger(7); break;
        case 'O': gles2_harness_set_trigger(8); break;
        case 'P': gles2_harness_set_trigger(9); break;
        
        case 'A': gles2_harness_set_controller_inc(0, true); break;
        case 'S': gles2_harness_set_controller_inc(1, true); break;
        case 'D': gles2_harness_set_controller_inc(2, true); break;
        case 'F': gles2_harness_set_controller_inc(3, true); break;
        case 'G': gles2_harness_set_controller_inc(4, true); break;
        case 'H': gles2_harness_set_controller_inc(5, true); break;
        case 'J': gles2_harness_set_controller_inc(6, true); break;
        case 'K': gles2_harness_set_controller_inc(7, true); break;
        case 'L': gles2_harness_set_controller_inc(8, true); break;
        case ';': gles2_harness_set_controller_inc(9, true); break;
        
        case 'Z': gles2_harness_set_controller_dec(0, true); break;
        case 'X': gles2_harness_set_controller_dec(1, true); break;
        case 'C': gles2_harness_set_controller_dec(2, true); break;
        case 'V': gles2_harness_set_controller_dec(3, true); break;
        case 'B': gles2_harness_set_controller_dec(4, true); break;
        case 'N': gles2_harness_set_controller_dec(5, true); break;
        case 'M': gles2_harness_set_controller_dec(6, true); break;
        case ',': gles2_harness_set_controller_dec(7, true); break;
        case '.': gles2_harness_set_controller_dec(8, true); break;
        case '/': gles2_harness_set_controller_dec(9, true); break;
        
        case '[': gles2_harness_debug_mode_prev(); break;
        case ']': gles2_harness_debug_mode_next(); break;
        
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
    }
    else if(action == GLFW_RELEASE) {
        switch(key) {
        case 'A': gles2_harness_set_controller_inc(0, false); break;
        case 'S': gles2_harness_set_controller_inc(1, false); break;
        case 'D': gles2_harness_set_controller_inc(2, false); break;
        case 'F': gles2_harness_set_controller_inc(3, false); break;
        case 'G': gles2_harness_set_controller_inc(4, false); break;
        case 'H': gles2_harness_set_controller_inc(5, false); break;
        case 'J': gles2_harness_set_controller_inc(6, false); break;
        case 'K': gles2_harness_set_controller_inc(7, false); break;
        case 'L': gles2_harness_set_controller_inc(8, false); break;
        case ';': gles2_harness_set_controller_inc(9, false); break;
        
        case 'Z': gles2_harness_set_controller_dec(0, false); break;
        case 'X': gles2_harness_set_controller_dec(1, false); break;
        case 'C': gles2_harness_set_controller_dec(2, false); break;
        case 'V': gles2_harness_set_controller_dec(3, false); break;
        case 'B': gles2_harness_set_controller_dec(4, false); break;
        case 'N': gles2_harness_set_controller_dec(5, false); break;
        case 'M': gles2_harness_set_controller_dec(6, false); break;
        case ',': gles2_harness_set_controller_dec(7, false); break;
        case '.': gles2_harness_set_controller_dec(8, false); break;
        case '/': gles2_harness_set_controller_dec(9, false); break;
        
        default: break;
        }
    }
}


int main(int argc, char * argv[])
{
    GLFWwindow* window;

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
    
#if defined RB_LINUX
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
        
        glfwGetFramebufferSize(window, &width, &height);

        gles2_harness_reshape(width, height);
        
        gles2_harness_update();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    gles2_harness_terminate();

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


#endif // RB_USE_GLES2_HARNESS


