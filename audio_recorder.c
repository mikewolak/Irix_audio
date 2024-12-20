#include "irix_audio.h"
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256
#define RECORD_DURATION 5.0  // seconds

int main() {
    // Initialize audio system
    int device_count = irix_audio_initialize();
    if (device_count < 0) {
        fprintf(stderr, "Failed to initialize audio: %s\n", irix_audio_get_last_error());
        return 1;
    }
    printf("Found %d audio devices\n", device_count);

    // Print device information
    for (int i = 0; i < device_count; i++) {
        IrixAudioDeviceInfo info;
        if (irix_audio_get_device_info(i, &info) == 0) {
            printf("Device %d:\n", i);
            printf("  Max Input Channels: %d\n", info.max_input_channels);
            printf("  Max Output Channels: %d\n", info.max_output_channels);
            printf("  Supported Sample Rates:");
            for (int j = 0; j < info.sample_rates_count; j++) {
                printf(" %d", info.sample_rates[j]);
            }
            printf("\n");
            free(info.sample_rates);
        }
    }

    // Prepare stream parameters
    IrixAudioStreamParams params = {
        .mode = IRIX_AUDIO_INPUT,
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

    // Prepare recording
    float buffer[BUFFER_SIZE];
    int total_frames = (int)(RECORD_DURATION * SAMPLE_RATE);
    int frames_read = 0;

    // Open output file
    FILE* output_file = fopen("recording.raw", "wb");
    if (!output_file) {
        fprintf(stderr, "Failed to open output file\n");
        irix_audio_close_stream(stream);
        return 1;
    }

    printf("Recording for %.2f seconds...\n", RECORD_DURATION);

    // Record audio
    while (frames_read < total_frames) {
        int read_count = (total_frames - frames_read < BUFFER_SIZE) ? 
                         total_frames - frames_read : BUFFER_SIZE;
        int result = irix_audio_read_frames(stream, buffer, read_count);
        
        if (result < 0) {
            fprintf(stderr, "Error reading audio frames: %s\n", irix_audio_get_last_error());
            break;
        }
        
        // Write to file
        fwrite(buffer, sizeof(float), result, output_file);
        frames_read += result;
    }

    // Cleanup
    fclose(output_file);
    irix_audio_close_stream(stream);
    irix_audio_cleanup();

    printf("Recorded %d frames to recording.raw\n", frames_read);
    return 0;
}
