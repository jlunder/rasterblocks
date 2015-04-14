#ifndef GLES2_HARNESS_H_INCLUDED
#define GLES2_HARNESS_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>


bool gles2_harness_init(int argc, char * argv[]);
int gles2_harness_serial_set_interface_attribs(int fd, int speed, int parity);
void gles2_harness_serial_set_blocking(int fd, int should_block);
void gles2_harness_init_serial(char const * dev);
void gles2_harness_process_serial(void);
int gles2_harness_read_serial(void);
void gles2_harness_reshape(int width, int height);
void gles2_harness_update(void);
void gles2_harness_draw_lights(void);
void gles2_harness_terminate(void);


#endif

