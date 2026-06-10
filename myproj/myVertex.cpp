#include "myVertex.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myFace.h"

myVertex::myVertex(void)
{
	point = NULL;
	originof = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
}

myVertex::~myVertex(void)
{
	if (normal) delete normal;
}

void myVertex::computeNormal()
{
	if (!originof) {
		normal->dX = 0.0;
		normal->dY = 0.0;
		normal->dZ = 0.0;
		return;
	}

	auto e = originof;
	auto step = e;

	normal->dX = 0.0;
	normal->dY = 0.0;
	normal->dZ = 0.0;

	int counter = 0;
	int max_iterations = 4096;
	int iterations = 0;

	do {
		iterations++;
		if (iterations > max_iterations) break;

		if (!step || !step->adjacent_face) break;

		if (step->adjacent_face->normal) {
			myVector3D* fn = step->adjacent_face->normal;
			normal->dX += fn->dX;
			normal->dY += fn->dY;
			normal->dZ += fn->dZ;

			counter += 1;
		}

		if (step->twin != nullptr) {
			step = step->twin->next;
		}
		else {
			break;
		}
	} while (e != step);

	if (counter > 0) {
		normal->dX /= counter;
		normal->dY /= counter;
		normal->dZ /= counter;
		normal->normalize();
	}
}