#include "face.h"

size_t Face::next_id { 1 };

Face::Face() : id(next_id++), half_edge()
{
    QListWidgetItem::setText(QString::number(id));
    // Generate a random color for each face.
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    color = glm::vec3(r, g, b);
}
