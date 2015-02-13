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


#include "rasterblocks.h"

#include <prussdrv.h>
#include <pruss_intc_mapping.h>


#define PRU_NUM 	0
#define ADDEND1		0x0010F012u
#define ADDEND2		0x0000567Au


#define AM33XX


#define RB_TARGET_PRUSS_DEVICE_STARTUP_COMMAND \
    "echo BB-BONE-PRU-01 > /sys/devices/bone_capemgr.9/slots"


static int LOCAL_exampleInit ( unsigned short pruNum );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/*****************************************************************************
* Local Variable Definitions                                                 *
*****************************************************************************/


/*****************************************************************************
* Intertupt Service Routines                                                 *
*****************************************************************************/


/*****************************************************************************
* Global Variable Definitions                                                *
*****************************************************************************/

static void *pruDataMem;
static unsigned int *pruDataMem_int;

#include "pruss_io_bin.h"

/*****************************************************************************
* Global Function Definitions                                                *
*****************************************************************************/

void rbPrussIoInitialize(RBConfiguration * pConfig)
{
    int ret;
    ret = system(RB_TARGET_PRUSS_DEVICE_STARTUP_COMMAND);
    if(ret != 0) {
        rbWarning("Failed to start up PRUSS\n");
    }

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    UNUSED(pConfig);
    
    rbInfo("Starting %s example.\n", "PRU_memAccessPRUDataRam");
    /* Initialize the PRU */
    prussdrv_init ();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        rbFatal("prussdrv_open open failed\n");
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    rbInfo("Initializing example.\n");
    LOCAL_exampleInit(PRU_NUM);

    /* Execute example on PRU */
    rbInfo("Executing example.\n");
    prussdrv_exec_code (PRU_NUM, rbPrussIoCode, sizeof rbPrussIoCode);


    /* Wait until PRU0 has finished execution */
    rbInfo("Waiting for HALT command.\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    rbInfo("PRU completed transfer.\n");
    prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

    /* Check if example passed */
    if ( LOCAL_examplePassed(PRU_NUM) )
    {
        rbInfo("Example executed succesfully.\r\n");
    }
    else
    {
        rbInfo("Example failed.\r\n");
    }
}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit ( unsigned short pruNum )
{
    //Initialize pointer to PRU data memory
    if (pruNum == 0)
    {
      prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);
    }
    else if (pruNum == 1)
    {
      prussdrv_map_prumem (PRUSS0_PRU1_DATARAM, &pruDataMem);
    }
    pruDataMem_int = (unsigned int*) pruDataMem;

    // Flush the values in the PRU data memory locations
    pruDataMem_int[1] = 0x00;
    pruDataMem_int[2] = 0x00;

    return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
  UNUSED(pruNum);
  return ((pruDataMem_int[1] ==  ADDEND1) & (pruDataMem_int[2] ==  ADDEND1 + ADDEND2));
}


void rbPrussIoShutdown(void)
{
    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable (PRU_NUM);
    prussdrv_exit ();
}


#endif
