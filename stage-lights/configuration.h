#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED


#include "stage_lights.h"


// Configurations subsystem
void slConfigurationSetDefaults(SLConfiguration * config);
void slConfigurationParseArgv(SLConfiguration * config, int argc,
    char * argv[]);
void slConfigurationLoad(SLConfiguration * config);
void slConfigurationSave(SLConfiguration const * config);


// Hot configuration subsystem
void slHotConfigurationInitialize(SLConfiguration const * config);
void slHotConfigurationShutdown(void);
void slHotConfigurationProcessAndUpdateConfiguration(SLConfiguration * config,
    bool * configurationModified);


#endif

