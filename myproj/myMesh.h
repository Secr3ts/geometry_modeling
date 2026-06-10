#pragma once
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include "myFace.h"
#include "myHalfedge.h"
#include "myVertex.h"
#include <vector>
#include <string>
#include <map>
#include <utility>

class myMesh
{
public:
	std::vector<myVertex *> vertices;
	std::vector<myHalfedge *> halfedges;
	std::vector<myFace *> faces;
	std::string name;

	void checkMesh();
	bool readFile(std::string filename);
	void computeNormals();
	void normalize();

	/* catmull clark & helpers*/
	/* subdivise the mesh with Catmull Clark */
	void subdivisionCatmullClark();

	/* loop subdivsion & helpers */
	/* subdivise the mesh with Loop */
	void subdivisionLoop();
	
	// Compute edge vertices
	void loopPhase1();
	// 

	void splitFaceTRIS(myFace *, myPoint3D *);

	void splitEdge(myHalfedge *, myPoint3D *);
	void splitFaceQUADS(myFace *, myPoint3D *);

	/* shortest edge collaspe & helpers */
	/* Collapse the shortest edge */
	void shortestEdgeCollapse();

	/* Attempts to find the shortest edge on the current mesh
		@returns a pointer to the shortest halfedge, or null if none found. 
	*/
	myHalfedge* shortestEdge();

	// Triangulates the mesh
	void triangulate();

	// Check if a face is triangulated
	// @param f face to check
	// @returns true or false
	bool triangulate(myFace *f);
	
	// Check if a mesh is triangulated
	// @returns true of false
	bool triangulated(); // shortest edge collapse
	
	// Triangulates a face based on its halfedge
	// @param current halfedge to manipulate
	// @returns the new created face
	myFace *triangulate(myHalfedge* current);

	// Check if a halfedge forms a valid ear for triangulation
	// An ear is valid if it's convex and has no interior points
	// @param current halfedge representing the ear vertex
	// @returns true if this vertex forms a valid ear, false otherwise
	bool isValidEar(myHalfedge* current);

	// Check if the face is convex
	// @param ah halfege
	// @param bh halfege
	// @param ch halfege
	// @returns true of false
	bool convex(myHalfedge* ah, myHalfedge* bh, myHalfedge* ch);
	
	// Check if a point (vertex..) is inside the triangle created by current, next(current) and prev(current)
	// @param current halfedge 
	// @return true of false
	bool point_inside(myHalfedge* current);
	
	// Apply a curve of revolution on current mesh
	// @param steps number of steps to 360° dedg
	// @param angle angle of the revolution
	void revolution(short steps, float angle, std::vector<myPoint3D>& profile);

	// Creates an edge from v1, v2 and add it to f. Twin handling also happening
	// @param v1 hedge vertex
	// @param v2 twin hedge vertex
	// @param f face eto add the first hedge to
	// @param twin_map pointer to the twin_map, for future twin connecting
	// @returns created halfedge
	myHalfedge* createEdge(myVertex* v1, myVertex* v2, myFace* f, std::map<std::pair<myVertex*, myVertex*>, myHalfedge*>& twin_map);
	
	void clear();

	/* utils method for our vectors */
	
	// Inline utility to reset visited flags on hedges
	inline void reset_flags() {
		for (auto he : halfedges) {
			he->visited = false;
		}
	};


	myMesh(void);
	~myMesh(void);
};

