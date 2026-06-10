#pragma once

class myHalfedge;
class myVector3D;
class myPoint3D;

class myFace
{
public:
	myHalfedge *adjacent_halfedge;
	
	myPoint3D* face_point; // Catmull-Clark Face Point (1/n * Σ(vertex->point))
	myVector3D *normal;

	int index; //use this variable as you wish.

	void computeNormal();
	void computeFacePoint(); // Helper method to compute the face point of the current face

	myFace(void);
	~myFace(void);
};