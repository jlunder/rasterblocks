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

#if defined RB_USE_GLFW3_GLES2_HARNESS || defined RB_USE_SDL2_GLES2_HARNESS

#include "gles2_harness.h"


#ifdef RB_USE_GLFW3_GLES2_HARNESS
#include <GLFW/glfw3.h>
#else
#include <GLES2/gl2.h>
#endif

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

#include "graphics_util.h"


RBRawLightFrame gles2_harness_frame;


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


static char const * light_frag =
#ifdef RB_USE_GLFW3_GLES2_HARNESS
    "uniform vec4 u_color;\n"
    "varying vec4 v_texCoord;\n"
#else
    "uniform lowp vec4 u_color;\n"
    "varying mediump vec4 v_texCoord;\n"
#endif
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

bool gles2_harness_init(int argc, char * argv[])
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
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
// Funny story, modern OpenGL differs from OpenGL ES (for us) ONLY in that ES
// only has glClearDepthf() and non-ES only has glClearDepth()..!
#ifdef RB_USE_GLFW3_GLES2_HARNESS
    glClearDepth(1.0f);
#else
    glClearDepthf(1.0f);
#endif
    
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
    
    //if(dev != NULL) {
    //    gles2_harness_init_serial(dev);
    //}
    
    rbInitialize(argc, argv);
    
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
        115200, 0);
    
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
    
    rbProcess(frame_nsec);
    
    gles2_harness_draw_lights(time);
}


float gles2_harness_red_transform(float red)
{
    return powf(red, 1.0f/3.0f);
}


float gles2_harness_green_transform(float green)
{
    return powf(green, 1.0f/3.0f);
}


float gles2_harness_blue_transform(float blue)
{
    return powf(blue, 1.0f/3.0f);
}


void gles2_harness_draw_lights(float time)
{
    GLfloat viewMatrix[16];
    GLfloat viewProjectionMatrix[16];
    GLfloat modelMatrix[16];
    
    // Lights are about 1" wide, in meters, and our boxes are 2u wide
    GLfloat const lightSize = 0.050 / 2.0f; //0.025f / 2.0f;
    float const lightSpacing = 0.0254f * 3.0f; // 3 inch spacing, in meters
    float const projectionAspect =
        (float)RB_PROJECTION_WIDTH / (float)RB_PROJECTION_HEIGHT;
    float overallScale = 1.0f;
    
    RBVector2 projectionOffset = vector2(
        -lightSpacing * (RB_PROJECTION_WIDTH - 1) * 0.5f,
        -lightSpacing * (RB_PROJECTION_HEIGHT - 1) * 0.5f);
    
    GLUS_UNUSED(time);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(projectionAspect < g_aspectRatio) {
        overallScale = lightSpacing * (RB_PROJECTION_HEIGHT + 2);
    }
    else {
        overallScale =
            lightSpacing * (RB_PROJECTION_WIDTH + 2) / g_aspectRatio;
    }
    
    glusLookAtf(viewMatrix,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.0f);
    glusOrthof(viewProjectionMatrix,
        -overallScale * g_aspectRatio * 0.5f,
        overallScale * g_aspectRatio * 0.5f,
        overallScale * 0.5f, -overallScale * 0.5f, -10.0f, 10.0f);
    glusMatrix4x4Multiplyf(viewProjectionMatrix, viewProjectionMatrix,
        viewMatrix);
    
    
    glUseProgram(g_program);
    
    
    glUniformMatrix4fv(g_viewProjectionMatrixLocation, 1, GL_FALSE,
        viewProjectionMatrix);
    
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    for(size_t i = 0; i < RB_NUM_PANELS; ++i) {
        RBVector2 linePos =
            v2add(v2scale(g_rbPanelConfigs[i].position, lightSpacing),
                projectionOffset);
        RBVector2 xinc =
            v2scale(g_rbPanelConfigs[i].orientation, lightSpacing);
        RBVector2 yinc = v2cross(xinc);
        
        for(size_t j = 0; j < RB_PANEL_HEIGHT; ++j) {
            RBVector2 pos = linePos;
            
            for(size_t k = 0; k < RB_PANEL_WIDTH; ++k) {
                glusMatrix4x4Identityf(modelMatrix);
                glusMatrix4x4Translatef(modelMatrix, pos.x, pos.y, 0);
                glusMatrix4x4Scalef(modelMatrix, lightSize, lightSize,
                    lightSize);
                glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE,
                    modelMatrix);

                glUniform4f(g_colorLocation,
                    gles2_harness_red_transform(
                        gles2_harness_frame.data[i][j][k].r * (1.0f / 255.0f)),
                    gles2_harness_green_transform(
                        gles2_harness_frame.data[i][j][k].g * (1.0f / 255.0f)),
                    gles2_harness_blue_transform(
                        gles2_harness_frame.data[i][j][k].b * (1.0f / 255.0f)),
                    0.0f);

                glDrawArrays(GL_TRIANGLES, 0, 36);
                
                pos = v2add(pos, xinc);
            }
            
            linePos = v2add(linePos, yinc);
        }
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


void rbLogOutputV(char const * format, va_list args)
{
    vprintf(format, args);
}


void rbLightOutputInitialize(RBConfiguration const * config)
{
    UNUSED(config);
}


void rbLightOutputShutdown(void)
{
}


void rbLightOutputShowLights(RBRawLightFrame const * pFrame)
{
    memcpy(&gles2_harness_frame, pFrame, sizeof gles2_harness_frame);
}


#endif // RB_USE_GLES2_HARNESS


