# Project 4: Implementation of Dictionary Codec

**Usage:** This program should be used for searching/scanning column files. 

**Compilation Arguments:** 
- The finished program is named "hash_dict.cpp"
- The program does not work without the "-omp" and "-lpthread" flags
- Example: g++ hash_dict.cpp -omp -lpthread -o hashdict.o

**Run Command/Arguments:**
- Argument 1: Column File Name
- Argument 2: Maximum Threads
- Argument 3 (Optional): Search Term
- Example: ./hashdict.o col_file.txt 10 Search_Term

**Libraries Used:**
- iostream
- utility
- string
- unordered_map
- bits/stdc++.h
- fstream
- pthread.h
- omp.h

**Code Structure:**

Our program first reads in a column file and encodes it. To encode the file, the file is split up into segments and each segment is assigned a thread which will compress that segment into its own small dictionary pointer, with its key stored in a map pointer. The dictionaries and keys are then merged in order, as the threads are joined. Then the dynamic memory is deleted, thus the file has completed its encoding into a hash table. This whole process is timed, and printed out so the user can see. If the project was reformatted to two files, one which encodes and saves the map and dictionary, and one which queries, then this operation would happen one time, therefore not affecting search/scan times. Thus, this will not be made a point in the experimental results section.

After creating the hash table, the program begins a new timer and searches the hash table for exact matches of the given term. The now-encoded dictionary is split into segments and each segment given a thread. Each thread uses SIMD instructions to iterate through its set and if a key match is found, the index is added to a vector of unsigned longs. These are consolodated in order as segments complete. Then, dynamic memory is deleted, the timer is stopped, and all indices are printed. If the user has instead entered a prefix, a timer begins, and the hash table is searched for values with the prefix of the entered value. All instances are returned to a vector of strings. These strings are searched individually using multithreading and SIMD instructions, much like previously. When all indices of searchable values have been found, the clock stops, the results are printed, then the time taken is printed.

In the vanilla column search (vanilla_search.cpp), the file is opened, then the clock begins. Each line is searched for the term, if found, it is added to a vector. After the entire file has been searched, the timer stops, the file is closed, the found incides are printed, and then the time is printed. This will give good experimental comparisons for the exact matches of the given term.

**Experimental Results:**


**Analysis and Conclusions:**

