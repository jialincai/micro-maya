#include "joint.h"

size_t Joint::next_id{ 0 };

Joint::Joint(OpenGLContext* context)
    : QTreeWidgetItem(), Drawable(context), name(""), id(next_id++),
    parent(nullptr), children(std::vector<uPtr<Joint>>()),
    pos(glm::vec3()), rot(glm::quat()), bind(glm::mat4()),
    selected(false)
{}

Joint::Joint(const Joint &j) : QTreeWidgetItem(), Drawable(j.mp_context),
                               name(j.name), id(j.id),
                               parent(j.parent),
                               pos(j.pos), rot(j.rot), bind(j.bind),
                               selected(j.selected)
{
    QTreeWidgetItem::setText(0, name);
    // Create deep copy of the children.
    for (auto const &c : j.children) {
        children.push_back(mkU<Joint>(*c));
    }
}

Joint::~Joint() {}

void Joint::setName(QString s) {
    name = s;
    QTreeWidgetItem::setText(0, name);
}

Joint& Joint::operator=(const Joint &n) {
    *this = Joint(n);
    return *this;
}

glm::mat4 Joint::getLocalTransformation() const {
    return glm::translate(glm::mat4(1.f), pos) * glm::mat4_cast(rot);
}

glm::mat4 Joint::getOverallTransformation() const {
    glm::mat4 tmat(1.f);

    const Joint* curr = this;
    while (curr != nullptr) {
        tmat = curr->getLocalTransformation() * tmat;
        curr = curr->parent;
    }

    return tmat;
}

void Joint::create() {
    std::vector<glm::vec4> posVBO, norVBO, colVBO;
    std::vector<GLuint> idx;

    // Colors
    glm::vec4 white = glm::vec4(1, 1, 1, 0);
    glm::vec4 red = glm::vec4(1, 0, 0, 0);
    glm::vec4 green = glm::vec4(0, 1, 0, 0);
    glm::vec4 blue = glm::vec4(0, 0, 1, 0);

    // Circle 1
    std::vector<glm::vec4> circlePos = getCirclePos();
    std::vector<glm::vec4> circlePosXY;
    for (auto const &p : circlePos) {
        circlePosXY.push_back(getOverallTransformation() * p);
    }
    pushCircle(posVBO, norVBO, colVBO, idx, circlePosXY,
               selected ? white : blue);

    // Circle 2
    std::vector<glm::vec4> circlePosXZ;
    glm::mat4 M = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(1, 0, 0));
    for (auto const &p : circlePos) {
        circlePosXZ.push_back(getOverallTransformation() * M * p);
    }
    pushCircle(posVBO, norVBO, colVBO, idx, circlePosXZ,
               selected ? white : green);

    // Circle 3
    std::vector<glm::vec4> circlePosYZ;
    M = glm::rotate(glm::mat4(1.f), glm::radians(90.0f), glm::vec3(0, 1, 0));
    for (auto const &p : circlePos) {
        circlePosYZ.push_back(getOverallTransformation() * M * p);
    }
    pushCircle(posVBO, norVBO, colVBO, idx, circlePosYZ,
               selected ? white : red);

    // Draw an edge from this joint to its parent.
    if (parent != nullptr) {
        posVBO.push_back(getOverallTransformation() * glm::vec4(0, 0, 0, 1));
        posVBO.push_back(parent->getOverallTransformation() * glm::vec4(0, 0, 0, 1));
        // This is just an arbitrary normal
        // since we're rendering the joint display
        // with the flat shader.
        norVBO.push_back(glm::normalize(glm::vec4(1.f)));
        norVBO.push_back(glm::normalize(glm::vec4(1.f)));
        colVBO.push_back(glm::vec4(1, 1, 0, 0));
        colVBO.push_back(glm::vec4(1, 0, 0, 0));
    }

    idx.push_back(posVBO.size() - 2);
    idx.push_back(posVBO.size() - 1);

    count = idx.size();

    // Bind and alot space to buffer.
    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, posVBO.size() * sizeof(glm::vec4), posVBO.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, norVBO.size() * sizeof(glm::vec4), norVBO.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, colVBO.size() * sizeof(glm::vec4), colVBO.data(), GL_STATIC_DRAW);
}

GLenum Joint::drawMode() {
    return GL_LINES;
}

void Joint::read(const QJsonObject &json, Joint* parent) {
    // Parse name and position
    setName(json["name"].toString());
    for (int i = 0; i < 3; i++) {
        pos[i] = json["pos"][i].toDouble();
    }

    // Parse rotation
    float angle = glm::radians(json["rot"][0].toDouble());
    float qx = json["rot"][1].toDouble() * glm::sin(angle / 2);
    float qy = json["rot"][2].toDouble() * glm::sin(angle / 2);
    float qz = json["rot"][3].toDouble() * glm::sin(angle / 2);
    float qw = glm::cos(angle / 2);
    rot = glm::quat(qw, qx, qy, qz);

    // Add this joint as a child to its parents
    if (parent != nullptr) {
        this->parent = parent;
        parent->children.push_back(mkU<Joint>(*this));
        parent->QTreeWidgetItem::addChild(parent->children.back().get());
    }

    // Parse children and set parent
    QJsonArray children_obj = json["children"].toArray();
    for (auto const &c : children_obj) {
        Joint child = Joint(this->mp_context);
        if (parent == nullptr) {
            child.read(c.toObject(), this);
        } else {
            child.read(c.toObject(), parent->children.back().get());
        }
    }
}

//--------------------------------------------------
// Helper functions
//--------------------------------------------------
std::vector<glm::vec4> Joint::getCirclePos() {
    int numSides = 12;
    std::vector<glm::vec4> pos;
    glm::vec4 p(0.5f, 0.f, 0.f, 1.f);
    float deg = glm::radians(360.f / numSides);
    for (int i = 0; i < numSides; i++)
    {
        glm::mat4 M = glm::rotate(glm::mat4(1.f), i * deg, glm::vec3(0, 0, 1));
        pos.push_back(M * p);
    }
    return pos;
}

void Joint::pushCircle(std::vector<glm::vec4> &pos,
                       std::vector<glm::vec4> &nor,
                       std::vector<glm::vec4> &col,
                       std::vector<GLuint> &idx,
                       std::vector<glm::vec4> &circlePos,
                       glm::vec4 color)
{
    unsigned int prevIdx = pos.size();
    for (auto const &p : circlePos) {
        pos.push_back(p);
        // This is just an arbitrary normal
        // since we're rendering the joint display
        // with the flat shader.
        nor.push_back(glm::normalize(glm::vec4(1, 1, 1, 1)));
        col.push_back(color);
    }

    // Triangulate face by adding indices to VBO.
    for (size_t i = prevIdx; i < pos.size(); i++) {
        idx.push_back(i);
        if (i == pos.size() - 1) {
            idx.push_back(prevIdx);
        } else {
            idx.push_back(i + 1);
        }
    }
}
