#include "control_input.h"


#define RB_MIDI_STATUS_FILTER 0x80
#define RB_MIDI_STATUS_REALTIME_FILTER 0xF8

#define RB_MIDI_STATUS_TYPE_MASK 0xF0

#define RB_MIDI_STATUS_TYPE_NOTE_OFF 0x80
#define RB_MIDI_STATUS_TYPE_NOTE_ON 0x90
#define RB_MIDI_STATUS_TYPE_POLYPHONIC_AFTERTOUCH 0xA0
#define RB_MIDI_STATUS_TYPE_CONTROL_CHANGE 0xB0
#define RB_MIDI_STATUS_TYPE_PROGRAM_CHANGE 0xC0
#define RB_MIDI_STATUS_TYPE_CHANNEL_AFTERTOUCH 0xD0
#define RB_MIDI_STATUS_TYPE_PITCH_BEND 0xE0
#define RB_MIDI_STATUS_TYPE_SYSTEM 0xF0

#define RB_MIDI_STATUS_SYSTEM_SYSEX 0xF0
#define RB_MIDI_STATUS_SYSTEM_MTC_QUARTER_FRAME 0xF1
#define RB_MIDI_STATUS_SYSTEM_SONG_POSITION_POINTER 0xF2
#define RB_MIDI_STATUS_SYSTEM_SONG_SELECT 0xF3
#define RB_MIDI_STATUS_SYSTEM_TUNE_REQUEST 0xF6
#define RB_MIDI_STATUS_SYSTEM_EOX 0xF7

#define RB_MIDI_STATUS_RESET 0xFF

#define RB_MIDI_TRIGGER_START_NOTE 0x3C
#define RB_MIDI_CONTROLLER_START_CONTROLLER 0x46
#define RB_MIDI_CONTROLLER_DEBUG_MODE 0x10


RBControlInputMidiParser g_rbMidiParser;


static RBControlInput g_rbControlInput;

static RBControls g_rbHarnessStoredControls;


static void rbControlInputMidiParserParseByteProcessRealtime(
    RBControlInputMidiParser * pParser, uint8_t incomingByte);
static void rbControlInputMidiParserParseByteProcessMessage(
    RBControlInputMidiParser * pParser);


void rbControlInputInitialize(RBConfiguration const * pConfig)
{
    g_rbControlInput = pConfig->controlInput;
    switch(g_rbControlInput) {
    case RBCI_NONE:
    case RBCI_TEST:
        break;
    case RBCI_HARNESS:
        rbControlInputHarnessInitialize(pConfig);
        break;
    case RBCI_BBB_UART4_MIDI:
        rbControlInputBbbUart4MidiInitialize(pConfig);
        break;
    case RBCI_PRUSS_MIDI:
        rbControlInputPrussMidiInitialize(pConfig);
        break;
    default:
        g_rbControlInput = RBCI_INVALID;
    	rbFatal("Invalid control input type %d\n", pConfig->controlInput);
    	break;
    }
}


void rbControlInputShutdown(void)
{
    switch(g_rbControlInput) {
    default:
    case RBCI_NONE:
    case RBCI_TEST:
        break;
    case RBCI_HARNESS:
        rbControlInputHarnessShutdown();
        break;
    case RBCI_BBB_UART4_MIDI:
        rbControlInputBbbUart4MidiShutdown();
        break;
    case RBCI_PRUSS_MIDI:
        rbControlInputPrussMidiShutdown();
        break;
    }
}


void rbControlInputRead(RBControls * pControls)
{
    switch(g_rbControlInput) {
    default:
    case RBCI_NONE:
    case RBCI_TEST:
        rbZero(pControls, sizeof *pControls);
        break;
    case RBCI_HARNESS:
        rbControlInputHarnessRead(pControls);
        break;
    case RBCI_BBB_UART4_MIDI:
        rbControlInputBbbUart4MidiRead(pControls);
        break;
    case RBCI_PRUSS_MIDI:
        rbControlInputPrussMidiRead(pControls);
        break;
    }
}


void rbControlInputMidiParserInitialize(RBControlInputMidiParser * pParser,
    RBConfiguration const * pConfig)
{
    pParser->state = RBCIMPS_CLEAR;
    for(size_t i = 0; i < LENGTHOF(pParser->controls.controllers); ++i) {
        pParser->controls.controllers[i] = 0.0f;
    }
    for(size_t i = 0; i < LENGTHOF(pParser->controls.triggers); ++i) {
        pParser->controls.triggers[i] = false;
    }
    // Brightness control defaults to full on -- hax
    pParser->controls.controllers[0] = 1.0f;
    pParser->controls.debugDisplayReset = true;
    pParser->controls.debugDisplayMode =
        ((pConfig->mode < 0) || (pConfig->mode >= RBDM_COUNT)) ? 0 :
            pConfig->mode;
}


RBControls const * rbControlInputMidiParserGetControls(
    RBControlInputMidiParser * pParser)
{
    return &pParser->controls;
}


void rbControlInputMidiParserResetControls(RBControlInputMidiParser * pParser)
{
    for(size_t i = 0; i < RB_NUM_TRIGGERS; ++i) {
        pParser->controls.triggers[i] = false;
    }
    pParser->controls.debugDisplayReset = false;
}


void rbControlInputMidiParserParseByte(RBControlInputMidiParser * pParser,
    uint8_t incomingByte)
{
    //rbInfo("Parsing byte: %02X\n", incomingByte);
    
    if((incomingByte & RB_MIDI_STATUS_FILTER) == RB_MIDI_STATUS_FILTER) {
        // Realtime messages are out-of-band -- they don't affect state or
        // processing otherwise.
        if((incomingByte & RB_MIDI_STATUS_REALTIME_FILTER) ==
                    RB_MIDI_STATUS_REALTIME_FILTER) {
            rbControlInputMidiParserParseByteProcessRealtime(pParser,
                incomingByte);
            // Remain in current state, unless that process call changed it!
            // Don't try to reparse the current message... we already did that
            // when the last byte was received.
            return;
        }
        else {
            // Resync
            if((pParser->state != RBCIMPS_CLEAR) &&
                    (pParser->state != RBCIMPS_RUNNING_STATUS) &&
                    !((pParser->state == RBCIMPS_SYSEX) &&
                        (incomingByte == RB_MIDI_STATUS_SYSTEM_EOX))) {
                rbDebugInfo("MIDI status interrupting message!\n");
            }
            if((incomingByte & RB_MIDI_STATUS_TYPE_MASK) ==
                    RB_MIDI_STATUS_TYPE_SYSTEM) {
                switch(incomingByte) {
                case RB_MIDI_STATUS_SYSTEM_SYSEX:
                    pParser->state = RBCIMPS_SYSEX;
                    break;
                default:
                case RB_MIDI_STATUS_SYSTEM_EOX:
                    pParser->state = RBCIMPS_CLEAR;
                    break;
                }
            }
            else {
                pParser->state = RBCIMPS_STATUS;
                pParser->message[0] = incomingByte;
            }
        }
    }
    else {
        switch(pParser->state) {
        case RBCIMPS_CLEAR:
            // Hmmm, out of sync?
            rbDebugInfo("MIDI message without status!\n");
            break;
        case RBCIMPS_STATUS:
        case RBCIMPS_RUNNING_STATUS:
            pParser->state = RBCIMPS_PARAM_0;
            pParser->message[1] = incomingByte;
            break;
        case RBCIMPS_PARAM_0:
            pParser->state = RBCIMPS_PARAM_1;
            pParser->message[2] = incomingByte;
            break;
        default:
            rbFatal("Invalid MIDI parser state\n");
            break;
        }
    }
    
    rbControlInputMidiParserParseByteProcessMessage(pParser);
}


void rbControlInputMidiParserParseByteProcessRealtime(
    RBControlInputMidiParser * pParser, uint8_t incomingByte)
{
    if(incomingByte == RB_MIDI_STATUS_RESET) {
        pParser->state = RBCIMPS_CLEAR;
        
        for(size_t i = 0; i < RB_NUM_CONTROLLERS; ++i) {
            pParser->controls.controllers[i] = 0.0f;
        }
        for(size_t i = 0; i < RB_NUM_TRIGGERS; ++i) {
            pParser->controls.triggers[i] = false;
        }
    }
}


void rbControlInputMidiParserParseByteProcessMessage(
    RBControlInputMidiParser * pParser)
{
    uint8_t status = pParser->message[0];
    
    switch(pParser->state) {
    default:
    case RBCIMPS_CLEAR:
    case RBCIMPS_SYSEX:
        // Nothing to process!
        break;
    case RBCIMPS_STATUS:
    case RBCIMPS_RUNNING_STATUS:
        if((status & RB_MIDI_STATUS_TYPE_MASK) ==
                RB_MIDI_STATUS_TYPE_SYSTEM) {
            switch(status) {
            // SYSEX should already have been caught!
            case RB_MIDI_STATUS_SYSTEM_MTC_QUARTER_FRAME:
            case RB_MIDI_STATUS_SYSTEM_SONG_POSITION_POINTER:
            case RB_MIDI_STATUS_SYSTEM_SONG_SELECT:
                // These messages want 1-2 bytes of payload, wait for more.
                break;
            default:
                // Discard!
                // Don't do running status for zero-payload messages.
                pParser->state = RBCIMPS_CLEAR;
            }
        }
        break;
    case RBCIMPS_PARAM_0:
        switch(status) {
        case RB_MIDI_STATUS_SYSTEM_MTC_QUARTER_FRAME:
        case RB_MIDI_STATUS_SYSTEM_SONG_SELECT:
            // Discard!
            pParser->state = RBCIMPS_RUNNING_STATUS;
            break;
        case RB_MIDI_STATUS_SYSTEM_SONG_POSITION_POINTER:
            // This message wants 2 bytes of payload, wait for more.
            break;
        default:
            // Zero-payload system messages should have all been caught!
            if((status & RB_MIDI_STATUS_TYPE_MASK) ==
                    RB_MIDI_STATUS_TYPE_CHANNEL_AFTERTOUCH)
            {
                pParser->state = RBCIMPS_RUNNING_STATUS;
            }
            // Everything else wants more payload.
        }
        break;
    case RBCIMPS_PARAM_1:
        switch(status & RB_MIDI_STATUS_TYPE_MASK) {
        case RB_MIDI_STATUS_TYPE_NOTE_ON:
            // message[2] == velocity; if 0, this is actually NOTE_OFF...
            if(pParser->message[2] != 0) {
                if((pParser->message[1] >= RB_MIDI_TRIGGER_START_NOTE) &&
                        (pParser->message[1] < (RB_MIDI_TRIGGER_START_NOTE +
                            RB_NUM_TRIGGERS))) {
                    pParser->controls.triggers[pParser->message[1] -
                        RB_MIDI_TRIGGER_START_NOTE] = true;
                    rbInfo("Trigger %d triggered\n",
                        pParser->message[1] - RB_MIDI_TRIGGER_START_NOTE);
                }
            }
            break;
        case RB_MIDI_STATUS_TYPE_CONTROL_CHANGE:
            {
                uint8_t midiCont = pParser->message[1];
                uint8_t midiVal = pParser->message[2];
                if((midiCont >= RB_MIDI_CONTROLLER_START_CONTROLLER) &&
                        (midiCont < (RB_MIDI_CONTROLLER_START_CONTROLLER +
                            RB_NUM_TRIGGERS))) {
                    size_t controller =
                        midiCont - RB_MIDI_CONTROLLER_START_CONTROLLER;
                    float value = (float)midiVal * 2.0f / 127.0f - 1.0f;
                    
                    pParser->controls.controllers[controller] = value;
                    rbInfo("Controller %d change: %.2f\n", controller, value);
                }
                else if(midiCont == RB_MIDI_CONTROLLER_DEBUG_MODE) {
                    pParser->controls.debugDisplayReset = true;
                    pParser->controls.debugDisplayMode =
                       (RBDebugDisplayMode)(midiVal * RBDM_COUNT / 128);
                    rbInfo("Debug mode change (via controller): %d\n",
                        pParser->controls.debugDisplayMode);
                }
            }
            break;
        default:
            // Ignore.
            break;
        }
        pParser->state = RBCIMPS_RUNNING_STATUS;
        break;
    }
}


#ifndef RB_USE_BBB_UART4_MIDI_CONTROL_INPUT
void rbControlInputBbbUart4MidiInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("BBB UART4 MIDI input not included in this build!");
}


void rbControlInputBbbUart4MidiShutdown(void)
{
}


void rbControlInputBbbUart4MidiRead(RBControls * pControls)
{
    UNUSED(pControls);
}
#endif


#ifndef RB_USE_PRUSS_IO
void rbControlInputPrussMidiInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("PRUSS MIDI input not included in this build!");
}


void rbControlInputPrussMidiShutdown(void)
{
}


void rbControlInputPrussMidiRead(RBControls * pControls)
{
    UNUSED(pControls);
}
#endif


void rbControlInputHarnessInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    
    rbZero(&g_rbHarnessStoredControls, sizeof g_rbHarnessStoredControls);
}


void rbControlInputHarnessShutdown(void)
{
}


void rbControlInputHarnessRead(RBControls * pControls)
{
    *pControls = g_rbHarnessStoredControls;
    
    for(size_t i = 0; i < RB_NUM_TRIGGERS; ++i) {
        g_rbHarnessStoredControls.triggers[i] = false;
    }
    g_rbHarnessStoredControls.debugDisplayReset = false;
}


RBControls * rbControlInputHarnessGetStoredControls(void)
{
    return &g_rbHarnessStoredControls;
}


