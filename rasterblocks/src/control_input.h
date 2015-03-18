#ifndef CONTROL_INPUT_H_INCLUDED
#define CONTROL_INPUT_H_INCLUDED


#include "rasterblocks.h"


typedef enum {
    RBCIMPS_CLEAR,
    RBCIMPS_STATUS,
    RBCIMPS_PARAM_0,
    RBCIMPS_PARAM_1,
    RBCIMPS_SYSEX,
} RBControlInputMidiParserState;


typedef struct {
    RBControlInputMidiParserState state;
    uint8_t message[3];
} RBControlInputMidiParser;


// Control input subsystem
void rbControlInputInitialize(RBConfiguration const * pConfig);
void rbControlInputShutdown(void);
void rbControlInputRead(RBControls * pControls);

void rbControlInputMidiParserInitialize(RBControlInputMidiParser * pParser);
void rbControlInputMidiParserParseByte(RBControlInputMidiParser * pParser,
    RBControls * pControls, uint8_t incomingByte);

void rbControlInputBbbUart4MidiInitialize(RBConfiguration const * pConfig);
void rbControlInputBbbUart4MidiShutdown(void);
void rbControlInputBbbUart4MidiRead(RBControls * pControls);


#endif

