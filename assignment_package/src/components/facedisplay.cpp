#include "facedisplay.h"
#include "components/halfedge.h"

FaceDisplay::FaceDisplay(OpenGLContext* context)
    : Drawable(context), representedFace(nullptr)
{}

void FaceDisplay::create()
{
    std::vector<glm::vec4> pos, nor, col;
    std::vector<GLuint> idx;

    HalfEdge* curr_he = representedFace->half_edge;
    do {
        // Add vertex position to VBO.
        pos.push_back(glm::vec4(curr_he->vertex->pos, 1));
        // Add vertex normal to VBO.
        nor.push_back(glm::vec4(glm::normalize(glm::cross(curr_he->vertex->pos       - curr_he->sym->vertex->pos,
                                                          curr_he->next->vertex->pos - curr_he->next->sym->vertex->pos)), 0));
        // Add vertex color to VBO.
        col.push_back(glm::vec4(glm::vec3(1, 1, 1) - representedFace->color, 0));

        curr_he = curr_he->next;
    } while (curr_he != representedFace->half_edge);

    // Triangulate face by adding indices to VBO.
    for (size_t i = 0; i < pos.size(); i++) {
        idx.push_back(i);
        if (i == pos.size() - 1) {
            idx.push_back(0);
        } else {
            idx.push_back(i + 1);
        }
    }

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4), nor.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

void FaceDisplay::setSelected(Face* f)
{
    representedFace = f;
    isSelected = true;
}

GLenum FaceDisplay::drawMode()  {
    return GL_LINES;
}
