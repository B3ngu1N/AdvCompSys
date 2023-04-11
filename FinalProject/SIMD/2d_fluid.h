/*
    2D Fluid Simulation with Navier-Stokes Equations Header File

    Credit - Jos Stam & Mike Ash

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/

int ITR = 16;
int N = 512;
#define SCALE 2

#include "p8g.hpp"

#include <iostream>
#include <cmath>
#include <cstring>
#include <omp.h>
// #include <immintrin.h>


class Fluid2D {
  public:
    Fluid2D(int sim_dimension, float diffusion, float viscosity, float dt_);
    
    ~Fluid2D() {
        delete[] this->Vx;
        delete[] this->Vx0;

        delete[] this->Vy;
        delete[] this->Vy0;

        delete[] this->density;
        delete[] this->s;
    }

    void AddDensity(int x, int y, float amount);

    void AddVelocity(int x, int y, float amountX, float amountY);

    void SimStep();

    void RenderDensity();
    void printDensity();
    void printVelocity();

  private:
    int size;
    float dt;
    float diff;
    float visc;

    float* Vx;
    float* Vy;

    float* Vx0;
    float* Vy0;

    float* s;
    float* density;

};

int IX(int i, int j);

void SetBoundaries(int b, float* in_x);

void LinSolve(int b, float* in_x, float* in_x0, float a, float c);

void Diffuse(int b, float* in_x, float* in_x0, float in_diff, float in_dt);

void Project(float* in_Vx, float* in_Vy, 
              float* p, float* div);

void Advect(int b, float* d, float* d0, float* in_Vx, 
            float* in_Vy, float in_dt);


