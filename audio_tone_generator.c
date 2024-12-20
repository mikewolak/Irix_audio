#include "irix_audio.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256
#define DURATION 5.0  // seconds
#define FREQUENCY 440.0  // A4 note

int main() {
    // Initialize audio system
    int device_count = irix_audio_initialize();
    if (device_count < 0) {
        fprintf(stderr, "Failed to initialize audio: %s\n", irix_audio_get_last_error());
        return 1;
    }
    printf("Found %d audio devices\n", device_count);

    // Prepare stream parameters
    IrixAudioStreamParams params = {
        .mode = IRIX_AUDIO_OUTPUT,
        .channels = 1,
        .sample_rate = SAMPLE_RATE,
        .buffer_size = BUFFER_SIZE
    };

    // Open audio stream
    IrixAudioStream* stream = irix_audio_open_stream(&params);
    if (!stream) {
        fprintf(stderr, "Failed to open stream: %s\n", irix_audio_get_last_error());
        return 1;
    }

    // Generate sine wave
    float buffer[BUFFER_SIZE];
    int total_frames = (int)(DURATION * SAMPLE_RATE);
    int frames_written = 0;

    for (int i = 0; frames_written < total_frames; i += BUFFER_SIZE) {
        // Fill buffer with sine wave
        for (int j = 0; j < BUFFER_SIZE; j++) {
            buffer[j] = 0.5f * sinf(2.0f * M_PI * FREQUENCY * (i + j) / SAMPLE_RATE);
        }

        // Write frames
        int write_count = (total_frames - frames_written < BUFFER_SIZE) ? 
                          total_frames - frames_written : BUFFER_SIZE;
        int result = irix_audio_write_frames(stream, buffer, write_count);
        
        if (result < 0) {
            fprintf(stderr, "Error writing audio frames: %s\n", irix_audio_get_last_error());
            break;
        }
        
        frames_written += result;
    }

    // Cleanup
    irix_audio_close_stream(stream);
    irix_audio_cleanup();

    printf("Played sine wave for %.2f seconds\n", DURATION);
    return 0;
}
