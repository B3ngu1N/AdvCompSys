# Advanced Computer Systems Project 2:
**Matrix-Matrix Multiplication with SIMD Instructions & Cache Miss Minimization**

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

![image](https://user-images.githubusercontent.com/112660711/218289147-07a5db41-e264-4d83-851a-09d3a065e44f.png)


These experimental results were gathered with the following hardware specifications:
- WSL Environment
- Intel Core i7-9750H CPU @ 2.60 Ghz
- 32.0 GB RAM at 2667 MHz
- L1d: 192 kB
- L1i: 192 kB
- L2: 1.5 MB
- L3: 12MB


**Analysis and Conclusions:**

The goal of this assignment was to give us understanding of SIMD programming and cache access optimization. By completing this project, we gained valuable hands-on experience with these concepts. We minimalized cache miss rate by storing the matrix as a vector from left to right then top to bottom and indexing in row order as much as possible. This increased efficiency and was the defining factor of our algorithm. We also explored both AVX and SSE commands in our program. This taught us multiple functions and applications of SIMD programming, and we successfully minimalized cache miss rates.

Along the way, we had many difficulties and problems with our program. Thinking of and defining our algorithm was especially difficult, and we brainstormed and drew out different ideas (e.g. the visual in the Code Structure section), which eventually led us to build a successful algorithm. A problem along the way was incorporating tiling in order to support larger dimensioned matrices. We originally used AVX commands for float matrix calculations, which worked, however, when we transitioned to 16-bit integer calculations, the necessary functions were not available with AVX. So we had to find applicable SSE functions to replace the AVX functions. Then, we realized SSE functions were stored as two 64-bit values instead of 8 16-bit values, making them much harder to index. To combat this, we used bit masking and bit shifting.

Our experimental results were as expected. Our programs is vastly more efficient than the naive program we tested against. The trend in the graph shows that as the matrix dimension increases, the SIMD program outperforms the naive program by more. In the case of a 2000x2000 float matrix, our program was 10x more efficient than its naive counterpart. In the cases of 2x2, 5x5, 10x10, 20x20, and 10000x10000, values were either too small or too large to compare, thus those were left out of the graph.

Creating efficient matrix-matrix multiplication algorithms has many real-world applications. According to sciencedirect.com, these include "network theory, solution of linear systems of equations, transformation of co-ordinate systems, and population modeling." Gaining efficiency in using SIMD commands and minimizing cache miss rate allows us to consider trade-offs, and make our code much more efficient than just doing one of the two. Therefore, this was a well designed laboratory exercise.
