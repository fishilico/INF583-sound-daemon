#ifndef PLAYER_H
#define PLAYER_H

// Std ints
#include <stdint.h>

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
} music_buffer_t;

int wave_opener(music_file_t * file_info);
int au_opener(music_file_t * file_info);
int open_music_file(const char *file_name, music_file_t *file_info);
int dsp_configuration(int const fd_dsp, music_file_t const * audio_file);
int open_music_buffer(const char *file_name, music_buffer_t *music_buf);
int close_music_buffer(music_buffer_t *music_buf);
int play_step_music_buffer(music_buffer_t *music_buf);
int eof_music_buffer(music_buffer_t *music_buf);
int play_file(const char *file_name);
int player_main(int argc, char ** argv);

#endif /* PLAYER_H */
