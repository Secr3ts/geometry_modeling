#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <utility>
#include <GL/glew.h>
#include "myvector3d.h"
#include <climits>
#include <vector>
#include <cmath>
#include "halfedgeTests.h"
// #include <math.h>

using namespace std;

static const double pi = 3.14159265358979323846;

myMesh::myMesh(void)
{
	/**** TODO ****/
}


myMesh::~myMesh(void)
{
	/**** TODO ****/
}

void myMesh::clear()
{
	for (unsigned int i = 0; i < vertices.size(); i++) if (vertices[i]) delete vertices[i];
	for (unsigned int i = 0; i < halfedges.size(); i++) if (halfedges[i]) delete halfedges[i];
	for (unsigned int i = 0; i < faces.size(); i++) if (faces[i]) delete faces[i];

	vector<myVertex*> empty_vertices;    vertices.swap(empty_vertices);
	vector<myHalfedge*> empty_halfedges; halfedges.swap(empty_halfedges);
	vector<myFace*> empty_faces;         faces.swap(empty_faces);
}

void myMesh::checkMesh()
{
	cout << "==================" << std::endl;
	vector<myHalfedge*>::iterator it;
	for (it = halfedges.begin(); it != halfedges.end(); it++)
	{
		if ((*it)->twin == NULL)
			break;
	}
	if (it != halfedges.end())
		cout << "Error! Not all edges have their twins!\n";
	else cout << "Each edge has a twin!\n";
	allTests(this);
	cout << "\n==================" << std::endl;
}

void myMesh::revolution(short steps, float angle, std::vector<myPoint3D>& profile) {
	if (profile.size() < 2) {
		std::cout << "2 vertices aren't enough to constitute revolution curves." << std::endl;
		return;
	}
	size_t n = profile.size();

	std::vector<std::vector<myVertex*>> rings(steps + 1);
	for (size_t k = 0; k < rings.size(); k++)
		rings[k].resize(n);
	for (size_t j = 0; j < n; j++) {
		myVertex* v = new myVertex();
		v->point = new myPoint3D(profile[j]);
		vertices.push_back(v);
		rings[0][j] = v;
	}

	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> t_map;

	/* create adjusted vertex based on angle */
	for (size_t i = 1; i < rings.size(); i++) {
		float t = (angle / steps) * i * (M_PI / 180.f);
		float cosT = cosf(t);
		float sinT = sinf(t);

		for (size_t l = 0; l < n; l++) {
			myPoint3D* oldPoint = &profile[l];

			myVertex* adjustedVertex = new myVertex();
			adjustedVertex->point = new myPoint3D(
				(oldPoint->X * cosT) - (oldPoint->Z * sinT),
				oldPoint->Y,
				(oldPoint->X * sinT) + (oldPoint->Z * cosT)
			);

			vertices.push_back(adjustedVertex);
			rings[i][l] = adjustedVertex;
		}
	}

	/* link, create twins */
	for (size_t o = 0; o < steps; o++) {
		size_t first = o;
		size_t second = o + 1;

		for (size_t p = 0; p < n; p++) {
			size_t next_p = (p + 1) % n;

			myVertex* v0_0 = rings[first][p];
			myVertex* v0_1 = rings[first][next_p];

			myVertex* v1_0 = rings[second][p];
			myVertex* v1_1 = rings[second][next_p];

			myFace* f = new myFace();
			faces.push_back(f);

			vector<myHalfedge*> loop;
			loop.push_back(createEdge(v0_0, v0_1, f, t_map));
			loop.push_back(createEdge(v0_1, v1_1, f, t_map));
			loop.push_back(createEdge(v1_1, v1_0, f, t_map));
			loop.push_back(createEdge(v1_0, v0_0, f, t_map));

			for (size_t q = 0; q < loop.size(); q++) {
				loop[q]->next = loop[(q + 1) % loop.size()];
				loop[(q + 1) % loop.size()]->prev = loop[q];
			}

			f->adjacent_halfedge = loop[0];
		}
	}

	/* clear originof, then fill them in*/
	for (myVertex* vertex : vertices) {
		vertex->originof = nullptr;
	}

	for (myHalfedge* halfedge : halfedges) {
		if (halfedge->source && !halfedge->source->originof && halfedge->adjacent_face) {
			halfedge->source->originof = halfedge;
		}
	}

	checkMesh();
	normalize();
}

myHalfedge* myMesh::createEdge(myVertex* v1, myVertex* v2, myFace* f, std::map<std::pair<myVertex*, myVertex*>, myHalfedge*>& twin_map) {
	myHalfedge* he = new myHalfedge();

	he->source = v1;
	he->adjacent_face = f;

	halfedges.push_back(he);

	auto fit = twin_map.find({ v2, v1 });
	if (fit != twin_map.end()) {
		he->twin = fit->second;
		fit->second->twin = he;
	}
	else {
		twin_map[{v1, v2}] = he;
	}

	return he;
}

bool myMesh::readFile(std::string filename)
{
	string s, t, u;
	vector<int> faceids;
	myHalfedge** hedges;

	ifstream fin(filename);
	if (!fin.is_open()) {
		cout << "Unable to open file!\n";
		return false;
	}
	name = filename;

	map<pair<int, int>, myHalfedge*> twin_map;
	map<pair<int, int>, myHalfedge*>::iterator it;

	while (getline(fin, s))
	{
		stringstream myline(s);
		myline >> t;

		if (t == "g") {}
		else if (t == "v")
		{
			float x, y, z;
			myline >> x >> y >> z;
			cout << "v " << x << " " << y << " " << z << endl;

			myPoint3D* p = new myPoint3D(x, y, z);
			myVertex* v = new myVertex;
			v->point = p;

			vertices.push_back(v);
		}
		else if (t == "mtllib") {}
		else if (t == "usemtl") {}
		else if (t == "s") {}
		else if (t == "f")
		{
			faceids.clear();
			while (myline >> u)
				faceids.push_back(atoi((u.substr(0, u.find("/"))).c_str()) - 1);

			if (faceids.size() < 3)
				continue;

			hedges = new myHalfedge * [faceids.size()];

			for (unsigned int i = 0; i < faceids.size(); i++)
				hedges[i] = new myHalfedge();

			myFace* f = new myFace();
			f->adjacent_halfedge = hedges[0];
			faces.push_back(f);

			for (unsigned int i = 0; i < faceids.size(); i++)
			{
				int iplusone = (i + 1) % faceids.size();
				int iminusone = (i - 1 + faceids.size()) % faceids.size();

				// YOUR CODE COMES HERE!

				// connect prevs, and next
				hedges[i]->next = hedges[iplusone];
				hedges[i]->prev = hedges[iminusone];
				hedges[i]->adjacent_face = f;


				// search for the twins using twin_map

				map<pair<int, int>, myHalfedge*>::iterator i_it = twin_map.find(make_pair(faceids[i], faceids[iplusone]));
				if (i_it != twin_map.end())
				{
					// twin found, register them both.
					hedges[i]->twin = i_it->second;
					i_it->second->twin = hedges[i];
				}

				// Register the hedge in twin_map
				// // Overload 8: insert from initializer_list
				// heights2.insert({ {"Kozume"s, 169.2}, {"Kuroo", 187.7} });
				// https://en.cppreference.com/w/cpp/container/map/insert
				twin_map.insert({
					{make_pair(faceids[iplusone], faceids[i]), hedges[i]}
					});


				// set originof
				hedges[i]->source = vertices[faceids[i]];
				vertices[faceids[i]]->originof = hedges[i];

				// push edges to halfedges in myMesh
				halfedges.push_back(hedges[i]);
			}
			delete[] hedges;
			// push faces to faces in myMesh
		}
	}

	checkMesh();
	normalize();
	// revolution(12, 360);
	computeNormals();

	return true;
}

bool myMesh::triangulated()
{
	for (myFace* face : faces)
	{
		if (!triangulate(face))
		{
			return false;
		}
	}
}

void myMesh::computeNormals()
{
	for each(myVertex * var in vertices)
	{
		var->computeNormal();
	}

	for each(myFace * var in faces)
	{
		var->computeNormal();
	}
}

void myMesh::normalize()
{
	if (vertices.size() < 1) return;

	int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0, tmpzmax = 0;

	for (unsigned int i = 0; i < vertices.size(); i++) {
		if (vertices[i]->point->X < vertices[tmpxmin]->point->X) tmpxmin = i;
		if (vertices[i]->point->X > vertices[tmpxmax]->point->X) tmpxmax = i;

		if (vertices[i]->point->Y < vertices[tmpymin]->point->Y) tmpymin = i;
		if (vertices[i]->point->Y > vertices[tmpymax]->point->Y) tmpymax = i;

		if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z) tmpzmin = i;
		if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z) tmpzmax = i;
	}

	double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
		ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
		zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

	double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
	scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i]->point->X -= (xmax + xmin) / 2;
		vertices[i]->point->Y -= (ymax + ymin) / 2;
		vertices[i]->point->Z -= (zmax + zmin) / 2;

		vertices[i]->point->X /= scale;
		vertices[i]->point->Y /= scale;
		vertices[i]->point->Z /= scale;
	}
}

void myMesh::shortestEdgeCollapse()
{
	this->reset_flags();

	/* Phase 1 shortest edge */
	myHalfedge* shortest_edge = this->shortestEdge();

	/* Phase 2 link condition */
	if (!shortest_edge)
	{
		return;
	}

	/* Phase 2.5: Validation before collapse */
	myVertex* va = shortest_edge->source;
	myVertex* vb = shortest_edge->twin->source;

	if (!va || !vb || !va->point || !vb->point)
	{
		std::cout << "Error: Invalid vertices for collapse." << std::endl;
		return;
	}

	myFace* f1 = shortest_edge->adjacent_face;
	myFace* f2 = shortest_edge->twin->adjacent_face;

	if (!f1 || !f2)
	{
		std::cout << "Error: Edge has null adjacent faces." << std::endl;
		return;
	}

	/* Phase 3: Compute new vertex position */
	myPoint3D tmp;
	tmp.zeroes();

	tmp += *vb->point;
	tmp += *va->point;
	tmp /= 2.0;

	vb->point->zeroes();
	*vb->point += tmp;

	/* Phase 4: Collect all halfedges around va and update them to vb */
	std::vector<myHalfedge*> ring_halfedges;

	if (va->originof)
	{
		myHalfedge* he = va->originof;
		myHalfedge* start = va->originof;

		do {
			if (!he) break;

			ring_halfedges.push_back(he);

			// Update source to vb
			if (he->source == va)
			{
				he->source = vb;
			}

			// Move to next halfedge in the ring around va
			he = he->prev->twin;
		} while (he != start && ring_halfedges.size() < 1000);
	}

	if (ring_halfedges.empty())
	{
		std::cout << "Error: Could not collect halfedges around va." << std::endl;
		return;
	}

	/* Phase 5 degen faces */
	/* face 1 */
	myHalfedge* e1 = shortest_edge->next;
	myHalfedge* e2 = shortest_edge->prev;

	if (f1 && e1 && e2)
	{
		myHalfedge* t1 = e1->twin;
		myHalfedge* t2 = e2->twin;

		if (t1)
		{
			t1->twin = t2;
		}

		if (t2)
		{
			t2->twin = t1;
		}
	}

	/* face 2 */
	myHalfedge* e3 = shortest_edge->twin->next;
	myHalfedge* e4 = shortest_edge->twin->prev;

	if (f2 && e3 && e4)
	{
		myHalfedge* t3 = e3->twin;
		myHalfedge* t4 = e4->twin;

		if (t3)
		{
			t3->twin = t4;
		}

		if (t4)
		{
			t4->twin = t3;
		}
	}

	/* Phase 6 originof */

	// va
	// myVertex* vb = shortest_edge->twin->source;
	myVertex* vc = e2->source;
	myVertex* vd = e4->source;

	if (vc && vc->originof == e2)
	{
		vc->originof = e1->twin;
	}

	if (vd && vd->originof == e4)
	{
		vd->originof = e3->twin;
	}

	if (vb && (vb->originof == shortest_edge->twin || vb->originof == e1)) {
		vb->originof = (e4 && e4->twin) ? e4->twin : nullptr;
	}

	if (vb && !vb->originof) {
		for (myHalfedge* he : halfedges) {
			if (he && he->source == vb &&
				he != shortest_edge->twin && he != e1 && he != e4) {
				vb->originof = he;
				break;
			}
		}
	}

	/* Phase 7: Cleanup and Memory Reclamation (Pre-C++20 Compatible) */

	if (f1) {
		faces.erase(std::remove(faces.begin(), faces.end(), f1), faces.end());
	}
	if (f2) {
		faces.erase(std::remove(faces.begin(), faces.end(), f2), faces.end());
	}

	if (va) {
		vertices.erase(std::remove(vertices.begin(), vertices.end(), va), vertices.end());
	}

	halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), shortest_edge), halfedges.end());

	if (shortest_edge->twin) {
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), shortest_edge->twin), halfedges.end());
	}
	if (e1) {
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), e1), halfedges.end());
	}
	if (e2) {
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), e2), halfedges.end());
	}
	if (e3) {
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), e3), halfedges.end());
	}
	if (e4) {
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), e4), halfedges.end());
	}

	/*

	// 2. Free up the heap memory safely
	if (f1) delete f1;
	if (f2) delete f2;

	if (vA) {
		if (vA->point) delete vA->point; // Clear coordinate memory
		delete vA;
	}

	if (e1) delete e1;
	if (e2) delete e2;
	if (e3) delete e3;
	if (e4) delete e4;

	if (shortest_edge->twin) delete shortest_edge->twin;
	delete shortest_edge;

	// Final verification pass

	*/
	checkMesh();
}


myHalfedge* myMesh::shortestEdge()
{
	double minimum = DBL_MAX;
	myHalfedge* min_he = nullptr;

	for (myHalfedge* he : halfedges)
	{
		if (!he)
		{
			continue;

		}

		if (he->visited)
		{
			continue;
		}

		myHalfedge* twin = he->twin;

		if (!twin)
		{
			continue;
		}

		myVertex* he_vertex = he->source;
		myVertex* twin_vertex = twin->source;

		if (!he_vertex || !twin_vertex)
		{
			continue;
		}

		myPoint3D* a = he_vertex->point;
		myPoint3D* b = twin_vertex->point;

		if (!a || !b)
		{
			continue;
		}

		double d = a->dist(*b);

		if (d < minimum)
		{
			minimum = d;
			min_he = he;
		}
	}

	if (min_he)
	{
		min_he->visited = true;
		min_he->twin->visited = true;
	}

	return min_he;
}

void myMesh::splitFaceTRIS(myFace* f, myPoint3D* p)
{
	if (triangulate(f))
		return;

	myHalfedge* start_he = f->adjacent_halfedge;
	auto he = start_he;

	do {

		he = he->next;
	} while (he != start_he);
}

void myMesh::splitEdge(myHalfedge* e1, myPoint3D* p)
{
	myVertex* old_v1_source = e1->source;
	myVertex* old_v2_source = (e1->twin != nullptr) ? e1->twin->source : nullptr;

	myHalfedge* e1_1 = new myHalfedge();
	myHalfedge* e1_2 = new myHalfedge();

	e1_1->adjacent_face = e1->adjacent_face;
	e1_2->adjacent_face = e1->adjacent_face;

	myVertex* v1_1 = new myVertex();
	myVertex* v1_2 = new myVertex();

	v1_1->originof = e1_1;
	v1_2->originof = e1_2;

	v1_1->point = old_v1_source->point;
	v1_2->point = p;

	e1_1->source = v1_1;
	e1_2->source = v1_2;

	e1->prev->next = e1_1;
	e1_1->prev = e1->prev;

	e1_1->next = e1_2;
	e1_2->prev = e1_1;

	e1->next->prev = e1_2;
	e1_2->next = e1->next;

	//v1_1->computeNormal();
	// v1_2->computeNormal();

	e1_1->adjacent_face->adjacent_halfedge = e1_1;
	halfedges.push_back(e1_1);
	halfedges.push_back(e1_2);
	vertices.push_back(v1_1);
	vertices.push_back(v1_2);

	myHalfedge* e2 = e1->twin;

	if (e2 != nullptr)
	{
		myHalfedge* e2_1 = new myHalfedge();
		myHalfedge* e2_2 = new myHalfedge();

		e2_1->adjacent_face = e2->adjacent_face;
		e2_2->adjacent_face = e2->adjacent_face;

		myVertex* v2_1 = new myVertex();
		myVertex* v2_2 = new myVertex();

		v2_1->originof = e2_1;
		v2_2->originof = e2_2;

		v2_1->point = p;
		v2_2->point = old_v2_source->point;

		e2_1->source = v2_1;
		e2_2->source = v2_2;

		e2->prev->next = e2_1;
		e2_1->prev = e2->prev;

		e2_1->next = e2_2;
		e2_2->prev = e2_1;

		e2->next->prev = e2_2;
		e2_2->next = e2->next;


		e1_1->twin = e2_1;
		e2_1->twin = e1_1;

		e1_2->twin = e2_2;
		e2_2->twin = e1_2;

		v2_1->computeNormal();
		v2_2->computeNormal();

		e1_2->adjacent_face->adjacent_halfedge = e1_2;

		halfedges.push_back(e2_1);
		halfedges.push_back(e2_2);
		vertices.push_back(v2_1);
		vertices.push_back(v2_2);

	}
	else
	{
		e1_1->twin = nullptr;
		e1_2->twin = nullptr;
	}

	halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), e1), halfedges.end());
	vertices.erase(std::remove(vertices.begin(), vertices.end(), old_v1_source), vertices.end());

	if (e2 != nullptr) {
		halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), e2), halfedges.end());
		vertices.erase(std::remove(vertices.begin(), vertices.end(), old_v2_source), vertices.end());
	}

	// delete old_v1_source;
	// delete e1;
	if (e2 != nullptr) {
		// delete old_v2_source;
		// delete e2;
	}
}

void myMesh::splitFaceQUADS(myFace* f, myPoint3D* p)
{

}


void myMesh::subdivisionCatmullClark()
{
	/* Phase 1 */

	/* Face points */
	for (myFace* face : faces)
	{
		if (face == nullptr || face->adjacent_halfedge == nullptr)
		{
			continue;
		}

		face->computeFacePoint();
	}

	/* Edge points */
	this->reset_flags();

	for (myHalfedge* hedge : halfedges)
	{
		if (!hedge)
		{
			continue;
		}
		if (hedge->visited)
		{
			continue;
		}

		myHalfedge* twin = hedge->twin;

		myFace* f1 = nullptr;
		myFace* f2 = nullptr;

		if (hedge)
		{
			f1 = hedge->adjacent_face;
		}

		if (twin)
		{
			f2 = twin->adjacent_face;

		}
		if (!f1)
		{
			continue;
		}

		if (!twin)
		{
			continue;
		}

		myPoint3D* edge_point = new myPoint3D();
		edge_point->zeroes();

		myPoint3D* p1 = hedge->source->point;
		myPoint3D* p2 = twin->source->point;

		if (f1 && f2) {
			myPoint3D* f1_point = f1->face_point;
			myPoint3D* f2_point = f2->face_point;

			*edge_point += *f1_point;
			*edge_point += *f2_point;
			*edge_point += *p1;
			*edge_point += *p2;

			*edge_point /= (double)4;

			hedge->edge_point = edge_point;
			twin->edge_point = edge_point;
		}
		else if (f1 && !f2) {
			*edge_point += *p1;
			*edge_point += *p2;

			*edge_point /= (double)2;

			hedge->edge_point = edge_point;
		}

		hedge->visited = true;

		if (twin)
			hedge->twin->visited = true;;
	}

	/* Vertex points */

	for (myVertex* v : vertices)
	{
		myHalfedge* start_he = v->originof;
		myHalfedge* he = start_he;

		if (!start_he)
			continue;

		/* F_avg */
		myPoint3D f_avg;
		f_avg.zeroes();

		double i = 0.0;
		do {
			if (!he) {
				break;
			}

			myFace* f_adj = he->adjacent_face;

			if (!f_adj)
			{
				he = he->prev->twin;
				continue;
			}

			myPoint3D* f_point = f_adj->face_point;

			if (!f_point)
			{
				he = he->prev->twin;
				continue;
			}

			f_avg += *f_point;

			i++;
			he = he->prev->twin;
		} while (he != start_he);

		if (i > 0.0) {
			f_avg /= i;
		}

		/* R_avg*/

		myPoint3D r_avg;
		r_avg.zeroes();

		he = start_he;
		i = 0.0; // a

		do {
			if (!he)
			{
				he = he->prev->twin;
			}

			myVertex* va = he->source;
			myHalfedge* twin = he->twin;

			if (twin && va) {
				r_avg += ((*va->point + *twin->source->point) / 2.0);
			}

			i++;
			he = he->prev->twin;
		} while (he != start_he);

		if (i != 0.0)
		{
			r_avg /= i;
		}

		myPoint3D* old_v = v->point;
		myPoint3D* new_v = new myPoint3D();
		new_v->zeroes();

		*new_v += f_avg;
		*new_v += r_avg * 2.0;
		*new_v += *old_v * (double)(i - 3);
		*new_v /= i;

		v->cm_point = new_v;
	}

	/* Phase 2*/
	std::vector<myVertex*> new_vertices;
	std::vector<myFace*> new_faces;
	std::vector<myHalfedge*> new_halfedges;

	std::map<myVertex*, myVertex*> vertex_map;
	std::map<myPoint3D*, myVertex*> edge_vertex_map;
	std::map<myFace*, myVertex*> face_vertex_map;

	for (myFace* face : faces) {
		if (!face || !face->face_point) continue;

		myVertex* nV = new myVertex();
		nV->point = face->face_point;
		nV->originof = nullptr;
		new_vertices.push_back(nV);
		face_vertex_map[face] = nV;
	}

	for (myHalfedge* hedge : halfedges) {
		if (!hedge || !hedge->edge_point) continue;

		if (edge_vertex_map.count(hedge->edge_point) == 0) {
			myVertex* nV = new myVertex();
			nV->point = hedge->edge_point;
			nV->originof = nullptr;
			new_vertices.push_back(nV);
			edge_vertex_map[hedge->edge_point] = nV;
		}
	}

	for (myVertex* v : vertices) {
		if (!v || !v->cm_point) continue;

		myVertex* nV = new myVertex();
		nV->point = v->cm_point;
		nV->originof = nullptr;
		new_vertices.push_back(nV);
		vertex_map[v] = nV;
	}

	std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> twin_map;

	for (myFace* face : faces) {
		if (!face || !face->adjacent_halfedge) continue;

		myHalfedge* start_he = face->adjacent_halfedge;
		myHalfedge* he = start_he;

		do {
			myVertex* nV1 = vertex_map[he->source];
			myVertex* nV2 = edge_vertex_map[he->edge_point];
			myVertex* nV3 = face_vertex_map[face];
			myVertex* nV4 = edge_vertex_map[he->prev->edge_point];

			if (!nV1 || !nV2 || !nV3 || !nV4) {
				he = he->next;
				continue;
			}

			myFace* nF = new myFace();
			new_faces.push_back(nF);

			myHalfedge* nh1 = new myHalfedge();
			myHalfedge* nh2 = new myHalfedge();
			myHalfedge* nh3 = new myHalfedge();
			myHalfedge* nh4 = new myHalfedge();

			new_halfedges.push_back(nh1);
			new_halfedges.push_back(nh2);
			new_halfedges.push_back(nh3);
			new_halfedges.push_back(nh4);

			nh1->source = nV1; nh1->next = nh2; nh1->prev = nh4; nh1->adjacent_face = nF;
			nh2->source = nV2; nh2->next = nh3; nh2->prev = nh1; nh2->adjacent_face = nF;
			nh3->source = nV3; nh3->next = nh4; nh3->prev = nh2; nh3->adjacent_face = nF;
			nh4->source = nV4; nh4->next = nh1; nh4->prev = nh3; nh4->adjacent_face = nF;

			nF->adjacent_halfedge = nh1;

			nV1->originof = nh1;
			nV2->originof = nh2;
			nV3->originof = nh3;
			nV4->originof = nh4;

			// Twin Linker Lambda Function
			// Generated by IA.
			auto link_twin = [&](myHalfedge* nh) {
				myVertex* src = nh->source;
				myVertex* dst = nh->next->source;
				std::pair<myVertex*, myVertex*> reverse_edge = { dst, src };

				if (twin_map.count(reverse_edge)) {
					myHalfedge* discovered_twin = twin_map[reverse_edge];
					nh->twin = discovered_twin;
					discovered_twin->twin = nh;
				}
				else {
					twin_map[{src, dst}] = nh;
				}
				};

			link_twin(nh1);
			link_twin(nh2);
			link_twin(nh3);
			link_twin(nh4);

			he = he->next;
		} while (he != start_he);
	}

	for (myFace* f : faces) { if (f) delete f; }
	for (myHalfedge* he : halfedges) { if (he) delete he; }
	for (myVertex* v : vertices) {
		if (v) {
			if (v->point) delete v->point;
			delete v;
		}
	}

	this->faces = new_faces;
	this->halfedges = new_halfedges;
	this->vertices = new_vertices;

	checkMesh();
}

static bool isInside(myHalfedge* a, myHalfedge* b, myHalfedge* c, myHalfedge* d)
{
	myPoint3D* pa = a->source->point;
	myPoint3D* pb = b->source->point;
	myPoint3D* pc = c->source->point;
	myPoint3D* pd = d->source->point;

	myVector3D AB = myVector3D(pa, pb);
	myVector3D AC = myVector3D(pa, pc);
	myVector3D DB = myVector3D(pd, pb);
	myVector3D DC = myVector3D(pd, pc);
	myVector3D DA = myVector3D(pd, pa);

	double ABC = abs((AB.crossproduct(AC).length())) / 2;
	double alpha = abs((DB.crossproduct(DC).length())) / (2 * ABC);
	double beta = abs((DC.crossproduct(DA).length())) / (2 * ABC);

	double gamma = 1 - alpha - beta;
	/*
	0≤α≤1
	0≤β≤1
	0≤γ≤1
	α+β+γ=1
	*/

	bool res = true;

	if (alpha > 1 || alpha < 0)
	{
		res = false;
	}

	if (beta > 1 || beta < 0)
	{
		res = false;
	}

	if (alpha + beta + gamma != 1)
	{
		res = false;
	}

	return res;
}

void myMesh::triangulate()
{
	std::vector<myFace*> m_faces = faces;
	for each(myFace * face in m_faces)
	{
		bool is_triangle = triangulate(face);

		if (is_triangle)
			continue;

		int max_iterations = 1000;
		int iteration = 0;

		while (iteration < max_iterations)
		{
			iteration++;

			myHalfedge* start_he = face->adjacent_halfedge;
			if (!start_he) break;

			myHalfedge* he = start_he;
			int edge_count = 0;
			do {
				edge_count++;
				he = he->next;
			} while (he != start_he && edge_count < 1000);

			if (edge_count <= 3) {
				face->computeNormal();
				break;
			}

			bool ear_found = false;
			he = start_he;

			for (int attempt = 0; attempt < edge_count; attempt++)
			{
				myHalfedge* current = he;

				if (!current) {
					he = he->next;
					continue;
				}

				if (isValidEar(current))
				{
					myFace* newFace = triangulate(current);
					if (newFace)
					{
						ear_found = true;
						break;
					}
				}

				he = he->next;
			}

			if (!ear_found)
			{
				std::cout << "Polygon degenerate" << std::endl;
				break;
			}
		}

		if (iteration >= max_iterations)
		{
			std::cout << "Hit iteration limit for face. " << std::endl;
		}
	}
}

//return false if already triangle, true othewise.
bool myMesh::triangulate(myFace* f)
{
	auto start_he = f->adjacent_halfedge;
	auto he = start_he;

	int i = 0;

	do {
		i += 1;

		he = he->next;
	} while (he != start_he);

	if (i > 3) {
		return false;
	}

	checkMesh();

	return true;
}

bool myMesh::isValidEar(myHalfedge* current)
{
	if (!current || !current->next || !current->prev)
		return false;

	if (!convex(current->prev, current, current->next))
		return false;

	if (point_inside(current))
		return false;

	return true;
}

myFace* myMesh::triangulate(myHalfedge* current)
{
	if (!current || !current->next || !current->prev)
		return nullptr;

	auto a = current->prev;
	auto b = current;
	auto c = current->next;

	auto beforeA = a->prev;
	auto afterC = c->next;

	myFace* oldFace = b->adjacent_face;
	myFace* newFace = new myFace();

	// new hedge
	myHalfedge* h1 = new myHalfedge();
	myHalfedge* h2 = new myHalfedge();

	h1->source = c->source;
	h2->source = a->source;

	h1->twin = h2;
	h2->twin = h1;

	a->next = b;
	b->prev = a;

	b->next = h1;
	h1->prev = b;

	h1->next = a;
	a->prev = h1;

	a->adjacent_face = newFace;
	b->adjacent_face = newFace;
	h1->adjacent_face = newFace;
	newFace->adjacent_halfedge = a;

	// Second face
	beforeA->next = h2;
	h2->prev = beforeA;

	h2->next = c;
	c->prev = h2;

	h2->adjacent_face = oldFace;
	c->adjacent_face = oldFace;
	beforeA->adjacent_face = oldFace;
	oldFace->adjacent_halfedge = h2;

	myHalfedge* it = c;
	while (it != h2)
	{
		it->adjacent_face = oldFace;
		it = it->next;
	}

	halfedges.push_back(h1);
	halfedges.push_back(h2);
	faces.push_back(newFace);

	newFace->computeNormal();
	oldFace->computeNormal();

	return newFace;
}

bool myMesh::convex(myHalfedge* ah, myHalfedge* bh, myHalfedge* ch) {
	auto u = myVector3D(bh->source->point, ah->source->point);
	auto v = myVector3D(ch->source->point, bh->source->point);

	auto n = ah->adjacent_face->normal;

	auto c = (u.crossproduct(v)) * *n;

	if (c > 0)
	{
		return true;
	}
	return false;
}

bool myMesh::point_inside(myHalfedge* current)
{
	myHalfedge* next = current->next;
	myHalfedge* prev = current->prev;

	myPoint3D* a = prev->source->point;
	myPoint3D* b = current->source->point;
	myPoint3D* c = next->source->point;

	myVector3D* n = current->adjacent_face->normal;

	myHalfedge* it = next->next;

	const double eps = 1e-9; // ia suggested
	while (it != prev)
	{
		myPoint3D* p = it->source->point;

		myVector3D ab(a, b);
		myVector3D ap(a, p);

		myVector3D bc(b, c);
		myVector3D bp(b, p);

		myVector3D ca(c, a);
		myVector3D cp(c, p);

		double s1 = (ab.crossproduct(ap)) * *n;
		double s2 = (bc.crossproduct(bp)) * *n;
		double s3 = (ca.crossproduct(cp)) * *n;

		if (s1 >= -eps && s2 >= -eps && s3 >= -eps)
			return true;
		it = it->next;
	}

	return false;
}

void myMesh::subdivisionLoop() {}

void myMesh::loopPhase1() {}