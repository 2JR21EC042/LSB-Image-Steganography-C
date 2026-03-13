#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(int argc,char *argv[])
{
     if (argv[1] == NULL)
    {
        return e_unsupported;
    }

    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

int main(int argc, char *argv[])
{
   if(argc<3)
   {
    printf("Insufficient arguements\n");
    return 1;
   }
    OperationType res = check_operation_type(argc, argv);

    if(res == e_encode)
    {
        EncodeInfo encInfo;
        if(argc<4)
        {
            printf("Insufficient arguements for encoding\n");
            return 1;
        }

        if(read_and_validate_encode_args(argv, &encInfo) != e_success)
        {
            printf("Validation failed\n");
            return 1;
        }

        if(do_encoding(&encInfo) != e_success)
        {
            printf("Encoding failed\n");
            return 1;
        }

        printf("Encoding successful\n");
    }
    else if (res==e_decode)
    {
        
        DecodeInfo decInfo;
        if(argc<3)
        {
            printf("Insufficient arguements for decoding\n");
             return 1;
        }
        if( read_and_validate_decode_args(argv,&decInfo)!=e_success)
        {
            printf("Error in validation");
            return 1;
        }
        if( do_decoding(&decInfo)!=e_success)
        {
            printf("error in decoding");
            return 1;
        }
        printf("decoding successfull\n");
        
        
    }
    return e_success;

    return 0;
}


    //res -> e_decode -> read_and_validate_decode_arg();  -> do_decoding();

    //res -> e_usupported -> error -> terminate
    //return 0;

