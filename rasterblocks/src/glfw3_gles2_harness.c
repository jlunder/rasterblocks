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
        RBControls * pControls = rbControlInputHarnessGetStoredControls();
        
        switch(key) {
        case 'Q': pControls->triggers[0] = true; break;
        case 'W': pControls->triggers[1] = true; break;
        case 'E': pControls->triggers[2] = true; break;
        case 'R': pControls->triggers[3] = true; break;
        case 'T': pControls->triggers[4] = true; break;
        case 'Y': pControls->triggers[5] = true; break;
        case 'U': pControls->triggers[6] = true; break;
        case 'I': pControls->triggers[7] = true; break;
        case 'O': pControls->triggers[8] = true; break;
        case 'P': pControls->triggers[9] = true; break;
        
        case 'A': pControls->controllers[0] = rbMinF(
            pControls->controllers[0] + 1.0f / 16, 1.0f); break;
        case 'S': pControls->controllers[1] = rbMinF(
            pControls->controllers[1] + 1.0f / 16, 1.0f); break;
        case 'D': pControls->controllers[2] = rbMinF(
            pControls->controllers[2] + 1.0f / 16, 1.0f); break;
        case 'F': pControls->controllers[3] = rbMinF(
            pControls->controllers[3] + 1.0f / 16, 1.0f); break;
        case 'G': pControls->controllers[4] = rbMinF(
            pControls->controllers[4] + 1.0f / 16, 1.0f); break;
        case 'H': pControls->controllers[5] = rbMinF(
            pControls->controllers[5] + 1.0f / 16, 1.0f); break;
        case 'J': pControls->controllers[6] = rbMinF(
            pControls->controllers[6] + 1.0f / 16, 1.0f); break;
        case 'K': pControls->controllers[7] = rbMinF(
            pControls->controllers[7] + 1.0f / 16, 1.0f); break;
        case 'L': pControls->controllers[8] = rbMinF(
            pControls->controllers[8] + 1.0f / 16, 1.0f); break;
        case ';': pControls->controllers[9] = rbMinF(
            pControls->controllers[9] + 1.0f / 16, 1.0f); break;
        
        case 'Z': pControls->controllers[0] = rbMaxF(
            pControls->controllers[0] - 1.0f / 16, -1.0f); break;
        case 'X': pControls->controllers[1] = rbMaxF(
            pControls->controllers[1] - 1.0f / 16, -1.0f); break;
        case 'C': pControls->controllers[2] = rbMaxF(
            pControls->controllers[2] - 1.0f / 16, -1.0f); break;
        case 'V': pControls->controllers[3] = rbMaxF(
            pControls->controllers[3] - 1.0f / 16, -1.0f); break;
        case 'B': pControls->controllers[4] = rbMaxF(
            pControls->controllers[4] - 1.0f / 16, -1.0f); break;
        case 'N': pControls->controllers[5] = rbMaxF(
            pControls->controllers[5] - 1.0f / 16, -1.0f); break;
        case 'M': pControls->controllers[6] = rbMaxF(
            pControls->controllers[6] - 1.0f / 16, -1.0f); break;
        case ',': pControls->controllers[7] = rbMaxF(
            pControls->controllers[7] - 1.0f / 16, -1.0f); break;
        case '.': pControls->controllers[8] = rbMaxF(
            pControls->controllers[8] - 1.0f / 16, -1.0f); break;
        case '/': pControls->controllers[9] = rbMaxF(
            pControls->controllers[9] - 1.0f / 16, -1.0f); break;
        
        case '[':
            if(pControls->debugDisplayMode > RBDM_OFF) {
                --pControls->debugDisplayMode;
                pControls->debugDisplayReset = true;
            }
            break;
        case ']':
            if(pControls->debugDisplayMode + 1 < RBDM_COUNT) {
                ++pControls->debugDisplayMode;
                pControls->debugDisplayReset = true;
            }
            break;
        
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
    }
    else if(action == GLFW_RELEASE) {
        switch(key) {
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


