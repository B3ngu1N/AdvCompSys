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
typedef std::vector<std::string> COL_DATA;

// Holds all Input & Output Data for Individual Threads
typedef struct dict_args {
    COL_DATA* dBlock;
    int dBlock_Len;
    MAP* dMapping;
    DICT* dDict;
} dict_args_t;

/*  CHECK
    Check that the condition holds. If it doesn't print a message and die.
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


// Read in column data from inFilename, put in input_data, return # of column entries
int readInFile(const char* inFilename, COL_DATA* input_data)
{
    // open input file
    std::ifstream infile;
    infile.open(inFilename); 
    if(!infile){
        perror("Input File Error");
        exit(1);
    }

    /*
        Read in all of the column data into a vector (line by line), then
        send that segmented data to pthreads to encode in segments. 
        Finally, merge that encoded data back together.
    */
    std::string input;
    int length=0;
    while(getline(infile, input)) {
        input_data->push_back(input);
        length++;
    }

    // Close input file
    infile.close();

    // Return the length/number of column data
    return length;
}


void* dictHelper(void* args)
{
    dict_args_t* dargs = (dict_args_t*)args;
    size_t const buffInSize = strlen(dargs->dBlock);
    
    

    pthread_exit(NULL);
}


void dictEncode(COL_DATA* input_data, int input_len, MAP* mapping, DICT* encoded)
{
    int num_threads = 8;
    // Find number of segments, round up
    int num_segments = ceil((float)input_len / (float)SEGMENT_LENGTH);

    dict_args_t **all_output = new dict_args_t*[num_threads];
    do {
        // Determine if number of threads input is more or less than num segments
        if(num_segments <= num_threads){
            num_threads = num_segments;
        }
        // Multithreaded Compress Helper
        pthread_t threads[num_threads];
        int rc;

        for(int i = 0; i < num_threads; i++){
            dict_args_t* dargs = new dict_args_t;
            dargs->dBlock = new COL_DATA;

            // ***** Need to copy segment of original vector data into a new vector
            //       to be distributed to the pthreads
            COL_DATA::iterator itr_beg, itr_end;
            itr_beg = input_data->begin() + (i*SEGMENT_LENGTH);
            itr_end = itr_beg + SEGMENT_LENGTH;
            if(itr_end > input_data->end()) itr_end = input_data->end(); 



            all_output[i] = dargs;

            rc = pthread_create(&threads[i], NULL, dictHelper, all_output[i]);
        }

        for(int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);



        }

        // Delete Dynammic Memory
        for(int i = 0; i < num_threads; i++){
            free(all_output[i]->dBlock);
        }

        num_segments -= num_threads;
    } while(num_segments > 0);


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

    // File Name
    const char* const inFilename = argv[1];
    // Data Variables
    MAP* mapping = new MAP;
    DICT* encoded = new DICT;
    COL_DATA* input_data = new COL_DATA;
    int input_len;

    input_len = readInFile(inFilename, input_data);

    // Timer Start
    start = clock();

    // Compress
    dictEncode(input_data, input_len, mapping, encoded);
    
    // Timer End
    end = clock();

    // Calculating total time taken by the program.
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by multithreaded dictionary encoding program is : " << std::fixed
         << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;

    return 0;
}
