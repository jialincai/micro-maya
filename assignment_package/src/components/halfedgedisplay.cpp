#include "halfedgedisplay.h"

HalfEdgeDisplay::HalfEdgeDisplay(OpenGLContext* context)
    : Drawable(context), representedHalfEdge(nullptr)
{}

void HalfEdgeDisplay::create()
{
    std::vector<glm::vec4> pos {glm::vec4(representedHalfEdge->vertex->pos, 1),
                                glm::vec4(representedHalfEdge->sym->vertex->pos, 1)};

    std::vector<glm::vec4> nor {glm::vec4(1, 1, 1, 0),
                                glm::vec4(1, 1, 1, 0)};

    std::vector<glm::vec4> col {glm::vec4(1, 1, 0, 0),
                                glm::vec4(1, 0, 0, 0)};

    std::vector<GLuint> idx {0, 1};

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

void HalfEdgeDisplay::setSelected(HalfEdge* he)
{
    representedHalfEdge = he;
    isSelected = true;
}

GLenum HalfEdgeDisplay::drawMode() {
    return GL_LINES;
}
