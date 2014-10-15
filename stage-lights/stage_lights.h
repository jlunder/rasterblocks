#ifndef STAGE_LIGHTS_H_INCLUDED
#define STAGE_LIGHTS_H_INCLUDED


#include <stdint.h>
#include <stdlib.h>


typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t x;
} Color;


#define STAGE_LIGHTS_NUM_LIGHTS 10


Color slLights[STAGE_LIGHTS_NUM_LIGHTS];


void slInitialize(void);
void slShutdown(void);

void slProcess(uint64_t nsSinceLastProcess);


#endif // STAGE_LIGHTS_H_INCLUDED

