#ifdef RB_USE_PRUSS_IO


#include "pruss_io.h"

#include "graphics_util.h"

#include <prussdrv.h>
#include <pruss_intc_mapping.h>


#define RB_PRUSS_IO_DEVICE_STARTUP_COMMAND \
    "sh -c \"modprobe uio_pruss && " \
    "(grep -q rb-pruss-io /sys/devices/bone_capemgr.9/slots || " \
    "echo rb-pruss-io > /sys/devices/bone_capemgr.9/slots)\""


#define RB_SHAREDRAM_PRU_BASE 0x00010000
#define RB_DATARAM_SIZE (1024 * 8)
#define RB_SHAREDRAM_SIZE (1024 * 12)


#define RB_MAKE_HOST_ADDRESS_DATARAM0(offset) \
    (rbPrussIoDataRam->pru0.raw + (offset))
#define RB_MAKE_HOST_ADDRESS_DATARAM1(offset) \
    (rbPrussIoDataRam->pru1.raw + (offset))
#define RB_MAKE_HOST_ADDRESS_SHAREDRAM(offset) (rbPrussIoSharedRam + (offset))
#define RB_MAKE_PRU0_ADDRESS_DATARAM0(offset) (offset)
#define RB_MAKE_PRU0_ADDRESS_DATARAM1(offset) \
    (RB_DATARAM_SIZE + (offset))
#define RB_MAKE_PRU0_ADDRESS_SHAREDRAM(offset) \
    (RB_SHAREDRAM_PRU_BASE + (offset))
#define RB_MAKE_PRU1_ADDRESS_DATARAM0(offset) \
    (RB_DATARAM_SIZE + (offset))
#define RB_MAKE_PRU1_ADDRESS_DATARAM1(offset) (offset)
#define RB_MAKE_PRU1_ADDRESS_SHAREDRAM(offset) \
    (RB_SHAREDRAM_PRU_BASE + (offset))


// The number of buffers is uniform across all input/output. We're double-
// buffered right now, triple-buffering would be ideal but to be really useful
// requires more complex synchronization to let whoever is filling the buffers
// steal back an already-full buffer and write new data into it.
#define RB_PRUSS_IO_NUM_BUFFERS 2

// The number of outputs we will output on simultaneously.
#define RB_PRUSS_IO_NUM_STRINGS 8

// The number of audio channels we'll read from the ADCs. Should match
// RB_AUDIO_CHANNELS. Independently set here (and checked) as a reminder to
// modify the PRU assembler source if you change RB_AUDIO_CHANNELS.
#define RB_PRUSS_IO_AUDIO_CHANNELS 2


#define RB_MODE_RUN   0
#define RB_MODE_PAUSE 1
#define RB_MODE_HALT  2

#define RB_OWNER_HOST 0
#define RB_OWNER_PRU0 1
#define RB_OWNER_PRU1 2

#define RB_COMMAND_MIDI_IDLE 0
#define RB_COMMAND_MIDI_READ 0x0001

#define RB_COMMAND_AUDIO_IDLE 0
#define RB_COMMAND_AUDIO_READ 0x0001

#define RB_COMMAND_LIGHT_IDLE            0
#define RB_COMMAND_LIGHT_2W_2MHZ         0x0001
#define RB_COMMAND_LIGHT_2W_10MHZ        0x0002
#define RB_COMMAND_LIGHT_1W_800KHZ       0x0003
#define RB_COMMAND_LIGHT_END_FRAME_PAUSE 0x0100

#define RB_STATUS_NOMINAL       0
#define RB_STATUS_ERROR_COMMAND 0x00000001
#define RB_STATUS_ERROR_OVERRUN 0x00000002


typedef struct {
    uint32_t owner;
    uint32_t frameNum;
    uint32_t command;
    uint32_t status;
    uint32_t address;
    uint32_t size;
    uint32_t capacity;
    uint32_t pad;
} RBPrussIoTransferControl;


typedef struct
{
    union {
        struct {
            uint32_t mode;
            uint32_t status;
            RBPrussIoTransferControl audioInput[RB_PRUSS_IO_NUM_BUFFERS];
            RBPrussIoTransferControl midiInput[RB_PRUSS_IO_NUM_BUFFERS];
        } control;
        uint8_t raw[RB_DATARAM_SIZE];
    } pru0;
    union {
        struct {
            uint32_t mode;
            uint32_t status;
            RBPrussIoTransferControl lightOutput[RB_PRUSS_IO_NUM_BUFFERS];
        } control;
        uint8_t raw[RB_DATARAM_SIZE];
    } pru1;
} RBPrussIoMemoryMap;


static RBPrussIoMemoryMap * rbPrussIoDataRam;
static uint8_t * rbPrussIoSharedRam;
static uint8_t * rbPrussIoAudioInputBufs[2];
static uint8_t * rbPrussIoMidiInputBufs[2];
static uint8_t * rbPrussIoLightOutputBufs[2];

static uint32_t rbPrussIoNextInputFrameNum;
static uint32_t rbPrussIoNextOutputFrameNum;


#include "pruss_io_pru0_bin.h"
#include "pruss_io_pru1_bin.h"


void rbPrussIoInitialize(RBConfiguration * pConfig)
{
    UNUSED(pConfig);
}


void rbPrussIoShutdown(void)
{
}


static void rbLightOutputPrussInitializeTransferBuffers(void);
void rbLightOutputPrussInitialize(RBConfiguration const * pConfig)
{
    int ret;
    
    rbPrussIoNextInputFrameNum = 0;
    rbPrussIoNextOutputFrameNum = 0;
    
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
    
    rbLightOutputPrussInitializeTransferBuffers();
    rbMemoryBarrier();
    
    rbInfo("Executing PRU code\n");
    prussdrv_exec_code(0, rbPrussIoPru0Code, sizeof rbPrussIoPru0Code);
    prussdrv_exec_code(1, rbPrussIoPru1Code, sizeof rbPrussIoPru1Code);
    
    rbPrussIoDataRam->pru0.control.mode = RB_MODE_RUN;
    rbPrussIoDataRam->pru1.control.mode = RB_MODE_RUN;
    rbMemoryBarrier();
}


void rbLightOutputPrussInitializeTransferBuffers(void)
{
    uint32_t bufferOffset;
    uint32_t bufferCapacity;
    
    // PRUs shouldn't be running, but what the heck, init them in "paused" mode
    rbPrussIoDataRam->pru0.control.mode = RB_MODE_PAUSE;
    rbPrussIoDataRam->pru0.control.status = RB_MODE_PAUSE;
    rbPrussIoDataRam->pru1.control.mode = RB_MODE_PAUSE;
    rbPrussIoDataRam->pru1.control.status = RB_MODE_PAUSE;
    
    // Set up the transfer buffers
    rbZero(RB_MAKE_HOST_ADDRESS_DATARAM0(0), RB_DATARAM_SIZE);
    rbZero(RB_MAKE_HOST_ADDRESS_DATARAM1(0), RB_DATARAM_SIZE);
    rbZero(RB_MAKE_HOST_ADDRESS_SHAREDRAM(0), RB_SHAREDRAM_SIZE);
    
    rbAssert(RB_PRUSS_IO_NUM_BUFFERS == 2);
    rbAssert(RB_PRUSS_IO_NUM_STRINGS >= RB_NUM_STRINGS);
    rbAssert(RB_PRUSS_IO_AUDIO_CHANNELS == RB_AUDIO_CHANNELS);
    
    // PRU0 structures: PRU0 handles audio and MIDI input
    bufferOffset = sizeof rbPrussIoDataRam->pru0.control;
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        bufferCapacity = RB_AUDIO_FRAMES_PER_VIDEO_FRAME *
            RB_PRUSS_IO_AUDIO_CHANNELS * sizeof (uint16_t);
        rbPrussIoDataRam->pru0.control.audioInput[i].owner = RB_OWNER_PRU0;
        rbPrussIoDataRam->pru0.control.audioInput[i].frameNum = i;
        rbPrussIoDataRam->pru0.control.audioInput[i].command =
            RB_COMMAND_AUDIO_IDLE;
        rbPrussIoDataRam->pru0.control.audioInput[i].status = RB_STATUS_NOMINAL;
        rbPrussIoDataRam->pru0.control.audioInput[i].address =
            RB_MAKE_PRU0_ADDRESS_DATARAM0(bufferOffset);
        rbPrussIoAudioInputBufs[i] =
            RB_MAKE_HOST_ADDRESS_DATARAM0(bufferOffset);
        rbPrussIoDataRam->pru0.control.audioInput[i].size = 0;
        rbPrussIoDataRam->pru0.control.audioInput[i].capacity = bufferCapacity;
        rbAssert(bufferOffset + bufferCapacity <= RB_DATARAM_SIZE);
        bufferOffset += bufferCapacity;
    }
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        bufferCapacity = RB_MIDI_MAX_CHARS_PER_VIDEO_FRAME;
        rbPrussIoDataRam->pru0.control.midiInput[i].owner = RB_OWNER_PRU0;
        rbPrussIoDataRam->pru0.control.midiInput[i].frameNum = i;
        rbPrussIoDataRam->pru0.control.midiInput[i].command =
            RB_COMMAND_MIDI_IDLE;
        rbPrussIoDataRam->pru0.control.midiInput[i].status = RB_STATUS_NOMINAL;
        rbPrussIoDataRam->pru0.control.midiInput[i].address =
            RB_MAKE_PRU0_ADDRESS_DATARAM0(bufferOffset);
        rbPrussIoMidiInputBufs[i] =
            RB_MAKE_HOST_ADDRESS_DATARAM0(bufferOffset);
        rbPrussIoDataRam->pru0.control.midiInput[i].size = 0;
        rbPrussIoDataRam->pru0.control.midiInput[i].capacity = bufferCapacity;
        rbAssert(bufferOffset + bufferCapacity <= RB_DATARAM_SIZE);
        bufferOffset += bufferCapacity;
    }
    
    // PRU1 structures: PRU1 handles light output
    bufferOffset = 0;
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        bufferCapacity = RB_SHAREDRAM_SIZE / 2;
        rbAssert(bufferCapacity >= (RB_NUM_PANELS_PER_STRING * RB_PANEL_WIDTH *
            RB_PANEL_HEIGHT * 3 * RB_PRUSS_IO_NUM_STRINGS));
        rbPrussIoDataRam->pru1.control.lightOutput[i].owner = RB_OWNER_HOST;
        rbPrussIoDataRam->pru1.control.lightOutput[i].frameNum = 0;
        rbPrussIoDataRam->pru1.control.lightOutput[i].command =
            RB_COMMAND_LIGHT_IDLE;
        rbPrussIoDataRam->pru1.control.lightOutput[i].status =
            RB_STATUS_NOMINAL;
        rbPrussIoDataRam->pru1.control.lightOutput[i].address =
            RB_MAKE_PRU1_ADDRESS_SHAREDRAM((RB_SHAREDRAM_SIZE / 2) * (i + 0));
        rbPrussIoLightOutputBufs[i] =
            RB_MAKE_HOST_ADDRESS_SHAREDRAM((RB_SHAREDRAM_SIZE / 2) * (i + 0));
        rbPrussIoDataRam->pru1.control.lightOutput[i].size = 0;
        rbPrussIoDataRam->pru1.control.lightOutput[i].capacity = bufferCapacity;
        rbAssert(bufferOffset + bufferCapacity <= RB_SHAREDRAM_SIZE);
        bufferOffset += bufferCapacity;
    }
}


void rbLightOutputPrussShutdown(void)
{
    rbPrussIoDataRam->pru0.control.mode = RB_MODE_HALT;
    rbPrussIoDataRam->pru1.control.mode = RB_MODE_HALT;
    rbMemoryBarrier();
    
    // Wait until PRU0 has finished execution
    rbInfo("Waiting for PRU HALT\n");
    prussdrv_pru_wait_event(PRU_EVTOUT_0);
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
    
    rbInfo("Closing PRUSS driver\n");
    prussdrv_pru_disable(0);
    prussdrv_exit();
}


void rbLightOutputPrussShowLights(RBRawLightFrame const * pFrame)
{
    size_t buf;
    size_t i;
    
    rbMemoryBarrier();
    // Check for free buffers
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        if(rbPrussIoDataRam->pru1.control.lightOutput[i].owner ==
                RB_OWNER_HOST) {
            if(rbPrussIoDataRam->pru1.control.lightOutput[i].status !=
                    RB_STATUS_NOMINAL) {
                rbWarning("PRUSS light output reports error (frame %d): %s "
                        "(%X)\n",
                    rbPrussIoDataRam->pru1.control.lightOutput[i].frameNum,
                    (rbPrussIoDataRam->pru1.control.lightOutput[i].status ==
                        RB_STATUS_ERROR_OVERRUN) ? "overrun" : "??",
                    rbPrussIoDataRam->pru1.control.lightOutput[i].status);
                rbPrussIoDataRam->pru1.control.lightOutput[i].status =
                    RB_STATUS_NOMINAL;
            }
        }
    }
    
    for(buf = 0; buf < RB_PRUSS_IO_NUM_BUFFERS; ++buf) {
        if(rbPrussIoDataRam->pru1.control.lightOutput[buf].owner ==
                RB_OWNER_HOST) {
            break;
        }
    }
    
    if(buf >= RB_PRUSS_IO_NUM_BUFFERS) {
        rbWarning("PRUSS frame queue full!\n");
        ++rbPrussIoNextOutputFrameNum;
        return;
    }
    
    // Fill output buffer in PRUSS RAM first...
    rbPrussIoDataRam->pru1.control.lightOutput[buf].frameNum =
        rbPrussIoNextOutputFrameNum++;
    rbPrussIoDataRam->pru1.control.lightOutput[buf].size =
        RB_NUM_PANELS_PER_STRING * RB_PANEL_WIDTH * RB_PANEL_HEIGHT * 3 *
        RB_PRUSS_IO_NUM_STRINGS;
    rbAssert(rbPrussIoDataRam->pru1.control.lightOutput[buf].size <=
        rbPrussIoDataRam->pru1.control.lightOutput[buf].capacity);
    i = 0;
    for(size_t l = 0; l < RB_NUM_PANELS_PER_STRING; ++l) {
        for(size_t k = 0; k < RB_PANEL_HEIGHT; k += 2) {
            for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
                uint64_t r = 0;
                uint64_t g = 0;
                uint64_t b = 0;
                
                for(size_t m = 0; m < RB_NUM_STRINGS; ++m) {
                    r |= (uint64_t)pFrame->data[
                        RB_NUM_PANELS_PER_STRING * m + l][k][j].r << (m * 8);
                    g |= (uint64_t)pFrame->data[
                        RB_NUM_PANELS_PER_STRING * m + l][k][j].g << (m * 8);
                    b |= (uint64_t)pFrame->data[
                        RB_NUM_PANELS_PER_STRING * m + l][k][j].b << (m * 8);
                }
                // Color order for WS2812 -- 2801 is brg
                *(uint64_t *)&rbPrussIoLightOutputBufs[buf][i +  0] = g;
                *(uint64_t *)&rbPrussIoLightOutputBufs[buf][i +  8] = r;
                *(uint64_t *)&rbPrussIoLightOutputBufs[buf][i + 16] = b;
                i += 3 * 8;
            }
            for(size_t j = RB_PANEL_WIDTH; j > 0; --j) {
                uint64_t r = 0;
                uint64_t g = 0;
                uint64_t b = 0;
                
                for(size_t m = 0; m < RB_NUM_STRINGS; ++m) {
                    r |= (uint64_t)pFrame->data[
                        RB_NUM_PANELS_PER_STRING * m + l][k + 1][j - 1].r <<
                            (m * 8);
                    g |= (uint64_t)pFrame->data[
                        RB_NUM_PANELS_PER_STRING * m + l][k + 1][j - 1].g <<
                            (m * 8);
                    b |= (uint64_t)pFrame->data[
                        RB_NUM_PANELS_PER_STRING * m + l][k + 1][j - 1].b <<
                            (m * 8);
                }
                // Color order for WS2812
                *(uint64_t *)&rbPrussIoLightOutputBufs[buf][i +  0] = g;
                *(uint64_t *)&rbPrussIoLightOutputBufs[buf][i +  8] = r;
                *(uint64_t *)&rbPrussIoLightOutputBufs[buf][i + 16] = b;
                i += 3 * 8;
            }
        }
    }
    
    rbAssert(i == rbPrussIoDataRam->pru1.control.lightOutput[buf].size);
    
    rbPrussIoDataRam->pru1.control.lightOutput[buf].command =
        RB_COMMAND_LIGHT_1W_800KHZ | RB_COMMAND_LIGHT_END_FRAME_PAUSE;
    
    // Ensure all data is committed to memory before kicking off the PRUSS
    rbMemoryBarrier();
    rbPrussIoDataRam->pru1.control.lightOutput[buf].owner = RB_OWNER_PRU1;
    rbMemoryBarrier();
}


#endif
