#ifdef RB_USE_PRUSS_IO


#include "pruss_io.h"

#include "control_input.h"
#include "graphics_util.h"

#include <prussdrv.h>
#include <pruss_intc_mapping.h>


#define RB_PRUSS_IO_DEVICE_STARTUP_COMMAND \
    "sh -c \"modprobe uio_pruss && " \
    "(grep -q rb-pruss-io /sys/devices/bone_capemgr.9/slots || " \
    "echo rb-pruss-io > /sys/devices/bone_capemgr.9/slots)\""


#define RB_PRUSS_IO_BUSY_WAIT_MS 10


#define RB_SHAREDRAM_PRU_BASE 0x00010000
#define RB_DATARAM_SIZE (1024 * 8)
#define RB_SHAREDRAM_SIZE (1024 * 12)


#define RB_MAKE_HOST_ADDRESS_DATARAM0(offset) \
    (g_rbPrussIoDataRam->pru0.raw + (offset))
#define RB_MAKE_HOST_ADDRESS_DATARAM1(offset) \
    (g_rbPrussIoDataRam->pru1.raw + (offset))
#define RB_MAKE_HOST_ADDRESS_SHAREDRAM(offset) (g_rbPrussIoSharedRam + (offset))
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

#define RB_PRUSS_IO_AUDIO_SCALE (2.0f / 4095.0f)


#define RB_MODE_INIT  0
#define RB_MODE_PAUSE 1
#define RB_MODE_RUN   2
#define RB_MODE_HALT  3

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

#define RB_STATUS_NOMINAL           0
#define RB_STATUS_ERROR_COMMAND     0x00000001
#define RB_STATUS_ERROR_OVERRUN     0x00000002
#define RB_STATUS_ERROR_ADC_DESYNC  0x00000004
#define RB_STATUS_ERROR_ADC_TIMEOUT 0x00000008
#define RB_STATUS_ERROR_ADC_NO_DATA 0x00000010


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


static bool g_rbPrussIoPrussRunning = false;
static bool g_rbPrussIoAudioInputRunning = false;
static bool g_rbPrussIoMidiInputRunning = false;
static bool g_rbPrussIoLightOutputRunning = false;

static RBPrussIoMemoryMap * g_rbPrussIoDataRam;
static uint8_t * g_rbPrussIoSharedRam;
static uint8_t * g_rbPrussIoAudioInputBufs[RB_PRUSS_IO_NUM_BUFFERS];
static uint8_t * g_rbPrussIoMidiInputBufs[RB_PRUSS_IO_NUM_BUFFERS];
static uint8_t * g_rbPrussIoLightOutputBufs[RB_PRUSS_IO_NUM_BUFFERS];

static uint32_t g_rbPrussIoNextInputFrameNum;
static uint32_t g_rbPrussIoNextOutputFrameNum;

static size_t g_rbPrussIoCapturedMidiSize;
static uint8_t g_rbPrussIoCapturedMidi[RB_MIDI_MAX_CHARS_PER_VIDEO_FRAME];
static uint16_t g_rbPrussIoCapturedAudio[RB_AUDIO_FRAMES_PER_VIDEO_FRAME][
    RB_PRUSS_IO_AUDIO_CHANNELS];

static RBControls g_rbLastControls;
static RBControlInputMidiParser g_rbMidiParser;


#include "pruss_io_pru0_bin.h"
#include "pruss_io_pru1_bin.h"


static void rbPrussIoStartPruss(void);
static void rbPrussIoStartPrussInitializeTransferBuffers(void);
static void rbPrussIoStopPruss(void);
static void rbPrussIoStopPrussIfNotInUse(void);
static void rbPrussIoStartPrussInput(void);
static void rbPrussIoStopPrussInputIfNotInUse(void);


void rbPrussIoInitialize(RBConfiguration * pConfig)
{
    UNUSED(pConfig);
    
    rbPrussIoShutdown();
    
    g_rbPrussIoPrussRunning = false;
    g_rbPrussIoAudioInputRunning = false;
    g_rbPrussIoMidiInputRunning = false;
    g_rbPrussIoLightOutputRunning = false;
    
    g_rbPrussIoNextInputFrameNum = 0;
    g_rbPrussIoNextOutputFrameNum = 0;
}


void rbPrussIoShutdown(void)
{
    rbPrussIoStopPruss();
}


void rbPrussIoStartPruss(void)
{
    int ret;
    
    if(g_rbPrussIoPrussRunning) {
        return;
    }
    
    ret = system(RB_PRUSS_IO_DEVICE_STARTUP_COMMAND);
    if(ret != 0) {
        rbWarning("Failed to start up PRUSS\n");
    }

    tpruss_intc_initdata prussIntCInitData = PRUSS_INTC_INITDATA;

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
    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void * *)&g_rbPrussIoDataRam);
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void * *)&g_rbPrussIoSharedRam);
    
    rbPrussIoStartPrussInitializeTransferBuffers();
    rbMemoryBarrier();
    
    rbInfo("Executing PRU code\n");
    
    prussdrv_exec_code(1, rbPrussIoPru1Code, sizeof rbPrussIoPru1Code);
    // Wait for init to be complete! Due to code history, PRU1 inits some
    // global PRU control stuff...
    while(g_rbPrussIoDataRam->pru1.control.status == RB_MODE_INIT) {
        rbSleep(rbTimeFromMs(RB_PRUSS_IO_BUSY_WAIT_MS));
        rbMemoryBarrier();
        rbInfo("Waiting for PRU1: mode=%d, status=%d\n",
            g_rbPrussIoDataRam->pru1.control.mode,
            g_rbPrussIoDataRam->pru1.control.status);
    }
    prussdrv_exec_code(0, rbPrussIoPru0Code, sizeof rbPrussIoPru0Code);
    
    g_rbPrussIoPrussRunning = true;
    g_rbPrussIoAudioInputRunning = false;
    g_rbPrussIoMidiInputRunning = false;
    g_rbPrussIoLightOutputRunning = false;
}


void rbPrussIoStartPrussInitializeTransferBuffers(void)
{
    uint32_t bufferOffset;
    uint32_t bufferCapacity;
    
    // Set up the transfer buffers
    rbZero(RB_MAKE_HOST_ADDRESS_DATARAM0(0), RB_DATARAM_SIZE);
    rbZero(RB_MAKE_HOST_ADDRESS_DATARAM1(0), RB_DATARAM_SIZE);
    rbZero(RB_MAKE_HOST_ADDRESS_SHAREDRAM(0), RB_SHAREDRAM_SIZE);
    
    // PRUs shouldn't be running, but what the heck, init them in "paused" mode
    g_rbPrussIoDataRam->pru0.control.mode = RB_MODE_PAUSE;
    g_rbPrussIoDataRam->pru0.control.status = RB_MODE_INIT;
    g_rbPrussIoDataRam->pru1.control.mode = RB_MODE_PAUSE;
    g_rbPrussIoDataRam->pru1.control.status = RB_MODE_INIT;
    
    rbAssert(RB_PRUSS_IO_NUM_BUFFERS == 2);
    rbAssert(RB_PRUSS_IO_NUM_STRINGS >= RB_MAX_LIGHT_STRINGS);
    rbAssert(RB_PRUSS_IO_AUDIO_CHANNELS == RB_AUDIO_CHANNELS);
    
    // PRU0 structures: PRU0 handles audio and MIDI input
    bufferOffset = sizeof g_rbPrussIoDataRam->pru0.control;
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        RBPrussIoTransferControl * pControl =
            &g_rbPrussIoDataRam->pru0.control.audioInput[i];
        bufferCapacity = RB_AUDIO_FRAMES_PER_VIDEO_FRAME *
            RB_PRUSS_IO_AUDIO_CHANNELS * sizeof (uint16_t);
        pControl->owner = RB_OWNER_PRU0;
        pControl->frameNum = i;
        pControl->command = RB_COMMAND_AUDIO_READ;
        pControl->status = RB_STATUS_NOMINAL;
        pControl->address = RB_MAKE_PRU0_ADDRESS_DATARAM0(bufferOffset);
        g_rbPrussIoAudioInputBufs[i] =
            RB_MAKE_HOST_ADDRESS_DATARAM0(bufferOffset);
        pControl->size = 0;
        pControl->capacity = bufferCapacity;
        rbAssert(bufferOffset + bufferCapacity <= RB_DATARAM_SIZE);
        bufferOffset += bufferCapacity;
    }
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        RBPrussIoTransferControl * pControl =
            &g_rbPrussIoDataRam->pru0.control.midiInput[i];
        bufferCapacity = RB_MIDI_MAX_CHARS_PER_VIDEO_FRAME;
        pControl->owner = RB_OWNER_PRU0;
        pControl->frameNum = i;
        pControl->command = RB_COMMAND_MIDI_READ;
        pControl->status = RB_STATUS_NOMINAL;
        pControl->address = RB_MAKE_PRU0_ADDRESS_DATARAM0(bufferOffset);
        g_rbPrussIoMidiInputBufs[i] =
            RB_MAKE_HOST_ADDRESS_DATARAM0(bufferOffset);
        pControl->size = 0;
        pControl->capacity = bufferCapacity;
        rbAssert(bufferOffset + bufferCapacity <= RB_DATARAM_SIZE);
        bufferOffset += bufferCapacity;
    }
    
    // PRU1 structures: PRU1 handles light output
    bufferOffset = 0;
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        RBPrussIoTransferControl * pControl =
            &g_rbPrussIoDataRam->pru1.control.lightOutput[i];
        bufferCapacity = RB_SHAREDRAM_SIZE / 2;
        rbAssert(bufferCapacity >= RB_MAX_LIGHTS * 3);
        pControl->owner = RB_OWNER_HOST;
        pControl->frameNum = 0;
        pControl->command = RB_COMMAND_LIGHT_IDLE;
        pControl->status = RB_STATUS_NOMINAL;
        pControl->address =
            RB_MAKE_PRU1_ADDRESS_SHAREDRAM((RB_SHAREDRAM_SIZE / 2) * (i + 0));
        g_rbPrussIoLightOutputBufs[i] =
            RB_MAKE_HOST_ADDRESS_SHAREDRAM((RB_SHAREDRAM_SIZE / 2) * (i + 0));
        pControl->size = 0;
        pControl->capacity = bufferCapacity;
        rbAssert(bufferOffset + bufferCapacity <= RB_SHAREDRAM_SIZE);
        bufferOffset += bufferCapacity;
    }
}


void rbPrussIoStopPruss(void)
{
    if(!g_rbPrussIoPrussRunning) {
        return;
    }
    
    g_rbPrussIoDataRam->pru0.control.mode = RB_MODE_HALT;
    g_rbPrussIoDataRam->pru1.control.mode = RB_MODE_HALT;
    rbMemoryBarrier();
    
    rbInfo("Waiting for PRU HALT\n");
    for(;;) {
        rbMemoryBarrier();
        if((g_rbPrussIoDataRam->pru0.control.mode == RB_MODE_HALT) &&
                (g_rbPrussIoDataRam->pru1.control.mode == RB_MODE_HALT)) {
            break;
        }
        rbSleep(rbTimeFromMs(RB_PRUSS_IO_BUSY_WAIT_MS));
    }
    
    rbInfo("Closing PRUSS driver\n");
    prussdrv_pru_disable(0);
    prussdrv_exit();
    
    // Reset variables, safety measure
    g_rbPrussIoPrussRunning = false;
    g_rbPrussIoAudioInputRunning = false;
    g_rbPrussIoMidiInputRunning = false;
    g_rbPrussIoLightOutputRunning = false;
    
    g_rbPrussIoDataRam = NULL;
    g_rbPrussIoSharedRam = NULL;
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        g_rbPrussIoAudioInputBufs[i] = NULL;
        g_rbPrussIoMidiInputBufs[i] = NULL;
        g_rbPrussIoLightOutputBufs[i] = NULL;
    }
}


void rbPrussIoStopPrussIfNotInUse(void)
{
    if(!g_rbPrussIoAudioInputRunning && !g_rbPrussIoMidiInputRunning &&
            !g_rbPrussIoLightOutputRunning) {
        rbPrussIoStopPruss();
    }
}


void rbPrussIoStartPrussInput(void)
{
    rbPrussIoStartPruss();
    
    if(!g_rbPrussIoAudioInputRunning && !g_rbPrussIoMidiInputRunning) {
        g_rbPrussIoDataRam->pru0.control.mode = RB_MODE_RUN;
        rbMemoryBarrier();
    }
    // It's up to the caller to set one of the "*Running" flags so we actually
    // STAY running after the next "Stop*IfNotInUse" calls...
}


void rbPrussIoStopPrussInputIfNotInUse(void)
{
    if(!g_rbPrussIoAudioInputRunning && !g_rbPrussIoMidiInputRunning) {
        g_rbPrussIoDataRam->pru0.control.mode = RB_MODE_PAUSE;
        rbMemoryBarrier();
    }
    rbPrussIoStopPrussIfNotInUse();
}


void rbPrussIoReadInput(void)
{
    uint32_t oldestBufFrameNum;
    size_t oldestBuf = RB_PRUSS_IO_NUM_BUFFERS;
    RBPrussIoTransferControl * pControl;
    
    if(!g_rbPrussIoAudioInputRunning && !g_rbPrussIoMidiInputRunning) {
        return;
    }
    
    do {
        rbMemoryBarrier();
        for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
            if(g_rbPrussIoDataRam->pru0.control.audioInput[i].owner ==
                    RB_OWNER_HOST) {
                // We are only looking for the *oldest* buffer.
                //TODO that frameNum < ... test is slightly bogus, it will do
                // the wrong thing when frameNum overflows
                if((oldestBuf == RB_PRUSS_IO_NUM_BUFFERS) ||
                        (g_rbPrussIoDataRam->pru0.control.audioInput[i]
                            .frameNum < oldestBufFrameNum)) {
                    oldestBuf = i;
                    oldestBufFrameNum = g_rbPrussIoDataRam->pru0.control
                        .audioInput[i].frameNum;
                }
            }
        }
        if(oldestBuf == RB_PRUSS_IO_NUM_BUFFERS) {
            //TODO use PRU interrupts here for min latency
            /*
            prussdrv_pru_wait_event(PRU_EVTOUT_0);
            prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
            */
            rbSleep(rbTimeFromMs(1));
        }
    } while(oldestBuf == RB_PRUSS_IO_NUM_BUFFERS);
    
    // Gather status flags
    pControl = &g_rbPrussIoDataRam->pru0.control.audioInput[oldestBuf];
    if(pControl->status != RB_STATUS_NOMINAL) {
        rbWarning("PRUSS audio reports error: %s (0x%X)\n",
            ((pControl->status & RB_STATUS_ERROR_OVERRUN) != 0) ? "overrun" :
                "??",
            pControl->status);
    }
    
    {
        char bb[10000];
        char * pb = bb;
        size_t size = (pControl->size > 20)?20:pControl->size;
        for(size_t i = 0; i < size; ++i) {
            pb += snprintf(pb, sizeof bb - (pb - bb), "%02X ", g_rbPrussIoAudioInputBufs[oldestBuf][i]);
        }
        rbInfo("Input audio[%d]: %s\n", pControl->size, bb);
    }
    
    rbInfo("Reading buffer %d: %d bytes/capacity %d\n", oldestBuf, pControl->size, pControl->capacity);
    rbAssert(pControl->size == sizeof g_rbPrussIoCapturedAudio);
    memcpy(g_rbPrussIoCapturedAudio, g_rbPrussIoAudioInputBufs[oldestBuf],
        sizeof g_rbPrussIoCapturedAudio);
    pControl->status = RB_STATUS_NOMINAL;
    pControl->command = RB_COMMAND_AUDIO_READ;
    pControl->frameNum = g_rbPrussIoNextInputFrameNum++;
    pControl->size = 0;
    rbMemoryBarrier();
    pControl->owner = RB_OWNER_PRU0;
    
    //TODO implement
    //TODO Ideally, we would capture multiple frames 
    UNUSED(g_rbPrussIoCapturedMidi);
    g_rbPrussIoCapturedMidiSize = 0;
}


void rbLightOutputPrussInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbPrussIoStartPruss();
    
    if(!g_rbPrussIoLightOutputRunning) {
        g_rbPrussIoDataRam->pru1.control.mode = RB_MODE_RUN;
        rbMemoryBarrier();
    }
    
    g_rbPrussIoLightOutputRunning = true;
}


void rbLightOutputPrussShutdown(void)
{
    if(g_rbPrussIoLightOutputRunning) {
        g_rbPrussIoDataRam->pru1.control.mode = RB_MODE_PAUSE;
        rbMemoryBarrier();
    }
    g_rbPrussIoLightOutputRunning = false;
    
    rbPrussIoStopPrussIfNotInUse();
}


void rbLightOutputPrussShowLights(RBRawLightFrame const * pFrame)
{
    size_t const numLightStrings = pFrame->numLightStrings;
    size_t const numLightsPerString = pFrame->numLightsPerString;
    size_t buf;
    size_t i;
    
    rbAssert(numLightStrings <= RB_PRUSS_IO_NUM_STRINGS);
    rbAssert(g_rbPrussIoLightOutputRunning);
    
    rbAssert(numLightsPerString * numLightStrings > 0);
    
    rbMemoryBarrier();
    // Check for free buffers
    for(size_t i = 0; i < RB_PRUSS_IO_NUM_BUFFERS; ++i) {
        if(g_rbPrussIoDataRam->pru1.control.lightOutput[i].owner ==
                RB_OWNER_HOST) {
            if(g_rbPrussIoDataRam->pru1.control.lightOutput[i].status !=
                    RB_STATUS_NOMINAL) {
                rbWarning("PRUSS light output reports error (frame %d): %s "
                        "(%X)\n",
                    g_rbPrussIoDataRam->pru1.control.lightOutput[i].frameNum,
                    (g_rbPrussIoDataRam->pru1.control.lightOutput[i].status ==
                        RB_STATUS_ERROR_OVERRUN) ? "overrun" : "??",
                    g_rbPrussIoDataRam->pru1.control.lightOutput[i].status);
                g_rbPrussIoDataRam->pru1.control.lightOutput[i].status =
                    RB_STATUS_NOMINAL;
            }
        }
    }
    
    for(buf = 0; buf < RB_PRUSS_IO_NUM_BUFFERS; ++buf) {
        if(g_rbPrussIoDataRam->pru1.control.lightOutput[buf].owner ==
                RB_OWNER_HOST) {
            break;
        }
    }
    
    if(buf >= RB_PRUSS_IO_NUM_BUFFERS) {
        rbWarning("PRUSS frame queue full!\n");
        ++g_rbPrussIoNextOutputFrameNum;
        return;
    }
    
    // Fill output buffer in PRUSS RAM first...
    g_rbPrussIoDataRam->pru1.control.lightOutput[buf].frameNum =
        g_rbPrussIoNextOutputFrameNum++;
    g_rbPrussIoDataRam->pru1.control.lightOutput[buf].size =
        numLightsPerString * RB_PRUSS_IO_NUM_STRINGS * 3;
    rbAssert(g_rbPrussIoDataRam->pru1.control.lightOutput[buf].size <=
        g_rbPrussIoDataRam->pru1.control.lightOutput[buf].capacity);
    
    i = 0;
    for(size_t j = 0; j < numLightsPerString; ++j) {
        uint64_t r = 0;
        uint64_t g = 0;
        uint64_t b = 0;
        
        for(size_t k = 0; k < numLightStrings; ++k) {
            size_t addr = numLightsPerString * k + j;
            size_t shift = k * 8;
            (void)addr;
            r |= (uint64_t)pFrame->data[addr].r << shift;
            g |= (uint64_t)pFrame->data[addr].g << shift;
            b |= (uint64_t)pFrame->data[addr].b << shift;
        }
        
        // Color order for WS2812 is grb -- 2801 is brg
        *(uint64_t *)&g_rbPrussIoLightOutputBufs[buf][i +  0] = g;
        *(uint64_t *)&g_rbPrussIoLightOutputBufs[buf][i +  8] = r;
        *(uint64_t *)&g_rbPrussIoLightOutputBufs[buf][i + 16] = b;
        
        i += 3 * RB_PRUSS_IO_NUM_STRINGS;
    }
    /*
    {
        char bb[10000];
        char * pb = bb;
        for(size_t i = 0; i < g_rbPrussIoDataRam->pru1.control.lightOutput[buf].size; ++i) {
            pb += snprintf(pb, sizeof bb - (pb - bb), "%02X ", g_rbPrussIoLightOutputBufs[buf][i]);
        }
        rbInfo("Output data[%d]: %s\n", g_rbPrussIoDataRam->pru1.control.lightOutput[buf].size, bb);
    }
    */
    rbAssert(i == g_rbPrussIoDataRam->pru1.control.lightOutput[buf].size);
    
    g_rbPrussIoDataRam->pru1.control.lightOutput[buf].command =
        RB_COMMAND_LIGHT_1W_800KHZ | RB_COMMAND_LIGHT_END_FRAME_PAUSE;
    
    // Ensure all data is committed to memory before kicking off the PRUSS
    rbMemoryBarrier();
    g_rbPrussIoDataRam->pru1.control.lightOutput[buf].owner = RB_OWNER_PRU1;
    rbMemoryBarrier();
}


void rbAudioInputPrussInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbPrussIoStartPrussInput();
    g_rbPrussIoAudioInputRunning = true;
    rbZero(g_rbPrussIoCapturedAudio, sizeof g_rbPrussIoCapturedAudio);
}


void rbAudioInputPrussShutdown(void)
{
    g_rbPrussIoAudioInputRunning = false;
    rbPrussIoStopPrussInputIfNotInUse();
}


void rbAudioInputPrussBlockingRead(RBRawAudio * pAudio)
{
    for(size_t j = 0; j < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++j) {
        for(size_t i = 0; i < RB_PRUSS_IO_AUDIO_CHANNELS; ++i) {
            pAudio->audio[j][i] = (float)g_rbPrussIoCapturedAudio[j][i] *
                RB_PRUSS_IO_AUDIO_SCALE - 1.0f;
        }
    }
}


void rbControlInputPrussMidiInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbPrussIoStartPrussInput();
    g_rbPrussIoMidiInputRunning = true;
    
    rbZero(&g_rbLastControls, sizeof g_rbLastControls);
    rbControlInputMidiParserInitialize(&g_rbMidiParser);
}


void rbControlInputPrussMidiShutdown(void)
{
    g_rbPrussIoMidiInputRunning = false;
    rbPrussIoStopPrussInputIfNotInUse();
}


void rbControlInputPrussMidiRead(RBControls * pControls)
{
    for(size_t i = 0; i < RB_NUM_TRIGGERS; ++i) {
        g_rbLastControls.triggers[i] = false;
    }
    
    rbAssert(g_rbPrussIoCapturedMidiSize <= sizeof g_rbPrussIoCapturedMidi);
    for(size_t i = 0; i < g_rbPrussIoCapturedMidiSize; ++i) {
        rbControlInputMidiParserParseByte(&g_rbMidiParser,
            &g_rbLastControls, g_rbPrussIoCapturedMidi[i]);
    }
    
    memcpy(pControls, &g_rbLastControls, sizeof *pControls);
}


#endif
