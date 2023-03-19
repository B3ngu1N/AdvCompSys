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

#define SEGMENT_LENGTH 16000

typedef std::unordered_map<std::string, unsigned long> MAP;
typedef std::vector<unsigned long> DICT;
typedef std::vector<std::string> COL_DATA;

struct Node {
    unsigned long hashVal;
    std::unordered_map<char, Node*>* next;
};

typedef std::unordered_map<char, Node*> NODE_MAP;

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


class Tree {
private:
    Node* root;
    MAP* mapping; // holds non-duplicate data to make merging easier

public:
    Tree() {
        mapping = new MAP;
        Node* newNode = new Node();
        newNode->hashVal = 0;
        NODE_MAP* next_nodemap = new NODE_MAP;
        newNode->next = next_nodemap;
        root = newNode;
    }

    Node* getRoot() {
        return root;
    }

    Node* createNode(unsigned long data) {
        Node* newNode = new Node();
        newNode->hashVal = data;
        NODE_MAP* next_nodemap = new NODE_MAP;
        newNode->next = next_nodemap;

        return newNode;
    }

    void insert_helper(Node* child, std::string& str2hash, std::string& new_str) {
        if(new_str.length()==0) return;
        char first_char = new_str[0]; // needed to have this beucase it was being annoying
        str2hash += new_str.substr(0, 1);
        std::string remaining = new_str.substr(1, new_str.length());

        NODE_MAP::iterator itr = child->next->find(first_char);
        // Character has already been added as a chain possibility
        if(itr != child->next->end()) {
            insert_helper(itr->second, str2hash, remaining);
        }
        else {
            Node* nextNode = createNode(hashFunc(str2hash));
            child->next->insert(std::make_pair(first_char, nextNode));
            insert_helper(nextNode, str2hash, remaining);
        }
    }

    void insert(std::string new_str) {
        unsigned long hash_val = hashFunc(new_str);
        if(this->mapping->find(new_str) == this->mapping->end()) {
            this->mapping->insert(std::make_pair(new_str, hash_val)); // to make merging easier

            char first_char = new_str[0]; // needed to have this beucase it was being annoying
            std::string first_str = new_str.substr(0, 1);
            std::string remaining = new_str.substr(1, new_str.length());

            NODE_MAP::iterator itr = root->next->find(first_char);
            // Character has already been added as a chain possibility
            if(itr != root->next->end()) {
                insert_helper(itr->second, first_str, remaining);
            }
            else {
                Node* nextNode = createNode(hashFunc(first_str));
                root->next->insert(std::make_pair(first_char, nextNode));
                insert_helper(nextNode, first_str, remaining);
            }
        }
        else return;
    }


    unsigned long getHashVal_helper(Node* child, int len_traveled, int str_len, std::string& new_str){
        if(new_str.length()==0 || len_traveled == str_len) return child->hashVal;
        char first_char = new_str[0]; // needed to have this beucase it was being annoying
        std::string remaining = new_str.substr(1, new_str.length());

        NODE_MAP::iterator itr = child->next->find(first_char);
        // Character has already been added as a chain possibility
        if(itr != child->next->end()) {
            return getHashVal_helper(itr->second, len_traveled+1, str_len, remaining);
        }
        else {
            return 0;
        }
    }

    unsigned long getHashVal(std::string& new_str) {
        int len_traveled = 0, str_len = new_str.length();
        char first_char = new_str[0]; // needed to have this beucase it was being annoying
        std::string remaining = new_str.substr(1, new_str.length());

        NODE_MAP::iterator itr = root->next->find(first_char);
        // Character has already been added as a chain possibility
        if(itr != root->next->end()) {
            return getHashVal_helper(itr->second, len_traveled+1, str_len, remaining);
        }
        else {
            return 0;
        }
    }

    void merge(Tree* b){
        this->mapping->merge(*(b->mapping));
        MAP::iterator itr = this->mapping->begin();
        for(; itr != this->mapping->end(); itr++){
            this->insert(itr->first);
        }
    }

    // printout for testing
    void traverse(Node* node){
        if (node != nullptr) {
            std::cout << node->hashVal << " ";

            NODE_MAP::iterator itr = node->next->begin();
            for(; itr != node->next->end(); itr++){
                std::cout << itr->first << " ";
                traverse(itr->second);
            }
            std::cout << std::endl;
        }
    }
};


// Holds all Input & Output Data for Individual Threads
typedef struct dict_args {
    COL_DATA::const_iterator dBlock_itr;
    int dBlock_Len;
    Tree* dTree;
    DICT* dDict;
} dict_args_t;


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
        dargs->dTree->insert(*(dargs->dBlock_itr));
        dargs->dDict->push_back(hash_val);
    }

    pthread_exit(NULL);
}


void dictEncode(COL_DATA* input_data, int input_len, Tree* mapping, DICT* encoded, int num_threads)
{
    // Find number of segments, round up
    unsigned int num_segments_orig = ceil((double)input_len / (double)SEGMENT_LENGTH);
    unsigned int thread_segment = 0, num_threads_orig = num_threads, num_segments = num_segments_orig;

    /*
        Encoding of data using Huffman Tree created and multithreading
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

            // ***** Need to copy segment of original vector data into a new vector
            //       to be distributed to the pthreads
            COL_DATA::const_iterator itr, itr_end;
            itr = input_data->begin() + (i*SEGMENT_LENGTH) 
                                + (thread_segment*num_threads_orig*SEGMENT_LENGTH);
            itr_end = itr + SEGMENT_LENGTH;
            if(itr_end > input_data->end()) itr_end = input_data->end(); 
            dargs->dBlock_Len = (int)(itr_end - itr);

            dargs->dBlock_itr = itr;
            dargs->dDict = new DICT;
            dargs->dTree = new Tree;

            all_output[i] = dargs;

            pthread_create(&threads[i], NULL, dictHelper, all_output[i]);
        }

        for(int i = 0; i < num_threads; i++){
            pthread_join(threads[i], NULL);

            // Append arrays to end of encoded
            encoded->insert(encoded->end(), 
                            all_output[i]->dDict->begin(), all_output[i]->dDict->end());

            // Merge in mappings
            mapping->merge(all_output[i]->dTree);
        }

        // Delete Dynamic Memory
        for(int i = 0; i < num_threads; i++){
            free(all_output[i]->dDict);
            free(all_output[i]->dTree);
        }

        num_segments -= num_threads;
        thread_segment++;
    } while(num_segments > 0);


    // Free all Dynamic Memory
    free(all_output);
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
    Tree* mapping = new Tree;
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

    // mapping->traverse(mapping->getRoot());

    // Calculating total time taken by the program.
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by multithreaded dictionary encoding program is : " << std::fixed
         << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;

    // Test reads, scans, etc




    return 0;
}
