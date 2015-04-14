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


.struct main_vars
    .u32 p_adc
    .u32 p_audio_buf
    .u32 p_midi_buf
    .u32 p_pru_ctrl
    .u32 status
.ends

.enter main
.assign buffer_control, r4, r11, audio_buf
.assign buffer_control, r12, r19, midi_buf
.assign main_vars, r20, *, l

    // On startup, we always briefly enter the pause state.
    
pause_entry:
    // Shut down/reset ADC, UART
    //TODO
    mov     l.p_adc, TSCADC_BASE
    mov     l.p_pru_ctrl, PRU0_PRU_CTRL_BASE

    // Disable ADC, disable write-protect
    mov     r0, 0x6
    sbbo    r0, l.p_adc, TSCADC_CTRL, 4
    
    mov     r0, 0x03FF
    sbbo    r0, l.p_adc, TSCADC_IRQENABLE_CLR, 4
    sbbo    r0, l.p_adc, TSCADC_IRQSTATUS, 4
    
    mov     r0, 3
    sbbo    r0, l.p_adc, TSCADC_DMAENABLE_CLR, 4
    
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
    // Disable ADC
    mov     r0, 0x4
    sbbo    r0, l.p_adc, TSCADC_CTRL, 4
    
    // Report that we're halted
    mov     r0, REGS_BASE + CONTROL_OFS
    mov     r1, PRU_MODE_HALT
    sbbo    r1, r0, OFFSET(pru_control.status), SIZE(pru_control.status)
    
    // And that's it!
    halt

run_entry:
    call    cycle_reset
    
    // Disable ADC, disable write-protect
    mov     r0, 0x6
    sbbo    r0, l.p_adc, TSCADC_CTRL, 4
    
    mov     r0, 1000
run_entry_adc_wait_idle_loop:
    lbbo    r0, l.p_adc, TSCADC_ADCSTAT, 4
    // Is the ADC busy? If not, done here.
    qbbc    run_entry_adc_wait_idle_fin, r0, 5
    // Yes: how long have we been waiting?
    lbbo    r0, l.p_pru_ctrl, PRU_CTRL_CYCLE, 4
    qbgt    run_entry_adc_wait_idle_loop, r0, r1
    // Fell through: too long!
    jmp     halt_entry
run_entry_adc_wait_idle_fin:

    // Smart idle mode
    mov     r0, 0xC
    sbbo    r0, l.p_adc, TSCADC_SYSCONFIG, 4
    
    // Clock divider to max speed
    mov     r0, 0
    sbbo    r0, l.p_adc, TSCADC_ADC_CLKDIV, 4

    // Configure IDLECONFIG, STEPCONFIG1-2, STEPDELAY1-2 registers
    mov     r0, (0x8 << 19) | (0x8 << 15)
    mov     r1, 0
    sbbo    r0, l.p_adc, TSCADC_IDLECONFIG, 8
    mov     r0, 0 << 19
    mov     r1, 0
    sbbo    r0, l.p_adc, TSCADC_STEPCONFIG1, 8
    mov     r0, 1 << 19
    or      r0, r0, 0x08 // 4x averaging
    mov     r1, (1 << 24) | 1
    sbbo    r0, l.p_adc, TSCADC_STEPCONFIG1 + 8, 8
    
    // Re-enable ADC, enable write-protect
    mov     r0, 0x3
    sbbo    r0, l.p_adc, TSCADC_CTRL, 4
    
    // We have no buffers yet, so we can't *swap*, we must *acquire*; this
    // also sets up l.status.
    call     swap_buffers_acquire
    
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
    // The ADC doesn't seem to have any way to trigger automatic periodic
    // conversion; so, to minimize timing jitter, we follow this process:
    // - Busy-wait until an exact time as measured by the PRU_CTRL_CYCLE
    //   counter
    // - Reset the counter (this should take a fixed amount of time)
    // - Trigger one conversion
    // - Do all our polling (which may take some variable amount of time)
    //
    // As long as polling takes less than the amount of time it takes for the
    // timer to count up to the next conversion deadline (2000 cycles at
    // 100kHz, which should be WAY plenty) we should have zero jitter in our
    // conversion timings.
    //
    // This also depends on our other polling not needing to happen more often
    // than once every ADC sample. Fortunately this is true: MIDI is max 3000
    // CPS, well below the 48kHz nominal audio sample rate.
    
    // Before we start, clear the ADC FIFO. This is a failsafe: the FIFO
    // should be empty here, but if it's not, when we pull data out after
    // conversion, it's going to be some stale data and possibly out of sync.
    lbbo    r1, l.p_adc, TSCADC_FIFO0COUNT, 4
    and     r0, r0, 0x7F
    mov     r3, TSCADC_FIFO0DATA_BASE
run_clear_adc_fifo_loop:
    qbeq    run_clear_adc_fifo_fin, r0.b0, 0
    sub     r0.b0, r0.b0, 1
    or      l.status, l.status, STATUS_ERROR_ADC_DESYNC
    lbbo    r0, r3, 0, 4
    jmp     run_clear_adc_fifo_loop
run_clear_adc_fifo_fin:
    
    // r0 gets the number of cycles to delay; this was tuned empirically for
    // 48kHz sampling. This is not the exact number of PRU cycles between
    // samples; there's a fixed but difficult to calculate number of extra
    // cycles introduced by the delay between reading/testing this counter and
    // loop and cycle_reset.
    mov     r0, 4000
    clr     r31, 5
run_sample_timing_wait_loop:
    lbbo    r1, l.p_pru_ctrl, PRU_CTRL_CYCLE, 4
    qbgt    run_sample_timing_wait_loop, r1, r0
    
    call    cycle_reset
    set     r31, 5
    
    // Enable STEP1 and STEP2, which starts conversion immediately
    mov     r0, 0x006
    sbbo    r0, l.p_adc, TSCADC_STEPENABLE, 4
run_poll_adc_skip_conversion_start:
    
    // Timing-critical code ends here. Now, all our regular polling...
    
    // Poll UART
    //TODO
    
    // Poll ADC last, to allow max overlap between ADC conversion time and
    // getting other useful work done.
run_poll_adc_loop:
    // Poll buffer capacity: need to be swapped?
    qblt    run_poll_adc_buffers_not_full_yet, audio_buf.capacity, audio_buf.size
    // swap_buffers can actually block indefinitely waiting for the host to
    // give up a buffer if none are available. If this happens, it will
    // probably destroy our timing and cause a partial frame loss. In this
    // situation, it will set a status bit which the host can use to detect
    // the problem... since this essentially means the host is overcommitted,
    // we're not going to worry about trying to fix it.
    call    swap_buffers
run_poll_adc_buffers_not_full_yet:
    
    // Wait for the present conversion to be finished -- if we don't wait, we
    // probably won't notice the conversion results 'til the next iteration of
    // the loop, well after it's been posted to the FIFO.
    // This reduces latency by a totally negligible amount but I like it.
    // r1 gets the cycle count for the timeout
    mov     r1, 2000
run_poll_adc_wait_conversion_loop:
    lbbo    r0, l.p_adc, TSCADC_ADCSTAT, 4
    // Is the ADC busy? If not, done here.
    qbbc    run_poll_adc_wait_conversion_fin, r0, 5
    // Yes: how long have we been waiting?
    lbbo    r0, l.p_pru_ctrl, PRU_CTRL_CYCLE, 4
    qbgt    run_poll_adc_wait_conversion_loop, r0, r1
    // Fell through: too long!
    jmp     run_poll_adc_timeout
run_poll_adc_wait_conversion_fin:
    
    // Check for conversion results in the FIFO -- if empty, something is
    // wrong with the ADC (like maybe we didn't load the BB-ADC dtbo, and it's
    // powered down)
    lbbo    r0, l.p_adc, TSCADC_FIFO0COUNT, 4
    qbgt    run_poll_adc_no_data, r0, 2
    // Process and store:
    mov     r3, TSCADC_FIFO0DATA_BASE
    lbbo    r0, r3, 0, 4
    // Expect ID 0 for result 0
    qbne    run_poll_adc_desync, r0.w2, 0
    lbbo    r1, r3, 0, 4
    // Expect ID 1 for result 1
    qbne    run_poll_adc_desync, r1.w2, 1
    // Shuffle the data into r0
    mov     r0.w2, r1.w0
    // Store data and update pointer
    sbbo    r0, audio_buf.address, audio_buf.size, 4
    add     audio_buf.size, audio_buf.size, 4
    // Go back to the start: we want to empty the FIFO if somehow multiple
    // values get stuck in there, also swap buffer immediately if possible, to
    // absolutely minimize latency
    jmp     run_poll_adc_fin
    
run_poll_adc_timeout:
    or      l.status, l.status, STATUS_ERROR_ADC_TIMEOUT
    call    swap_buffers
    jmp     run_poll_adc_fin
    
run_poll_adc_no_data:
    or      l.status, l.status, STATUS_ERROR_ADC_NO_DATA
    call    swap_buffers
    jmp     run_poll_adc_fin
    
run_poll_adc_desync:
    or      l.status, l.status, STATUS_ERROR_ADC_DESYNC
    call    swap_buffers
    jmp     run_poll_adc_fin
    
run_poll_adc_fin:
    
    jmp     run_loop



swap_buffers:
    // Remember our last status: this is how we transfer back info about
    // overruns, etc.
    mov     audio_buf.status, l.status
    mov     midi_buf.status, l.status
    // First, hand the buffers we're currently working with back to the host
    sbbo    audio_buf, l.p_audio_buf, 0, SIZE(audio_buf)
    sbbo    midi_buf, l.p_midi_buf, 0, SIZE(midi_buf)
    // Make sure ownership is transferred LAST! This is important
    mov     audio_buf.owner, OWNER_HOST
    sbbo    audio_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    mov     midi_buf.owner, OWNER_HOST
    sbbo    midi_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    
    // Trigger interrupt to wake up host!
    mov     r31.b0, PRU0_ARM_INTERRUPT + 16
    
swap_buffers_acquire:
    // Buffer 0 available?
    mov     l.p_audio_buf, REGS_BASE + AUDIO_0_OFS
    lbbo    audio_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    // Audio buffer owned by us?
    qbne    swap_buffers_buffer_0_unavailable, audio_buf.owner, OWNER_PRU0
    // Yes, check MIDI buffer as well
    mov     l.p_midi_buf, REGS_BASE + MIDI_0_OFS
    lbbo    midi_buf.owner, l.p_midi_buf, OFFSET(midi_buf.owner), SIZE(midi_buf.owner)
    qbne    swap_buffers_buffer_0_unavailable, midi_buf.owner, OWNER_PRU0
    // Fell through, both are ours, roll with this one!
    jmp     swap_buffers_read_buffer
swap_buffers_buffer_0_unavailable:
    
    // Buffer 1 available?
    mov     l.p_audio_buf, REGS_BASE + AUDIO_1_OFS
    lbbo    audio_buf.owner, l.p_audio_buf, OFFSET(audio_buf.owner), SIZE(audio_buf.owner)
    // Audio buffer owned by us?
    qbne    swap_buffers_buffer_1_unavailable, audio_buf.owner, OWNER_PRU0
    // Yes, check MIDI buffer as well
    mov     l.p_midi_buf, REGS_BASE + MIDI_1_OFS
    lbbo    midi_buf.owner, l.p_midi_buf, OFFSET(midi_buf.owner), SIZE(midi_buf.owner)
    qbne    swap_buffers_buffer_1_unavailable, midi_buf.owner, OWNER_PRU0
    // Fell through, both are ours, roll with this one!
    jmp     swap_buffers_read_buffer
swap_buffers_buffer_1_unavailable:
    
    // Nothing available! Uh oh. Note the error and start busy-waiting until
    // something does become available.
    or      l.status, l.status, STATUS_ERROR_OVERRUN
    jmp     swap_buffers_acquire
    
swap_buffers_read_buffer:
    lbbo    audio_buf, l.p_audio_buf, OFFSET(audio_buf), SIZE(audio_buf)
    lbbo    midi_buf, l.p_midi_buf, OFFSET(midi_buf), SIZE(midi_buf)
    mov     l.status, STATUS_NOMINAL
    ret



cycle_reset:
    lbbo    r1, l.p_pru_ctrl, PRU_CTRL_CONTROL, 4
    clr     r1, 3
    sbbo    r1, l.p_pru_ctrl, PRU_CTRL_CONTROL, 4
    mov     r0, 0
    sbbo    r0, l.p_pru_ctrl, PRU_CTRL_CYCLE, 4
    set     r1, 3
    sbbo    r1, l.p_pru_ctrl, PRU_CTRL_CONTROL, 4
    ret



delay:
    qbeq    delay_loop_fin, r0, 0
delay_loop:
    sub     r0, r0, 1
    qbne    delay_loop, r0, 0
delay_loop_fin:
    ret



.leave main
