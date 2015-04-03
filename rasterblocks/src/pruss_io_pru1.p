.setcallreg r3.w0
.origin 0
.entrypoint 0

#include "pruss_io.hp"


#define REGS_BASE  (0x00000000)

#define CONTROL_OFS (0)
#define LIGHT_0_OFS (SIZE(pru_control) + SIZE(buffer_control) * 0)
#define LIGHT_1_OFS (SIZE(pru_control) + SIZE(buffer_control) * 1)


.struct main_vars
    .u32 mode
.ends


.struct frame_vars
    .u32 p_buf
    
    .u32 bytes_count
    .u32 bits_count
    
    .u32 p_set_out
    .u32 p_clear_out
    .u32 p_bytes
    .u32 bits_0
    .u32 bits_1
    .u32 pat0x08040201
    .u32 pat0x80402010
.ends

    // Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
    mov     r0, 0x00000000
    mov     r1, CTBIR_0
    sbbo    r0, r1, 0, 4

    // Enable OCP master port
    lbco    r0, CONST_PRUCFG, 0x4, 4
    clr     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    sbco    r0, CONST_PRUCFG, 0x4, 4

    // Reset internal pinmux to defaults
    mov     r0.b0, 0
    sbco    r0.b0, CONST_PRUCFG, 0x40, 1
    
    // Enable OCP master port
    lbco    r0, CONST_PRUCFG, 0x0, 4
    clr     r0, r0, 3         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    sbco    r0, CONST_PRUCFG, 0x0, 4
    
    /*
    mov     r0, 0
    sbco    r0, CONST_INTC, , 4
    */
    
    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
    mov     r0, 0x00000120
    mov     r1, CTPPR_0
    sbbo    r0, r1, 0, 4

    // Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0]
    // field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
    mov     r0, 0x00100000
    mov     r1, CTPPR_1
    sbbo    r0, r1, 0, 4

.enter main
.assign buffer_control, r4, r11, buf
.assign main_vars, r12, *, l
    
main_loop:
/*
    mov     r30.w0, 0
inc_loop:
    add     r30.b0, r30.b0, 1
    qbne    inc_loop, r30.b0, 0
*/
    clr     r30.t6
    set     r30.t7
    clr     r30.t7
    
    // Check if we are still in RUN mode globally
    mov     r0, REGS_BASE + CONTROL_OFS
    lbbo    l.mode, r0, OFFSET(pru_control.mode), SIZE(pru_control.mode)
    // First, acknowledge that we've read the requested mode
    sbbo    l.mode, r0, OFFSET(pru_control.status), SIZE(pru_control.status)
    // Run mode? Check the buffers to see if there's anything work available
    qbeq    main_run, l.mode, PRU_MODE_RUN
    // Pause mode? Don't do anything, just keep checking mode
    qbeq    main_loop, l.mode, PRU_MODE_PAUSE
    
    // Fell through; nope, halt!
    // Send notification to host for program completion
    mov     r31.b0, PRU0_ARM_INTERRUPT+16
    halt
    
main_run:
    // Check if FRAME0 has any data ready
    mov     r0, REGS_BASE + LIGHT_0_OFS
    lbbo    buf.owner, r0, OFFSET(buf.owner), SIZE(buf.owner)
    qbeq    output_frame, buf.owner, OWNER_PRU1
    
    // Check if FRAME1 has any data ready
    mov     r0, REGS_BASE + LIGHT_1_OFS
    lbbo    buf.owner, r0, OFFSET(buf.owner), SIZE(buf.owner)
    qbeq    output_frame, buf.owner, OWNER_PRU1
    
    // Nada, return to main loop
    jmp     main_loop
    
    
output_frame:
    // A frame is ready!
    
    // Previously we only loaded ownership data, fetch the whole control struct
    lbbo    buf, r0, 0, SIZE(buf)
    
    // During processing, our status is "nominal". This also makes our default
    // status success.
    mov     buf.status, STATUS_BUSY
    sbbo    buf.status, r0, OFFSET(buf.status), SIZE(buf.status)
    
    // Select the output method and go
    and     r1, buf.command, COMMAND_LIGHT_MODE_MASK
    qbeq    output_frame_2w_2mhz, r1, COMMAND_LIGHT_MODE_2W_2MHZ
    qbeq    output_frame_2w_10mhz, r1, COMMAND_LIGHT_MODE_2W_10MHZ
    qbeq    output_frame_1w_800khz, r1, COMMAND_LIGHT_MODE_1W_800KHZ
    
    // Couldn't find valid mode? Report back error status
    mov     buf.status, STATUS_ERROR_COMMAND
    sbbo    buf.status, r0, OFFSET(buf.status), SIZE(buf.status)
    mov     buf.owner, OWNER_HOST
    sbbo    buf.owner, r0, OFFSET(buf.owner), SIZE(buf.owner)
    jmp     main_loop
    
.leave main


.enter frame
.assign buffer_control, r4, r11, buf
.assign frame_vars, r12, *, l

output_frame_2w_2mhz:
output_frame_2w_10mhz:
output_frame_1w_800khz:
    set     r30.t6
    
    mov     l.p_buf, r0
    
    mov     l.p_set_out, GPIO2 | GPIO_SETDATAOUT
    mov     l.p_clear_out, GPIO2 | GPIO_CLEARDATAOUT
    
    // Divide "size" by 8: we output groups of 8 bytes at a time
    lsr     l.bytes_count, buf.size, 3
    mov     l.p_bytes, buf.address
    
    mov     l.pat0x08040201, 0x08040201
    mov     l.pat0x80402010, 0x80402010
    
of800k_words_loop:
    set     r30.t5
    clr     r30.t5
    
    // Fetch the next 2 words: fills bits_0 AND bits_1!
    // Data is interleaved: this is 1 byte for each of 8 outputs.
    lbbo    l.bits_0, l.p_bytes, 0, 8
    add     l.p_bytes, l.p_bytes, 8
    
    mov     l.bits_count, 8
    
of800k_bits_loop:
    mov     r30.w0, 0x004B//0x01FB
    
    // This next stanza repacks the 7th bits of each byte in l.bits_0,1 into
    // r0.w0, with each 7th bit assigned to one of the bit positions of the
    // r30 GPOs, so we can set the GPOs all at once.
    
    // Shift the 7th bits of l.bits_0.b0-3 into positions 0-3, respectively
    lsr     r0.b0, l.bits_0.b0, 7
    lsr     r0.b1, l.bits_0.b1, 6
    lsr     r0.b2, l.bits_0.b2, 5
    lsr     r0.b3, l.bits_0.b3, 4
    // Clear the other bits, preparing for the merge
    and     r0, r0, l.pat0x08040201
    
    // Shift the 7th bits of l.bits_1.b0-3 into positions 4-7, respectively
    lsr     r1.b0, l.bits_0.b0, 3
    lsr     r1.b1, l.bits_0.b1, 2
    lsr     r1.b2, l.bits_0.b2, 1
    lsr     r1.b3, l.bits_0.b3, 0
    // Clear the other bits, preparing for the merge
    and     r1, r1, l.pat0x80402010
    
    // Merge -- divide and conquer
    or      r0, r0, r1
    or      r0.w0, r0.w0, r0.w1
    or      r0.b0, r0.b0, r0.b1
    
    // Clear other bits, keep just the single reordered byte
    and     r0.w0, r0.w0, 0xFF
    
    // Bit 2 is an input, so we have to split the byte and shift the upper
    // half left 1... the final packing is 0-1,3-8
    lsl     r0.w1, r0.w0, 1
    and     r0.w1.b0, r0.w1.b0, 0xF8
    and     r0.w0.b0, r0.w0.b0, 0x03
    or      r0.w0, r0.w0, r0.w1
    
    or      r1.w0, r0.w0, 0x40

    mov     r0, 27
    call    delay
    

    // Aaaaaand output!
    mov     r30.w0, r1.w0
    
    mov     r0, 38
    call    delay
    
    mov     r30.w0, 0x0040
    
    // We are shifting the bits out MSB-first, shift left
    lsl     l.bits_0, l.bits_0, 1
    lsl     l.bits_1, l.bits_1, 1
    
    mov     r0, 32
    call    delay
    
    // Loop boilerplate for inner loop (shifting out bits of the word)
    sub     l.bits_count, l.bits_count, 1
    qbne    of800k_bits_loop, l.bits_count, 0
    
    // Loop boilerplate for outer loop (fetching groups of 8 bytes)
    sub     l.bytes_count, l.bytes_count, 1
    qbne    of800k_words_loop, l.bytes_count, 0
    
    jmp     of_check_pause

of_check_pause:
    // Check if we are supposed to pause at end-of-frame
    mov     r1, COMMAND_LIGHT_END_FRAME_PAUSE
    and     r0, buf.status, r1
    qbeq    no_pause, r0, 0
    
    // 200,000 insns = 1ms
    mov     r0, 100000
    call    delay
    
no_pause:
    // Frame output normally
    mov     buf.status, STATUS_NOMINAL
    sbbo    buf.status, l.p_buf, OFFSET(buf.status), SIZE(buf.status)
    
    // Indicate that the frame is complete!
    mov     buf.owner, OWNER_HOST
    sbbo    buf.owner, l.p_buf, OFFSET(buf.owner), SIZE(buf.owner)
    
    jmp     main_loop
    
.leave frame


delay:
    sub     r0, r0, 1
    qbne    delay, r0, 0
    ret

