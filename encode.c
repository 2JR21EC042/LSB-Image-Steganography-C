#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot=strrchr(argv[2],'.');
    if(dot!=NULL && strcmp(dot,".bmp")==0)
    {
        encInfo->src_image_fname=argv[2];
    }
    else{
        printf("Not an .bmp file!\n");
        return e_failure;
    }

    //argv[3] -> .txt -> encInfo->secret_fname = argv[3] -> strcpy(encInfo->extn_secret_file, strstr(argv[3], "."))

    encInfo->secret_fname=argv[3];

    char *ext = strrchr(argv[3], '.');  // Better than strstr

    if(ext != NULL)
    {
        strcpy(encInfo->extn_secret_file, ext);
    }
    else
    {
        printf("Secret file has no extension\n");
        return e_failure;
    }


    //argv[4] -> NULL-> store "stego.bmp" in structure
    //   -> no NULL -> .bmp -> store argv[4] in structure
    if(argv[4]!=NULL)
    {
        char *opt_argv=strrchr(argv[4],'.');
        if(opt_argv!=NULL && strcmp(opt_argv,".bmp")==0)
        {
            encInfo->stego_image_fname=argv[4];
        }
        else
        return e_failure;
    }
    else
    encInfo->stego_image_fname="stego.bmp";



    return e_success;
}


uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);   // VERY IMPORTANT
    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if((encInfo->image_capacity-54) > (strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(int) + encInfo->size_secret_file )*8)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[8];
    for(int i = 0; i < size; i++)
    {
        if (fread(arr, 1, 8, fptr_src_image) != 8)
           return e_failure;
        encode_byte_to_lsb(data[i], arr);
        if (fwrite(arr, 1, 8, fptr_stego_image) != 8)
           return e_failure;
    }
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    return encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), encInfo->fptr_src_image, encInfo->fptr_stego_image);
}

Status do_encoding(EncodeInfo *encInfo)
{
    // open_files() == e_failure -> error -> return e_failure
    if(open_files(encInfo)!=e_success)
    {
        printf("Error in opening files\n");
        return e_failure;
    }

    //check_capacity() == e_failure -> error -> return e_failure
    if(check_capacity(encInfo)!=e_success)
    {
        printf("Error in checking capacity\n");
        return e_failure;
    }


    //copy_bmp_header() == e_failure -> error -> return e_failure
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)!=e_success)
    {
        printf("Error in bmp header\n");
        return e_failure;
    }

    //encode_magic_string() == e_failure -> error -> return e_failure
    if(encode_magic_string(MAGIC_STRING, encInfo)!=e_success)
    {
        printf("Error in encoding magic string\n");
        return e_failure;
    }

   if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo)!=e_success)
    {
        printf("Error in encoding the the secret file size\n");
        return e_failure;
    }

     if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo)!=e_success)
    {
        printf("Error in the encoding secret file extension!\n");
        return e_failure;
    }

     if(encode_secret_file_size(encInfo->size_secret_file, encInfo)!=e_success)
    {
        printf("Error in the encoding of secret file size\n");
        return e_failure;
    }

    //
    if(encode_secret_file_data(encInfo)!=e_success)
    {
        printf("Error in the secret data!\n");
        return e_failure;
    }

    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)!=e_success)
    {
        printf("Error in copying the remaining elements!\n");
        return e_failure;
    }




    //Status encode_byte_to_lsb(char data, char *image_buffer);
    /*
    for i=0 to 8;
    image buffer is char array
    image buffer[i]=image buffer[i]&(~1);
    clear that
    char get (data & 1<<i)>>i;

    return e succes
    */

   /*
   Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);
   char arr[8];
   loop i=0 to size times
   fread(arr,8,1,fptr_src);
   encode_byte_datalsb(data[i],arr);
   fwrite(arr,8,1,fptr_stego);
   */
   /*
   Status integer_to_lsb(int data, char *image buffer);
    for i=0 to 32  because integer;
    image buffer is char array
    image buffer[i]=image buffer[i]&(~1);
    clear that
    char get (data & 1<<i)>>i;

   */


    //encode_magic_string() == e_failure -> error -> return e_failure

    /*
    call encode file extension size fun
    char arr[32];
    int size=strlen(file_ext);
    fread(arr,32,1,fptr src image);
    encode_int_data(size,arr);
    fwriet(arr,32,1,fptr image);
    */

    /*
    Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);
    call Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);
   char arr[8];
   loop i=0 to size times
   fread(arr,8,1,fptr_src);
   encode_byte_datalsb(data[i],arr);
   fwrite(arr,8,1,fptr_stego);
   return e success
    */

    /*
    call Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);
    */
   /*
   call Status encode_secret_file_data(EncodeInfo *encInfo);
   */

   /*
   Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);
   */
  return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        // Step 1: Clear the 0th bit (LSB)
        image_buffer[i] = image_buffer[i] & (~1);

        // Step 2: Extract i-th bit from data
        char get = (data & (1 << i)) >> i;

        // Step 3: Set that bit into image buffer
        image_buffer[i] = image_buffer[i] | get;
    }

    return e_success;
}



Status encode_int_to_lsb(int data, char *image_buffer)
{
    for(int i = 0; i < 32; i++)
    {
        // Step 1: Clear LSB of image byte
        image_buffer[i] = image_buffer[i] & (~1);

        // Step 2: Extract i-th bit from integer
        char get = (data & (1 << i)) >> i;

        // Step 3: Insert that bit into image byte
        image_buffer[i] = image_buffer[i] | get;
    }

    return e_success;
}


Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    int len = strlen(file_extn);

    if (encode_data_to_image((char *)file_extn, len, encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }

    return e_success;
}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    unsigned char arr[32];

    // Read 32 bytes from source image
    if (fread(arr, 1, 32, encInfo->fptr_src_image) != 32)
        return e_failure;

    // Encode integer into those 32 bytes
    encode_int_to_lsb(size, (char *)arr);

    // Write modified bytes to stego image
    if (fwrite(arr, 1, 32, encInfo->fptr_stego_image) != 32)
        return e_failure;

    return e_success;
}

Status encode_secret_file_size(long size, EncodeInfo *encInfo)
{
    unsigned char arr[32];
    if(fread(arr, 1, 32,encInfo->fptr_src_image)!=32)
    return e_failure;

    encode_int_to_lsb(size, arr);

    if(fwrite(arr, 1, 32, encInfo->fptr_stego_image)!=32)
    return e_failure;

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    // Move secret file pointer to beginning
    fseek(encInfo->fptr_secret, 0, SEEK_SET);

    // Read each byte from secret file
    while (fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        // Encode this one byte into image
        if (encode_data_to_image(&ch, 1,encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        {
            return e_failure;
        }
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    // Read until end of source image
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        if (fwrite(&ch, 1, 1, fptr_dest) != 1)
            return e_failure;
    }

    return e_success;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    fseek(fptr_src_image,0,SEEK_SET);

    if (fread(header, 1, 54, fptr_src_image) != 54)
        return e_failure;

    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
        return e_failure;

    return e_success;
}


    
