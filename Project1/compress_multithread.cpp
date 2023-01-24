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

typedef struct compress_args {
    char* cBlock;
    char* cOut;
    size_t outdata_length;
} compress_args_t;

void* compressHelper(void* args)
{
    compress_args_t* cargs = (compress_args_t*)args;
    size_t const buffInSize = strlen(cargs->cBlock);
    size_t const buffOutSize = ZSTD_compressBound(buffInSize);
    cargs->cOut = (char*)malloc_orDie(buffOutSize);


    ZSTD_CStream* const cstream = ZSTD_createCStream();
    if (cstream==NULL) { fprintf(stderr, "ZSTD_createCStream() error \n"); exit(10); }
    size_t const initResult = ZSTD_initCStream(cstream, 1);
    if (ZSTD_isError(initResult)) {
        fprintf(stderr, "ZSTD_initCStream() error : %s \n",
                    ZSTD_getErrorName(initResult));
        exit(11);
    }

    ZSTD_inBuffer input = { cargs->cBlock, buffInSize, 0 };
    
    ZSTD_outBuffer output = { cargs->cOut, buffOutSize, 0 };
    int chk=0;
    chk = ZSTD_compressStream(cstream, &output , &input);  
    if (ZSTD_isError(chk)) {
            fprintf(stderr, "ZSTD_compressStream() error : %s \n",
                            ZSTD_getErrorName(chk));
            exit(12);
    } 

    size_t const remainingToFlush = ZSTD_endStream(cstream, &output);   /* close frame */
    if (remainingToFlush) { fprintf(stderr, "not fully flushed"); exit(13); }

    // cargs->cOut = (char*)buffOut;
    cargs->outdata_length = output.pos;

    ZSTD_freeCStream(cstream);
    pthread_exit(NULL);
    // return NULL;
}


static void compressFile(const char* inName, const char* outName, int num_threads, int cLevel)
{
    FILE* const fout = fopen_orDie(outName, "wb");    

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

    // Multithreaded Compress Helper
    do{
        // Determine if number of threads input is more or less than num segments
        if(num_segments <= num_threads){
            num_threads = num_segments;
        }
        // Identify threads in use
        pthread_t threads[num_threads];

        // Assign data pointer to applicable 16kB segment for each thread
        int rc;
        compress_args_t **all_output = new compress_args_t*[num_threads];
        for(int i = 0; i < num_threads; i++){
            compress_args_t* cargs = new compress_args_t;
            cargs->cBlock = new char[SEGMENT_LENGTH];

            // Read data in by 16kB chunks -> array of 16kB C strings
            infile.read(cargs->cBlock, SEGMENT_LENGTH);
            all_output[i] = cargs;
            
            rc = pthread_create(&threads[i], NULL, compressHelper, all_output[i]);
        }

        // Waits for threads to finish in order and writes to output file
        for(int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);
            fwrite_orDie(all_output[i]->cOut, all_output[i]->outdata_length, fout);
        }
        
        // Decrement number of segments left to compress
        num_segments -= num_threads;
    }while(num_segments > 0);

    // Close the Output File
    fclose_orDie(fout);

    // Free all Dynamic Memory
    // TO BE ADDED

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
    const char* const exeName = argv[0]; // Name of file to compress

    if (argc==2) { // Need two runtime input arguements
        printf("Wrong Arguments\n");
        printf("%s NUM_THREADS FILE\n", exeName);
        return 1;
    }

    // Max number of threads (other than main thread)
    int num_threads = atoi(argv[1]);
    CHECK(num_threads != 0, "Must have >0 threads");

    // File Names
    const char* const inFilename = argv[2];
    const char* outFileName = createOutFilename(inFilename);

    compressFile(inFilename, outFileName, num_threads, 1);

    return 0;
}
