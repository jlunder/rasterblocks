#ifndef PRUSS_IO_H_INCLUDED
#define PRUSS_IO_H_INCLUDED


#include "rasterblocks.h"


void rbPrussIoInitialize(RBConfiguration * pConfig);
void rbPrussIoShutdown(void);
void rbPrussIoReadInput(void);


#endif // PRUSS_IO_H_INCLUDED
