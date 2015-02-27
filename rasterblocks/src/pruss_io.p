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
.entrypoint 0

#include "pruss_io.hp"


#define GPIO0 0x44E07000
#define GPIO1 0x4804C000
#define GPIO2 0x481AC000
#define GPIO_OE 0x134
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194


.struct main_vars
    .u32 status
.ends


.struct frame_vars
    .u32 p_buf
    
    .u32 bytes_count
    .u32 bits_count
    
    .u32 p_set_out
    .u32 p_clear_out
    .u32 p_bytes
    .u32 bits
    .u32 data_bit
    .u32 clock_bit
    .u32 sync_bit
.ends


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


.struct global_control
    .u32 status
.ends

.struct buffer_control
    .u32 status
    .u32 address
    .u32 size
    .u32 capacity
.ends


#define REGS_BASE  (0)
#define GLOBAL_OFS (0)
#define FRAME0_OFS (SIZE(global_control) + SIZE(buffer_control) * 0)
#define FRAME1_OFS (SIZE(global_control) + SIZE(buffer_control) * 1)
#define MIDI0_OFS  (SIZE(global_control) + SIZE(buffer_control) * 2)
#define MIDI1_OFS  (SIZE(global_control) + SIZE(buffer_control) * 3)


#ifdef AM33XX

    // Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
    mov     r0, 0x00000000
    mov     r1, CTBIR_0
    ST32    r0, r1

#endif

    // Enable OCP master port
    lbco    r0, CONST_PRUCFG, 4, 4
    clr     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    sbco    r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
    mov     r0, 0x00000120
    mov     r1, CTPPR_0
    ST32    r0, r1

    // Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0]
    // field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
    mov     r0, 0x00100000
    mov     r1, CTPPR_1
    ST32    r0, r1
    
.enter main
.assign buffer_control, r4, r7, buf
.assign main_vars, r8, *, l

main_loop:
    // Check if FRAME0 has any data ready
    mov     r0, REGS_BASE + FRAME0_OFS
    lbbo    buf, r0, 0, SIZE(buf)
    and     r1, buf.status, FRAME_STATUS_MASK
    qbeq    output_frame, r1, FRAME_STATUS_READY
    
    // Check if FRAME1 has any data ready
    mov     r0, REGS_BASE + FRAME1_OFS
    lbbo    buf, r0, 0, SIZE(buf)
    and     r1, buf.status, FRAME_STATUS_MASK
    qbeq    output_frame, r1, FRAME_STATUS_READY
    
    // Check if we are still in RUN mode globally
    mov     r0, REGS_BASE + GLOBAL_OFS
    lbbo    l.status, r0, OFFSET(global_control.status), SIZE(global_control.status)
    qbeq    main_loop, l.status, GLOBAL_STATUS_RUN
    
    // Fell through; nope, halt!
    // Send notification to host for program completion
    mov     r31.b0, PRU0_ARM_INTERRUPT+16
    halt
    
output_frame:
    // A frame is ready! Select the output method and go
    and     r1, buf.status, FRAME_MODE_MASK
    qbeq    output_frame_2w_2mhz, r1, FRAME_MODE_2W_2MHZ
    qbeq    output_frame_2w_10mhz, r1, FRAME_MODE_2W_10MHZ
    qbeq    output_frame_1w_800khz, r1, FRAME_MODE_1W_800KHZ
    
    // Couldn't find valid mode? Report back error status
    mov     r1, FRAME_STATUS_ERROR
    sbbo    r1, r0, OFFSET(buf.status), SIZE(buf.status)
    jmp     main_loop
    
.leave main


.enter frame
.assign buffer_control, r4, r7, buf
.assign frame_vars, r8, *, l

output_frame_2w_2mhz:
    mov     l.p_buf, r0
    
    mov     l.p_set_out, GPIO2 | GPIO_SETDATAOUT
    mov     l.p_clear_out, GPIO2 | GPIO_CLEARDATAOUT
    mov     l.data_bit, 1 << 6
    mov     l.clock_bit, 1 << 7
    mov     l.sync_bit, 1 << 8
    
    // Generate a sync pulse for syncing the oscilloscope
    sbbo    l.sync_bit, l.p_set_out, 0, 4
    sbbo    l.sync_bit, l.p_clear_out, 0, 4
    
    mov     l.bytes_count, buf.size
    mov     l.p_bytes, buf.address
    
of2m_words_loop:
    // Fetch the next word -- this might overrun the buffer, should be okay
    lbbo    r0, l.p_bytes, 0, 4
    add     l.p_bytes, l.p_bytes, 4
    // Reverse byte order: data is shifted out most significant bit first, but
    // least significant byte first...
    mov     l.bits.b0, r0.b3
    mov     l.bits.b1, r0.b2
    mov     l.bits.b2, r0.b1
    mov     l.bits.b3, r0.b0
    
    qble    of2m_full_word, l.bytes_count, 4
    
    // Fall through: this is a partial word (1-3 bytes).
    lsl     l.bits_count, l.bytes_count, 3
    jmp     of2m_partial_word
    
of2m_full_word:
    mov     l.bits_count, 32

of2m_partial_word:
    
of2m_bits_loop:
    // Clock goes low
    sbbo    l.clock_bit, l.p_clear_out, 0, 4
    
    // Test MSB of our data -- #31
    qbbs    of2m_bits_1, l.bits, 31
    // Fall through: bit is clear
    sbbo    l.data_bit, l.p_clear_out, 0, 4
    jmp     of2m_bits_0
of2m_bits_1:
    // Bit is set
    sbbo    l.data_bit, l.p_set_out, 0, 4
    mov     l.bits, l.bits
of2m_bits_0:
    
    // We are shifting the bits out MSB-first, shift left
    lsl     l.bits, l.bits, 1
    
    // Delay to reduce the clock rate to 2MHz
    mov     r0, 21
of2m_delay_0_loop:
    sub     r0, r0, 1
    qbne    of2m_delay_0_loop, r0, 0
    
    // Output should be stable now, clock high
    sbbo    l.clock_bit, l.p_set_out, 0, 4
    
    // Delay to reduce the clock rate to 2MHz (keep the duty cycle even!)
    mov     r0, 22
of2m_delay_1_loop:
    sub     r0, r0, 1
    qbne    of2m_delay_1_loop, r0, 0
    
    // Loop boilerplate for inner loop (shifting out bits of the word)
    sub     l.bits_count, l.bits_count, 1
    qbne    of2m_bits_loop, l.bits_count, 0
    
    // Loop boilerplate for outer loop (bytes)
    // Return to main if bytes < 4!
    qbge    of2m_words_loop_done, l.bytes_count, 4
    // Subtract after to avoid underflow
    sub     l.bytes_count, l.bytes_count, 4
    // Else, continue looping...
    jmp     of2m_words_loop

of2m_words_loop_done:
    jmp of_check_pause




output_frame_2w_10mhz:
    mov     l.p_buf, r0
    
    mov     l.p_set_out, GPIO2 | GPIO_SETDATAOUT
    mov     l.p_clear_out, GPIO2 | GPIO_CLEARDATAOUT
    mov     l.data_bit, 1 << 6
    mov     l.clock_bit, 1 << 7
    mov     l.sync_bit, 1 << 8
    
    // Generate a sync pulse for syncing the oscilloscope
    sbbo    l.sync_bit, l.p_set_out, 0, 4
    sbbo    l.sync_bit, l.p_clear_out, 0, 4
    
    mov     l.bytes_count, buf.size
    mov     l.p_bytes, buf.address
    
of10m_words_loop:
    // Fetch the next word -- this might overrun the buffer, should be okay
    lbbo    r0, l.p_bytes, 0, 4
    add     l.p_bytes, l.p_bytes, 4
    // Reverse byte order: data is shifted out most significant bit first, but
    // least significant byte first...
    mov     l.bits.b0, r0.b3
    mov     l.bits.b1, r0.b2
    mov     l.bits.b2, r0.b1
    mov     l.bits.b3, r0.b0
    
    qble    of10m_full_word, l.bytes_count, 4
    
    // Fall through: this is a partial word (1-3 bytes).
    lsl     l.bits_count, l.bytes_count, 3
    jmp     of10m_partial_word
    
of10m_full_word:
    mov     l.bits_count, 32

of10m_partial_word:
    
of10m_bits_loop:
    // Clock goes low
    sbbo    l.clock_bit, l.p_clear_out, 0, 4
    
    // Test MSB of our data -- #31
    qbbs    of10m_bits_1, l.bits, 31
    // Fall through: bit is clear
    sbbo    l.data_bit, l.p_clear_out, 0, 4
    jmp     of10m_bits_0
of10m_bits_1:
    // Bit is set
    sbbo    l.data_bit, l.p_set_out, 0, 4
    mov     l.bits, l.bits
of10m_bits_0:
    
    // We are shifting the bits out MSB-first, shift left
    lsl     l.bits, l.bits, 1
    
    // Output should be stable now, clock high
    sbbo    l.clock_bit, l.p_set_out, 0, 4
    
    // Loop boilerplate for inner loop (shifting out bits of the word)
    sub     l.bits_count, l.bits_count, 1
    qbne    of10m_bits_loop, l.bits_count, 0
    
    // Loop boilerplate for outer loop (bytes)
    // Return to main if bytes <= 4!
    qbgt    of10m_words_loop_done, l.bytes_count, 4
    // Subtract after to avoid underflow
    sub     l.bytes_count, l.bytes_count, 4
    // Else, continue looping...
    jmp     of10m_words_loop

of10m_words_loop_done:
    jmp of_check_pause



output_frame_1w_800khz:
of_check_pause:
    // Check if we are supposed to pause at end-of-frame
    and     r0, buf.status, FRAME_MODE_PAUSE
    qbeq    no_pause, r0, 0
    
    // 200,000 insns = 1ms
    mov     r0, 100000
pause_loop:
    sub     r0, r0, 1
    qbne    pause_loop, r0, 0
    
no_pause:
    
    // Indicate that the frame is complete!
    mov     r1, FRAME_STATUS_IDLE
    sbbo    r1, l.p_buf, OFFSET(buf.status), SIZE(buf.status)
    
    jmp     main_loop
    
.leave frame

