#ifndef FACEDISPLAY_H
#define FACEDISPLAY_H

#include "drawable.h"
#include "components/face.h"

class FaceDisplay : public Drawable
{   
private:
    Face* representedFace;
    bool isSelected = false;

    friend class MyGL;

public:
    FaceDisplay(OpenGLContext* context);

    // Creates VBO data to make a visual
    // representation of the currently selected Face
    void create() override;

    // Change which Face representedFace points to
    void setSelected(Face* v);

    GLenum drawMode() override;
};

#endif // FACEDISPLAY_H
