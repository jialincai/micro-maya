#include "vertex.h"

size_t Vertex::next_id{ 1 };

Vertex::Vertex(glm::vec3 pos) : id(next_id++), pos(pos), half_edge(),
                                infl_joints{nullptr, nullptr}, infl_weights{0, 0}
{
    QListWidgetItem::setText(QString::number(id));
}
