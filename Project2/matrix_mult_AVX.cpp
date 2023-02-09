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
  Need to add padding to eliminate issues with 
*/
Matrix<float> operator*(const Matrix<float>& a, const Matrix<float>& b) {
  Matrix<float> c(a.Rows());  // Assuming square matrix
  for (int i = 0; i < a.Rows(); i++) {
    for (int j = 0; j < b.Cols(); j++) {
      __m256 sum = _mm256_setzero_ps();
      for (int k = 0; k < a.Cols(); k++) {
        __m256 a_ = _mm256_loadu_ps(&a(i, k));
        __m256 b_ = _mm256_loadu_ps(&b(k, j));
        sum = _mm256_add_ps(sum, _mm256_mul_ps(a_, b_));
      }
      float res[8];
      _mm256_storeu_ps(res, sum);
      c(i, j) = res[0] + res[1] + res[2] + res[3] + res[4] + res[5] + res[6] + res[7];
    }
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
  A.printMatrixWithPad();
  std::cout << std::endl;

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