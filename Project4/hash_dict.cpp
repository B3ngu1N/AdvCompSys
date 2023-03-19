/*
    Dictionary implementation using a hash table encoding.

    @author Thomas Petr
    @author Ben Haft
*/

#include <iostream>
#include <utility>
#include <string>
#include <unordered_map>
#include <bits/stdc++.h>
#include <fstream>
#include <pthread.h>
#include <omp.h>

#define SEGMENT_LENGTH 4

typedef std::unordered_map<std::string, unsigned long> MAP;
typedef std::vector<unsigned long> DICT;
typedef std::vector<unsigned long> INDICES;
typedef std::vector<std::string> COL_DATA;

// Holds all Input & Output Data for Individual Threads
typedef struct dict_args {
    COL_DATA::const_iterator dBlock_itr;
    int dBlock_Len;
    MAP* dMapping;
    DICT* dDict;
} dict_args_t;

// Holds information for multithreaded searching
typedef struct search_args {
    DICT::const_iterator sBlock_itr;
    int sBlock_Len;
    unsigned int start_index;
    unsigned long hashVal;
    INDICES* sBlock_Indices;
} search_args_t;

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


// Hash function for unique identifiers over each thread
unsigned long hashFunc(const std::string& key)
{
    unsigned long hash = 5381;

    for (int i = 0; i<key.length(); i++)
        hash = ((hash << 5) + hash) + key[i]; /* hash * 33 + key[i] */

    return hash;
}


// Read in column data from inFilename, put in input_data, return # of column entries
unsigned int readInFile(const char* inFilename, COL_DATA* input_data)
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
    unsigned int length=0;
    while(getline(infile, input)) {
        std::remove_if(input.begin(), input.end(), ::isspace); //remove whitespace (\n)
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

    COL_DATA::const_iterator itr_end = dargs->dBlock_itr + dargs->dBlock_Len;
    for(; dargs->dBlock_itr!=itr_end; dargs->dBlock_itr++){
        unsigned long hash_val = hashFunc(*(dargs->dBlock_itr));
        dargs->dMapping->insert(std::make_pair(*(dargs->dBlock_itr), hash_val));
        dargs->dDict->push_back(hash_val);
    }

    pthread_exit(NULL);
}


void dictEncode(COL_DATA* input_data, int input_len, MAP* mapping, DICT* encoded, int num_threads)
{
    // Find number of segments, round up
    unsigned int num_segments = ceil((double)input_len / (double)SEGMENT_LENGTH);
    unsigned int thread_segment = 0, num_threads_orig = num_threads;

    /*
        Encoding
    */
    dict_args_t **all_output = new dict_args_t*[num_threads];
    do {
        // Determine if number of threads input is more or less than num segments
        if(num_segments <= num_threads){
            num_threads = num_segments;
        }
        // Multithreaded Compress Helper
        pthread_t threads[num_threads];

        for(int i = 0; i < num_threads; i++){
            dict_args_t* dargs = new dict_args_t;

            // Get start location and end location as iterators
            COL_DATA::const_iterator itr, itr_end;
            itr = input_data->begin() + (i*SEGMENT_LENGTH) 
                                + (thread_segment*num_threads_orig*SEGMENT_LENGTH);
            itr_end = itr + SEGMENT_LENGTH;
            if(itr_end > input_data->end()) itr_end = input_data->end(); 
            dargs->dBlock_Len = (int)(itr_end - itr);

            dargs->dBlock_itr = itr;
            dargs->dDict = new DICT;
            dargs->dMapping = new MAP;

            all_output[i] = dargs;

            pthread_create(&threads[i], NULL, dictHelper, all_output[i]);
        }

        for(int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);

            // Append arrays to end of encoded
            encoded->insert(encoded->end(), 
                            all_output[i]->dDict->begin(), all_output[i]->dDict->end());

            // Merge in mappings
            mapping->merge(*(all_output[i]->dMapping));
        }

        // Delete Dynamic Memory
        for(int i = 0; i < num_threads; i++){
            free(all_output[i]->dDict);
            free(all_output[i]->dMapping);
        }

        num_segments -= num_threads;
        thread_segment++;
    } while(num_segments > 0);


    // Free all Dynamic Memory
    free(all_output);
}


// Find members of the mapping with given prefix, store strings in output
void findPrefixMembers(std::string& prefix, MAP* mapping, std::vector<std::string> output)
{
    MAP::iterator itr = mapping->begin();
    unsigned long pre_hash = hashFunc(prefix);
    for(; itr != mapping->end(); itr++){
        if(itr->second >= pre_hash){
            std::string prefix_chk = (itr->first).substr(0, prefix.length());
            if(pre_hash == hashFunc(prefix_chk)){
                output.push_back(itr->first);
            }
        }
    }
}


void* findHelper(void* args)
{
    search_args_t* sargs = (search_args_t*)args;

    DICT::const_iterator itr_end = sargs->sBlock_itr + sargs->sBlock_Len;
    DICT::const_iterator itr_begin = sargs->sBlock_itr;
    #pragma omp parallel for
    for(; sargs->sBlock_itr!=itr_end; sargs->sBlock_itr++){
        if(*(sargs->sBlock_itr) == sargs->hashVal){
            sargs->sBlock_Indices->push_back((unsigned long)(sargs->sBlock_itr - itr_begin) + sargs->start_index);
        }
    }

    pthread_exit(NULL);
}


void find(std::string& new_str, MAP* mapping, DICT* encoded, INDICES* output, int num_threads)
{
    MAP::iterator itr = mapping->find(new_str);
    if(itr == mapping->end()) {
        return;
    }
    else {
        // Find number of segments, round up
        unsigned int num_segments = ceil((double)encoded->size() / (double)SEGMENT_LENGTH);
        unsigned int thread_segment = 0, num_threads_orig = num_threads;
        unsigned long hash2Find = hashFunc(new_str);

        /*
            Breakup into SIMD search threads
        */
        search_args_t **all_output = new search_args_t*[num_threads];
        do {
            // Determine if number of threads input is more or less than num segments
            if(num_segments <= num_threads){
                num_threads = num_segments;
            }
            // Multithreaded Compress Helper
            pthread_t threads[num_threads];

            for(int i = 0; i < num_threads; i++){
                search_args_t* sargs = new search_args_t;

                // Get start location and end location as iterators
                DICT::const_iterator itr, itr_end;
                itr = encoded->begin() + (i*SEGMENT_LENGTH) 
                                    + (thread_segment*num_threads_orig*SEGMENT_LENGTH);
                itr_end = itr + SEGMENT_LENGTH;
                if(itr_end > encoded->end()) itr_end = encoded->end(); 
                sargs->sBlock_Len = (int)(itr_end - itr);

                sargs->sBlock_itr = itr;
                sargs->sBlock_Indices = new INDICES;
                sargs->hashVal = hash2Find;
                sargs->start_index = (i*SEGMENT_LENGTH) 
                                    + (thread_segment*num_threads_orig*SEGMENT_LENGTH);

                all_output[i] = sargs;

                pthread_create(&threads[i], NULL, findHelper, all_output[i]);
            }

            for(int i = 0; i < num_threads; i++){
                pthread_join(threads[i], NULL);

                output->insert(output->end(), all_output[i]->sBlock_Indices->begin(),
                                            all_output[i]->sBlock_Indices->end());
            }

            // Delete Dynamic Memory
            for(int i = 0; i < num_threads; i++){
                free(all_output[i]->sBlock_Indices);
            }

            num_segments -= num_threads;
            thread_segment++;
        } while(num_segments > 0);


        // Free all Dynamic Memory
        free(all_output);
    }
}



int main(int argc, char** argv)
{
    clock_t start, end;
    const char* const exeName = argv[0]; // Name of file to compress
    int num_threads = 8;

    if (argc<2 || argc>3) { // Need 2 or 3 runtime arguments
        printf("Wrong Arguments\n");
        printf("%s INFILE (NUM_THREADS)\n", exeName);
        return 1;
    }

    // File Name
    const char* const inFilename = argv[1];
    num_threads = atoi(argv[2]);
    // Data Variables
    MAP* mapping = new MAP;
    DICT* encoded = new DICT;
    COL_DATA* input_data = new COL_DATA;
    unsigned int input_len;

    input_len = readInFile(inFilename, input_data);

    // Timer Start
    start = clock();

    // Compress
    dictEncode(input_data, input_len, mapping, encoded, num_threads);
    
    // Timer End
    end = clock();

    // Calculating total time taken by the program.
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by multithreaded dictionary encoding program is : " << std::fixed
         << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;

    
    INDICES* results = new INDICES;
    std::string find_str = "pazayouzlg";
    // Test reads, scans, etc
    start = clock();

    find(find_str, mapping, encoded, results, num_threads);

    end = clock();

    std::cout << "Indices (" << find_str << "): ";
    for(int i = 0; i<results->size(); i++){
        std::cout << (*results)[i] << " ";
    }
    std::cout << std::endl;

    // Calculating total time taken by the program.
    time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by multithreaded SIMD dictionary encoding read is : " << std::fixed
         << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;

    return 0;
}
