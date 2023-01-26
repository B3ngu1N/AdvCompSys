**Advanced Computer Systems Project 1 - Multiple Thread Data Compression**

**Usage:** This program should be used for compressing a file into a ".zst" file type. In most cases this will make the file size decrease significantly.

**Compilation Arguments:** 
- The finished program is named "Project1_Final.cpp"
- The program does not work without the "-lzstd" flag
- Example: g++ Project1_Final.cpp -lzstd -o test.o

**Run Command/Arguments:**
- Argument 1: num_threads = Max Number of Worker Threads {Does Not Include Main Thread}; Total Threads Used = num_threads + 1
- Argument 2: exeName = Input File to be Compressed
- Argument 3 (Optional):  cLevel = Level of Compression {Default = 1}
- Example: ./test.o 15 littlewomen.txt

**Libraries Used:**
- zstd.h
- pthread.h
- iostream
- fstream
- cmath
- time.h
- bits/stdc++.h
- stdio.h
- stdlib.h
- string.h
- common.h (from zstd.h GitHub examples: https://github.com/facebook/zstd/tree/dev/examples)

**Code Structure:**

Main:
- Assigns variables to runtime arguments
- Checks that runtime arguements are correct
- Calls createOutFilename() function
- Reads clock
- Calls compressFile() function
- Reads clock
- Writes compression time

createOutFilename:
- Creates output file name in memory with file type ".zst"
- Returns pointer to memory location

compressFile:
- Opens input file
- Finds file length
- Divides file length by number of 16kB segments, then rounds up
- Loop until all segments have been processed:
  - Read 16kB of data starting from applicable address
  - Assign necessary struct variables
  - Create thread to compress data with compressHelper()
  - Write to output file in order of thread number
  - Delete dynamic memory
  - Mark processed segments
- Print input file name, input file length, output file length, output file name
- Close files
- Free all dynamic memory

compressHelper:
- Compresses 16kB block
- Checks for errors in compression
- Calculates size of output data
- Tells thread to exit
- Returns struct with input data pointer, output data pointer, output data length, and compression level

**Analysis and Conclusions:**

  This laboratory exercise was assigned to teach us how to use two real-world tools in software engineering: multithreading and data compression. We successfully completed the lab by utilizing the ZSTD and pthread libraries. While programming our project, we were mindful of efficiency and allocated memory. We found a solution to read data in by 16kB chunks and assign threads to compress the data as the input file was read. This would improve efficiency when compressing large files because some threads could be compressing while the file was still being read and this is an efficient usage of memory since the entire input data was not read at one time. We also freed up dynamic memory when we saw fit. 

  There were many obstacles to overcome while working on this project. In the beginning, there is little documentation of the “-lzstd” flag that is necessary for compiling any program which uses the ZSTD library. We tried using the streaming compression API and turned back to simple compression because the program was overcomplicated, and we felt it was more suited to a single thread operation. We also attempted to use open_memstream() as a means to create a buffer of dynamic length. This failed because when we used fwrite(), the data did not appear in our buffer. Another main error we ran into was a faulty decompressing file. The program would only decompress the first frame of the file, so we thought our program was not iterating correctly. We figured out that since we were segmenting our output data, we needed to use the streaming compression API. This allowed us to successfully decompress our “.zst” file.
  
  Regarding our experimental results, we found there was an optimized thread count for our 22MB tested file. We think this may be due to delay from overhead. However, we did find that using more than one thread is clearly faster than using than using one thread. Overall, multithreading significantly improves time, but clearly does not add outstanding performance gains.

  This lab has many real-world applications. Multithreading is an efficient way to load any top of large data. Compression is built into many storage systems and databases. Therefore, pairing them together is a great exercise. We learned how to use multithreading and compression to our advantage in this lab, and are capable of applying this to programming operations in the future.


