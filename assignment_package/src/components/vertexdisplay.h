#ifndef VERTEXDISPLAY_H
#define VERTEXDISPLAY_H

#include "drawable.h"
#include "components/vertex.h"


class VertexDisplay : public Drawable
{
private:
    Vertex *representedVertex;
    bool isSelected = false;

    friend class MyGL;

public:
    VertexDisplay(OpenGLContext* context);

    // Creates VBO data to make a visual
    // representation of the currently selected Vertex
    void create() override;

    // Change which Vertex representedVertex points to
    void setSelected(Vertex* v);

    GLenum drawMode() override;
};

#endif // VERTEXDISPLAY_H
