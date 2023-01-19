/*
Project 1 - ZSTD Compression with Multithreading
Dependencies: pthreads, zstd, standard libraries

** Compilation must include -lzstd flag

Authors: Ben Haft, Thomas Petr
*/

#include <zstd.h>
// #include <pthread.h>

#include <stdio.h>     // printf
#include <stdlib.h>    // free
#include <string.h>    // strlen, strcat, memset
#include "common.h"    // Helper functions, CHECK(), and CHECK_ZSTD()

static void compress_file(const char* ifname, const char* ofname, int comp_level)
{
    FILE* fin = fopen(ifname, "rb");
    FILE* fout = fopen(ofname, "wb");

    
    
    size_t fSize;
    void* const fBuff = mallocAndLoadFile_orDie(fname, &fSize);
    size_t const cBuffSize = ZSTD_compressBound(fSize);
    void* const cBuff = malloc(cBuffSize);

    /* Compress.
     * If you are doing many compressions, you may want to reuse the context.
     * See the multiple_simple_compression.c example.
     */
    size_t const cSize = ZSTD_compress(cBuff, cBuffSize, fBuff, fSize, 1);
    CHECK_ZSTD(cSize);

    saveFile_orDie(oname, cBuff, cSize);

    /* success */
    printf("%25s : %6u -> %7u - %s \n", fname, (unsigned)fSize, (unsigned)cSize, oname);

    free(fBuff);
    free(cBuff);
}

static char* createOF(char* filename)
{
    size_t inL = strlen(filename);
    void* outSpace = malloc(inL);
    memset(outSpace, 0, inL);    
    strncpy((char*)outSpace, filename, inL-4);
    strcat((char*)outSpace, ".zst");
    return (char*)outSpace;
}

int main(int argc, char** argv)
{
    char* exeName = argv[0];

    if (argc!=2) {
        printf("wrong arguments\n");
        printf("Must follow: %s FILE\n", exeName);
        return 1;
    }

    char* inFilename = argv[1];
    char* outFilename = createOF(inFilename);
    compress_orDie(inFilename, outFilename);
    free(outFilename);
    return 0;
}




