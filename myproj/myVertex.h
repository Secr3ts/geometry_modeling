#pragma once
#include "mypoint3d.h"

class myHalfedge;
class myVector3D;

class myVertex
{
public:
	myPoint3D *point;
	myHalfedge *originof;

	myPoint3D* cm_point; // temporary point in catmull-clark.
	int index;  //use as you wish.

	myVector3D *normal;

	void computeNormal();
	myVertex(void);
	~myVertex(void);
};
