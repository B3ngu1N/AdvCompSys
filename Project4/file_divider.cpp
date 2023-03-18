/*
    Program for splitting up large test file given to us into usable small test files.
*/
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    string filename = "Column.txt"; // name of input file
    int segment_size = 512000; // size of each segment in bytes
    int segment_number = 0; // number of current segment
    int segment_size_counter = 0; // counter for current segment size
    string output_filename = "output_"; // prefix for output filenames
    
    ifstream infile(filename);
    if (!infile) {
        cerr << "Error: could not open input file." << endl;
        return 1;
    }
    
    while (!infile.eof()) {
        char buffer[1024];
        infile.read(buffer, 1024);
        int read_bytes = infile.gcount();
        
        if (read_bytes == 0) {
            break; // end of file
        }
        
        segment_size_counter += read_bytes;
        
        if (segment_size_counter > segment_size) {
            // current segment is too large, start new segment
            segment_number++;
            segment_size_counter = read_bytes;
        }
        
        ofstream outfile(output_filename + to_string(segment_number) + ".txt", ios::app);
        if (!outfile) {
            cerr << "Error: could not open output file." << endl;
            return 1;
        }
        
        outfile.write(buffer, read_bytes);
        if(segment_number > 0){ //How many output files/segments written
            break;
        }
    }
    
    return 0;
}