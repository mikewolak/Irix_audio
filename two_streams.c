#include "irix_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Configuration parameters
#define BASE_RATE 0.005
#define TIME 2.0
#define BUFFER_SIZE 512

// Data type configuration
typedef float MY_TYPE;
#define SCALE 1.0

void usage(void) {
    fprintf(stderr, "\nusage: twostreams N fs <device>\n");
    fprintf(stderr, "    where N = number of channels,\n");
    fprintf(stderr, "    fs = the sample rate,\n");
    fprintf(stderr, "    and device = the device to use (default = 0).\n\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int chans, fs, device = 0;
    long frames, counter = 0;
    MY_TYPE *buffer1, *buffer2;
    IrixAudioStream *stream1 = NULL, *stream2 = NULL;
    FILE *fd;
    double *data = NULL;

    // Minimal command-line checking
    if (argc != 3 && argc != 4) usage();

    chans = atoi(argv[1]);
    fs = atoi(argv[2]);
    if (argc == 4)
        device = atoi(argv[3]);

    // Initialize audio system
    int device_count = irix_audio_initialize();
    if (device_count < 0) {
        fprintf(stderr, "Failed to initialize audio: %s\n", irix_audio_get_last_error());
        return EXIT_FAILURE;
    }

    // Prepare output stream parameters
    IrixAudioStreamParams output_params = {
        .mode = IRIX_AUDIO_OUTPUT,
        .channels = chans,
        .sample_rate = fs,
        .buffer_size = BUFFER_SIZE
    };

    // Prepare input stream parameters
    IrixAudioStreamParams input_params = {
        .mode = IRIX_AUDIO_INPUT,
        .channels = chans,
        .sample_rate = fs,
        .buffer_size = BUFFER_SIZE
    };

    // Open output stream
    stream1 = irix_audio_open_stream(&output_params);
    if (!stream1) {
        fprintf(stderr, "Failed to open output stream: %s\n", irix_audio_get_last_error());
        goto cleanup;
    }

    // Open input stream
    stream2 = irix_audio_open_stream(&input_params);
    if (!stream2) {
        fprintf(stderr, "Failed to open input stream: %s\n", irix_audio_get_last_error());
        goto cleanup;
    }

    // Allocate buffers
    buffer1 = malloc(BUFFER_SIZE * chans * sizeof(MY_TYPE));
    buffer2 = malloc(BUFFER_SIZE * chans * sizeof(MY_TYPE));
    data = calloc(chans, sizeof(double));

    if (!buffer1 || !buffer2 || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        goto cleanup;
    }

    // Playback phase
    frames = (long) (fs * TIME);
    printf("\nStarting sawtooth playback stream for %f seconds.\n", TIME);
    
    while (counter < frames) {
        // Generate sawtooth wave
        for (int i = 0; i < BUFFER_SIZE; i++) {
            for (int j = 0; j < chans; j++) {
                buffer1[i*chans+j] = (MY_TYPE) (data[j] * SCALE);
                data[j] += BASE_RATE * (j+1+(j*0.1));
                if (data[j] >= 1.0) data[j] -= 2.0;
            }
        }

        // Write output
        int result = irix_audio_write_frames(stream1, buffer1, BUFFER_SIZE);
        if (result < 0) {
            fprintf(stderr, "Error writing frames: %s\n", irix_audio_get_last_error());
            goto cleanup;
        }

        counter += BUFFER_SIZE;
    }

    printf("\nStarting recording stream for %f seconds.\n", TIME);

    // Open file for recording
    fd = fopen("test.raw", "wb");
    if (!fd) {
        fprintf(stderr, "Failed to open output file\n");
        goto cleanup;
    }

    // Reset counter for recording
    counter = 0;
    while (counter < frames) {
        // Read input
        int result = irix_audio_read_frames(stream2, buffer2, BUFFER_SIZE);
        if (result < 0) {
            fprintf(stderr, "Error reading frames: %s\n", irix_audio_get_last_error());
            fclose(fd);
            goto cleanup;
        }

        // Write to file
        fwrite(buffer2, sizeof(MY_TYPE), chans * result, fd);
        counter += result;
    }

    fclose(fd);
    printf("\nRecording complete. Wrote to test.raw\n");

    // Optional duplex-like operation
    printf("\nStarting quasi-duplex playback and recording.\n");
    counter = 0;
    while (counter < frames) {
        // Read input
        int read_result = irix_audio_read_frames(stream2, buffer2, BUFFER_SIZE);
        if (read_result < 0) {
            fprintf(stderr, "Error reading frames in duplex mode: %s\n", 
                    irix_audio_get_last_error());
            goto cleanup;
        }

        // Copy input to output
        memcpy(buffer1, buffer2, sizeof(MY_TYPE) * chans * read_result);
        int write_result = irix_audio_write_frames(stream1, buffer1, read_result);
        if (write_result < 0) {
            fprintf(stderr, "Error writing frames in duplex mode: %s\n", 
                    irix_audio_get_last_error());
            goto cleanup;
        }

        counter += read_result;
    }

cleanup:
    // Cleanup resources
    if (stream1) irix_audio_close_stream(stream1);
    if (stream2) irix_audio_close_stream(stream2);
    free(buffer1);
    free(buffer2);
    free(data);
    irix_audio_cleanup();

    return EXIT_SUCCESS;
}
