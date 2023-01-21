/*
Project 1 - ZSTD Compression with Multithreading
Dependencies: pthreads, zstd, standard libraries

** Compilation must include -lzstd flag

Authors: Ben Haft, Thomas Petr
*/

#include <zstd.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <cmath>

#include <stdio.h>     // printf
#include <stdlib.h>    // free
#include <string.h>    // strlen, strcat, memset
#include "common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()

#define SEGMENT_LENGTH 16000

typedef struct compress_args
{
  const char *fname;
  char *outName;
  int cLevel;
} compress_args_t;



static void compressFile(const char* inName, const char* outName, int num_threads, int cLevel)
{
    FILE* const fout = fopen_orDie(outName, "wb");
    size_t const buffOutSize = ZSTD_compressBound(SEGMENT_LENGTH);  /* can always flush a full block */
    void*  const buffOut = malloc_orDie(buffOutSize);
    void*  const buffIn  = malloc_orDie(SEGMENT_LENGTH * sizeof(char));

    // open input file
    std::ifstream infile;
    infile.open(inName); 
    if(!infile){
        perror("Input File Error");
        exit(1);
    }

    // Break input file into 16kb segments
    // 16kB = 16000 char's
    // Find file length
    infile.seekg(0, infile.end);
    int length = infile.tellg();
    infile.seekg(0, infile.beg);

    // Find number of 16kB segments, round up
    int num_segments = ceil((float)length / (float)SEGMENT_LENGTH);

    // Read data in by 16kB chunks -> array of 16kB C strings
    char** input_data = new char*[num_segments];
    for(int i = 0; i < num_segments; i++)
    {
        char* buffer = new char[SEGMENT_LENGTH];
        infile.read(buffer, SEGMENT_LENGTH);

        input_data[i] = buffer;
    }

    // Fancy Lambda Function for Threads - Compression of 16kB Blocks
    auto compressHelper = [](char* cBlock) {
        ZSTD_CStream* const cstream = ZSTD_createCStream();
        if (cstream==NULL) { fprintf(stderr, "ZSTD_createCStream() error \n"); exit(10); }
        size_t const initResult = ZSTD_initCStream(cstream, 1);
        if (ZSTD_isError(initResult)) {
            fprintf(stderr, "ZSTD_initCStream() error : %s \n",
                        ZSTD_getErrorName(initResult));
            exit(11);
        }

        size_t read, toRead = SEGMENT_LENGTH * sizeof(char);
        ZSTD_inBuffer input = { cBlock, read, 0 };
        while (input.pos < input.size) {
            ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
            toRead = ZSTD_compressStream(cstream, &output , &input);   
            if (ZSTD_isError(toRead)) {
                fprintf(stderr, "ZSTD_compressStream() error : %s \n",
                                ZSTD_getErrorName(toRead));
                exit(12);
            }
        }
    };

    // Determine if number of threads input is more or less than num segments
    if(num_segments < num_threads){
        num_threads = num_segments;
        // Multithreaded Compress Helper
        pthread_t threads[num_threads];
        int rc;

        for(int i = 0; i < num_threads; i++){
            rc = pthread_create(&threads[i], NULL, compressHelper, input_data[i]);
        }

    }
    else{
        pthread_t threads[num_threads];

        do{
            // Multithreaded Compress Helper


            num_segments -= num_threads;
        } while(num_segments > 0);
    }


/* OLD
    pthread_t *threads = malloc_orDie(argc * sizeof(pthread_t));
    compress_args_t *args = malloc_orDie(argc * sizeof(compress_args_t));

    pthread_create (&threads[i], NULL, compressFile, &args[i]);

    for (unsigned i = 0; i < argc; i++)
      pthread_join (threads[i], NULL);
*/

}


/*  Creates Outfilename by appending ".zst" to the end of the C string input name
    Done this way so that any file type can be compressed (without affecting the name)
    Helper function from the ZSTD GitHub */
static char* createOutFilename(const char* filename)
{
    size_t const inL = strlen(filename);
    size_t const outL = inL + 5;
    void* const outSpace = malloc(outL);
    memset(outSpace, 0, outL);
    strcat((char*)outSpace, filename);
    strcat((char*)outSpace, ".zst");
    return (char*)outSpace;
}


int main(int argc, const char** argv)
{
    const char* const exeName = argv[0];

    if (argc<=3) {
        printf("Wrong Arguments\n");
        printf("%s NUM_THREADS FILE\n", exeName);
        return 1;
    }

    int num_threads = atoi (argv[1]);
    CHECK(num_threads != 0, "Must have >0 threads");

    // int level = atoi (argv[2]);
    // CHECK(level != 0, "can't parse LEVEL!");

    // File Names
    const char* const inFilename = argv[2];
    const char* outName = createOutFilename(inFilename);

    compressFile(inFilename, outName, num_threads, 1);

    
    return 0;
}
