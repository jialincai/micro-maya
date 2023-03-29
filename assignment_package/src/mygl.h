#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"
#include "mesh.h"
#include "components/joint.h"
#include "components/vertexdisplay.h"
#include "components/halfedgedisplay.h"
#include "components/facedisplay.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QJsonDocument>
#include <QJsonObject>

#include <unordered_map>
#include <unordered_set>

//--------------------------------------------------
// TYPEDEFs and HASH structs
//--------------------------------------------------
typedef std::pair<Vertex*, Vertex*> ENDPT;

struct PAIRHASH {
    std::size_t operator()(const ENDPT& vp) const
    {
        return std::hash<Vertex*>()(vp.first) ^
           (std::hash<Vertex*>()(vp.second) << 1);
    }
};

struct PTRHASH {
    template<typename T>
    std::size_t operator()(const T* ptr) const
    {
        return std::hash<const T*>()(ptr);
    }
};

typedef std::unordered_map<Face*, Vertex*, PTRHASH> CENTROID_MAP;
typedef std::unordered_map<ENDPT, HalfEdge*, PAIRHASH> ENDPT_MAP;
typedef std::unordered_set<Vertex*, PTRHASH> VPTR_SET;
//--------------------------------------------------
// END
//--------------------------------------------------

class MyGL
    : public OpenGLContext
{
    Q_OBJECT
private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)

    ShaderProgram m_progSkelaton;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;

    Mesh m_mesh;
    bool mesh_loaded;

    VertexDisplay m_vertDisplay;
    HalfEdgeDisplay m_heDisplay;
    FaceDisplay m_faceDisplay;

    uPtr<Joint> joint;
    bool joint_loaded;

    Joint* selectedJoint;

    glm::mat4 bindMats[30];
    glm::mat4 overallTMats[30];

friend class MainWindow;

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void populateWidgets();
    ENDPT generateKey(Vertex* v_ptr1, Vertex* v_ptr2);
    void load_OBJ(const QString OBJ_file);

    void load_JSON(const QString JSON_file);
    void read(const QJsonObject &json);
    void traverseCalcBind(Joint* j);
    void traverseCreate(Joint* j);
    void traverseDraw(Joint* j);
    void traverseSkin(Vertex* curr, Joint** closest, Joint** nextClosest,
                      float* minDist, float* nextMinDist, Joint* j) const;

    void skinMesh();
    void updateUnifMats();
    void initializeUnifMats(Joint* j);

    // CATMULL stuff is happening here
    void splitEdge(HalfEdge* he);
    void triangulateFace();

    CENTROID_MAP createCentroids();
    VPTR_SET createSmoothMidpts(CENTROID_MAP &cm);
    VPTR_SET getAdjMidpts(Vertex* v) const;
    VPTR_SET getIncCentroids(Vertex* v, CENTROID_MAP &cm) const;
    void smoothOrigVerts(VPTR_SET& vs, CENTROID_MAP &cm);
    void quadrangulateFace(Face* f, CENTROID_MAP &cm);

protected:
    void keyPressEvent(QKeyEvent *e);

signals:
    void sig_sendVertex(QListWidgetItem*);
    void sig_sendFace(QListWidgetItem*);
    void sig_sendEdge(QListWidgetItem*);
    void sig_sendJoint(QTreeWidgetItem*);

    void sig_clearListWidgets();
    void sig_clearTreeWidget();

public slots:
    void slot_setSelectedVertex(QListWidgetItem*);
    void slot_setSelectedHalfEdge(QListWidgetItem*);
    void slot_setSelectedFace(QListWidgetItem*);
    void slot_setSelectedJoint(QTreeWidgetItem*);

    void slot_splitEdge();
    void slot_triangulateFace();
    void slot_subdivide();

    void slot_changeRed(const double &d);
    void slot_changeGreen(const double &d);
    void slot_changeBlue(const double &d);

    void slot_changeX(const double &d);
    void slot_changeY(const double &d);
    void slot_changeZ(const double &d);

    void slot_rotateX();
    void slot_rotateY();
    void slot_rotateZ();

    void slot_skinMesh();
};


#endif // MYGL_H
