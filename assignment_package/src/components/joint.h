#ifndef JOINT_H
#define JOINT_H

#include "la.h"
#include "smartpointerhelp.h"
#include "drawable.h"

#include <QJsonObject>
#include <QTreeWidgetItem>
#include <QJsonArray>

class Joint : public QTreeWidgetItem, public Drawable {
private:
    static size_t next_id;
    //--------------------------------------------------------------------------------
    // Member variables
    //--------------------------------------------------------------------------------
    QString name;
    size_t id;
    Joint* parent;
    std::vector<uPtr<Joint>> children;

    glm::vec3 pos;
    glm::quat rot;
    glm::mat4 bind;

    bool selected;

    friend class MyGL;
    friend class Mesh;

public:
    //--------------------------------------------------------------------------------
    // Constructors // Destructors
    //--------------------------------------------------------------------------------
    Joint(OpenGLContext* context);
    Joint(const Joint &n);

    virtual ~Joint();

    //-------------------------------------------------------- ------------------------
    // Accessors // Mutators
    //--------------------------------------------------------------------------------
    void setName(QString s);

    //--------------------------------------------------------------------------------
    // Operator Functions
    //--------------------------------------------------------------------------------
    Joint& operator=(const Joint &n);

    //--------------------------------------------------------------------------------
    // Public member Functions
    //--------------------------------------------------------------------------------
    // Returns a mat4 that represents the
    // concatenation of a joint's position and rotation.
    glm::mat4 getLocalTransformation() const;
    // Returns a mat4 that represents the
    // concatentation of this joint's local transformation
    // with the transformations of its chain of parent joints.
    glm::mat4 getOverallTransformation() const;

    // Creates VBO data to make a visual
    // representation of the currently selected Joint
    // and a line to its parent.
    void create() override;

    GLenum drawMode() override;

    void read(const QJsonObject &json, Joint*  parent);

private:
    //--------------------------------------------------------------------------------
    // Private helper functions
    //--------------------------------------------------------------------------------
    // Returns the vertex positions of a 12-sided polygon.
    // Along the x/y axis.
    std::vector<glm::vec4> getCirclePos();

    // Add the attributes of a circle to the VBO.
    void pushCircle(std::vector<glm::vec4> &pos,
                    std::vector<glm::vec4> &nor,
                    std::vector<glm::vec4> &col,
                    std::vector<GLuint> &idx,
                    std::vector<glm::vec4> &circlePos,
                    glm::vec4 color);

};

#endif // JOINT_H
