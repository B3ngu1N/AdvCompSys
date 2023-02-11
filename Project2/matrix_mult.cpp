/*
  This program uses AVX SIMD functions to parallelize a matrix-matrix multiplication.

  Compilation: g++ matrix_mult.cpp -mavx2 -o mult_avx.o

  @author Thomas Petr
  @author Ben Haft
  @date 2/4/2023
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <bits/stdc++.h>
#include <immintrin.h>
#include <emmintrin.h>

/*
  Custom matrix datatype/object to allow for ease of implementation for both 
  short and float datatypes without massive reusage of code.

  Note: For datatypes of 2-byte length it will use SSE instructions and for 
  datatypes of 4-byte length it will use AVX instructions. This was done to 
  streamline the math required for 16-bit datatypes, which AVX instructions 
  have poor utility functions for. This reduces our possible performance gain 
  of using smaller datatypes but made it much easier to implement.
*/
template <typename T>
class Matrix {
 public:
 // Need to add padding to allocation to allow for 0s at end of rows and columns
 // to be a multiple of 256 bits
  Matrix(int matrix_dim) : rows_(matrix_dim), cols_(matrix_dim) {
    // choose SSE or AVX depending on input type
    int avx_or_sse = 256;
    if(sizeof(T) < 4) avx_or_sse = 128;
    int segment_breakup = avx_or_sse/(sizeof(T)*8);
    int padding_dim = ceil((float)matrix_dim/segment_breakup) * segment_breakup;
    data_ = new T[padding_dim * padding_dim];
    memset(data_, (T)0.0, sizeof(T) * padding_dim * padding_dim);
    rows_padding = padding_dim;
    cols_padding = padding_dim;
  }
  
  ~Matrix() {
    delete[] data_;
  }

  int Rows() const { return rows_; }
  int Cols() const { return cols_; }
  int RowsPad() const { return rows_padding; }
  int ColsPad() const { return cols_padding; }
  T* Data() { return data_; }
  const T* Data() const { return data_; }

  T& operator()(int i, int j) { return data_[i * cols_padding + j]; }
  const T& operator()(int i, int j) const { return data_[i * cols_padding + j]; }

  void setVal(int i, int j, T val) { data_[i * cols_padding + j] = val; }
  void printMatrix();
  void printMatrixWithPad();

 private:
  int rows_;
  int cols_;
  int rows_padding;
  int cols_padding;
  T* data_;
};


template <>
void Matrix<float>::printMatrix()
{
  int i, j;
  for(i=0; i < rows_; ++i){
    for(j=0; j < cols_; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(7) << std::setfill(' ') << std::setprecision(4)
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

template <>
void Matrix<short>::printMatrix()
{
  int i, j;
  for(i=0; i < rows_; ++i){
    for(j=0; j < cols_; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(5) << std::setfill(' ')
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

template <>
void Matrix<float>::printMatrixWithPad()
{
  int i, j;
  for(i=0; i < rows_padding; ++i){
    for(j=0; j < cols_padding; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(5) << std::setfill(' ') << std::setprecision(2)
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

template <>
void Matrix<short>::printMatrixWithPad()
{
  int i, j;
  for(i=0; i < rows_padding; ++i){
    for(j=0; j < cols_padding; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(5) << std::setfill(' ') 
                << static_cast<float>(data_[i*cols_padding + j]);
    }
    std::cout << std::endl;
  }
}

/*
  Partial Mult Helper Function - Does 8x8 dimension float multiplication
*/
void multHelper_8x8(const Matrix<float>& a, int i_A, int j_A,
                             const Matrix<float>& b, int i_B, int j_B,
                             Matrix<float>& c)
{
  int seg_breakup = 256/(sizeof(float)*8);
  __m256 sum_current_row = _mm256_setzero_ps();
  for (int i_a = i_A; i_a < i_A+seg_breakup; i_a++) { 
    __m256 a_ = _mm256_loadu_ps(&a(i_a, j_A)); // Get row of 8x elements from A
    float a_row[seg_breakup]; // 8x 32bit floats in 256 bits
    _mm256_storeu_ps(a_row, a_); // Breakup row of data into individual elements

    for (int i_b = i_B, a_row_itr=0; i_b < i_B+seg_breakup; i_b++, a_row_itr++) { // Go through rows of B
      __m256 dup_rVal = _mm256_set1_ps(a_row[a_row_itr]); // duplicate value from row to all values in __m256

      __m256 b_row = _mm256_loadu_ps(&b(i_b, j_B)); // load in row of B

      // Multiply every value of B row with the duplicated A row value, 
      // then add each value to the sums for the row. Each sum value is
      // the value to be put in the position of the 8x8 row value.
      sum_current_row = _mm256_add_ps(sum_current_row, _mm256_mul_ps(dup_rVal, b_row));
    }

    // Load in the information already at C(i_a, j_B) and add new information to it
    __m256 temp = _mm256_loadu_ps(&c(i_a, j_B)); // get current result at c(i_a,j_B)
    sum_current_row = _mm256_add_ps(sum_current_row, temp); // add current result to new results
    _mm256_storeu_ps(&c(i_a, j_B), sum_current_row); // all results for the current row of 8x output elements
    sum_current_row = _mm256_setzero_ps(); // reset the sums to 0.0
  }
}

/*
  Intel AVX Instruction-Based Matrix Multiplication for float data-type
*/
Matrix<float> operator*(const Matrix<float>& a, const Matrix<float>& b)
{
  Matrix<float> c(a.Rows());  // Assuming square matrix

  int seg_breakup = 256/(sizeof(float)*8);
  int num_256xBlock = ceil((float)c.Rows()/seg_breakup); // find the number of 256 bit segments
  
  // Must use Tiling with 8x8 Matrices
  // i & j correspond to tile/block of 8x8 matrix locations
  for(int i = 0; i < num_256xBlock; i++) {
    for(int j = 0; j < num_256xBlock; j++) {
      for (int k = 0; k < num_256xBlock; k++) {
        multHelper_8x8(a,i*seg_breakup,k*seg_breakup, b,k*seg_breakup,j*seg_breakup, c);
      }
    }
  }

  return c;
}


/*
  Partial Mult Helper Function - Does 8x8 dimension short multiplication
*/
void multHelper_8x8(const Matrix<short>& a, int i_A, int j_A,
                             const Matrix<short>& b, int i_B, int j_B,
                             Matrix<short>& c)
{
  int seg_breakup = 128/(sizeof(short)*8);
  __m128i sum_current_row = _mm_setzero_si128();
  for (int i_a = i_A; i_a < i_A+seg_breakup; i_a++) { 
    __m128i a_ = _mm_loadu_si128((__m128i const*)&a(i_a, j_A)); // Get row of 8x elements from A

    // The __m128i datatype stores information very differently from the __m256.
    // It stores the information as 2 64bit integers, so to get the information we
    // actually desire we decided to use bit-shifting and bit-masking.
    short a_row[seg_breakup] = {(short) ((a_[0] & 0x000000000000FFFF)),
                                (short) ((a_[0] & 0x00000000FFFF0000)>>(8*2)),
                                (short) ((a_[0] & 0x0000FFFF00000000)>>(8*4)),
                                (short) ((a_[0] & 0xFFFF000000000000)>>(8*6)),
                                (short) ((a_[1] & 0x000000000000FFFF)),
                                (short) ((a_[1] & 0x00000000FFFF0000)>>(8*2)),
                                (short) ((a_[1] & 0x0000FFFF00000000)>>(8*4)),
                                (short) ((a_[1] & 0xFFFF000000000000)>>(8*6))  }; // 8x 16bit short in 128 bits

    for (int i_b = i_B, a_row_itr=0; i_b < i_B+seg_breakup; i_b++, a_row_itr++) { // Go through rows of B
      __m128i dup_rVal = _mm_set1_epi16(a_row[a_row_itr]); // duplicate value from row to all values in __m128i

      __m128i b_row = _mm_loadu_si128((__m128i const*)&b(i_b, j_B)); // load in row of B

      // Multiply every value of B row with the duplicated A row value, 
      // then add each value to the sums for the row. Each sum value is
      // the value to be put in the position of the 8x8 row value.
      sum_current_row = _mm_add_epi16(sum_current_row, _mm_mullo_epi16(dup_rVal, b_row));
    }

    // Load in the information already at C(i_a, j_B) and add new information to it
    __m128i temp = _mm_loadu_si128((__m128i const*)&c(i_a, j_B)); // get current result at c(i_a,j_B)
    sum_current_row = _mm_add_epi16(sum_current_row, temp); // add current result to new results
    _mm_store_si128((__m128i*)&c(i_a, j_B), sum_current_row); // all results for the current row of 8x output elements
    sum_current_row = _mm_setzero_si128(); // reset the sums to 0
  }
}

/*
  Intel SSE Instruction-Based Matrix Multiplication for short data-type
*/
Matrix<short> operator*(const Matrix<short>& a, const Matrix<short>& b)
{
  Matrix<short> c(a.Rows());  // Assuming square matrix

  int seg_breakup = 128/(sizeof(short)*8);
  int num_128xBlock = ceil((float)c.Rows()/seg_breakup); // find the number of 128 bit segments
  
  // Must use Tiling with 8x8 Matrices
  // i & j correspond to tile/block of 8x8 matrix locations
  for(int i = 0; i < num_128xBlock; i++) {
    for(int j = 0; j < num_128xBlock; j++) {
      for (int k = 0; k < num_128xBlock; k++) {
        multHelper_8x8(a,i*seg_breakup,k*seg_breakup, b,k*seg_breakup,j*seg_breakup, c);
      }
    }
  }

  return c;
}


int main(int argc, const char** argv)
{
  clock_t start, end;
  const char* const exeName = argv[0]; // Name of file to compress

  if (argc != 3) { 
      printf("Wrong Arguments\n");
      printf("%s MATRIX_DIM DATA_TYPE\n", exeName);
      printf("DATA_TYPE = i (short) or f (float)\n");
      return 1;
  }

  // Square Matrix Dimensions
  int matrix_dim = atoi(argv[1]);
  std::string argv2 = argv[2];

  if(argv2 == "f")
  {
    // Setup of Matrix A and Matrix B
    // Values calculated with random float function
    int i, j;
    Matrix<float> A = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        A.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
      }
    }
    Matrix<float> B = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        B.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
      }
    }

    // Print out initial matrices (if dim < 20)
    if(matrix_dim <= 20)
    {
      A.printMatrix();
      std::cout << std::endl;
      B.printMatrix();
      std::cout << std::endl;
    }

    // Timer Start
    start = clock();

    // Matrix multiplication
    Matrix<float> C = A * B;

    // Timer End
    end = clock();

    // Print out final multiplied matrix
    if(matrix_dim <= 20) {
      C.printMatrix();
      std::cout << std::endl;
    }

    // Calculating total time taken by the program.
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by SIMD matrix multiplication is : " << std::fixed
          << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;
  }

  else if(argv2 == "i")
  {
    // Setup of Matrix A and Matrix B
    // Values calculated with random function
    int i, j;
    Matrix<short> A = Matrix<short>(matrix_dim); //value cap of a short is 65535
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        A.setVal(i, j, (short)(rand() % 10)); // float between 0 and 9
      }
    }
    Matrix<short> B = Matrix<short>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        B.setVal(i, j, (short)(rand() % 10)); // float between 0 and 9
      }
    }

    // Print out initial matrices (if dim < 20)
    if(matrix_dim <= 20)
    {
      A.printMatrix();
      std::cout << std::endl;
      B.printMatrix();
      std::cout << std::endl;
    }

    // Timer Start
    start = clock();

    // Matrix multiplication
    Matrix<short> C = A * B;

    // Timer End
    end = clock();

    // Print out final multiplied matrix
    if(matrix_dim <= 20) {
      C.printMatrix();
      std::cout << std::endl;
    }

    // Calculating total time taken by the program.
    double time_taken = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Time taken by SIMD matrix multiplication is : " << std::fixed
          << time_taken << std::setprecision(5);
    std::cout << " sec " << std::endl;
  }
  else{
    std::cout << "DATA_TYPE Input is Invalid - Try 'i' or 'f'" << std::endl;
  }

  if(matrix_dim > 20) {
    std::cout << "Did not print out matrices because matrix dimension >20." 
              << std::endl;
  }

  return 0;
}