#include "irix_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256
#define LOOPBACK_DURATION 5.0  // seconds

int main() {
    // Initialize audio system
    int device_count = irix_audio_initialize();
    if (device_count < 0) {
        fprintf(stderr, "Failed to initialize audio: %s\n", irix_audio_get_last_error());
        return 1;
    }
    printf("Found %d audio devices\n", device_count);

    // Prepare input stream parameters
    IrixAudioStreamParams input_params = {
        .mode = IRIX_AUDIO_INPUT,
        .channels = 1,
        .sample_rate = SAMPLE_RATE,
        .buffer_size = BUFFER_SIZE
    };

    // Prepare output stream parameters
    IrixAudioStreamParams output_params = {
        .mode = IRIX_AUDIO_OUTPUT,
        .channels = 1,
        .sample_rate = SAMPLE_RATE,
        .buffer_size = BUFFER_SIZE
    };

    // Open input and output streams
    IrixAudioStream* input_stream = irix_audio_open_stream(&input_params);
    IrixAudioStream* output_stream = irix_audio_open_stream(&output_params);

    if (!input_stream || !output_stream) {
        fprintf(stderr, "Failed to open streams: %s\n", irix_audio_get_last_error());
        
        if (input_stream) irix_audio_close_stream(input_stream);
        if (output_stream) irix_audio_close_stream(output_stream);
        
        return 1;
    }

    // Prepare buffers
    float input_buffer[BUFFER_SIZE];
    float output_buffer[BUFFER_SIZE];
    int total_frames = (int)(LOOPBACK_DURATION * SAMPLE_RATE);
    int frames_processed = 0;

    printf("Starting audio loopback test for %.2f seconds...\n", LOOPBACK_DURATION);

    while (frames_processed < total_frames) {
        // Determine how many frames to process
        int read_count = (total_frames - frames_processed < BUFFER_SIZE) ? 
                         total_frames - frames_processed : BUFFER_SIZE;

        // Read input
        int read_result = irix_audio_read_frames(input_stream, input_buffer, read_count);
        if (read_result < 0) {
            fprintf(stderr, "Error reading input frames: %s\n", irix_audio_get_last_error());
            break;
        }

        // Immediately write input back to output (loopback)
        int write_result = irix_audio_write_frames(output_stream, input_buffer, read_result);
        if (write_result < 0) {
            fprintf(stderr, "Error writing output frames: %s\n", irix_audio_get_last_error());
            break;
        }

        frames_processed += read_result;
    }

    // Cleanup
    irix_audio_close_stream(input_stream);
    irix_audio_close_stream(output_stream);
    irix_audio_cleanup();

    printf("Loopback test completed. Processed %d frames.\n", frames_processed);
    return 0;
}
