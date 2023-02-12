# Advanced Computer Systems Project 2 - Matrix-Matrix Multiplication with SIMD Instructions & Cache Miss Minimization

**Usage:** This program should be used for square matrix-matrix calculations. 

**Compilation Arguments:** 
- The finished program is named "matrix_mult.cpp"
- The program does not work without the "-mavx" flag
- Example: g++ matrix_mult.cpp -mavx -o mult.o

**Run Command/Arguments:**
- Argument 1: Matrix Dimension
- Argument 2: Matrix Type: 'i' for 16 bit signed integer; 'f' for float
- Example: ./mult.o 1000 f

**Libraries Used:**
- stdio.h
- stdlib.h
- time.h
- iostream
- string
- bits/stdc++.h
- immintrin.h
- emmintrin.h

**Code Structure:**

Our program takes inputs of matrix dimension and matrix data type. With the matrix data type, the program differentiates whether to create a class of matrix that holds float or short (16-bit integer) values. Two square matrices are defined with the correct data type and the given dimension. They are each filled randomly with values using the rand() function. If the inputted dimension is less than or equal to 20, each matrix is printed, such that the values can be checked in the terminal. The timer begins, our algorithm (described below) runs, and the timer ends. Then with the same condition as before, the output matrix is printed. The timing of the calculation is also listed.

![image](https://user-images.githubusercontent.com/112660711/218283406-faf6dc23-df5d-4e1c-a075-506c6deee4fb.png)

Our algorithm works in 8x8 tiles. Referencing the image above, in each tile the first value of A is duplicated 8 times into a vector (since 2x2 matrix, the picture shows the first two duplications). Then the row full first row of matrix B is multiplied by each of the duplicated A values. Then, the process repeats with the next value in the first row of matrix A. When the first row is compelted, the program moves on to the second row.

We also created a simple naive program named matrix_mult_naive.cpp. This can be used as a comparison to our efficient matrix multiplication. The naive program only uses for loops and embedded calculation, without cache miss optimization or SIMD instructions. Results are shown below in the experimental results section.

**Experimental Results:**

![image](https://user-images.githubusercontent.com/112660711/218287107-3819c4c1-b6f0-455f-bdb4-7aa079e867b6.png)


These experimental results were gathered with the following hardware specifications:
- WSL Environment
- Intel Core i7-9750H CPU @ 2.60 Ghz
- 32.0 GB RAM at 2667 MHz
- L1d: 192 kB
- L1i: 192 kB
- L2: 1.5 MB
- L3: 12MB


**Analysis and Conclusions:**
