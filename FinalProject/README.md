# Final Project - 2D Fluid Simulation

**Usage:** This program simulates the movement of 2D fluids.

## Compilation, Dependencies, & Runtime Arguments

**Compilation Arguments:** 
- There are many variations of SIMD usage, but the compilation below should work on all of them.
- Example: g++ -std=c++11 visualize.cpp 2d_fluid.cpp -g -O3 -fopenmp -mavx2 -L. -lp8g++ -Wl,-rpath=. -o vis2d.o

**Run Command/Arguments:**
- Argument 1: Matrix Size
- Argument 2: Target FPS
- Example: ./vis2d.o 512 10

**Libraries Used:**
- iostream
- cmath
- cstring
- immintrin.h
- omp.h
- fsteam

## Navier-Stokes Fluid Equations

The main Navier-Stokes equations are the theoretical basis for all fluid movement. This includes terms for diffusion and convection. When the diffusion term is greater than the convection term, the fluid tends to flow more smoothly, and when the convection term is greater than the diffusion term, the flow becomes erratic and turbulent. Physicists are currently figuring out how to solve the equation, as while it models fluid movements, there is no way to know exactly what a fluid system will look like after a certain amount of time.

Jos Stam divided the Navier-Stokes equations for two-dimensional fluid simulation. He purposed that one equation models the movement of the velocity field and one the density field. As mass and momentum are both conserved in fluid mechanics. This brings us to our two main equations:

![image](https://user-images.githubusercontent.com/112660711/233766054-5660b673-229f-44c8-9de3-61d6a2df8849.png)

The first Equation is: the partial derivative of velocity is equal to the convection, diffusion, and its initial value

The second Equation is: the partial derivative of density is equal to the convection, diffusion, and its initial value

Each cell in our simulation is given a velocity and density value of zero (in two discrete arrays) when the simulation begins. Calculations are completed in matrices of all velocity and density values around the current cell. They first satisfy the second term in the equations above (diffusion); this is an iterative process where the fluid gets closer to the real value upon each iteration. Then, the program applies the velocity field to the density field to calculate the movement of the density field. The visualization is the density field. This is a basic definition and can be expanded upon more beginning on page 8 in Michael Ash's thesis paper: [Simulation and Visualization of a 3D Fluid](https://github.com/B3ngu1N/AdvCompSys/blob/main/FinalProject/Final_Project_Papers/thesis-en.pdf)

## Fluid Simulation Implementation


## Visualization

Using C++ library p8g to build 2D visualization of fluid (C++ Version of Processing).


[p8g GitHub Repository](https://bernhardfritz.github.io/p8g/docs/get-started)

## Improvements

**SIMD:**



**Continuity:**
To make the program continuous throughout devices a few addons were implemented. First, the user may choose the matrix size, and second the user may choose their desired frames per second. The matrix size represents one dimension of the velocity/density matrix. With a larger value, there will be significantly more calculations present, which slows down the simulation calculations. In order to combat this, a desired frames per second is allowed. One process that can be limited is the iterations of the diffusion term calculation. Thus, the program allows the user to choose the amount of updates they would like to see in their simulation per second and the program times each calculation and either increases or decreases the amount of iterations of the diffusion calculation. This allows the program to run at the same speed across devices, no matter the hardware.


## Conclusions



