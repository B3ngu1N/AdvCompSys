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
- stdio.h
- stdlib.h
- string.h
- common.h (from zstd.h GitHub examples: https://github.com/facebook/zstd/tree/dev/examples)

**Code Structure:**

Main:
- Assigns variables to runtime arguments
- Checks that runtime arguements are correct
- Calls createOutFilename() function
- Calls compressFile() function

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
