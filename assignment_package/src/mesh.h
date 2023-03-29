#ifndef MESH_H
#define MESH_H

#include "components/halfedge.h"
#include "drawable.h"
#include "smartpointerhelp.h"
#include <vector>

class Mesh :  public Drawable
{
private:
    std::vector<uPtr<Face>> faces;
    std::vector<uPtr<HalfEdge>> half_edges;
    std::vector<uPtr<Vertex>> vertices;

    bool skinned;

    friend class MyGL;

public:
    Mesh(OpenGLContext* context);

    void create() override;
};

#endif // MESH_H
