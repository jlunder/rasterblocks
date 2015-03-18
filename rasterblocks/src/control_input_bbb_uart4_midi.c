#ifdef RB_USE_BBB_UART4_MIDI_CONTROL_INPUT


#include "control_input.h"

#include <asm/termbits.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>


#define RB_CONTROL_INPUT_BBB_UART4_STARTUP_COMMAND \
    "sh -c 'grep -q BB-UART4 /sys/devices/bone_capemgr.9/slots || " \
    "echo BB-UART4 > /sys/devices/bone_capemgr.9/slots'"
#define RB_MIDI_BAUD 30000


static RBControlInputMidiParser g_rbMidiParser;
static RBControls g_rbLastControls;
static int g_rbUartFd = -1;


int rbControlInputBbbUart4MidiInitialize(RBConfiguration const * pConfig)
{
	struct termios2 tio;
    int ret;
    
    rbControlInputBbbUart4MidiShutdown();
    
    ret = system(RB_CONTROL_INPUT_BBB_UART4_STARTUP_COMMAND);
    if(ret != 0) {
        rbWarning("Failed to start up UART4\n");
    }

	// Configure UART4 for raw 8-bit input at MIDI data rate
	g_rbUartFd = open(device,O_RDWR|O_NOCTTY);
	rbVerify(g_rbUartFd == 0);
    rbVerify(ioctl(g_rbUartFd, TCGETS2, &tio) == 0);
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_cflag = CS8 | BOTHER | HUPCL | CREAD | CLOCAL;
    tio.c_lflag = 0;
    tio.c_ispeed = RB_MIDI_BAUD;
    tio.c_ospeed = RB_MIDI_BAUD;
    rbVerify(ioctl(g_rbUartFd, TCSETS2, &tio) == 0);
    
    rbControlInputMidiParserInitialize(&g_rbMidiParser);
}


void rbControlInputBbbUart4MidiShutdown(void)
{
    if(g_rbUartFd >= 0) {
        close(g_rbUartFd);
        g_rbUartFd = -1;
    }
}


void rbControlInputBbbUart4MidiRead(RBControls * pControls)
{
    // Keep reading input as long as it looks like there's a full buffer
    for(;;) {
        uint8_t buf[128];
        int amountRead;
        
        amountRead = read(g_rbUartFd, sizeof buf, buf);
        rbAssert(amountRead >= 0);
        
        // Use the default MIDI parser to actually process the data
        for(int i = 0; i < amountRead; ++i) {
            rbControlInputMidiParserParseByte(&g_rbMidiParser,
                &g_rbLastControls, buf[i]);
        }
        if(amountRead < sizeof buf) {
            break;
        }
    }
    
    memcpy(pControls, &g_rbLastControls, sizeof *pControls);
}


#endif // RB_USE_BBB_UART4_CONTROL_INPUT
