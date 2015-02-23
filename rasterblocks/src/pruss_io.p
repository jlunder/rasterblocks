// *
// * PRU_memAccessPRUDataRam.p
// *
// * Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
// *
// *
// *  Redistribution and use in source and binary forms, with or without
// *  modification, are permitted provided that the following conditions
// *  are met:
// *
// *    Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// *
// *    Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the
// *    distribution.
// *
// *    Neither the name of Texas Instruments Incorporated nor the names of
// *    its contributors may be used to endorse or promote products derived
// *    from this software without specific prior written permission.
// *
// *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// *
// *

// *
// * ============================================================================
// * Copyright (c) Texas Instruments Inc 2010-12
// *
// * Use of this software is controlled by the terms and conditions found in the
// * license agreement under which this software has been supplied or provided.
// * ============================================================================
// *

// *****************************************************************************/
// file:   PRU_memAccessPRUDataRam.p
//
// brief:  PRU access of internal Data Ram.
//
//
//  (C) Copyright 2012, Texas Instruments, Inc
//
//  author     M. Watkins
//
//  version    0.1     Created
// *****************************************************************************/

.origin 0
.entrypoint MEMACCESSPRUDATARAM

.struct main_vars_t
    .u32 repeat_count
    
    .u32 colors_count
    .u32 bits_count
    
    .u32 p_set_out
    .u32 p_clear_out
    .u32 p_colors
    .u32 color
    .u32 data_bit
    .u32 clock_bit
    .u32 sync_bit
.ends

#include "pruss_io.hp"

MEMACCESSPRUDATARAM:

#ifdef AM33XX

    // Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
    MOV       r0, 0x00000000
    MOV       r1, CTBIR_0
    ST32      r0, r1

#endif

/*
    //Load 32 bit value in r1
    MOV       r1, 0x0010f012

    //Load address of PRU data memory in r2
    MOV       r2, 0x0004

    // Move value from register to the PRU local data memory using registers
    ST32      r1,r2

    // Load 32 bit value into r3
    MOV       r3, 0x0000567A

    LBCO      r4, CONST_PRUDRAM, 4, 4 //Load 4 bytes from memory location c3(PRU0/1 Local Data)+4 into r4 using constant table

    // Add r3 and r4
    ADD       r3, r3, r4

    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
    SBCO      r3, CONST_PRUDRAM, 8, 4
*/


#define GPIO0 0x44E07000
#define GPIO1 0x4804C000
#define GPIO2 0x481AC000
#define GPIO_OE 0x134
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194


    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
    MOV     r0, 0x00000120
    MOV       r1, CTPPR_0
    ST32      r0, r1

    // Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0]
    // field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
    MOV     r0, 0x00100000
    MOV       r1, CTPPR_1
    ST32      r0, r1

.enter main_scope
.assign main_vars_t, r1, *, main
    mov     main.p_set_out, GPIO2 | GPIO_SETDATAOUT
    mov     main.p_clear_out, GPIO2 | GPIO_CLEARDATAOUT
    mov     main.data_bit, 1 << 6
    mov     main.clock_bit, 1 << 7
    mov     main.sync_bit, 1 << 8
    
    mov     main.repeat_count, 10000
    
repeat_loop:
    mov     main.colors_count, 1536
    mov     main.p_colors, 0x00000000 + 4
    
colors_loop:
    lbbo    main.color, main.p_colors, 0, 4
    add     main.p_colors, main.p_colors, 4
    
    mov     main.bits_count, 24
    sbbo    main.sync_bit, main.p_set_out, 0, 4
    sbbo    main.sync_bit, main.p_clear_out, 0, 4
    
bits_loop:
    sbbo    main.clock_bit, main.p_clear_out, 0, 4
    
    qbbs    bits_1, main.color, 0
    sbbo    main.data_bit, main.p_clear_out, 0, 4
    jmp     bits_0
bits_1:
    sbbo    main.data_bit, main.p_set_out, 0, 4
    mov     main.color, main.color
bits_0:

    lsr     main.color, main.color, 1
    
    sbbo    main.clock_bit, main.p_set_out, 0, 4
    
    sub     main.bits_count, main.bits_count, 1
    qbne    bits_loop, main.bits_count, 0
    
    sub     main.colors_count, main.colors_count, 1
    qbne    colors_loop, main.colors_count, 0
    
    sub     main.repeat_count, main.repeat_count, 1
    qbne    repeat_loop, main.repeat_count, 0
    
.leave main_scope

/* 100ns
    MOV r0, 100000000
DEL1:
    MOV r2, 0xFF<<6
    MOV r3, GPIO2 | GPIO_SETDATAOUT
    SBBO r2, r3, 0, 4
    
    mov r2, r2
    mov r2, r2
    mov r2, r2
    //////////
    mov r2, r2
    mov r2, r2
    //////////
    
    MOV r2, 0xFF<<6
    MOV r3, GPIO2 | GPIO_CLEARDATAOUT
    SBBO r2, r3, 0, 4
    
    mov r2, r2
    //////////
    mov r2, r2
    mov r2, r2
    //////////

    SUB r0, r0, 1
    QBNE DEL1, r0, 0
*/

#ifdef AM33XX

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16

#else

    MOV R31.b0, PRU0_ARM_INTERRUPT

#endif

    HALT
