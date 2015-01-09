#include "configuration_json.h"

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
    if(rbStricmp(val, "FILE") == 0) {
        return RBAI_FILE;
    }
    else if(rbStricmp(val, "OPENAL") == 0) {
        return RBAI_OPENAL;
    }
    else if(rbStricmp(val, "ALSA") == 0) {
        return RBAI_ALSA;
    }
    else {
        rbError("Invalid audio source type: %s\n",val);
        return RBAI_INVALID;
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
    else {
        rbError("Invalid audio source type: %s\n",val);
        return RBLO_INVALID;
    }
}

static void parseJsonObject(RBConfiguration * config,json_object * jobj)
{
    json_object_object_foreach(jobj, key, val) {
        const char* s_val = json_object_get_string(val);
        if(strcmp(key, "logLevel") == 0) {
            config->logLevel = getLogLevel(s_val);
        }
        else if(strcmp(key, "audioInput") == 0) {
            config->audioInput = getAudioInput(s_val);
        }
        else if(strcmp(key, "audioInputParam") == 0) {
            rbStrlcpy(config->audioInputParam, s_val,
                sizeof config->audioInputParam);
        }
        else if(strcmp(key, "lightOutput") == 0) {
            config->lightOutput = getLightOutput(s_val);
        }
        else if(strcmp(key, "lightOutputParam") == 0) {
            rbStrlcpy(config->lightOutputParam, s_val,
                sizeof config->lightOutputParam);
        }
        else if (strcmp(key, "lowCutoff") == 0) {
            config->lowCutoff = atof(json_object_get_string(val));
        }
        else if (strcmp(key, "hiCutoff") == 0) {
            config->hiCutoff = atof(json_object_get_string(val));
        }
        else if (strcmp(key, "agcMax") == 0) {
            config->agcMax = atof(json_object_get_string(val));
        }
        else if (strcmp(key, "agcMin") == 0) {
            config->agcMin = atof(json_object_get_string(val));
        }
        else if (strcmp(key, "agcStrength") == 0) {
            config->agcStrength = atof(json_object_get_string(val));
        }
        else if (strcmp(key, "brightness") == 0) {
            config->brightness = atof(json_object_get_string(val));
        }
    }
}

void rbParseJson(RBConfiguration * config, char* filename) {
    char * buffer = 0;
    size_t length;
    FILE * f = fopen (filename, "rb");

    if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer) {
            size_t result = fread (buffer, 1, length, f);
            if(result!=length) {
                rbError("Error reading %s\n",filename);
            }
        }
        fclose (f);
    }

    if (buffer) {
        json_object * jobj = json_tokener_parse(buffer);
        if(jobj != NULL) {
            parseJsonObject(config,jobj);
        }
        else {
	    rbError("Failed to parse %s\n", filename);
	}
    }
    else {
        rbError("Failed to read %s\n", filename);
    }
}
