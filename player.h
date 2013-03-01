#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <pthread.h>

// Music file handle
typedef struct
{
    FILE *file;
    uint_fast32_t oss_format;
    uint_fast32_t channels;
    uint_fast32_t sample_rate;
    uint_fast32_t bits_per_sample;
    uint_fast32_t data_size;
} music_file_t;

/**
 * Playing music state is a buffer with a file descriptor to /dev/dsp
 * and a music file
 */
typedef struct {
    int fd_dsp;
    music_file_t info;
    size_t buf_size;
    unsigned char *buf;

    // Playing thread, mutex and condition
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    // State: playing file, in a pause
    int playing;
    int pausing;
} music_buffer_t;

int wave_opener(music_file_t * file_info);
int au_opener(music_file_t * file_info);
int open_music_file(const char *file_name, music_file_t *file_info);
int dsp_configuration(int const fd_dsp, music_file_t const * audio_file);
int init_music_buffer(music_buffer_t *music_buf);
int destroy_music_buffer(music_buffer_t *music_buf);
int open_music_buffer(const char *file_name, music_buffer_t *music_buf);
int close_music_buffer(music_buffer_t *music_buf);
int play_step_music_buffer(music_buffer_t *music_buf);
int eof_music_buffer(music_buffer_t *music_buf);
int play_loop_music_buffer(music_buffer_t *music_buf);
int start_play_loop_music_buffer(pthread_t *thread, music_buffer_t *music_buf);
int stop_play_loop_music_buffer(pthread_t thread, music_buffer_t *music_buf);
int pause_loop_music_buffer(music_buffer_t *music_buf);
int resume_loop_music_buffer(music_buffer_t *music_buf);
int play_file(const char *file_name);
int player_main(int argc, char ** argv);

#endif /* PLAYER_H */
