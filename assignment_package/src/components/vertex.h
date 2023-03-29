#ifndef VERTEX_H
#define VERTEX_H

#include "la.h"
#include "components/joint.h"
#include <QListWidgetItem>

class HalfEdge;

class Vertex : public QListWidgetItem
{
private:
    static size_t next_id;

    size_t id;
    glm::vec3 pos;
    HalfEdge* half_edge;

    Joint* infl_joints[2];
    float infl_weights[2];

    friend class Mesh;
    friend class HalfEdge;
    friend class VertexDisplay;
    friend class HalfEdgeDisplay;
    friend class FaceDisplay;
    friend class MyGL;

public:
    Vertex(glm::vec3 pos);
};



#endif // VERTEX_H
