#ifndef CONTROL_INPUT_H_INCLUDED
#define CONTROL_INPUT_H_INCLUDED


#include "rasterblocks.h"


typedef enum {
    RBCIMPS_CLEAR,
    RBCIMPS_STATUS,
    RBCIMPS_RUNNING_STATUS,
    RBCIMPS_PARAM_0,
    RBCIMPS_PARAM_1,
    RBCIMPS_SYSEX,
} RBControlInputMidiParserState;


typedef struct {
    RBControls controls;
    RBControlInputMidiParserState state;
    uint8_t message[3];
} RBControlInputMidiParser;


// This is available purely for other modules to use - control_input doesn't
// use it by itself.
extern RBControlInputMidiParser g_rbMidiParser;


// Control input subsystem
void rbControlInputInitialize(RBConfiguration const * pConfig);
void rbControlInputShutdown(void);
void rbControlInputRead(RBControls * pControls);

void rbControlInputMidiParserInitialize(RBControlInputMidiParser * pParser,
    RBConfiguration const * pConfig);
RBControls const * rbControlInputMidiParserGetControls(
    RBControlInputMidiParser * pParser);
void rbControlInputMidiParserResetControls(
    RBControlInputMidiParser * pParser);
void rbControlInputMidiParserParseByte(RBControlInputMidiParser * pParser,
    uint8_t incomingByte);

void rbControlInputBbbUart4MidiInitialize(RBConfiguration const * pConfig);
void rbControlInputBbbUart4MidiShutdown(void);
void rbControlInputBbbUart4MidiRead(RBControls * pControls);

void rbControlInputPrussMidiInitialize(RBConfiguration const * pConfig);
void rbControlInputPrussMidiShutdown(void);
void rbControlInputPrussMidiRead(RBControls * pControls);


#endif

