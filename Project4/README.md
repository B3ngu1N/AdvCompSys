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

![image](https://user-images.githubusercontent.com/112660711/226413948-642704e4-d323-426d-9f42-58c83220dbe6.png)


These experimental results were gathered with the following hardware specifications:
- WSL Environment
- Intel Core i7-9750H CPU @ 2.60 Ghz
- 32.0 GB RAM at 2667 MHz
- L1d: 192 kB
- L1i: 192 kB
- L2: 1.5 MB
- L3: 12MB

**Analysis and Conclusions:**

The aim of this laboratory was to give hands-on experience of compressing data and speeding up search/scan operations using a dictionary codec. The main learning objective was to the intertwine hash table (or B-tree) with efficient data encoding and scanning. Our group succeeded in gaining hands-on experience and learning how to process data efficiently. To do this, we used multithreading and SIMD instructions. 

Like any project, there errors along the way. The first main problem we came across was implementing the prefix search option. Originally we thought a B-tree would be a much simpler way of making the program, but we found that it was far slower to encode than a hash table with approximately the same search time. Thus, we swapped back to hash table prefix encoding. Also, we ran into some VSCode bugs along the way, where one program would run on one laptop and not on another with the same softwares and libraries included.

Our experimental results yield magnitudes of difference between hash_dict.cpp and vanilla_search.cpp. Ultimately, this is as expected, as we are able to search/scan through encoded data significantly faster with multithreading. Through lots of experimental testing, I found an approximate optimized segment length for data search speed, which is 64000. I also found that while multithreading decreases encoding time, it slightly increases search/scan time, which we were comparing in our experimental results. Therefore, for the experimental results section I used single threadding for our search times. However, on the other work environment we used, multithreading does speed up the process until a certain arbitrary number of threads, just like in Project 2.

This project can be improved by being broken up. If we split files into encoding and searching, where encoding only had to happen once and the hash table could then be searched through, that would improve efficiency. I also think creating some vanilla prefix scanning tool may have been a good idea, so to compare the prefix scan speeds. However, the project description is suiting given the time frame to complete the lab. Ultimately, data processing has many applications, as efficiency in breaking down large files is extremely important when gathering large amounts of data. 
