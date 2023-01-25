/*
Project 1 - ZSTD Compression with Multithreading
Dependencies: pthreads, zstd, standard libraries
Uses "common.h" from ZSTD GitHub (for error handling)

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

// Holds all Input & Output Data for Individual Threads
typedef struct compress_args {
    char* cBlock;
    char* cOut;
    size_t outdata_length;
    int cLevel;
} compress_args_t;

/*  
    Helper Function - Compresses a XXkB Block/Segment (Determined by SEGMENT_LENGTH)
    Input is a compress_args_t* struct, void output (adjusts struct's dynamically 
    allocated memory)
*/
void* compressHelper(void* args)
{
    compress_args_t* cargs = (compress_args_t*)args;
    size_t const buffInSize = strlen(cargs->cBlock);
    size_t const buffOutSize = ZSTD_compressBound(buffInSize);
    
    // Compress 16kB Block into cOut of Input Struct
    size_t cSize = ZSTD_compress(cargs->cOut, buffOutSize, 
                                 cargs->cBlock, buffInSize, 
                                 cargs->cLevel);
    CHECK_ZSTD(cSize);

    // Set Compressed Output Length to Input Struct
    cargs->outdata_length = cSize;

    pthread_exit(NULL);
}


/* 
    Compress File Function - Multithreaded using Pthreads
    Inputs: inFileName, outFileName, User Set # of Threads, Compression Level 1-22
*/ 
static void compressFile(const char* inName, const char* outName, 
                         int num_threads, int cLevel)
{
    FILE* fout = fopen_orDie(outName, "wb");

    // open input file
    std::ifstream infile;
    infile.open(inName); 
    if(!infile){
        perror("Input File Error");
        exit(1);
    }

    // Find file length
    infile.seekg(0, infile.end);
    int length = infile.tellg();
    infile.seekg(0, infile.beg);

    // Find number of 16kB segments, round up
    int num_segments = ceil((float)length / (float)SEGMENT_LENGTH);

    compress_args_t **all_output = new compress_args_t*[num_threads];
    int cSize = 0;
    do {
        // Determine if number of threads input is more or less than num segments
        if(num_segments <= num_threads){
            num_threads = num_segments;
        }
        // Multithreaded Compress Helper
        pthread_t threads[num_threads];
        int rc;

        for(int i = 0; i < num_threads; i++){
            compress_args_t* cargs = new compress_args_t;
            cargs->cBlock = new char[SEGMENT_LENGTH];

            // Read data in by 16kB chunks -> array of 16kB C strings
            // Advantage for large text files because it doesn't read all data
            // in at once (efficient usage of memory)
            infile.read(cargs->cBlock, SEGMENT_LENGTH);

            size_t const buffInSize = strlen(cargs->cBlock);
            size_t const buffOutSize = ZSTD_compressBound(buffInSize);
            char* temp = new char[buffOutSize];
            cargs->cOut = temp;
            cargs->outdata_length = 0;
            cargs->cLevel = cLevel;

            all_output[i] = cargs;

            rc = pthread_create(&threads[i], NULL, compressHelper, all_output[i]);
        }

        for(int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);
            
            fwrite_orDie(all_output[i]->cOut, all_output[i]->outdata_length, fout);
            cSize += (int)all_output[i]->outdata_length;
        }

        // Delete Dynammic Memory
        for(int i = 0; i < num_threads; i++){
            free(all_output[i]->cBlock);
            free(all_output[i]->cOut);
            all_output[i]->outdata_length = 0;
        }

        num_segments -= num_threads;
    } while(num_segments > 0);

    /* success */
    printf("%25s : %6u -> %7u - %s \n", 
            inName, (unsigned)length, (unsigned)cSize, outName);

    // Close the Input and Output File
    infile.close();
    fclose_orDie(fout);

    // Free all Dynamic Memory
    free(all_output);
}


/*  
    Creates Outfilename by appending ".zst" to the end of the C string input name
    Done this way so that any file type can be compressed (without affecting the name)
    Helper function from the ZSTD GitHub 
*/
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

    if (argc<3 || argc>4) { // Need 3 or 4 runtime input arguements
        printf("Wrong Arguments\n");
        printf("%s NUM_THREADS FILE CLevel=1\n", exeName);
        printf("Clevel==1 by Default\n");
        return 1;
    }

    int cLevel = 1;
    if(argc==4) cLevel = atoi(argv[3]);

    // Max number of threads (other than main thread)
    int num_threads = atoi(argv[1]);
    CHECK(num_threads != 0, "Must have >0 threads");

    // File Names
    const char* const inFilename = argv[2];
    const char* outFileName = createOutFilename(inFilename);

    compressFile(inFilename, outFileName, num_threads, cLevel);

    return 0;
}
