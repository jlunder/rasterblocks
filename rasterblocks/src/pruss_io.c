/*
 * PRU_memAccessPRUDataRam.c
 *
 * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/*
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-12
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

/*****************************************************************************
* PRU_memAccessPRUDataRam.c
*
* The PRU reads and stores values into the PRU Data RAM memory. PRU Data RAM
* memory has an address in both the local data memory map and global memory
* map. The example accesses the local Data RAM using both the local address
* through a register pointed base address and the global address pointed by
* entries in the constant table.
*
******************************************************************************/


/*****************************************************************************
* Include Files                                                              *
*****************************************************************************/


#ifdef RB_USE_PRUSS_IO


#include "pruss_io.h"

#include "graphics_util.h"

#include <prussdrv.h>
#include <pruss_intc_mapping.h>


#define PRU_NUM 	0
#define ADDEND1		0x0010F012u
#define ADDEND2		0x0000567Au


#define AM33XX


#define RB_PRUSS_IO_DEVICE_STARTUP_COMMAND \
    "sh -c \"modprobe uio_pruss && " \
    "(grep -q rb-pruss-io /sys/devices/bone_capemgr.9/slots || " \
    "echo rb-pruss-io > /sys/devices/bone_capemgr.9/slots)\""


#define GLOBAL_STATUS_RUN    0
#define GLOBAL_STATUS_HALT   1

#define FRAME_STATUS_MASK    0x03
#define FRAME_MODE_MASK      0x0C
#define FRAME_STATUS_IDLE    0x00
#define FRAME_STATUS_ERROR   0x01
#define FRAME_STATUS_READY   0x02
#define FRAME_MODE_2W_2MHZ   0x00
#define FRAME_MODE_2W_10MHZ  0x04
#define FRAME_MODE_1W_800KHZ 0x08
#define FRAME_MODE_PAUSE     0x10

#define MIDI_STATUS_EMPTY 0
#define MIDI_STATUS_FULL  1


typedef struct {
    uint32_t status;
    uint32_t address;
    uint32_t size;
    uint32_t capacity;
} RBPrussIoBufferRegs;

typedef struct
{
    uint32_t status;
    RBPrussIoBufferRegs frame[2];
    RBPrussIoBufferRegs midi[2];
} RBPrussIoMemoryMap;


#define rbMemoryBarrier() asm volatile("": : :"memory")

static RBPrussIoMemoryMap * rbPrussIoDataRam;
static uint8_t * rbPrussIoSharedRam;


#include "pruss_io_pru0_bin.h"
#include "pruss_io_pru1_bin.h"


/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

void rbPrussIoInitialize(RBConfiguration * pConfig)
{
    UNUSED(pConfig);
}


void rbPrussIoShutdown(void)
{
}


void rbLightOutputPrussInitialize(RBConfiguration const * pConfig)
{
    int ret;
    
    ret = system(RB_PRUSS_IO_DEVICE_STARTUP_COMMAND);
    if(ret != 0) {
        rbWarning("Failed to start up PRUSS\n");
    }

    tpruss_intc_initdata prussIntCInitData = PRUSS_INTC_INITDATA;

    UNUSED(pConfig);
    
    rbInfo("Opening PRUSS driver\n");
    prussdrv_init();

    ret = prussdrv_open(PRU_EVTOUT_0);
    if(ret != 0)
    {
        // Retry -- sometimes on first open after reboot the driver takes a
        // while to come up
        rbSleep(rbTimeFromMs(5000));
        ret = system(RB_PRUSS_IO_DEVICE_STARTUP_COMMAND);
        ret = ret || prussdrv_open(PRU_EVTOUT_0);
        if(ret != 0) {
            rbFatal("prussdrv_open open failed\n");
        }
    }

    rbInfo("Initializing PRUSS memory mapping and data\n");
    prussdrv_pruintc_init(&prussIntCInitData);
    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void * *)&rbPrussIoDataRam);
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void * *)&rbPrussIoSharedRam);
    rbPrussIoDataRam->status = GLOBAL_STATUS_RUN;
    rbPrussIoDataRam->frame[0].status = FRAME_STATUS_IDLE;
    rbPrussIoDataRam->frame[0].address = 0x00010000;
    rbPrussIoDataRam->frame[0].size = 0;
    rbPrussIoDataRam->frame[0].capacity = 1024 * 6;
    rbPrussIoDataRam->frame[1].status = FRAME_STATUS_IDLE;
    rbPrussIoDataRam->frame[1].address = 0x00010000 + 1024 * 6;
    rbPrussIoDataRam->frame[1].size = 0;
    rbPrussIoDataRam->frame[1].capacity = 1024 * 6;
    rbZero(rbPrussIoSharedRam, 1024 * 12);
    rbMemoryBarrier();
    
    rbInfo("Executing PRU code\n");
    prussdrv_exec_code (0, rbPrussIoPru0Code, sizeof rbPrussIoPru0Code);
}


void rbLightOutputPrussShutdown(void)
{
    rbPrussIoDataRam->status = GLOBAL_STATUS_HALT;
    rbMemoryBarrier();
    
    // Wait until PRU0 has finished execution
    rbInfo("Waiting for PRU HALT\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
    
    rbInfo("Closing PRUSS driver\n");
    prussdrv_pru_disable(0);
    prussdrv_exit();
}


void rbLightOutputPrussShowLights(RBRawLightFrame const * pFrame)
{
    size_t buf;
    size_t i;
    
    rbMemoryBarrier();
    for(size_t i = 0; i < LENGTHOF(rbPrussIoDataRam->frame); ++i) {
        if(rbPrussIoDataRam->frame[i].status == FRAME_STATUS_ERROR) {
            rbWarning("PRUSS reports frame error!\n");
            rbPrussIoDataRam->frame[i].status = FRAME_STATUS_IDLE;
        }
    }
    
    for(buf = 0; buf < LENGTHOF(rbPrussIoDataRam->frame); ++buf) {
        if(rbPrussIoDataRam->frame[buf].status == FRAME_STATUS_IDLE) {
            break;
        }
    }
    
    if(buf >= LENGTHOF(rbPrussIoDataRam->frame)) {
        rbWarning("PRUSS frame queue full!\n");
        return;
    }
    
    // Fill output buffer in PRUSS RAM first...
    rbAssert(RB_NUM_LIGHTS * 3 < rbPrussIoDataRam->frame[buf].capacity);
    rbPrussIoDataRam->frame[buf].size = RB_NUM_LIGHTS * 3;
    i = rbPrussIoDataRam->frame[buf].address - 0x00010000;
    for(size_t l = 0; l < RB_NUM_PANELS; ++l) {
        for(size_t k = 0; k < RB_PANEL_HEIGHT; k += 2) {
            for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            /*
                // Color order for WS2801
                rbPrussIoSharedRam[i + 0] = pFrame->data[l][k][j].b;
                rbPrussIoSharedRam[i + 1] = pFrame->data[l][k][j].r;
                rbPrussIoSharedRam[i + 2] = pFrame->data[l][k][j].g;
                */
                // Color order for WS2812
                rbPrussIoSharedRam[i + 0] = pFrame->data[l][k][j].g;
                rbPrussIoSharedRam[i + 1] = pFrame->data[l][k][j].r;
                rbPrussIoSharedRam[i + 2] = pFrame->data[l][k][j].b;
                i += 3;
            }
            for(size_t j = RB_PANEL_WIDTH; j > 0; --j) {
            /*
                // Color order for WS2801
                rbPrussIoSharedRam[i + 0] = pFrame->data[l][k][j].b;
                rbPrussIoSharedRam[i + 1] = pFrame->data[l][k][j].r;
                rbPrussIoSharedRam[i + 2] = pFrame->data[l][k][j].g;
                */
                // Color order for WS2812
                rbPrussIoSharedRam[i + 0] = pFrame->data[l][k + 1][j - 1].g;
                rbPrussIoSharedRam[i + 1] = pFrame->data[l][k + 1][j - 1].r;
                rbPrussIoSharedRam[i + 2] = pFrame->data[l][k + 1][j - 1].b;
                i += 3;
            }
        }
    }
    rbAssert(i <= 1024 * 12);
    // Ensure all data is committed to memory before kicking off the PRUSS
    rbMemoryBarrier();
    // Do a full memory barrier just to be safe... pretty sure this is not
    // actually needed, but hey why not.
    __sync_synchronize();
    rbPrussIoDataRam->frame[buf].status =
        FRAME_STATUS_READY | FRAME_MODE_1W_800KHZ | FRAME_MODE_PAUSE;
//        FRAME_STATUS_READY | FRAME_MODE_2W_10MHZ;
    rbMemoryBarrier();
}


#endif
