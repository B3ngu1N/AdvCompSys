/*
    Vanilla Search/Scan

    Compilation: g++ vanilla_search.cpp -g -o van.o

    @author Thomas Petr
    @author Ben Haft
*/
#include <iostream>
#include <fstream>
#include <string>
#include <bits/stdc++.h>

int main(int argc, char** argv)
{
    clock_t start, end;
    const char* const exeName = argv[0]; // Name of .o file to run
    const char* const inFilename = argv[1]; // Name of column file to search
    std::string search_term = argv[2]; // Search term
    std::vector<size_t> indices;

    if (argc == 2) { // Need 3 arguments
        printf("Wrong Arguments\n");
        printf("%s INFILE Search_Term\n", exeName);
        return 1;
    }

    // Open the file
    std::ifstream file(inFilename);
    if (!file.is_open()) {
        std::cout << "Error opening file!" << std::endl;
        return 1;
    }

    // Timer Start
    start = clock();

    // Search for matching lines
    std::string line;
    size_t line_number = 1;
    while (std::getline(file, line)) {
        if (line.find(search_term) != std::string::npos) {
            indices.push_back(line_number);
        }
        line_number++;
    }
    // Timer End
    end = clock();

    // Close the file
    file.close();

    // Print the found lines
    std::cout << "Indices: ";
    for (auto i : indices) {
        std::cout << i << " ";
    }
    std::cout << std::endl;


    // Calculating total time taken by the program.
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by vanilla read is : " << std::fixed
         << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;
    
    return 0;
}