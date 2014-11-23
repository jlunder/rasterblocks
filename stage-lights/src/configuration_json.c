#include "configuration_json.h"

// Different include path in different build environments -- TARGET_HARNESS
// distinguishes them now, this is not a great solution though
#ifdef SL_USE_TARGET_HARNESS
#include <json/json.h>
#else
#include <json-c/json.h>
#endif

// see http://linuxprograms.wordpress.com/2010/08/19/json_parser_json-c/

SLLogLevel getLogLevel(const char* val) {
	if(strcmp(val, "SLLL_INFO") == 0) {
		return SLLL_INFO;
	}
	else if(strcmp(val, "SLLL_WARNING") == 0) {
		return SLLL_WARNING;
	}
	else if(strcmp(val, "SLLL_ERROR") == 0) {
		return SLLL_ERROR;
	} 
	else {
		slError("Invalid log level: %s, using SLL_INFO",val);
		return SLLL_INFO;
	}
}
SLAudioInputSource getAudioSource(const char* val) {
	if(strcmp(val, "SLAIS_ALSA") == 0) {
		return SLAIS_ALSA;
	}
	else if(strcmp(val, "SLAIS_FILE") == 0) {
		return SLAIS_FILE;
	}
	else {
		slError("Invalid audio source type: %s",val);
		return SLAIS_INVALID;
	}
}

void parseJsonObject(SLConfiguration * config,json_object * jobj) {
	json_object_object_foreach(jobj, key, val) {
		const char* s_val = json_object_get_string(val);
		if(strcmp(key, "logLevel") == 0) {
			config->logLevel = getLogLevel(s_val);
		}
		else if(strcmp(key, "audioSource") == 0) {
			config->audioSource = getAudioSource(s_val);
		}
		else if(strcmp(key, "audioSourceParam") == 0) {
			strcpy(config->audioSourceParam,s_val);
		}
		else if(strcmp(key, "monitorAudio") == 0) {
			config->monitorAudio = json_object_get_boolean(val);
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
	}
}
void slParseJson(SLConfiguration * config,char* filename) {

	char * buffer = 0;
	size_t length;
	FILE * f = fopen (filename, "rb");

	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = malloc (length);
		if (buffer)
		{
			size_t result = fread (buffer, 1, length, f);
			if(result!=length) {
				slError("Error reading %s",filename);
			}
		}
		fclose (f);
	}

	if (buffer)
	{
		json_object * jobj = json_tokener_parse(buffer);
		parseJsonObject(config,jobj);
	} else {
		slError("Failed to read %s",filename);
	}
}
