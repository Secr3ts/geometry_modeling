#pragma once

#include <stdio.h>
#include <tchar.h>

class myVertex;
class myFace;
class myPoint3D;

class myHalfedge
{
public:
	myVertex *source; 
	myFace *adjacent_face;
	
	bool visited = false; // flag in case a "physical edge" only visit is required
	myPoint3D* edge_point; // smoothened edge point.
	
	myHalfedge *next;  
	myHalfedge *prev;  
	myHalfedge *twin;

	int index; //use as you wish.

	void computeEdgePoint(); // Method to compute the smoothened edge point

	myHalfedge(void);
	void copy(myHalfedge *);
	~myHalfedge(void);
};