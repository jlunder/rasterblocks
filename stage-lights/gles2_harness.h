#ifndef GLES2_HARNESS_H_INCLUDED
#define GLES2_HARNESS_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>


void gles2_harness_main(int argc, char * argv[]);

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


#endif

