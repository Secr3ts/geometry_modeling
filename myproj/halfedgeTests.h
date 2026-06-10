#pragma once
#include "myMesh.h"
#include <vector>
#include "myHalfedge.h"
#include <iostream>
#include "myFace.h"

#ifndef HE_TESTS_H
#define HE_TESTS_H
inline bool symmetricalTwin(myMesh* m) {
	const std::vector<myHalfedge*>& halfedges = m->halfedges;

	int i = 0;
	int j = 0;
	bool result = true;
	for (auto he : halfedges) {
		if (!he->twin) {
			i++;
			std::cout << "Halfedge with origin " << he->source->point->to_s() << " doesn't have a twin." << std::endl;
			continue;
		}
		
		if (he->twin->twin != he) {
			j++;
			result = false;
			std::cout << "Halfedge with origin " << he->source->point->to_s() << " doesn't have a symmetrical twin." << std::endl;
		}
	}

	std::cout << "Missing twins: " << i << std::endl;
	std::cout << "Non-symmetrical twins: " << j << std::endl;

	if (result)
		std::cout << "Halfedges are symmetrical." << std::endl;

	return result;
}

inline bool sourceTwin(myMesh* m) {
	const std::vector<myHalfedge*>& halfedges = m->halfedges;

	bool result = true;
	int i = 0;
	for (auto he : halfedges) {
		if (!he->twin) {
			i++;
			std::cout << "Halfedge with origin " << he->source->point->to_s() << " doesn't have a twin." << std::endl;
			continue;
		}

		if (!he->next) {
			std::cout << "Halfedge with origin " << he->source->point->to_s() << " doesn't have a next." << std::endl;
			continue;
		}

		if (he->twin->source != he->next->source) {
			result = false;
			std::cout << "Halfedge with origin " << he->twin->source->point->to_s() << " isn't a properly sourced twin." << std::endl;
		}
	}

	std::cout << "Missing twins: " << i << std::endl;

	if (result)
		std::cout << "Halfedges are correctly sourced." << std::endl;

	return result;

}

inline bool faceCycle(myMesh* m) {
	size_t max_iter;
	size_t curr_iter;
	bool result;
	const std::vector<myFace*>& faces = m->faces;

	max_iter = 32;
	curr_iter = 0;
	result = true;

	for (auto face : faces) {
		myHalfedge* start_he = face->adjacent_halfedge;
		myHalfedge* he = start_he;
	
		do {
			curr_iter++;
			he = he->next;
		} while (he != start_he || max_iter > curr_iter);
		
		if (max_iter == curr_iter && he != start_he) {
			std::cout << "Face with adj he " << start_he->source->point->to_s() << "is broken (infinite loop ?) or has more than 32 edges." << std::endl;
			result = false;
		}
	}

	if (result) {
		std::cout << "Faces can be cycled properly." << std::endl;
	}

	return result;
}

inline bool coherentHalfedges(myMesh* m) {
	const std::vector<myHalfedge*>& halfedges = m->halfedges;
	bool result;

	result = true;

	for (myHalfedge* he : halfedges) {
		if (he->next->prev != he || he->prev->next != he) {
			std::cout << "Halfedge with origin " << he->source->point->to_s() << " is not coherent." << std::endl;
			result = false;
		}
	}

	if (result) {
		std::cout << "Halfedges are coherent." << std::endl;
	}
	
	return result;
}

inline bool nullSource(myMesh* m) {
	const std::vector<myHalfedge*>& halfedges = m->halfedges;
	bool result;
	size_t i;
	
	i = 0;
	result = true;
	
	for (myHalfedge* he : halfedges) {
		if (he->source == nullptr) {
			std::cout << "Halfedge at index" << i << " has a nullptr source." << std::endl;
			result = false;
		}

		i++;
	}

	if (result) {
		std::cout << "Halfedges have non-null sources." << std::endl;
	}

	return result;

}

inline bool adjacentCoherent(myMesh* m) {
	size_t max_iter;
	size_t curr_iter;
	bool result;
	const std::vector<myFace*>& faces = m->faces;

	result = true;
	max_iter = 64;
	curr_iter = 0;

	for (auto face : faces) {
		myHalfedge* start_he = face->adjacent_halfedge;
		myHalfedge* he = start_he;
		
		do {
			if (he->adjacent_face != face) {
				std::cout << "Halfedge with origin " << he->source->point->to_s() << " isn't linked to the proper face. " << std::endl;
				result = false;
			}

			curr_iter++;
			he = he->next;
		} while (he != start_he || curr_iter <= max_iter);

	}

	if (result) {
		std::cout << "Halfedges reference the same face." << std::endl;
	}

	return result;
}

inline bool startHe(myMesh* m) {
	bool result;
	const std::vector<myFace*>& faces = m->faces;

	result = true;
	
	for (auto face : faces) {
		myHalfedge* start_he = face->adjacent_halfedge;
		
		if (start_he->adjacent_face != face) {
			result = false;
			/*
				TEST WITH CERR
			*/
			std::cerr << "Face with adj halfedge with origin: " << start_he->source->point->to_s() << " has wrong starting halfedge->adjacent_face." << std::endl;
		}
	}

	if (result) {
		std::cout << "Faces have correct starting halfedge." << std::endl;
	}

	return result;
}

inline bool startV(myMesh* m) {
	bool result;
	const std::vector<myVertex*>& vertices = m->vertices;

	result = true;

	for (auto vertex: vertices) {
		myHalfedge* start_he = vertex->originof;
		if (!start_he) {
			continue;
		}

		if (start_he->source != vertex) {
			result = false;
			/*
				TEST WITH CERR
			*/
			std::cerr << "Vertex with origin: " << start_he->source->point->to_s() << " has wrong originof." << std::endl;
		}
	}

	if (result) {
		std::cout << "Vertices have correct halfedge assignment." << std::endl;
	}

	return result;
}

inline bool cycleVertex(myMesh* m) {
	bool result;
	size_t max_iter;
	size_t curr_iter;

	result = true;
	max_iter = 64;
	curr_iter = 0;
	const std::vector<myVertex*>& vertices = m->vertices;

	for (myVertex* vertex : vertices) {
		myHalfedge* start_he = vertex->originof;
		auto he = start_he;
		
		do {
			curr_iter++;
			if (he && he->prev) {
				he = he->prev->twin;
			}
		} while (he && he != start_he || curr_iter <= max_iter);
	
		if (max_iter >= curr_iter && he != start_he) {
			std::cout << "Problem cycling around vertex: " << vertex->point->to_s() << std::endl;

			result = false;
		}
	}

	if (result) {
		std::cout << "Cycled around vertices." << std::endl;
	}

	return result;
}

inline bool coherentSourceOrigin(myMesh* m)
{
	const std::vector<myVertex*>& vertices = m->vertices;
	const std::vector<myHalfedge*>& halfedges = m->halfedges;

	bool result = true;
	for (myVertex* v : vertices)
	{
		myHalfedge* he = v->originof;
		if (!he) 
		{
			continue;
		}
		if (he->source != v)
		{
			std::cout << "Vertex incoherent: " << v->point->to_s() << std::endl;
			result = false;
			break;
		}
	}

	for (myHalfedge* he : halfedges)
	{
		myVertex* v = he->source;

		if (he == v->originof)
		{
			std::cout << "Vertex incoherent: " << v->point->to_s() << std::endl;
			result = false;
			break;
		}
	}

	if (!result)
	{
		std::cout << "OriginOf & Source aren't coherent." << std::endl;
	}

	return result;
}

inline void allTests(myMesh* m) {
	bool symTwin = symmetricalTwin(m);
	bool srcTwin = sourceTwin(m);
	bool fCycle = faceCycle(m);
	bool cHe = coherentHalfedges(m);
	bool nSrc = nullSource(m);
	bool adjHe = adjacentCoherent(m);
	bool startAdjHe = startHe(m);
	bool startHeVe = startV(m);
	bool vCycle = cycleVertex(m);
	bool ofSrc = coherentSourceOrigin(m);
}
#endif