#ifndef HALFEDGEDISPLAY_H
#define HALFEDGEDISPLAY_H

#include "drawable.h"
#include "components/halfedge.h"

class HalfEdgeDisplay : public Drawable
{
private:
    HalfEdge* representedHalfEdge;
    bool isSelected = false;

    friend class MyGL;

public:
    HalfEdgeDisplay(OpenGLContext* context);

    // Creates VBO data to make a visual
    // representation of the currently selected HalfEdge
    void create() override;

    void setSelected(HalfEdge* he);

    GLenum drawMode() override;
};

#endif // HALFEDGEDISPLAY_H
