#include "myFace.h"
#include "myvector3d.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <GL/glew.h>

myFace::myFace(void)
{
	adjacent_halfedge = NULL;
	normal = new myVector3D(1.0, 1.0, 1.0);
	face_point = new myPoint3D(0.f, 0.f, 0.f);
}

myFace::~myFace(void)
{
	if (normal) delete normal;
}

void myFace::computeNormal()
{
	normal->setNormal(
		adjacent_halfedge->source->point, 
		adjacent_halfedge->next->source->point, 
		adjacent_halfedge->prev->source->point);
}

void myFace::computeFacePoint()
{
	myHalfedge* he = this->adjacent_halfedge;
	myHalfedge* start_he = he;

	myPoint3D* face_point = new myPoint3D();
	face_point->zeroes();

	double i = 0;
	do {
		i++;
		*face_point += *he->source->point;
		he = he->next;
	} while (he != start_he);

	*face_point /= i;

	this->face_point = face_point;
}
