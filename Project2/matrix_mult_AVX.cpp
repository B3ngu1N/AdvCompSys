/*
  This program uses AVX SIMD functions to parallelize a matrix-matrix multiplication.

  Compilation: g++ matrix_mult.cpp -mavx -mfma -o mult_avx.o

  @author Thomas Petr
  @author Ben Haft
  @date 2/4/2023
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <bits/stdc++.h>
#include <cstdint>
#include <cstring>
#include <immintrin.h>

template <typename T>
class Matrix {
 public:
 // Need to add padding to allocation to allow for 0s at end of rows and columns
 // to be a multiple of 256 bits
  Matrix(int matrix_dim) : rows_(matrix_dim), cols_(matrix_dim) {
    int segment_breakup = 256/(sizeof(T)*8);
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
      std::cout << std::setw(5) << std::setfill(' ') << std::setprecision(2)
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



/*
  Intel AVX Instruction-Based Matrix Multiplication
*/
Matrix<float> operator*(const Matrix<float>& a, const Matrix<float>& b) {
  Matrix<float> c(a.Rows());  // Assuming square matrix

  __m256 sum_current_row = _mm256_setzero_ps();

  for (int i = 0; i < a.Rows(); i++) { 
    __m256 a_ = _mm256_loadu_ps(&a(i, 0)); // Get row of 8x elements from A
    float a_row[8]; // 8x 32bit floats in 256 bits
    _mm256_storeu_ps(a_row, a_); // Breakup row of data into individual elements

    for (int j = 0; j < b.Rows(); j++) { // Go through rows of B
      __m256 dup_rVal = _mm256_set1_ps(a_row[j]); // duplicate value from row to all values in __m256

      __m256 b_row = _mm256_loadu_ps(&b(j, 0)); // load in row of B

      sum_current_row = _mm256_add_ps(sum_current_row, _mm256_mul_ps(dup_rVal, b_row));
    }

    _mm256_storeu_ps(&(c(i, 0)), sum_current_row); // all results for the current row of 8x output elements
    sum_current_row = _mm256_setzero_ps();
  }

  return c;
}



int main(int argc, const char** argv)
{
  clock_t start, end;
  const char* const exeName = argv[0]; // Name of file to compress

  if (argc != 2) { 
      printf("Wrong Arguments\n");
      printf("%s MATRIX_DIM\n", exeName);
      return 1;
  }

  // Square Matrix Dimensions
  int matrix_dim = atoi(argv[1]);

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
  C.printMatrixWithPad();
  std::cout << std::endl;


  // Calculating total time taken by the program.
  double time_taken = double(end - start) / CLOCKS_PER_SEC;
  std::cout << "Time taken by SIMD matrix multiplication is : " << std::fixed
        << time_taken << std::setprecision(5);
  std::cout << " sec " << std::endl;

  return 0;
}