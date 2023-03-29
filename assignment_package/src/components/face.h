#ifndef FACE_H
#define FACE_H

#include "la.h"
#include <QListWidgetItem>

class HalfEdge;

class Face : public QListWidgetItem
{
private:
    static size_t next_id;

    size_t id;
    glm::vec3 color;
    HalfEdge* half_edge;

    friend class Mesh;
    friend class HalfEdge;
    friend class FaceDisplay;
    friend class MyGL;

public:
    Face();
};

#endif // FACE_H
