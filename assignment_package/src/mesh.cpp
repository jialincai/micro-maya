#include "mesh.h"

Mesh::Mesh(OpenGLContext* context) : Drawable(context),
                                     skinned(false)
{}

void Mesh::create() {
    // Collect geometry vertice attributes
    // to later setup VBO's for the shader program.
    std::vector<glm::vec4> pos_VBO, nor_VBO, col_VBO;
    std::vector<GLuint> idx;

    std::vector<glm::vec2> jointWts_VBO;
    std::vector<glm::ivec2> jointIDs_VBO;

    int global_vct = 0;
    for (const auto& face : faces) {
        int face_vct = 0;
        HalfEdge* curr_he = face->half_edge;
        do {
            // Add vertex position to VBO.
            pos_VBO.push_back(glm::vec4(curr_he->vertex->pos, 1));
            // Add vertex normal to VBO.
            nor_VBO.push_back(glm::vec4(glm::normalize(glm::cross(curr_he->vertex->pos        - curr_he->sym->vertex->pos,
                                        curr_he->next->vertex->pos  - curr_he->next->sym->vertex->pos)), 0));
            // Add vertex color to VBO.
            col_VBO.push_back(glm::vec4(face->color, 0));

            if (skinned) {
                // Add vertex joint weights to VBO
                jointWts_VBO.push_back(glm::vec2(curr_he->vertex->infl_weights[0],
                                                 curr_he->vertex->infl_weights[1]));
                // Add vertex jointID's to VBO
                jointIDs_VBO.push_back(glm::ivec2(curr_he->vertex->infl_joints[0]->id,
                                                  curr_he->vertex->infl_joints[1]->id));
            }

            curr_he = curr_he->next;
            face_vct++;
        } while (curr_he != face->half_edge);

        // Triangulate face by adding indices to VBO.
        for (int i = 0; i < face_vct - 2; i++) {
            idx.push_back(global_vct);
            idx.push_back(global_vct + i + 1);
            idx.push_back(global_vct + i + 2);
        }

        global_vct += face_vct;
    }
    count = idx.size();

    // Setup VBO's.
    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    // Pass the data stored in idx into the bound buffer, reading a number of bytes equal to
    // idx.size() multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos_VBO.size() * sizeof(glm::vec4), pos_VBO.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor_VBO.size() * sizeof(glm::vec4), nor_VBO.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col_VBO.size() * sizeof(glm::vec4), col_VBO.data(), GL_STATIC_DRAW);

    if (skinned) {
        generateWts();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufWts);
        mp_context->glBufferData(GL_ARRAY_BUFFER, jointWts_VBO.size() * sizeof(glm::vec2), jointWts_VBO.data(), GL_STATIC_DRAW);

        generateIDs();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufIDs);
        mp_context->glBufferData(GL_ARRAY_BUFFER, jointIDs_VBO.size() * sizeof(glm::ivec2), jointIDs_VBO.data(), GL_STATIC_DRAW);
    }
}
