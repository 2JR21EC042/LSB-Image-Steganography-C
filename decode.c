#include <stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    
    /* Check stego image file */
    if(argv[2] == NULL)
    {
        printf("ERROR: No stego image file provided\n");
        return e_failure;
    }

    char *extn = strrchr(argv[2], '.');

    if(extn == NULL || strcmp(extn, ".bmp") != 0)
    {
        printf("ERROR: Stego image must be .bmp file\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    /* Check output file */
    if(argv[3] != NULL)
    {
        decInfo->output_fname = argv[3];
    }
    else
    {
        decInfo->output_fname = "output.txt";
    }

    return e_success;
}


Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image=fopen( decInfo->stego_image_fname,"rb");     //open stego image in read mode
    if(decInfo->fptr_stego_image==NULL)
    {
        printf("Unable to open stego image\n");
        return e_failure;
    }
    decInfo->fptr_output=fopen(decInfo->output_fname,"wb");         //open output file in writing mode
    if(decInfo->fptr_output==NULL)
    {
        printf("Unabe to open output file\n");
        return e_failure;
    }
    return e_success;
}


Status skip_bmp_header(FILE *fptr_stego_image)
{
    if(fseek(fptr_stego_image,54,SEEK_SET)!=0)
    {
        printf("Unabe to skip bmp header\n");
        return e_failure;
    }
    return e_success;
}


Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image)
{
    char image_buffer[8];
    for(int i=0;i<size;i++)
    {
        if(fread(image_buffer,1,8,fptr_stego_image)!=8)
            return e_failure;

        decode_byte_from_lsb(&data[i],image_buffer);
    }
    
    return e_success;
}

Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    
    *data = 0;

    for (int i = 0; i < 8; i++)
    {
        *data |= ((image_buffer[i] & 1) << i);
    }

    return e_success;
}



Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic[3];
    if(decode_data_from_image(magic,2,decInfo->fptr_stego_image)==e_failure)
    {
        return e_failure;
    }
    magic[2]='\0'; 
    if(strcmp(magic,"#*")==0)
    {
        return e_success;
    }
    else{
        return e_failure;
    }
}

Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    if(decode_data_from_image((char *)size,sizeof(int),decInfo->fptr_stego_image)==e_failure)
    {
        return e_failure;
    }
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    int extn_size;

    if (decode_secret_file_extn_size(&extn_size, decInfo) == e_failure)
        return e_failure;

    if (extn_size >= MAX_FILE_SUFFIX)
    {
        return e_failure;
    }

    if (decode_data_from_image(decInfo->extn_secret_file,
                               extn_size,
                               decInfo->fptr_stego_image) == e_failure)
        return e_failure;

    decInfo->extn_secret_file[extn_size] = '\0';

   

    return e_success;
}

Status decode_secret_file_size(int *size, DecodeInfo *decInfo)
{
    if(decode_data_from_image((char *)size,sizeof(int),decInfo->fptr_stego_image)==e_failure)
    {
        return e_failure;
    }
    
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char ch;
    for(long i=0;i<decInfo->size_secret_file;i++)
    {
        if(decode_data_from_image(&ch,1,decInfo->fptr_stego_image)==e_failure)
        {
            return e_failure;
        }
        fwrite(&ch,1,1,decInfo->fptr_output);

    }
    return e_success;
}


Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) == e_failure)
        return e_failure;

    if (skip_bmp_header(decInfo->fptr_stego_image) == e_failure)
{
    printf("ERROR: Unable to skip BMP header\n");
    return e_failure;
}

    if(decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR: Magic string mismatch\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo)==e_failure)
    {
        return e_failure;
    }
    if(decode_secret_file_size(&decInfo->size_secret_file, decInfo)==e_failure)
    {
        return e_failure;
    }
    if(decode_secret_file_data(decInfo)==e_failure)
    {
        return e_failure;
    }
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return e_success;
}



