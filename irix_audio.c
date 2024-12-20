// IRIX Audio Library
// Extracted from RtAudio library
// Original Copyright (c) 2001-2005 Gary P. Scavone

#include "irix_audio.h"
#include <dmedia/audio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// Error handling
static char last_error_message[1024] = {0};

const char* irix_audio_get_last_error() {
    return last_error_message;
}

// Internal device structure 
typedef struct {
    long output_resource;
    long input_resource;
    int max_output_channels;
    int max_input_channels;
    int min_output_channels;
    int min_input_channels;
    int *sample_rates;
    int sample_rates_count;
    unsigned int native_formats;
} IrixAudioDevice;

// Global device list
static IrixAudioDevice* devices = NULL;
static int num_devices = 0;

// Initialize audio devices
int irix_audio_initialize() {
    int result, outs, ins, i;
    ALvalue *vls = NULL;

    // Count total number of devices
    num_devices = alQueryValues(AL_SYSTEM, AL_DEVICES, 0, 0, 0, 0);
    if (num_devices < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Error counting devices: %s", alGetErrorString(oserror()));
        return -1;
    }

    if (num_devices <= 0) return 0;

    // Allocate device array
    devices = calloc(num_devices, sizeof(IrixAudioDevice));
    vls = (ALvalue *) malloc(num_devices * sizeof(ALvalue));

    // Get output devices
    outs = alQueryValues(AL_SYSTEM, AL_DEFAULT_OUTPUT, vls, num_devices, 0, 0);
    if (outs < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Error getting output devices: %s", alGetErrorString(oserror()));
        free(vls);
        return -1;
    }

    // Process output devices
    for (i = 0; i < outs; i++) {
        IrixAudioDevice* device = &devices[i];
        device->output_resource = vls[i].i;

        // Get channel information
        ALvalue channels_value;
        if (alQueryValues(device->output_resource, AL_CHANNELS, &channels_value, 1, 0, 0) < 0) {
            snprintf(last_error_message, sizeof(last_error_message), 
                     "Error getting output channels: %s", alGetErrorString(oserror()));
            continue;
        }
        device->max_output_channels = channels_value.i;
        device->min_output_channels = 1;

        // Get sample rates
        ALparamInfo rate_info;
        if (alGetParamInfo(device->output_resource, AL_RATE, &rate_info) < 0) {
            snprintf(last_error_message, sizeof(last_error_message), 
                     "Error getting output sample rates: %s", alGetErrorString(oserror()));
            continue;
        }

        // Allocate and populate sample rates
        int sample_rates[] = {4000, 5512, 8000, 9600, 11025, 16000, 22050,
                               32000, 44100, 48000, 88200, 96000, 176400, 192000};
        device->sample_rates = malloc(sizeof(sample_rates));
        device->sample_rates_count = 0;

        for (int j = 0; j < sizeof(sample_rates)/sizeof(sample_rates[0]); j++) {
            if (sample_rates[j] >= rate_info.min.i && sample_rates[j] <= rate_info.max.i) {
                device->sample_rates[device->sample_rates_count++] = sample_rates[j];
            }
        }

        // Native formats
        device->native_formats = 0x37; // SINT8, SINT16, FLOAT32, FLOAT64
    }

    // Get input devices
    ins = alQueryValues(AL_SYSTEM, AL_DEFAULT_INPUT, &vls[outs], num_devices - outs, 0, 0);
    if (ins < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Error getting input devices: %s", alGetErrorString(oserror()));
        free(vls);
        return -1;
    }

    // Process input devices
    for (i = outs; i < ins + outs; i++) {
        IrixAudioDevice* device = &devices[i];
        device->input_resource = vls[i].i;

        // Similar processing as output devices...
        ALvalue channels_value;
        if (alQueryValues(device->input_resource, AL_CHANNELS, &channels_value, 1, 0, 0) < 0) {
            snprintf(last_error_message, sizeof(last_error_message), 
                     "Error getting input channels: %s", alGetErrorString(oserror()));
            continue;
        }
        device->max_input_channels = channels_value.i;
        device->min_input_channels = 1;

        // Get sample rates
        ALparamInfo rate_info;
        if (alGetParamInfo(device->input_resource, AL_RATE, &rate_info) < 0) {
            snprintf(last_error_message, sizeof(last_error_message), 
                     "Error getting input sample rates: %s", alGetErrorString(oserror()));
            continue;
        }

        // Allocate and populate sample rates
        int sample_rates[] = {4000, 5512, 8000, 9600, 11025, 16000, 22050,
                               32000, 44100, 48000, 88200, 96000, 176400, 192000};
        device->sample_rates = malloc(sizeof(sample_rates));
        device->sample_rates_count = 0;

        for (int j = 0; j < sizeof(sample_rates)/sizeof(sample_rates[0]); j++) {
            if (sample_rates[j] >= rate_info.min.i && sample_rates[j] <= rate_info.max.i) {
                device->sample_rates[device->sample_rates_count++] = sample_rates[j];
            }
        }

        // Native formats
        device->native_formats = 0x37; // SINT8, SINT16, FLOAT32, FLOAT64
    }

    free(vls);
    return num_devices;
}

// Get device count
int irix_audio_get_device_count() {
    return num_devices;
}

// Get device information
int irix_audio_get_device_info(int device_index, IrixAudioDeviceInfo* info) {
    if (device_index < 0 || device_index >= num_devices) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Invalid device index: %d", device_index);
        return -1;
    }

    IrixAudioDevice* device = &devices[device_index];
    
    // Clear previous info
    memset(info, 0, sizeof(IrixAudioDeviceInfo));

    // Output device info
    if (device->output_resource > 0) {
        info->max_output_channels = device->max_output_channels;
        info->min_output_channels = device->min_output_channels;
    }

    // Input device info 
    if (device->input_resource > 0) {
        info->max_input_channels = device->max_input_channels;
        info->min_input_channels = device->min_input_channels;
    }

    // Sample rates
    info->sample_rates = malloc(device->sample_rates_count * sizeof(int));
    memcpy(info->sample_rates, device->sample_rates, device->sample_rates_count * sizeof(int));
    info->sample_rates_count = device->sample_rates_count;

    // Native formats
    info->native_formats = device->native_formats;

    return 0;
}

// Open an audio stream
IrixAudioStream* irix_audio_open_stream(IrixAudioStreamParams* params) {
    ALconfig al_config;
    ALport port;
    long resource;
    ALpv pvs[2];

    // Validate parameters
    if (!params || params->channels <= 0 || params->sample_rate <= 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Invalid stream parameters");
        return NULL;
    }

    // Get a new ALconfig structure
    al_config = alNewConfig();
    if (!al_config) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Cannot create AL config: %s", alGetErrorString(oserror()));
        return NULL;
    }

    // Set channels
    if (alSetChannels(al_config, params->channels) < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Cannot set %d channels: %s", params->channels, alGetErrorString(oserror()));
        alFreeConfig(al_config);
        return NULL;
    }

    // Select resource based on mode
    resource = (params->mode == IRIX_AUDIO_OUTPUT) ? AL_DEFAULT_OUTPUT : AL_DEFAULT_INPUT;

    // Open the port
    const char* port_mode = (params->mode == IRIX_AUDIO_OUTPUT) ? "w" : "r";
    port = alOpenPort("Irix Audio Port", port_mode, al_config);
    if (!port) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Cannot open audio port: %s", alGetErrorString(oserror()));
        alFreeConfig(al_config);
        return NULL;
    }

    // Allocate stream structure
    IrixAudioStream* stream = malloc(sizeof(IrixAudioStream));
    stream->port = port;
    stream->mode = params->mode;
    stream->channels = params->channels;
    stream->sample_rate = params->sample_rate;
    stream->buffer_size = params->buffer_size;

    // Set sample rate
    pvs[0].param = AL_MASTER_CLOCK;
    pvs[0].value.i = AL_CRYSTAL_MCLK_TYPE;
    pvs[1].param = AL_RATE;
    pvs[1].value.ll = alDoubleToFixed((double)params->sample_rate);
    
    if (alSetParams(resource, pvs, 2) < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Cannot set sample rate: %s", alGetErrorString(oserror()));
        alClosePort(port);
        free(stream);
        return NULL;
    }

    alFreeConfig(al_config);
    return stream;
}

// Write audio frames
int irix_audio_write_frames(IrixAudioStream* stream, void* buffer, int frames) {
    if (!stream || stream->mode != IRIX_AUDIO_OUTPUT) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Invalid stream or mode");
        return -1;
    }

    int written = alWriteFrames(stream->port, buffer, frames);
    if (written < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Error writing frames: %s", alGetErrorString(oserror()));
    }

    return written;
}

// Read audio frames
int irix_audio_read_frames(IrixAudioStream* stream, void* buffer, int frames) {
    if (!stream || stream->mode != IRIX_AUDIO_INPUT) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Invalid stream or mode");
        return -1;
    }

    int read = alReadFrames(stream->port, buffer, frames);
    if (read < 0) {
        snprintf(last_error_message, sizeof(last_error_message), 
                 "Error reading frames: %s", alGetErrorString(oserror()));
    }

    return read;
}

// Close an audio stream
void irix_audio_close_stream(IrixAudioStream* stream) {
    if (!stream) return;

    if (stream->port) {
        alClosePort(stream->port);
    }
    free(stream);
}

// Cleanup
void irix_audio_cleanup() {
    if (!devices) return;

    for (int i = 0; i < num_devices; i++) {
        free(devices[i].sample_rates);
    }
    free(devices);
    devices = NULL;
    num_devices = 0;
}
