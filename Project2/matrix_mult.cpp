/*
  This program uses AVX SIMD functions to parallelize a matrix-matrix multiplication.

  Compilation: g++ matrix_mult.cpp -mavx -mfma -o mult.o

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
#include <omp.h>

template <typename T>
class Matrix {
 public:
  Matrix(int matrix_dim) : rows_(matrix_dim), cols_(matrix_dim) {
    data_ = new T[matrix_dim * matrix_dim];
    memset(data_, 0, sizeof(T) * matrix_dim * matrix_dim);
  }
  
  ~Matrix() {
    delete[] data_;
  }

  int Rows() const { return rows_; }
  int Cols() const { return cols_; }
  T* Data() { return data_; }
  const T* Data() const { return data_; }

  T& operator()(int i, int j) { return data_[i * cols_ + j]; }
  const T& operator()(int i, int j) const { return data_[i * cols_ + j]; }

  void setVal(int i, int j, T val) { data_[i * cols_ + j] = val; }
  void printMatrix();

 private:
  int rows_;
  int cols_;
  T* data_;
};


template <>
void Matrix<float>::printMatrix()
{
  int i, j;
  for(i=0; i < rows_; ++i){
    for(j=0; j < cols_; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(7) << std::setfill(' ') << std::setprecision(3)
                << static_cast<float>(data_[i*cols_ + j]);
    }
    std::cout << std::endl;
  }
}

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

template<>
void Matrix<int16_t>::printMatrix()
{
  int i, j;
  for(i=0; i < rows_; ++i){
    for(j=0; j < cols_; ++j){
      if (j != 0) { std::cout << " "; }
      std::cout << std::setw(7) << std::setfill(' ')
                << static_cast<float>(data_[i*cols_ + j]);
    }
    std::cout << std::endl;
  }
}

Matrix<int16_t> operator*(const Matrix<int16_t>& a, const Matrix<int16_t>& b) {
  Matrix<int16_t> c(a.Rows());  // Assuming square matrix
  int Segs = ceil(a.Rows()/16.);
  int Remainder = a.Rows() % 16;
  for (int n = 0; n < a.Cols(); n++){ // Moves down column
    for (int m = 0; m < a.Rows(); m++){ // Moves across row
      for (int i = 0; i < Segs; i++){ // Moves to next segment
        for (int j = 0; j < 16; j++){ // Moves across row inside segment 
          for (int k = 0; k < a.Rows()*a.Cols(); k = k + a.Cols()){ // Moves across column
            
          }
        }
      }
    }
  }
  return c;
}
// __m256 sum = _mm256_setzero_ps();
// for (int k = 0; k < a.Cols(); k++) {
//   __m256 a_ = _mm256_loadu_ps(&a(i, k));
//   __m256 b_ = _mm256_loadu_ps(&b(k, j));
//   sum = _mm256_add_ps(sum, _mm256_mul_ps(a_, b_));
// }
// float res[8];
// _mm256_storeu_ps(res, sum);
// c(i, j) = res[0] + res[1] + res[2] + res[3] + res[4] + res[5] + res[6] + res[7];

// void nativeMatrix();

int main(int argc, const char** argv)
{
  clock_t start1, start2, end1, end2;
  const char* const exeName = argv[0]; // Name of file to compress

  if (argc != 3) { 
      printf("Wrong Arguments\n");
      printf("%s MATRIX_DIM\n", exeName);
      return 1;
  }

  // Square Matrix Dimensions
  int matrix_dim = atoi(argv[1]);

  // int ? selection
  char data_type = atoi(argv[2]);

  if ((data_type != 'f') & (data_type != 'i')){
    printf("Wrong Arguments\n");
    printf("Input 'f' for float and 'i' for integer");
    return 1;
  }

  // Setup of Matrix A and Matrix B
  // Values calculated with random float function
  Matrix<float> Af = Matrix<float>(matrix_dim);
  Matrix<float> Bf = Matrix<float>(matrix_dim);
  Matrix<float> Cf = Matrix<float>(matrix_dim);
  Matrix<int16_t> Ai = Matrix<int16_t>(matrix_dim);
  Matrix<int16_t> Bi = Matrix<int16_t>(matrix_dim);
  Matrix<int16_t> Ci = Matrix<int16_t>(matrix_dim);


  int i, j;
  if (data_type == 'f'){
    Af = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        Af.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
      }
    }
    Bf = Matrix<float>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        Bf.setVal(i, j, rand() / (RAND_MAX + 1.)); // float between 0 and 1
      }
    }
  }
  else{
    Ai = Matrix<int16_t>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        Ai.setVal(i, j, (int16_t)rand()); // float between 0 and 1
      }
    }
    Bi = Matrix<int16_t>(matrix_dim);
    for( i = 0; i < matrix_dim; ++i) {
      for( j = 0;  j < matrix_dim; ++j) {
        Bi.setVal(i, j, (int16_t)rand()); // float between 0 and 1
      }
    }
  }

  // Print out initial matrices (if dim < 20)
  if(matrix_dim <= 20)
  {
    if (data_type == 'f'){
      Af.printMatrix();
      std::cout << std::endl;
      Bf.printMatrix();
      std::cout << std::endl;
    }
      Ai.printMatrix();
      std::cout << std::endl;
      Bi.printMatrix();
      std::cout << std::endl;
  }

  // Timer Start
  start1 = clock();

  // Matrix multiplication
  if (data_type == 'f') Cf = Af * Bf;
  else Ci = Ai * Bi;


  // Timer End
  end1 = clock();

  // Timer Start
  start2 = clock();

  // Native Matrix multiplication
  // nativeMatrix();

  // Timer End
  end2 = clock();


  // Print out final multiplied matrix
  if(matrix_dim <= 20)
  {
    if (data_type == 'f') Cf.printMatrix();
    else Ci.printMatrix();
    std::cout << std::endl;
  }

  // Calculating total time taken by the program.
  double time_taken1 = double(end1 - start1) / CLOCKS_PER_SEC;
  std::cout << "Time taken by SIMD matrix multiplication is : " << std::fixed
        << time_taken1 << std::setprecision(5);
  std::cout << " sec " << std::endl;

  // Calculating total time taken by the program.
  double time_taken2 = double(end2 - start2) / CLOCKS_PER_SEC;
  std::cout << "Time taken by Native matrix multiplication is : " << std::fixed
        << time_taken2 << std::setprecision(5);
  std::cout << " sec " << std::endl;

  return 0;
}