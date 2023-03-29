#include "halfedge.h"

size_t HalfEdge::next_id { 1 };

HalfEdge::HalfEdge()
    : id(next_id++), next(), vertex(), sym(), face()
{
    QListWidgetItem::setText(QString::number(id));
}

void HalfEdge::set_vertex(Vertex* vertex)
{
    this->vertex = vertex;
    vertex->half_edge = this;
}

void HalfEdge::set_face(Face* face)
{
    this->face = face;
    face->half_edge = this;
}

void HalfEdge::set_next(HalfEdge* next)
{
    this->next = next;
}

void HalfEdge::set_sym(HalfEdge* sym_edge)
{
    this->sym = sym_edge;
    sym_edge->sym = this;
}
