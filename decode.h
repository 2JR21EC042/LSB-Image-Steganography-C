#ifndef DECODE_H
#define DECODE_H


#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_FILE_SUFFIX 10

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output Secret File Info */
    char *output_fname;
    FILE *fptr_output;

    char extn_secret_file[MAX_FILE_SUFFIX];
    int size_secret_file;

} DecodeInfo;

/* Function Prototypes */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);
Status skip_bmp_header(FILE *fptr_stego_image);

Status decode_magic_string(DecodeInfo *decInfo);
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(int *size, DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image);
Status decode_byte_from_lsb(char *data, char *image_buffer);

#endif
