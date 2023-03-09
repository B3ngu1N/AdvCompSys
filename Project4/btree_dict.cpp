/*
    Dictionary implementation using a hash table encoding.

    @author Thomas Petr
    @author Ben Haft
*/

#include <iostream>
#include <string>
#include <unordered_map>
#include <bits/stdc++.h>
#include <fstream>
#include <pthread.h>

#define SEGMENT_LENGTH 64000

typedef std::unordered_map<std::string, int> MAP;
typedef std::vector<int> DICT;

// Holds all Input & Output Data for Individual Threads
typedef struct dict_args {
    char* cBlock;
    char* cOut;
    size_t outdata_length;
} dict_args_t;

/* CHECK
 * Check that the condition holds. If it doesn't print a message and die.
 */
#define CHECK(cond, ...)                        \
    do {                                        \
        if (!(cond)) {                          \
            fprintf(stderr,                     \
                    "%s:%d CHECK(%s) failed: ", \
                    __FILE__,                   \
                    __LINE__,                   \
                    #cond);                     \
            fprintf(stderr, "" __VA_ARGS__);    \
            fprintf(stderr, "\n");              \
            exit(1);                            \
        }                                       \
    } while (0)


void* dictHelper(void* args)
{
    dict_args_t* cargs = (dict_args_t*)args;
    size_t const buffInSize = strlen(cargs->cBlock);
    
    

    pthread_exit(NULL);
}


void dictEncode(const char* inFilename, MAP* mapping, DICT* encoded)
{
    int num_threads = 4;
    // open input file
    std::ifstream infile;
    infile.open(inFilename); 
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

    dict_args_t **all_output = new dict_args_t*[num_threads];
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
            dict_args_t* cargs = new dict_args_t;
            cargs->cBlock = new char[SEGMENT_LENGTH];

            //********************************************* Current issue: this would read in \n characters
            infile.read(cargs->cBlock, SEGMENT_LENGTH);


            all_output[i] = cargs;

            rc = pthread_create(&threads[i], NULL, dictHelper, all_output[i]);
        }

        for(int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);            
        }

        // Delete Dynammic Memory
        for(int i = 0; i < num_threads; i++){
            free(all_output[i]->cBlock);
        }

        num_segments -= num_threads;
    } while(num_segments > 0);


    // Close the Input and Output File
    infile.close();

    // Free all Dynamic Memory
    free(all_output);
}


int main(int argc, char** argv)
{
    clock_t start, end;
    const char* const exeName = argv[0]; // Name of file to compress

    if (argc!=2) { // Need 2 runtime arguments
        printf("Wrong Arguments\n");
        printf("%s INFILE\n", exeName);
        return 1;
    }

    // File Names
    const char* const inFilename = argv[2];
    MAP* mapping = new MAP;
    DICT* encoded = new DICT;

    // Timer Start
    start = clock();

    // Compress
    dictEncode(inFilename, mapping, encoded);
    
    // Timer End
    end = clock();

    // Calculating total time taken by the program.
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by multithreaded dictionary encoding program is : " << std::fixed
         << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;

    return 0;
}
