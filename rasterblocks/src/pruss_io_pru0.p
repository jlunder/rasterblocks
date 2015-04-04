.setcallreg r3.w0
.origin 0
.entrypoint 0

#include "pruss_io.hp"


#define REGS_BASE  (0x00000000)

#define CONTROL_OFS (0)
#define AUDIO_0_OFS (SIZE(pru_control) + SIZE(buffer_control) * 0)
#define AUDIO_1_OFS (SIZE(pru_control) + SIZE(buffer_control) * 1)
#define MIDI_0_OFS  (SIZE(pru_control) + SIZE(buffer_control) * 2)
#define MIDI_1_OFS  (SIZE(pru_control) + SIZE(buffer_control) * 3)


#define TSADC_BASE 0x44e0d000

#define CONTROL 0x0040
#define SPEED   0x004c
#define STEP1   0x0064
#define DELAY1  0x0068
#define STATUS  0x0044
#define STEPCONFIG  0x0054
#define FIFO0COUNT  0x00e4

#define ADC_FIFO0DATA   (ADC_BASE + 0x0100)

.struct run_vars
    .u32 p_audio_buf
    .u32 p_midi_buf
    .u32 status
.ends

    // On startup, we always briefly enter the pause state.
    
pause_entry:
    // Shut down/reset ADC, UART
    //TODO
    
    // Echo back the status to indicate we are in the requested mode
    mov     r0, REGS_BASE + CONTROL_OFS
    mov     r1, PRU_MODE_PAUSE
    sbbo    r1, r0, OFFSET(pru_control.status), SIZE(pru_control.status)
    
pause_loop:
    // Check if we are still in RUN mode globally
    mov     r0, REGS_BASE + CONTROL_OFS
    lbbo    r1, r0, OFFSET(pru_control.mode), SIZE(pru_control.mode)
    // So, what is that mode?
    qbeq    run_entry, r1, PRU_MODE_RUN
    qbeq    pause_loop, r1, PRU_MODE_PAUSE
    // Fell through; must be halt!
    jmp     halt_entry

halt_entry:
    mov     r0, REGS_BASE + CONTROL_OFS
    mov     r1, PRU_MODE_HALT
    sbbo    r1, r0, OFFSET(pru_control.status), SIZE(pru_control.status)
    halt

.enter run
.assign buffer_control, r4, r11, audio_buf
.assign buffer_control, r12, r19, midi_buf
.assign run_vars, r20, *, l

run_entry:
    mov     r3, 0
    mov     l.status, STATUS_NOMINAL
    // We haven't acquired any buffers yet, so first things first means
    // jumping into the middle of the loop, actually...
    jmp     run_acquire_buffers

run_loop:
    // Check if we are still in RUN mode globally
    mov     r0, REGS_BASE + CONTROL_OFS
    lbbo    r1, r0, OFFSET(pru_control.mode), SIZE(pru_control.mode)
    // So, what is that mode?
    qbeq    run_run, r1, PRU_MODE_RUN
    // Fell through; must be halt or pause. Go to pause to trigger peripheral
    // shutdown; this is a bit of a hack, but eh.
    jmp     pause_entry

run_run:
    // Our main loop consists of polling our peripherals. None of this code is
    // particularly time-critical: even if we're sampling at 96kHz, we still
    // get ~2,000 cycles between ADC events, which is probably about 10x our
    // current worst case, even with all the possible memory stalls.
    
    // Poll buffer capacity: need to be swapped?
    qbge    run_swap_buffers, audio_buf.capacity, audio_buf.size
run_poll_peripherals:
    // Poll ADC
    //TODO
    // Poll UART
    //TODO
    
    //TODO remove this test code
    mov     r0, 100000
delay_loop:
    sub     r0, r0, 1
    qbne    delay_loop, r0, 0
    
    mov     r2.w0, 0x0101
    add     r3.w0, r3.w0, r2.w0
    add     r3.w2, r3.w2, r2.w0
    add     r2, audio_buf.address, audio_buf.size
    sbbo    r3, r2, 0, 4
    add     audio_buf.size, audio_buf.size, 4
    
    jmp     run_loop
    
run_swap_buffers:
    // First, hand the buffers we're currently working with over to the host
    sbbo    audio_buf, l.p_audio_buf, 0, SIZE(audio_buf)
    sbbo    midi_buf, l.p_midi_buf, 0, SIZE(midi_buf)
    // Make sure ownership is transferred LAST! This is important
    mov     audio_buf.owner, OWNER_HOST
    sbbo    audio_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    mov     midi_buf.owner, OWNER_HOST
    sbbo    midi_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    
run_acquire_buffers:
    // Buffer 0 available?
    mov     l.p_audio_buf, REGS_BASE + AUDIO_0_OFS
    lbbo    audio_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    // Audio buffer owned by us?
    qbne    run_check_buffer_1, audio_buf.owner, OWNER_PRU0
    // Yes, check MIDI buffer as well
    mov     l.p_midi_buf, REGS_BASE + MIDI_0_OFS
    lbbo    midi_buf.owner, l.p_midi_buf, OFFSET(midi_buf.owner), SIZE(midi_buf.owner)
    qbne    run_check_buffer_1, midi_buf.owner, OWNER_PRU0
    // Fell through, both are ours, roll with this one!
    jmp     run_read_buffers
    
run_check_buffer_1:
    // Buffer 1 available?
    mov     l.p_audio_buf, REGS_BASE + AUDIO_1_OFS
    lbbo    audio_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    // Audio buffer owned by us?
    qbne    run_no_buffers_available, audio_buf.owner, OWNER_PRU0
    // Yes, check MIDI buffer as well
    mov     l.p_midi_buf, REGS_BASE + MIDI_1_OFS
    lbbo    midi_buf.owner, l.p_midi_buf, OFFSET(midi_buf.owner), SIZE(midi_buf.owner)
    qbne    run_no_buffers_available, midi_buf.owner, OWNER_PRU0
    // Fell through, both are ours, roll with this one!
    jmp     run_read_buffers
    
run_no_buffers_available:
    // Nothing available! Uh oh.
    or      l.status, l.status, STATUS_ERROR_OVERRUN
    jmp     run_acquire_buffers
    
run_read_buffers:
    lbbo    audio_buf, l.p_audio_buf, OFFSET(audio_buf), SIZE(audio_buf)
    lbbo    midi_buf, l.p_midi_buf, OFFSET(midi_buf), SIZE(midi_buf)
    // Remember our last status: this is how we transfer back info about
    // overruns, etc.
    mov     audio_buf.status, l.status
    mov     midi_buf.status, l.status
    mov     l.status, STATUS_NOMINAL
    jmp     run_poll_peripherals
/*

#define PRU0_ARM_INTERRUPT 19


// Register allocations
#define adc_  r6
#define fifo0data r7
#define out_buff  r8
#define locals r9

#define value r10
#define channel   r11
#define ema   r12
#define encoders  r13
#define cap_delay r14

#define tmp0  r1
#define tmp1  r2
#define tmp2  r3
#define tmp3  r4
#define tmp4  r5

.origin 0
.entrypoint START

START:
    lbco    r0, C4, 4, 4                    // Load Bytes Constant Offset (?)
    clr     r0, r0, 4                        // Clear bit 4 in reg 0
    sbco    r0, C4, 4, 4                    // Store Bytes Constant Offset

    mov     adc_, ADC_BASE
    mov     fifo0data, ADC_FIFO0DATA
    mov     locals, 0

    lbbo    tmp0, locals, 0, 4                // check eyecatcher
    mov     tmp1, 0xbeef1965                //
    qbne    QUIT, tmp0, tmp1                // bail out if does not match

    lbbo    out_buff, locals, 0x0c, 4
    lbbo    ema, locals, 0x1c, 4
    lbbo    encoders, locals, 0x40, 4

    // Read CAP_DELAY value into the register for convenience
    lbbo    cap_delay, locals, 0xc4, 4
    
    // Disable ADC
    lbbo    tmp0, adc_, CONTROL, 4
    mov     tmp1, 0x1
    not     tmp1, tmp1
    and     tmp0, tmp0, tmp1
    sbbo    tmp0, adc_, CONTROL, 4
    
    // Put ADC capture to its full speed
    mov     tmp0, 0
    sbbo    tmp0, adc_, SPEED, 4

    // Configure STEPCONFIG registers for all 8 channels
mov     tmp0, STEP1
    mov     tmp1, 0
    mov     tmp2, 0

FILL_STEPS:
    lsl     tmp3, tmp1, 19
    sbbo    tmp3, adc_, tmp0, 4
    add     tmp0, tmp0, 4
    sbbo    tmp2, adc_, tmp0, 4
    add     tmp1, tmp1, 1
    add     tmp0, tmp0, 4
    qbne    FILL_STEPS, tmp1, 8

    // Enable ADC with the desired mode (make STEPCONFIG registers writable, use tags, enable)
    lbbo    tmp0, adc_, CONTROL, 4
    or      tmp0, tmp0, 0x7
    sbbo    tmp0, adc_, CONTROL, 4
    
CAPTURE:
    // check if we need to delay our main loop (to control capture frequency)
    qbne    CAPTURE_DELAY, cap_delay, 0
NO_DELAY:
    
    mov     tmp0, 0x1fe    
    sbbo    tmp0, adc_, STEPCONFIG, 4   // write STEPCONFIG register (this triggers capture)

    // check for exit flag
    lbbo    tmp0, locals, 0x08, 4   // read runtime flags
    qbne    QUIT, tmp0.b0, 0

    
    // check for oscilloscope mode
    lbbo    tmp0, locals, 0x14, 4
    qbeq    NO_SCOPE, tmp0, 0
    
    sub     tmp0, tmp0, 4
    sbbo    tmp0, locals, 0x14, 4
    lbbo    tmp0, locals, 0x10, 4
    lbbo    tmp0, locals, tmp0, 4
    sbbo    tmp0, out_buff, 0, 4
    add     out_buff, out_buff, 4

NO_SCOPE:

// increment ticks
    lbbo    tmp0, locals, 0x04, 4
    add     tmp0, tmp0, 1
    sbbo    tmp0, locals, 0x04, 4
    
    // increment encoder ticks
    lbbo    tmp0, locals, 0x58, 8
    add     tmp1, tmp1, 1
    max     tmp0, tmp1, tmp0
    sbbo    tmp0, locals, 0x58, 8

    lbbo    tmp0, locals, 0x98, 8
    add     tmp1, tmp1, 1
    max     tmp0, tmp1, tmp0
    sbbo    tmp0, locals, 0x98, 8
    sbbo    tmp0, locals, 0x98, 8

WAIT_FOR_FIFO0:
    lbbo    tmp0, adc_, FIFO0COUNT, 4
    qbne    WAIT_FOR_FIFO0, tmp0, 8

READ_ALL_FIFO0:  // lets read all fifo content and dispatch depending on pin type
    lbbo    value, fifo0data, 0, 4
    lsr     channel, value, 16
    and     channel, channel, 0xf
    mov     tmp1, 0xfff
    and     value, value, tmp1

    // here we have true captured value and channel
    qbne    NOT_ENC0, encoders.b0, channel
    mov     channel, 0
    call    PROCESS
    jmp     NEXT_CHANNEL

NOT_ENC0:
    qbne    NOT_ENC1, encoders.b1, channel
    mov     channel, 1
    call    PROCESS
    jmp     NEXT_CHANNEL

NOT_ENC1:
    lsl     tmp1, channel, 2   // to byte offset
    add     tmp1, tmp1, 0x20   // base of the EMA values
    lbbo    tmp2, locals, tmp1, 4
    lsr     tmp3, tmp2, ema
    sub     tmp3, value, tmp3
    add     tmp2, tmp2, tmp3
    sbbo    tmp2, locals, tmp1, 4

NEXT_CHANNEL:
    sub     tmp0, tmp0, 1
    qbne    READ_ALL_FIFO0, tmp0, 0

    jmp     CAPTURE

QUIT:
    mov     R31.b0, PRU0_ARM_INTERRUPT+16   // Send notification to Host for program completion
    halt

CAPTURE_DELAY:
    mov     tmp0, cap_delay
DELAY_LOOP:
    sub     tmp0, tmp0, 1
    qbne    DELAY_LOOP, tmp0, 0
    jmp     NO_DELAY

PROCESS:                                // lets process wheel encoder value
    lsl     channel, channel, 6
    add     channel, channel, 0x44
    lbbo    &tmp1, locals, channel, 16     // load tmp1-tmp4 (threshold, raw, min, max)
    mov     tmp2, value
    min     tmp3, tmp3, value
    max     tmp4, tmp4, value
    sbbo    &tmp1, locals, channel, 16     // store min/max etc
    add     tmp2, tmp3, tmp1                // tmp2 = min + threshold
    qblt    MAYBE_TOHIGH, value, tmp2
    add     tmp2, value, tmp1               // tmp2 = value + threshold
    qblt    MAYBE_TOLOW, tmp4, tmp2

    // zero out delays
    add     channel, channel, 32
    mov     tmp1, 0
    mov     tmp2, 0
    sbbo    &tmp1, locals, channel, 8

    ret

MAYBE_TOHIGH:
    add     channel, channel, 28
    lbbo    &tmp1, locals, channel, 12 // load tmp1-tmp3 with (delay, up_count, down_count)
    add     tmp2, tmp2, 1               // up_count++
    mov     tmp3, 0                     // down_count=0
    sbbo    &tmp1, locals, channel, 12
    qblt    TOHIGH, tmp2, tmp1
    
    ret

MAYBE_TOLOW:
    add     channel, channel, 28
    lbbo    &tmp1, locals, channel, 12 // load tmp1-tmp3 with (delay, up_count, down_count)
    add     tmp3, tmp3, 1               // down_count++
    mov     tmp2, 0                     // up_count=0
    sbbo    &tmp1, locals, channel, 12
    qblt    TOLOW, tmp3, tmp1
    
    ret

TOLOW:
    mov     tmp3, 0
    mov     tmp2, 0
    sbbo    &tmp1, locals, channel, 12  // up_count = down_count = 0
    
    sub     channel, channel, 20
    mov     tmp2, value                  // min = max = value
    mov     tmp3, value
    sbbo    &tmp2, locals, channel, 8
    
    add     channel, channel, 8
    lbbo    &tmp2, locals, channel, 12  // ticks, speed, acc
    add     tmp2, tmp2, 1                // ticks++
    mov     tmp3, tmp4                   // speed = acc
    mov     tmp4, 0                      // acc = 0
    sbbo    &tmp2, locals, channel, 12
    ret
    
TOHIGH:
    mov     tmp3, 0
    mov     tmp2, 0
    sbbo    &tmp1, locals, channel, 12  // up_count=0, down_count=0

    sub     channel, channel, 20
    mov     tmp2, value                  // min = max = value
    mov     tmp3, value
    sbbo    &tmp2, locals, channel, 8
    ret

    
.leave main

*/