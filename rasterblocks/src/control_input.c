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


static void rbControlInputMidiParserParseByteProcessRealtime(
    RBControlInputMidiParser * pParser, RBControls * pControls,
    uint8_t incomingByte);
static void rbControlInputMidiParserParseByteProcessMessage(
    RBControlInputMidiParser * pParser, RBControls * pControls);


void rbControlInputInitialize(RBConfiguration const * pConfig)
{
    rbControlInputBbbUart4MidiInitialize(pConfig);
}


void rbControlInputShutdown(void)
{
    rbControlInputBbbUart4MidiShutdown();
}


void rbControlInputRead(RBControls * pControls)
{
    rbControlInputBbbUart4MidiRead(pControls);
}


void rbControlInputMidiParserInitialize(RBControlInputMidiParser * pParser)
{
    pParser->state = RBCIMPS_CLEAR;
}


void rbControlInputMidiParserParseByte(RBControlInputMidiParser * pParser,
    RBControls * pControls, uint8_t incomingByte)
{
    //rbInfo("Parsing byte: %02X\n", incomingByte);
    
    if((incomingByte & RB_MIDI_STATUS_FILTER) == RB_MIDI_STATUS_FILTER) {
        // Realtime messages are out-of-band -- they don't affect state or
        // processing otherwise.
        if((incomingByte & RB_MIDI_STATUS_REALTIME_FILTER) ==
                    RB_MIDI_STATUS_REALTIME_FILTER) {
            rbControlInputMidiParserParseByteProcessRealtime(pParser,
                pControls, incomingByte);
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
    
    rbControlInputMidiParserParseByteProcessMessage(pParser, pControls);
}


void rbControlInputMidiParserParseByteProcessRealtime(
    RBControlInputMidiParser * pParser, RBControls * pControls,
    uint8_t incomingByte)
{
    if(incomingByte == RB_MIDI_STATUS_RESET) {
        pParser->state = RBCIMPS_CLEAR;
        
        for(size_t i = 0; i < RB_NUM_CONTROLLERS; ++i) {
            pControls->controllers[i] = 0.0f;
        }
        for(size_t i = 0; i < RB_NUM_TRIGGERS; ++i) {
            pControls->triggers[i] = false;
        }
    }
}


void rbControlInputMidiParserParseByteProcessMessage(
    RBControlInputMidiParser * pParser, RBControls * pControls)
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
                    pControls->triggers[pParser->message[1] -
                        RB_MIDI_TRIGGER_START_NOTE] = true;
                    rbInfo("Trigger %d triggered\n",
                        pParser->message[1] - RB_MIDI_TRIGGER_START_NOTE);
                }
            }
            break;
        case RB_MIDI_STATUS_TYPE_CONTROL_CHANGE:
            if((pParser->message[1] >= RB_MIDI_CONTROLLER_START_CONTROLLER) &&
                    (pParser->message[1] <
                        (RB_MIDI_CONTROLLER_START_CONTROLLER +
                            RB_NUM_TRIGGERS))) {
                pControls->controllers[pParser->message[1] -
                    RB_MIDI_CONTROLLER_START_CONTROLLER] =
                        (float)pParser->message[2] * 2.0f / 127.0f - 1.0f;
                rbInfo("Controller %d change: %.2f\n",
                    pParser->message[1] - RB_MIDI_CONTROLLER_START_CONTROLLER,
                    pControls->controllers[pParser->message[1] -
                        RB_MIDI_CONTROLLER_START_CONTROLLER]);
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
}


void rbControlInputBbbUart4MidiShutdown(void)
{
}


void rbControlInputBbbUart4MidiRead(RBControls * pControls)
{
    UNUSED(pControls);
}
#endif
