//g++ -std=c++11 visualize.cpp 2d_fluid.cpp -g -L. -lp8g++ -Wl,-rpath=. -o vis2d.o

#include "2d_fluid.h"

using namespace p8g;

Fluid2D* fluid;

void p8g::draw() {
    colorMode(p8g::HSB);
	stroke(52);
	strokeWeight(2);

	int cx = (int)((0.5 * width) / SCALE);
	int cy = (int)((0.5 * height) / SCALE);
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			fluid->AddDensity(cx + i, cy + j, ((rand() / RAND_MAX) * 100.0 + 50));
		}
	}

	for (int i = 0; i < 2; i++) {
		fluid->AddVelocity(cx, cy, 0.2, 0);
	}
    
	fluid->SimStep();	
    fluid->RenderDensity();
}

void p8g::keyPressed() {}
void p8g::keyReleased() {}
void p8g::mouseMoved() {}
void p8g::mousePressed() {}
void p8g::mouseReleased() {}
void p8g::mouseWheel(float delta) {}

int main() 
{
	// fluid = new Fluid2D(N, 0.0, 0.0000001, 0.2);
    fluid = new Fluid2D(N, 0.0, 0.0000001, 0.2);

	run(600, 600, "2D Fluid Simulation");

	return 0;
}


