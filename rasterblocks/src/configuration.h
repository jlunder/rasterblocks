#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED


#include "rasterblocks.h"


// Configurations subsystem
void rbConfigurationSetDefaults(RBConfiguration * config);
void rbConfigurationParseArgv(RBConfiguration * config, int argc,
    char * argv[]);
void rbConfigurationLoad(RBConfiguration * config);
void rbConfigurationSave(RBConfiguration const * config);


// Hot configuration subsystem
void rbHotConfigurationInitialize(RBConfiguration const * config);
void rbHotConfigurationShutdown(void);
void rbHotConfigurationProcessAndUpdateConfiguration(RBConfiguration * config,
    bool * configurationModified);


#endif

