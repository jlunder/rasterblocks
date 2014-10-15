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

#ifdef STAGE_LIGHTS_USE_GLES2_HARNESS

#include "gles2_harness.h"


#if defined STAGE_LIGHTS_LINUX
#include <GL/glew.h>
#endif

#include <GL/gl.h>

#if defined STAGE_LIGHTS_LINUX
#include <GL/glfw.h>
#define GLFW_V2
#elif defined STAGE_LIGHTS_OSX
#include <GLFW/glfw3.h>
#define GLFW_V3
#endif

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


static GLuint g_program;
static GLuint g_vertShader;
static GLuint g_fragShader;

static GLint g_viewProjectionMatrixLocation;
static GLint g_modelMatrixLocation;
static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLuint g_verticesVBO;

static GLfloat g_aspectRatio;

#define GLES2_HARNESS_LINE_BUF_EMPTY    1
#define GLES2_HARNESS_LINE_BUF_FULL     2
#define GLES2_HARNESS_LINE_BUF_OVERRUN  3
#define GLES2_HARNESS_LINE_BUF_FLUSHING 4

#define GLES2_HARNESS_LINE_NOT_READY 1
#define GLES2_HARNESS_LINE_READY     2
#define GLES2_HARNESS_LINE_LOST      3

size_t gles2_harness_serial_count;
int gles2_harness_line_buf_state = GLES2_HARNESS_LINE_BUF_EMPTY;
char gles2_harness_serial_buf[1024];
char gles2_harness_line_buf[1024];
int gles2_harness_serial_fd = -1;

float gles2_harness_fake_input_time = 0.0f;

float gles2_harness_dist = 1.0f;
float gles2_harness_horizontal_pos = 0.0f;
float gles2_harness_vertical_pos = 0.0f;


bool gles2_harness_init(char const * dev);
int gles2_harness_serial_set_interface_attribs(int fd, int speed, int parity);
void gles2_harness_serial_set_blocking(int fd, int should_block);
void gles2_harness_init_serial(char const * dev);
void gles2_harness_process_serial(void);
int gles2_harness_read_serial(void);
void gles2_harness_reshape(int width, int height);
void gles2_harness_update(float time);
void gles2_harness_generate_motion_input(float time);
void gles2_harness_process_lookaround_input(float time);
void gles2_harness_draw_lights(float time);
void gles2_harness_terminate(void);


#ifdef GLFW_V3
static void error_callback(int error, const char* description)
{
    UNUSED(error);
    
    fputs(description, stderr);
}
#endif


static void key_callback(
#ifdef GLFW_V3
	GLFWwindow* window,
#endif
	int key,
#ifdef GLFW_V3
	int scancode, int action,
#endif
	int mods)
{
#ifdef GLFW_V3
	UNUSED(window);
    UNUSED(scancode);
#endif
	UNUSED(key);
    UNUSED(mods);
    
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
#ifdef GLFW_V3
    GLFWwindow* window;
#endif
#ifdef GLFW_V2
	GLFWvidmode vm;
#endif
#ifdef STAGE_LIGHTS_LINUX
    struct timespec lastts;
    
    clock_gettime(CLOCK_MONOTONIC, &lastts);
#endif
#ifdef STAGE_LIGHTS_OSX
    uint64_t lasttime = mach_absolute_time();
    mach_timebase_info_data_t timebase;
    
    mach_timebase_info(&timebase);
#endif

#ifdef GLFW_V3
    glfwSetErrorCallback(error_callback);
#endif

    if (!glfwInit())
        exit(EXIT_FAILURE);


#ifdef GLFW_V3
    window = glfwCreateWindow(640, 480, "Stage Lights", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
#endif
#ifdef GLFW_V2
	glfwGetDesktopMode(&vm);
	glfwSetWindowTitle("Stage Lights");
    if (!glfwOpenWindow(640, 480, 0, 0, 0, 0, 32, 0, GLFW_WINDOW))
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
#endif

#ifdef GLFW_V3
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
#endif
#ifdef GLFW_V2
    //glfwMakeContextCurrent();

    glfwSetKeyCallback(key_callback);
#endif
    
#if defined STAGE_LIGHTS_LINUX
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error %s\n", glewGetErrorString(err));
        exit(1);
    }
#endif
    
    gles2_harness_init(argc > 1 ? argv[1] : NULL);

#ifdef GLFW_V3
    while (!glfwWindowShouldClose(window))
#endif
#ifdef GLFW_V2
	while (glfwGetWindowParam(GLFW_OPENED))
#endif
    {
        int width = 0, height = 0;
#ifdef STAGE_LIGHTS_LINUX
        struct timespec ts;
#endif
#ifdef STAGE_LIGHTS_OSX
        uint64_t time;
#endif
        uint64_t time_ns;

#ifdef GLFW_V3
        glfwGetFramebufferSize(window, &width, &height);
#endif
#ifdef GLFW_V2
        glfwGetWindowSize(&width, &height);
#endif

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

#ifdef GLFW_V3
        glfwSwapBuffers(window);
#endif
#ifdef GLFW_V2
        glfwSwapBuffers();
#endif
        glfwPollEvents();
    }
    
    gles2_harness_terminate();

#ifdef GLFW_V3
    glfwDestroyWindow(window);
#endif
#ifdef GLFW_V2
    glfwCloseWindow();
#endif

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


static char const * light_frag =
    //"precision lowp float;\n"
    "\n"
    "uniform vec4 u_color;\n"
    "\n"
    "varying vec4 v_texCoord;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "	 gl_FragColor = vec4(u_color.xyz, 1);\n"
    "}\n";

static char const * light_vert =
    "uniform mat4 u_viewProjectionMatrix;\n"
    "uniform mat4 u_modelMatrix;\n"
    "\n"
    "attribute vec4 a_vertex;\n"
    "\n"
    "varying vec4 v_color;\n"
    "varying vec4 v_texCoord;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "	gl_Position = u_viewProjectionMatrix * u_modelMatrix * a_vertex;\n"
    "	v_texCoord = a_vertex;\n"
    "}\n";


static void show_info_log(
    GLuint object,
    void (* glGet__iv)(GLuint, GLenum, GLsizei *),
    void (* glGet__InfoLog)(GLuint, GLsizei, GLsizei *, GLchar *)
)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

bool gles2_harness_init(char const * dev)
{
    GLchar const * vert_source = light_vert;
    GLint vert_length = strlen(light_vert);
    GLchar const * frag_source = light_frag;
    GLint frag_length = strlen(light_frag);
    GLint result;
    
    g_vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(g_vertShader, 1, &vert_source, &vert_length);
    glCompileShader(g_vertShader);
    glGetShaderiv(g_vertShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        fprintf(stderr, "Failed to compile vertex shader\n");
        show_info_log(g_vertShader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(g_vertShader);
        exit(2);
    }
    
    g_fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_fragShader, 1, &frag_source, &frag_length);
    glCompileShader(g_fragShader);
    glGetShaderiv(g_fragShader, GL_COMPILE_STATUS, &result);
    if (!result) {
        fprintf(stderr, "Failed to compile fragment shader\n");
        show_info_log(g_fragShader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(g_fragShader);
        exit(2);
    }
    
    g_program = glCreateProgram();
    glAttachShader(g_program, g_vertShader);
    glAttachShader(g_program, g_fragShader);
    glLinkProgram(g_program);
    glGetProgramiv(g_program, GL_LINK_STATUS, &result);
    if (!result) {
        fprintf(stderr, "Failed to compile link shader program\n");
        show_info_log(g_program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(g_program);
        exit(2);
    }
    
    g_viewProjectionMatrixLocation = glGetUniformLocation(g_program, "u_viewProjectionMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program, "u_modelMatrix");
    g_colorLocation = glGetUniformLocation(g_program, "u_color");
    
    g_vertexLocation = glGetAttribLocation(g_program, "a_vertex");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // premultiplied alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClearDepthf(1.0f);
    
    GLfloat lightVertices[] = {
        // cube
        -1.0f,  1.0f, -1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,
        
        -1.0f, -1.0f, -1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,
        
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f,
         
        -1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,
        
        
        -1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,
        
        -1.0f, -1.0f, -1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
        
         1.0f,  1.0f, -1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, -1.0f, 1.0f,
         
         1.0f, -1.0f, -1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        
        
        -1.0f, -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f,
        
         1.0f, -1.0f, -1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f,
        
        -1.0f,  1.0f, -1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f,  1.0f, -1.0f, 1.0f,
         
        -1.0f,  1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f, 1.0f,
    };
    
    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (lightVertices), lightVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    if(dev != NULL) {
        gles2_harness_init_serial(dev);
    }
    
    slInitialize();
    
    return true;
}


int gles2_harness_serial_set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        perror("error from tcgetattr");
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
                                    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
#ifdef STAGE_LIGHTS_OSX
    tty.c_cflag &= ~CRTSCTS;
#endif

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        perror("error from tcsetattr");
        return -1;
    }
    return 0;
}

void gles2_harness_serial_set_blocking(int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        perror("error from tcgetattr");
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        perror("error setting term attributes");
    }
}


void gles2_harness_init_serial(char const * dev)
{
    gles2_harness_serial_fd = open(dev, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    
    if(gles2_harness_serial_fd < 0) {
        perror("error opening tty");
        return;
    }
    
    // set speed to 115,200 bps, 8n1 (no parity)
    gles2_harness_serial_set_interface_attribs(gles2_harness_serial_fd,
        B115200, 0);
    
    // set no blocking
    gles2_harness_serial_set_blocking(gles2_harness_serial_fd, 0);
}


void gles2_harness_process_serial(void)
{
    int n;
    
    if(gles2_harness_serial_fd < 0) {
        return;
    }
    
    if(sizeof gles2_harness_serial_buf - gles2_harness_serial_count == 0) {
        return;
    }
    
    n = read(gles2_harness_serial_fd,
        gles2_harness_serial_buf + gles2_harness_serial_count,
        sizeof gles2_harness_serial_buf - gles2_harness_serial_count);
    if(n <= 0) {
        return;
    }
    
    gles2_harness_serial_count += n;
    assert(gles2_harness_serial_count <= sizeof gles2_harness_serial_buf);
    
    if(gles2_harness_line_buf_state == GLES2_HARNESS_LINE_BUF_EMPTY) {
        size_t eol;
        
        for(eol = 0; eol < gles2_harness_serial_count; ++eol) {
            if(gles2_harness_serial_buf[eol] == '\n') {
                break;
            }
        }
        
        if(eol < gles2_harness_serial_count) {
            assert(gles2_harness_serial_buf[eol] == '\n');
            assert(gles2_harness_serial_count >= eol + 1);
            assert(eol + 1 <= sizeof gles2_harness_line_buf);
            // copy the full line into the line buf
            memcpy(gles2_harness_line_buf, gles2_harness_serial_buf, eol);
            gles2_harness_line_buf[eol] = '\0';
            // pack the read buffer down -- +1 to skip \n
            memmove(gles2_harness_serial_buf,
                gles2_harness_serial_buf + eol + 1,
                gles2_harness_serial_count - (eol + 1));
            gles2_harness_serial_count -= eol + 1;
            gles2_harness_line_buf_state = GLES2_HARNESS_LINE_BUF_FULL;
        }
        else if(eol >= (sizeof gles2_harness_line_buf - 1)) {
            gles2_harness_line_buf_state = GLES2_HARNESS_LINE_BUF_OVERRUN;
            gles2_harness_line_buf[0] = '\0';
        }
    }
    else if(gles2_harness_line_buf_state == GLES2_HARNESS_LINE_BUF_FLUSHING) {
        size_t eol;
        
        for(eol = 0; eol < gles2_harness_serial_count; ++eol) {
            if(gles2_harness_serial_buf[eol] == '\n') {
                break;
            }
        }
        if(eol < gles2_harness_serial_count) {
            // found eol, need to preserve characters after that
            assert(gles2_harness_serial_buf[eol] == '\n');
            assert(gles2_harness_serial_count >= eol + 1);
            // pack the read buffer down -- +1 to skip \n
            memmove(gles2_harness_serial_buf,
                gles2_harness_serial_buf + eol + 1,
                gles2_harness_serial_count - (eol + 1));
            gles2_harness_serial_count -= eol + 1;
            gles2_harness_line_buf_state = GLES2_HARNESS_LINE_BUF_EMPTY;
        }
        else {
            // no EOL, just clear the buffer
            gles2_harness_serial_count = 0;
        }
        
        // recurse and try to fill that line buf again
        gles2_harness_process_serial();
    }
}


int gles2_harness_read_serial(void)
{
    if(gles2_harness_serial_fd < 0) {
        return GLES2_HARNESS_LINE_NOT_READY;
    }
    
    gles2_harness_process_serial();
    
    switch(gles2_harness_line_buf_state) {
    case GLES2_HARNESS_LINE_BUF_EMPTY:
    case GLES2_HARNESS_LINE_BUF_FLUSHING:
        return GLES2_HARNESS_LINE_NOT_READY;
    case GLES2_HARNESS_LINE_BUF_FULL:
        gles2_harness_line_buf_state = GLES2_HARNESS_LINE_BUF_EMPTY;
        return GLES2_HARNESS_LINE_READY;
    default:
    case GLES2_HARNESS_LINE_BUF_OVERRUN:
        gles2_harness_line_buf_state = GLES2_HARNESS_LINE_BUF_FLUSHING;
        return GLES2_HARNESS_LINE_LOST;
    }
}


void gles2_harness_reshape(int width, int height)
{
    // Set the viewport depending on the width and height of the window.
    glViewport(0, 0, width, height);
    
    g_aspectRatio = (GLfloat)width / (GLfloat)height;
}


void gles2_harness_update(float time)
{
    int64_t frame_nsec = (int64_t)round(time * 1.0e9);
    
    slProcess(frame_nsec);
    
    gles2_harness_draw_lights(time);
}


void gles2_harness_draw_lights(float time)
{
    GLfloat viewMatrix[16];
    GLfloat viewProjectionMatrix[16];
    GLfloat modelMatrix[16];
    
    GLfloat lightSize = 0.02f;
    
    UNUSED(time);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glusLookAtf(viewMatrix,
        gles2_harness_dist * cosf(gles2_harness_horizontal_pos),
            gles2_harness_dist * -sinf(gles2_harness_horizontal_pos),
            gles2_harness_vertical_pos + 2.f,
        0.0f, 0.0f, -2.0f,
        0.0f, 0.0f, 1.0f);
    glusPerspectivef(viewProjectionMatrix, 45.0f, g_aspectRatio, 0.1f, 1000.0f);
    glusMatrix4x4Multiplyf(viewProjectionMatrix, viewProjectionMatrix, viewMatrix);
    
    
    glUseProgram(g_program);
    
    
    glUniformMatrix4fv(g_viewProjectionMatrixLocation, 1, GL_FALSE, viewProjectionMatrix);
    
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    for(size_t i = 0; i < STAGE_LIGHTS_NUM_LIGHTS; ++i) {
    	float x = ((float)i - (float)(STAGE_LIGHTS_NUM_LIGHTS - 1) / 2.0f) * lightSize * 2;
    	
        /////////
        glusMatrix4x4Identityf(modelMatrix);
        glusMatrix4x4Translatef(modelMatrix, x, 0, 0);
        glusMatrix4x4Scalef(modelMatrix, lightSize, lightSize, lightSize);
        glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);

        glUniform4f(g_colorLocation,
            slLights[i].r * (1.0f / 255.0f),
            slLights[i].g * (1.0f / 255.0f),
            slLights[i].b * (1.0f / 255.0f),
            0.0f);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        ////////
    }
    
    glDisableVertexAttribArray(g_vertexLocation);
    
    glUseProgram(0);
}


void gles2_harness_terminate(void)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDeleteBuffers(1, &g_verticesVBO);
    
    glUseProgram(0);
    
    glDeleteProgram(g_program);
    glDeleteShader(g_vertShader);
    glDeleteShader(g_fragShader);
}


void utilLogOutput(char const * format, va_list args)
{
    vprintf(format, args);
}


#endif // STAGE_LIGHTS_USE_GLES2_HARNESS


