/*
    2D Fluid Simulation with Navier-Stokes Equations Header File

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/
#include "matrix_mult.h"
#include <stdlib.h>

class Fluid2D {
  public:
    Fluid2D(int sim_dimension, float diffusion, float viscosity, float dt_) : 
            size(sim_dimension), dt(dt_), diff(diffusion), visc(viscosity) {
        this->Vx = Matrix<float>(sim_dimension);
        this->Vy = Matrix<float>(sim_dimension);

        this->Vx0 = Matrix<float>(sim_dimension);
        this->Vy0 = Matrix<float>(sim_dimension);

        this->s = Matrix<float>(sim_dimension);
        this->density = Matrix<float>(sim_dimension);
    }
    
    ~Fluid2D() {}

    void AddDensityScalar(int x, int y, float amount);

    void AddVelocityScalar(int x, int y, float amountX, float amountY);

    void SimStep();

    void SetBoundaries(int b);

    void LinSolve(int b, float a, float c, int itr);

    void Diffuse(int b, float diff, float dt, int itr);

    void Project(int itr);

    void Advect(int b, float dt);


  private:
    int size;
    float dt;
    float diff;
    float visc;

    Matrix<float> Vx;
    Matrix<float> Vy;

    Matrix<float> Vx0;
    Matrix<float> Vy0;

    Matrix<float> s;
    Matrix<float> density;

};

