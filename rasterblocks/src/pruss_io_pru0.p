.origin 0
.entrypoint 0

#include "pruss_io.hp"


#define REGS_BASE  (0x00000000)


.struct main_vars
    .u32 status
.ends

.enter main
.assign buffer_control, r4, r7, buf
.assign main_vars, r8, *, l

    // Actually, just don't do anything right now.
    halt
    
    // Leave this loop skeleton for later when we do want to do something...
    
main_loop:
    // Check if we are still in RUN mode globally
    mov     r0, REGS_BASE + GLOBAL_OFS
    lbbo    l.status, r0, OFFSET(global_control.status), SIZE(global_control.status)
    qbeq    main_loop, l.status, GLOBAL_STATUS_RUN
    
    // Fell through; nope, halt!
    // Send notification to host for program completion
    //mov     r31.b0, PRU1_ARM_INTERRUPT+16
    halt
    
    
.leave main

