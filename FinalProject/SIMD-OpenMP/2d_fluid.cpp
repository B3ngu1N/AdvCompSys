/*
    2D Fluid Simulation with Navier-Stokes Equations - Currently not Multithreaded/SIMD with OpenMP

    Credit - Jos Stam & Mike Ash

    @author Thomas Petr
    @author Ben Haft
    @date 4/19/2023
*/

#include "2d_fluid.h"

int ITR = 16;
int N = 512;

Fluid2D::Fluid2D(int sim_dimension, float diffusion, float viscosity, float dt_)
{
    this->size = sim_dimension;
    this->diff = diffusion;
    this->visc = viscosity;
    this->dt = dt_;

    this->Vx = (float*)malloc(sizeof(float) * sim_dimension * sim_dimension);
    memset(this->Vx, 0.0, sizeof(float) * sim_dimension * sim_dimension);

    this->Vy = (float*)malloc(sizeof(float) * sim_dimension * sim_dimension);
    memset(this->Vy, 0.0, sizeof(float) * sim_dimension * sim_dimension);

    this->Vx0 = (float*)malloc(sizeof(float) * sim_dimension * sim_dimension);
    memset(this->Vx0, 0.0, sizeof(float) * sim_dimension * sim_dimension);

    this->Vy0 = (float*)malloc(sizeof(float) * sim_dimension * sim_dimension);
    memset(this->Vy0, 0.0, sizeof(float) * sim_dimension * sim_dimension);

    this->s = (float*)malloc(sizeof(float) * sim_dimension * sim_dimension);
    memset(this->s, 0.0, sizeof(float) * sim_dimension * sim_dimension);

    this->density =(float*)malloc(sizeof(float) * sim_dimension * sim_dimension);
    memset(this->density, 0.0, sizeof(float) * sim_dimension * sim_dimension);
}

void Fluid2D::AddDensity(int x, int y, float amount)
{
    this->density[IX(x, y)] += amount;
}

void Fluid2D::AddVelocity(int x, int y, float amountX, float amountY)
{
    this->Vx[IX(x, y)] += amountX;
    this->Vy[IX(x, y)] += amountY;
}

void Fluid2D::SimStep()
{
    Diffuse(1, this->Vx0, this->Vx, this->visc, this->dt);
    Diffuse(2, this->Vy0, this->Vy, this->visc, this->dt);

    Project(this->Vx0, this->Vy0, this->Vx, this->Vy);

    Advect(1, this->Vx, this->Vx0, this->Vx0, this->Vy0, this->dt);
    Advect(2, this->Vy, this->Vy0, this->Vx0, this->Vy0, this->dt);

    Project(this->Vx, this->Vy, this->Vx0, this->Vy0);

    Diffuse(0, this->s, this->density, this->diff, this->dt);
    Advect(0, this->density, this->s, this->Vx, this->Vy, this->dt);
}

void Fluid2D::RenderDensity()
{
    p8g::colorMode(p8g::HSB);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int x = i * SCALE;
            int y = j * SCALE;
            float d = this->density[IX(i, j)];
            p8g::fill(((int)d+50)%255, 0, (int)d); //x, 220, (int)d
            p8g::noStroke();
            p8g::rect(x, y, SCALE, SCALE);
        }
    }
}

void Fluid2D::printDensity()
{
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            std::cout << this->density[IX(i, j)] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void Fluid2D::printVelocity()
{
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            std::cout << this->Vx[IX(i, j)] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int IX(int i, int j)
{
    return i + j*N;
}

void SetBoundaries(int b, float* in_x)
{
    // #pragma omp parallel loop
    for (int i = 1; i < N-1; i++) {
        in_x[IX(i, 0)] = b == 2 ? -in_x[IX(i, 1)] : in_x[IX(i, 1)];
        in_x[IX(i, N-1)] = b == 2 ? -in_x[IX(i, N-2)] : in_x[IX(i, N-2)];
    }

    // #pragma omp parallel loop
    for (int j = 1; j < N-1; j++) {
        in_x[IX(0, j)] = b == 1 ? -in_x[IX(1, j)] : in_x[IX(1, j)];
        in_x[IX(N-1, j)] = b == 1 ? -in_x[IX(N-2, j)] : in_x[IX(N-2, j)];
    }

    in_x[IX(0, 0)] = 0.5 * (in_x[IX(1, 0)] 
                            + in_x[IX(0, 1)]);
    in_x[IX(0, N-1)] = 0.5 * (in_x[IX(1, N-1)] 
                            + in_x[IX(0, N-2)]);
    in_x[IX(N-1, 0)] = 0.5 * (in_x[IX(N-2, 0)] 
                            + in_x[IX(N-1, 1)]);
    in_x[IX(N-1, N-1)] = 0.5 * (in_x[IX(N-2, N-1)] 
                            + in_x[IX(N-1, N-2)]);
}

void LinSolve(int b, float* in_x, float* in_x0, float a, float c)
{
    float cRecip = 1.0 / c;
    for (int t = 0; t < ITR; t++) {
        #pragma omp parallel loop
        for (int j = 1; j < N-1; j++) {
            for (int i = 1; i < N-1; i++) {
                in_x[IX(i, j)] = (in_x0[IX(i, j)] +
                                a * (in_x[IX(i+1, j)] +
                                    in_x[IX(i-1, j)] +
                                    in_x[IX(i, j+1)] +
                                    in_x[IX(i, j-1)])) * 
                                cRecip;
            }
        }
        SetBoundaries(b, in_x);
    }
}

void Diffuse(int b, float* in_x, float* in_x0, float in_diff, float in_dt)
{
    float a = in_dt * in_diff * (N - 2) * (N - 2);
    LinSolve(b, in_x, in_x0, a, 1 + 6 * a);
}

void Project(float* in_Vx, float* in_Vy, float* p, float* div)
{
    #pragma omp parallel loop
    for (int j = 1; j < N-1; j++) {
        for (int i = 1; i < N-1; i++) {
            div[IX(i, j)] = (-0.5 *
                            (in_Vx[IX(i+1, j)] -
                            in_Vx[IX(i-1, j)] +
                            in_Vy[IX(i, j+1)] -
                            in_Vy[IX(i, j-1)])) / N;
            p[IX(i, j)] = 0.0;
        }
    }

    SetBoundaries(0, div);
    SetBoundaries(0, p);
    LinSolve(0, p, div, 1, 6);

    #pragma omp parallel loop
    for (int j = 1; j < N-1; j++) {
        for (int i = 1; i < N-1; i++) {
            in_Vx[IX(i, j)] -= 0.5 * (p[IX(i+1, j)] 
                                    - p[IX(i-1, j)]) * N;
            in_Vy[IX(i, j)] -= 0.5 * (p[IX(i, j+1)] 
                                    - p[IX(i, j-1)]) * N;
        }
    }

    SetBoundaries(1, in_Vx);
    SetBoundaries(2, in_Vy);
}

void Advect(int b, float* d, float* d0, float* in_Vx, float* in_Vy, float in_dt)
{
    int j, jtmp, i, itmp;
    float Ntmp = N - 2;
    float dtX = in_dt * Ntmp, dtY = in_dt * Ntmp;

    float tmp1, tmp2, tmp3;
    float i_flr, i_ceil, j_flr, j_ceil;
    float x, y, s0, s1, t0, t1;

    // #pragma omp parallel loop
    for (j = 1, jtmp = 1; j < N-1; j++, jtmp++) {
        for (i = 1, itmp = 1; i < N-1; i++, itmp++) {
            tmp1 = dtX * in_Vx[IX(i, j)];
            tmp2 = dtY * in_Vy[IX(i, j)];
            x = (float)itmp - tmp1;
            y = (float)jtmp - tmp2;

            if(x < 0.5) x = 0.5;
            if(x > Ntmp + 0.5) x = Ntmp + 0.5;

            i_flr = floor(x);
            i_ceil = i_flr + 1;
            
            if(y < 0.5) y = 0.5;
            if(y > Ntmp + 0.5) y = Ntmp + 0.5;

            j_flr = floor(y);
            j_ceil = j_flr + 1;

            s1 = x - i_flr;
            s0 = 1.0 - s1;
            t1 = y - j_flr;
            t0 = 1.0 - t1;

            d[IX(i, j)] = s0 * (t0 * d0[IX(i_flr, j_flr)] + t1 * d0[IX(i_flr, j_ceil)]) +
                        s1 * (t0 * d0[IX(i_ceil, j_flr)] + t1 * d0[IX(i_ceil, j_ceil)]);
        }
    }

    SetBoundaries(b, d);
}

