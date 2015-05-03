#include "configuration_json.h"

#include "graphics_util.h"

// Different include path in different build environments -- TARGET_HARNESS
// distinguishes them now, this is not a great solution though
#ifdef RB_USE_TARGET_HARNESS
#include <json/json.h>
#else
#include <json-c/json.h>
#endif

// see http://linuxprograms.wordpress.com/2010/08/19/json_parser_json-c/


static RBLogLevel getLogLevel(const char* val)
{
    if(rbStricmp(val, "INFO") == 0) {
        return RBLL_INFO;
    }
    else if(rbStricmp(val, "WARNING") == 0) {
        return RBLL_WARNING;
    }
    else if(rbStricmp(val, "ERROR") == 0) {
        return RBLL_ERROR;
    } 
    else {
        rbError("Invalid log level: %s, using RBL_INFO",val);
        return RBLL_INFO;
    }
}


static RBAudioInput getAudioInput(const char* val)
{
    if(rbStricmp(val, "TEST") == 0) {
        return RBAI_TEST;
    }
    else if(rbStricmp(val, "FILE") == 0) {
        return RBAI_FILE;
    }
    else if(rbStricmp(val, "OPENAL") == 0) {
        return RBAI_OPENAL;
    }
    else if(rbStricmp(val, "ALSA") == 0) {
        return RBAI_ALSA;
    }
    else if(rbStricmp(val, "PRUSS") == 0) {
        return RBAI_PRUSS;
    }
    else {
        rbError("Invalid audio input type: %s\n",val);
        return RBAI_INVALID;
    }
}


static RBControlInput getControlInput(const char* val)
{
    if(rbStricmp(val, "NONE") == 0) {
        return RBCI_NONE;
    }
    else if(rbStricmp(val, "TEST") == 0) {
        return RBCI_TEST;
    }
    else if(rbStricmp(val, "HARNESS") == 0) {
        return RBCI_TEST;
    }
    else if(rbStricmp(val, "BBB_UART4_MIDI") == 0) {
        return RBCI_BBB_UART4_MIDI;
    }
    else if(rbStricmp(val, "PRUSS_MIDI") == 0) {
        return RBCI_PRUSS_MIDI;
    }
    else {
        rbError("Invalid control input type: %s\n",val);
        return RBCI_INVALID;
    }
}


static RBLightOutput getLightOutput(const char* val)
{
    if(rbStricmp(val, "OPENGL") == 0) {
        return RBLO_OPENGL;
    }
    else if(rbStricmp(val, "PIXELPUSHER") == 0) {
        return RBLO_PIXELPUSHER;
    }
    else if(rbStricmp(val, "SPIDEV") == 0) {
        return RBLO_SPIDEV;
    }
    else if(rbStricmp(val, "PRUSS") == 0) {
        return RBLO_PRUSS;
    }
    else {
        rbError("Invalid light output type: %s\n",val);
        return RBLO_INVALID;
    }
}


static void parsePanelList(RBConfiguration * pConfig, json_object * jobj)
{
    size_t const maxPanels =
        RB_MAX_LIGHTS / (RB_PANEL_WIDTH * RB_PANEL_HEIGHT);
    RBPanelConfig panelConfigs[maxPanels];
    size_t numPanels = json_object_array_length(jobj);
    
    rbVerify(numPanels < maxPanels);
    for(size_t i = 0; i < numPanels; ++i) {
        json_object * jel = json_object_array_get_idx(jobj, i);
        
        if(jel == NULL) {
            continue;
        }
        panelConfigs[i].position = vector2(
            (float)json_object_get_double(json_object_array_get_idx(jel, 0)),
            (float)json_object_get_double(json_object_array_get_idx(jel, 1)));
        panelConfigs[i].uInc = vector2(
            (float)json_object_get_double(json_object_array_get_idx(jel, 2)),
            (float)json_object_get_double(json_object_array_get_idx(jel, 3)));
        panelConfigs[i].vInc = vector2(
            (float)json_object_get_double(json_object_array_get_idx(jel, 4)),
            (float)json_object_get_double(json_object_array_get_idx(jel, 5)));
    }
    rbComputeLightPositionsFromPanelList(pConfig->lightPositions,
        RB_MAX_LIGHTS, panelConfigs, numPanels);
}


static void parseConfigObject(RBConfiguration * pConfig, json_object * jobj)
{
    json_object_object_foreach(jobj, key, val) {
        const char* s_val = json_object_get_string(val);
        if(strcmp(key, "logLevel") == 0) {
            pConfig->logLevel = getLogLevel(s_val);
        }
        else if(strcmp(key, "audioInput") == 0) {
            pConfig->audioInput = getAudioInput(s_val);
        }
        else if(strcmp(key, "controlInput") == 0) {
            pConfig->controlInput = getControlInput(s_val);
        }
        else if(strcmp(key, "audioInputParam") == 0) {
            rbStrlcpy(pConfig->audioInputParam, s_val,
                sizeof pConfig->audioInputParam);
        }
        else if(strcmp(key, "lightOutput") == 0) {
            pConfig->lightOutput = getLightOutput(s_val);
        }
        else if(strcmp(key, "lightOutputParam") == 0) {
            rbStrlcpy(pConfig->lightOutputParam, s_val,
                sizeof pConfig->lightOutputParam);
        }
        else if(strcmp(key, "lowCutoff") == 0) {
            pConfig->lowCutoff = atof(json_object_get_string(val));
        }
        else if(strcmp(key, "hiCutoff") == 0) {
            pConfig->hiCutoff = atof(json_object_get_string(val));
        }
        else if(strcmp(key, "agcMax") == 0) {
            pConfig->agcMax = atof(json_object_get_string(val));
        }
        else if(strcmp(key, "agcMin") == 0) {
            pConfig->agcMin = atof(json_object_get_string(val));
        }
        else if(strcmp(key, "agcStrength") == 0) {
            pConfig->agcStrength = atof(json_object_get_string(val));
        }
        else if(strcmp(key, "mode") == 0) {
            pConfig->mode = atoi(json_object_get_string(val));
        }
        else if(strcmp(key, "brightness") == 0) {
            pConfig->brightness = atof(json_object_get_string(val));
        }
        else if(strcmp(key, "projectionWidth") == 0) {
            pConfig->projectionWidth = atoi(json_object_get_string(val));
        }
        else if(strcmp(key, "projectionHeight") == 0) {
            pConfig->projectionHeight = atoi(json_object_get_string(val));
        }
        else if(strcmp(key, "numLightStrings") == 0) {
            pConfig->numLightStrings = atoi(json_object_get_string(val));
            rbVerify(pConfig->numLightStrings <= RB_MAX_LIGHT_STRINGS);
        }
        else if(strcmp(key, "numLightsPerString") == 0) {
            pConfig->numLightsPerString = atoi(json_object_get_string(val));
            rbVerify(pConfig->numLightsPerString <= RB_MAX_LIGHTS);
        }
        else if(strcmp(key, "panels") == 0) {
            json_object * jel = NULL;
            if(json_object_object_get_ex(jobj, key, &jel)) {
                parsePanelList(pConfig, jel);
            }
        }
    }
}


void rbParseJson(RBConfiguration * pConfig, char* filename) {
    char * buffer = 0;
    size_t length;
    FILE * f = fopen (filename, "rb");

    if(f != NULL) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length + 1);
        if (buffer) {
            size_t result = fread (buffer, 1, length, f);
            if(result!=length) {
                rbError("Error reading %s\n", filename);
            }
            buffer[length] = '\0';
        }
        fclose (f);
    }

    if(buffer != NULL) {
        json_object * jobj = json_tokener_parse(buffer);
        if(jobj != NULL) {
            parseConfigObject(pConfig, jobj);
        }
        else {
            rbError("Failed to parse %s\n", filename);
        }
    }
    else {
        rbError("Failed to read %s\n", filename);
    }
}


