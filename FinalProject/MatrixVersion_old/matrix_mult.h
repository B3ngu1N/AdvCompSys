/*
    matrix_mult.h Header File

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <bits/stdc++.h>
#include <cmath>
#include <immintrin.h>

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
    Matrix() {
        this->rows_ = 0;
        this->rows_padding = 0;
        this->cols_ = 0;
        this->cols_padding = 0;
        this->data_ = NULL;
    };
    // Need to add padding to allocation to allow for 0s at end of rows and columns
    // to be a multiple of 256 bits
    Matrix(int matrix_dim) : rows_(matrix_dim), cols_(matrix_dim) {
        // choose SSE or AVX depending on input type
        int avx_or_sse = 256;
        if(sizeof(T) < 4) avx_or_sse = 128;
        int segment_breakup = avx_or_sse/(sizeof(T)*8);
        int padding_dim = ceil((float)matrix_dim/segment_breakup) * segment_breakup;
        data_ = (T*)malloc(sizeof(T) * padding_dim * padding_dim);
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

    T& operator()(int i, int j) { return data_[i + j * cols_padding]; }          //************** Flipped from original file
    const T& operator()(int i, int j) const { return data_[i + j * cols_padding]; }

    void setVal(int i, int j, T val) { data_[i + j * cols_padding] = val; }
    void printMatrix();
    void printMatrixWithPad();

  private:
    int rows_;
    int cols_;
    int rows_padding;
    int cols_padding;
    T* data_;
};

