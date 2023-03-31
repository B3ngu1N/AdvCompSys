/*
    2D Fluid Simulation with Navier-Stokes Equations

    @author Thomas Petr
    @author Ben Haft
    @date 3/30/2023
*/

#include "2d_fluid.h"

void Fluid2D::AddDensityScalar(int x, int y, float amount)
{
    this->density(x, y) += amount;
}

void Fluid2D::AddVelocityScalar(int x, int y, float amountX, float amountY)
{
    this->Vx(x, y) += amountX;
    this->Vy(x, y) += amountY;
}

void Fluid2D::SimStep()
{

}

void Fluid2D::SetBoundaries(int b)
{

}

void Fluid2D::LinSolve(int b, float a, float c, int itr)
{

}

void Fluid2D::Diffuse(int b, float diff, float dt, int itr)
{

}

void Fluid2D::Project(int itr)
{

}

void Fluid2D::Advect(int b, float dt)
{

}




