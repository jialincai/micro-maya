#ifndef HALFEDGE_H
#define HALFEDGE_H

#include "components/face.h"
#include "components/vertex.h"
#include <QListWidgetItem>

class HalfEdge : public QListWidgetItem
{
private:
    static size_t next_id;

    size_t id;
    HalfEdge* next;
    Vertex* vertex;
    HalfEdge* sym;
    Face* face;

    friend class Mesh;
    friend class HalfEdgeDisplay;
    friend class FaceDisplay;
    friend class MyGL;

public:
    HalfEdge();

    void set_vertex(Vertex* vertex);
    void set_face(Face* face);
    void set_next(HalfEdge* next);
    void set_sym(HalfEdge* sym_edge);
};

#endif // HALFEDGE_H
