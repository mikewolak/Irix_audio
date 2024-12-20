// IRIX Audio Library Header
// Extracted from RtAudio library
// Original Copyright (c) 2001-2005 Gary P. Scavone

#ifndef IRIX_AUDIO_H
#define IRIX_AUDIO_H

#include <dmedia/audio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Audio stream modes
typedef enum {
    IRIX_AUDIO_INPUT,
    IRIX_AUDIO_OUTPUT,
    IRIX_AUDIO_DUPLEX
} IrixAudioMode;

// Supported audio formats
typedef enum {
    IRIX_AUDIO_SINT8 = 0x1,     // 8-bit signed integer
    IRIX_AUDIO_SINT16 = 0x2,    // 16-bit signed integer
    IRIX_AUDIO_SINT24 = 0x4,    // 24-bit signed integer
    IRIX_AUDIO_SINT32 = 0x8,    // 32-bit signed integer
    IRIX_AUDIO_FLOAT32 = 0x10,  // 32-bit float
    IRIX_AUDIO_FLOAT64 = 0x20   // 64-bit float
} IrixAudioFormat;

// Device information structure
typedef struct {
    int max_output_channels;
    int min_output_channels;
    int max_input_channels;
    int min_input_channels;
    int* sample_rates;
    int sample_rates_count;
    unsigned int native_formats;
} IrixAudioDeviceInfo;

// Stream parameters structure
typedef struct {
    IrixAudioMode mode;
    int channels;
    int sample_rate;
    int buffer_size;
} IrixAudioStreamParams;

// Audio stream structure
typedef struct {
    ALport port;
    IrixAudioMode mode;
    int channels;
    int sample_rate;
    int buffer_size;
} IrixAudioStream;

// Function prototypes
const char* irix_audio_get_last_error();

// Device management
int irix_audio_initialize();
int irix_audio_get_device_count();
int irix_audio_get_device_info(int device_index, IrixAudioDeviceInfo* info);
void irix_audio_cleanup();

// Stream management
IrixAudioStream* irix_audio_open_stream(IrixAudioStreamParams* params);
void irix_audio_close_stream(IrixAudioStream* stream);

// Audio I/O
int irix_audio_write_frames(IrixAudioStream* stream, void* buffer, int frames);
int irix_audio_read_frames(IrixAudioStream* stream, void* buffer, int frames);

#ifdef __cplusplus
}
#endif

#endif // IRIX_AUDIO_H
