#include "mygl.h"
#include <la.h>

#include <iostream>
#include <utility>
#include <QApplication>
#include <QFile>
#include <QKeyEvent>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this),
      m_progSkelaton(this),
      m_glCamera(),
      m_mesh(this), mesh_loaded(false),
      m_vertDisplay(this), m_heDisplay(this), m_faceDisplay(this),
      joint(mkU<Joint>(this)), joint_loaded(false),
      selectedJoint(nullptr),
      bindMats{}, overallTMats{}
{
    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the skelaton deform shader.
    m_progSkelaton.create(":/glsl/skelaton.vert.glsl", ":/glsl/lambert.frag.glsl");

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(m_glCamera.eye);
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    m_progSkelaton.setViewProjMatrix(m_glCamera.getViewProj());
    m_progSkelaton.setCamPos(m_glCamera.eye);


    if (mesh_loaded) {
        if (m_mesh.skinned) {
            m_progSkelaton.setModelMatrix(glm::mat4(1.f));
            m_progSkelaton.draw(m_mesh);
        } else {
            m_progLambert.setModelMatrix(glm::mat4(1.f));
            m_progLambert.draw(m_mesh);
        }
        glDisable(GL_DEPTH_TEST);
        if (m_vertDisplay.isSelected) {
            m_progFlat.draw(m_vertDisplay);
        }
        if (m_heDisplay.isSelected) {
            m_progFlat.draw(m_heDisplay);
        }
        if (m_faceDisplay.isSelected) {
            m_progFlat.draw(m_faceDisplay);
        }
        glEnable(GL_DEPTH_TEST);
    } else {
        //Create a model matrix. This one rotates the square by PI/4 radians then translates it by <-2,0,0>.
        //Note that we have to transpose the model matrix before passing it to the shader
        //This is because OpenGL expects column-major matrices, but you've
        //implemented row-major matrices.
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2,0,0)) * glm::rotate(glm::mat4(), 0.25f * 3.14159f, glm::vec3(0,1,0));
        //Send the geometry's transformation matrix to the shader
        m_progLambert.setModelMatrix(model);
        //Draw the example sphere using our lambert shader
        m_progLambert.draw(m_geomSquare);

        //Now do the same to render the cylinder
        //We've rotated it -45 degrees on the Z axis, then translated it to the point <2,2,0>
        model = glm::translate(glm::mat4(1.0f), glm::vec3(2,2,0)) * glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0,0,1));
        m_progLambert.setModelMatrix(model);
        m_progLambert.draw(m_geomSquare);
    }

    // Draw joints
    if (joint_loaded) {
        glDisable(GL_DEPTH_TEST);
        traverseDraw(joint.get());
        glEnable(GL_DEPTH_TEST);
    }
}

// This functions sends signals to the UIWindow
// to populate List Wigets with mesh components.
void MyGL::populateWidgets() {
    // Send vertices, faces, and edge pointers to QListWidget
    for (auto const &v : m_mesh.vertices) {
        emit sig_sendVertex(v.get());
    }
    for (auto const &e : m_mesh.half_edges) {
        emit sig_sendEdge(e.get());
    }
    for (auto const &f : m_mesh.faces) {
        emit sig_sendFace(f.get());
    }
}

// In order to ensure key pairs match regardless of order,
// we ensure the vertex pointer with the smaller address is
// always first.
ENDPT MyGL::generateKey(Vertex* v1, Vertex* v2) {
    if (v1 < v2) {
        return ENDPT(v1, v2);
    }
    return ENDPT(v2, v1);
}

void MyGL::load_OBJ(const QString OBJ_file) {
    // reset all the component id variables
    Vertex::next_id = 1;
    HalfEdge::next_id = 1;
    Face::next_id = 1;

    m_mesh = Mesh(this);
    emit sig_clearListWidgets();

    QFile file(OBJ_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    ENDPT_MAP seen_vps;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if        (line.size() < 2) {
            continue;
        }

        // Populate mesh vertices.
        // At this point, the vertices' half edge pointers are not set.
        if        (line.first(2) == "v ") {
            QStringList list = line.split(' ');
            m_mesh.vertices.push_back(mkU<Vertex>(Vertex(glm::vec3(list[1].toFloat(),
                                                                   list[2].toFloat(),                                                           list[3].toFloat()))));
        // Populate mesh faces and half edges.
        } else if (line.first(2) == "f ") {
            // Create face.
            m_mesh.faces.push_back(mkU<Face>(Face()));
            bool first_he = true;
            HalfEdge* first_he_ptr = nullptr;
            QStringList list = line.split(' ');
            for (int i = 1; i < list.size(); ++i) {
                // Find a half edges's endpoint vertices.
                int curr_vi = list[i].split('/')[0].toInt() - 1;
                int next_vi;
                if (i + 1 == list.size()) {
                    next_vi = list[1].split('/')[0].toInt() - 1;
                } else {
                    next_vi = list[i + 1].split('/')[0].toInt() - 1;
                }
                // Create a half edge.
                // At this point, sym half edges are not set yet.
                m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
                HalfEdge* this_he_ptr = m_mesh.half_edges.back().get();
                // Update face and vertex half edge pointers.
                this_he_ptr->set_vertex(m_mesh.vertices[next_vi].get());
                this_he_ptr->set_face(m_mesh.faces.back().get());

                // Add the half edges vertex pair to the map
                // if they haven't been encountered.
                // If a vertex pair has been encountered, set sym half edges.
                // In this implementation, Vertex ptr with the lower address is first in the pair.
                ENDPT key = generateKey(m_mesh.vertices[curr_vi].get(), m_mesh.vertices[next_vi].get());
                auto seen = seen_vps.find(key);
                if (seen != seen_vps.end()) {
                    this_he_ptr->set_sym(seen_vps.at(key));
                } else {
                    seen_vps[key] = this_he_ptr;
                }

                // The first half edge of each face is a special case.
                if (first_he) {
                    // We need to save the first half edge to set as the last half edge's next.
                    first_he_ptr = this_he_ptr;
                    // There doesn't exist a previous half edge
                    // so we can skip the rest of this loop.
                    first_he = false;
                    continue;
                }

                // Set the previous edge's next half edge pointer to this half edge.
                m_mesh.half_edges[m_mesh.half_edges.size() - 2]->set_next(this_he_ptr);

                // Last half edge has it's next pointer pointing to first half edge.
                if (i + 1 == list.size()) {
                    this_he_ptr->set_next(first_he_ptr);
                }
            }
        }
    }
    mesh_loaded = true;
    m_mesh.create();
    populateWidgets();
}

void MyGL::load_JSON(const QString JSON_file) {
   Joint::next_id = 0;
   if (mesh_loaded) {
       m_mesh.skinned = false;
   }
   emit sig_clearTreeWidget();

   QFile loadFile(JSON_file);
   if (!loadFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
       return;
   }

   QByteArray jointData = loadFile.readAll();
   QJsonDocument loadDoc(QJsonDocument::fromJson(jointData));

   read(loadDoc.object());

   // Traverse the joint tree and create every joint
   traverseCalcBind(joint.get());
   traverseCreate(joint.get());

   joint_loaded = true;
   emit sig_sendJoint(joint.get());
}

void MyGL::read(const QJsonObject &json)
{
    if (json.contains("root") && json["root"].isObject()) {
        joint->read(json["root"].toObject(), nullptr);
    }
}

void MyGL::traverseCalcBind(Joint* j) {
    for (auto &child : j->children) {
        traverseCalcBind(child.get());
    }
    j->bind = glm::inverse(j->getOverallTransformation());
}

void MyGL::traverseCreate(Joint* j) {
    for (auto &child : j->children) {
        traverseCreate(child.get());
    }

    j->create();
}

void MyGL::traverseDraw(Joint* j) {
    for (auto &child : j->children) {
        traverseDraw(child.get());
    }

    m_progFlat.draw(*j);
}

void MyGL::traverseSkin(Vertex* curr, Joint** closest, Joint** nextClosest,
                        float* minDist, float* nextMinDist, Joint* j) const {
    float distToJoint = glm::length(glm::vec4(curr->pos, 1) - j->getOverallTransformation() * glm::vec4(0, 0, 0, 1));
    if (distToJoint < *minDist || (distToJoint < *minDist && *nextClosest == nullptr)) {
        *nextMinDist = *minDist;
        *nextClosest = *closest;

        *minDist = distToJoint;
        *closest = j;
    } else if (distToJoint < *nextMinDist) {
        *nextMinDist = distToJoint;
        *nextClosest = j;
    }

    for (auto &child : j->children) {
        traverseSkin(curr, closest, nextClosest, minDist, nextMinDist, child.get());
    }
}

void MyGL::skinMesh() {
    for (auto const &v : m_mesh.vertices) {
        Joint* closest = nullptr;
        Joint* nextClosest = nullptr;

        float minDist = INFINITY;
        float nextMinDist = INFINITY;

        // Traverse the mesh and keep track of the joint ptrs
        // and distaces.
        traverseSkin(v.get(), &closest, &nextClosest, &minDist, &nextMinDist, joint.get());

        v->infl_joints[0] = closest;
        v->infl_joints[1] = nextClosest;

        float sum = minDist + nextMinDist;
        v->infl_weights[0] = 1 - minDist / sum;
        v->infl_weights[1] = 1 - nextMinDist / sum;
    }

    m_mesh.skinned = true;
    updateUnifMats();
    m_mesh.create();
}

// Recalculate uniform matrics and send to shader.
void MyGL::updateUnifMats() {
    initializeUnifMats(joint.get());
    m_progSkelaton.setBindMats(bindMats);
    m_progSkelaton.setOverallTMats(overallTMats);
}

void MyGL::initializeUnifMats(Joint* j) {
    for (auto &child : j->children) {
        initializeUnifMats(child.get());
    }

    bindMats[j->id] = j->bind;
    overallTMats[j->id] = j->getOverallTransformation();
}

// CATMULL stuff is happening here
// This function splits the passed in edge
// by adding a vertex in the middle.
void MyGL::splitEdge(HalfEdge* he) {
    HalfEdge* he_sym = he->sym;

    // V3 is the average of the endpoints of the selected half-edge
    glm::vec3 v1_pos = he->vertex->pos;
    glm::vec3 v2_pos = he_sym->vertex->pos;
    glm::vec3 v3_pos = (v1_pos + v2_pos);
    v3_pos /= 2;
    m_mesh.vertices.push_back(mkU<Vertex>(Vertex(v3_pos)));
    Vertex* v3 = m_mesh.vertices.back().get();

    // Create two new half edges
    // with the same face and vertex pointers as the original
    m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
    HalfEdge* he_copy = m_mesh.half_edges.back().get();
    // Update face and vertex half edge pointers.
    he_copy->set_vertex(he->vertex);
    he_copy->set_face(he->face);
    m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
    HalfEdge* he_sym_copy = m_mesh.half_edges.back().get();
    // Update face and vertex half edge pointers.
    he_sym_copy->set_vertex(he_sym->vertex);
    he_sym_copy->set_face(he_sym->face);

    // Rearrange pointers to correct data structure flow
    he_copy->set_next(he->next);
    he_sym_copy->set_next(he_sym->next);
    he->set_next(he_copy);
    he_sym->set_next(he_sym_copy);

    he->set_vertex(v3);
    he_sym->set_vertex(v3);

    he->set_sym(he_sym_copy);
    he_sym->set_sym(he_copy);

    // Send newly create vertex and half edges to GUI
    emit sig_sendVertex(v3);
    emit sig_sendEdge(he_copy);
    emit sig_sendEdge(he_sym_copy);
}

// This function recursively calls itself
// splitting out triangles from the N-gon face
// until entire face is triangulated.
void MyGL::triangulateFace() {
    Face* f = m_faceDisplay.representedFace;

    // If no face selected, do nothing
    if (f == nullptr) {
        return;
    }

    HalfEdge* he = f->half_edge;
    // Recursively triangulate
    if (he->next->next->next == he) {
        return;
    } else {
        // Create two new half edges
        m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
        HalfEdge* he_newface = m_mesh.half_edges.back().get();
        he_newface->set_vertex(he->vertex);
        m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
        HalfEdge* he_oldface = m_mesh.half_edges.back().get();
        // Update face and vertex half edge pointers.
        he_oldface->set_vertex(he->next->next->vertex);
        he_newface->set_sym(he_oldface);

        // Create a new face
        m_mesh.faces.push_back(mkU<Face>(Face()));
        Face* new_f = m_mesh.faces.back().get();
        he->next->set_face(new_f);
        he->next->next->set_face(new_f);
        he_newface->set_face(new_f);

        he_oldface->set_face(f);

        // Set next pointers
        he_oldface->set_next(he->next->next->next);
        he->next->next->set_next(he_newface);
        he_newface->set_next(he->next);
        he->set_next(he_oldface);

        // Send newly created face half edges.
        emit sig_sendEdge(he_newface);
        emit sig_sendEdge(he_oldface);
        emit sig_sendFace(new_f);

        // Reset the half edge to which this face points.
        he->set_face(f);
        triangulateFace();
    }
}

// This function adds centroids to the mesh
// and populates a map such that
// key = face*, value = centroid*.
CENTROID_MAP MyGL::createCentroids() {
    std::unordered_map<Face*, Vertex*, PTRHASH> centroids;

    for (auto &f : m_mesh.faces) {
        // Calculate the centroid position
        // by averaging all vertices of the face.
        int counter = 0;
        glm::vec3 avg_pos(0);
        HalfEdge* curr = f->half_edge;
        do {
            avg_pos += curr->vertex->pos;
            counter++;
            curr = curr->next;
        } while (curr != f->half_edge);
        avg_pos /= counter;

        // Create centroid.
        // Add the face and centroid to map.
        m_mesh.vertices.push_back(mkU<Vertex>(Vertex(avg_pos)));
        Vertex* centroid = m_mesh.vertices.back().get();
        centroids[f.get()] = centroid;

        // Send centroid to List Widget
        emit sig_sendVertex(centroid);
    }

    return centroids;
}

// This function adds smoothed midpoints to the mesh.
// It also splits edges with the smooth midpoint.
// It returns a set of all the original vertices of the mesh.
VPTR_SET MyGL::createSmoothMidpts(CENTROID_MAP &cm) {
    VPTR_SET og_verts;

    for (size_t i = 0; i < m_mesh.vertices.size() - cm.size(); i++) {
        og_verts.insert(m_mesh.vertices[i].get());
    }

    // We only want to iterate through the original edges.
    // Additionally if a half edge is included,
    // it's sym does not need to be.
    std::unordered_set<HalfEdge*, PTRHASH> syms;
    std::vector<HalfEdge*> edges_to_split;
    for (auto const &e : m_mesh.half_edges) {
        if (syms.find(e.get()) == syms.end()) {
            edges_to_split.push_back(e.get());
            syms.insert(e->sym);
        }
    }

    for (auto const &e : edges_to_split) {
        // Split the edge
        splitEdge(e);

        // Calculate the smoothed midpoint position
        glm::vec3 midpt_pos(0);

        // There are two cases.
        // 1) Edge has two incident faces
        if (e->sym->face != nullptr) {
            midpt_pos += e->next->vertex->pos;
            midpt_pos += e->sym->vertex->pos;
            midpt_pos += cm.at(e->face)->pos;
            midpt_pos += cm.at(e->sym->face)->pos;
            midpt_pos /= 4;
        }
        // 2) Edge has one incident face
        else {
            midpt_pos += e->next->vertex->pos;
            midpt_pos += e->sym->vertex->pos;
            midpt_pos += cm.at(e->face)->pos;
            midpt_pos /= 3;
        }

        // And modify the newly created vertex
        // from the split edge.
        e->vertex->pos = midpt_pos;
    }
    return og_verts;
}

VPTR_SET MyGL::getAdjMidpts(Vertex* v) const {
    VPTR_SET adjVerts;
    HalfEdge* curr = v->half_edge;
    do {
        curr = curr->next;
        adjVerts.insert(curr->vertex);
        curr = curr->sym;
    } while (curr != v->half_edge);
    return adjVerts;
}

VPTR_SET MyGL::getIncCentroids(Vertex* v, CENTROID_MAP &cm) const {
    VPTR_SET incCentroids;
    HalfEdge* curr = v->half_edge;
    do {
        curr = curr->next;
        incCentroids.insert(cm.at(curr->face));
        curr = curr->sym;
    } while (curr != v->half_edge);
    return incCentroids;
}

void MyGL::smoothOrigVerts(VPTR_SET& vs, CENTROID_MAP &cm) {
    for (auto const &vptr : vs) {
        glm::vec3 og_pos = vptr->pos;

        VPTR_SET adj_verts = getAdjMidpts(vptr);
        glm::vec3 adjv_sum(0);
        for (auto const &v : adj_verts) {
            adjv_sum += v->pos;
        }

        VPTR_SET inc_centroids = getIncCentroids(vptr, cm);
        glm::vec3 incc_sum(0);
        for (auto const &c : inc_centroids) {
            incc_sum += c->pos;
        }

        int n = adj_verts.size();

        og_pos *= (n - 2);
        og_pos /= n;
        adjv_sum /= (n * n);
        incc_sum /= (n * n);

        vptr->pos = og_pos + adjv_sum + incc_sum;
    }
}

void MyGL::quadrangulateFace(Face* f, CENTROID_MAP &cm) {
    HalfEdge* curr = f->half_edge->next;
    curr->set_face(f);

    HalfEdge* connection_edge;
    HalfEdge* next_connection_edge = curr->next;
    HalfEdge* next_face_edge = curr->next->next;

    HalfEdge* start_edge =curr->next->next;

    HalfEdge* first_centroid_to_midpt_edge;
    HalfEdge* prev_midpt_to_centroid;

    bool loop = true;

    while (loop) {
        Vertex* prev_midpt = curr->vertex;      // Store previous midpoint.
        curr = next_face_edge;                  // Update starting edge of new face.
        connection_edge = next_connection_edge; // Update edge that points to starting edge.
        next_connection_edge = curr->next;      // Store pointer to next connection edge.
        next_face_edge = curr->next->next;      // Store pointer to starting edge of next new face.

        // Create two new half edges
        m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
        HalfEdge *midpt_to_centroid = m_mesh.half_edges.back().get();
        midpt_to_centroid->set_vertex(cm.at(f));
        m_mesh.half_edges.push_back(mkU<HalfEdge>(HalfEdge()));
        HalfEdge *centroid_to_midpt = m_mesh.half_edges.back().get();
        centroid_to_midpt->set_vertex(prev_midpt);

        // Send to UI
        emit sig_sendEdge(midpt_to_centroid);
        emit sig_sendEdge(centroid_to_midpt);

        // Last quadrangulated face is a special case.
        // We don't need to create a new face.
        Face* face;
        if (next_face_edge == start_edge) {
            face = f;
        // Create new face
        } else {
            m_mesh.faces.push_back(mkU<Face>(Face()));
            face = m_mesh.faces.back().get();
            // Send to UI
            emit sig_sendFace(face);
        }

        // Update half-edge face pointers.
        midpt_to_centroid->set_face(face);
        centroid_to_midpt->set_face(face);
        connection_edge->set_face(face);
        curr->set_face(face);

        // Update half edge next pointers.
        curr->set_next(midpt_to_centroid);
        midpt_to_centroid->set_next(centroid_to_midpt);
        centroid_to_midpt->set_next(connection_edge);

        // Set sym pointers.
        // First quadrangulated face is a special case,
        // since no edge has been previously created.
        if (curr == start_edge) {
            first_centroid_to_midpt_edge = centroid_to_midpt;
            prev_midpt_to_centroid = midpt_to_centroid;

        } else if (next_face_edge == start_edge) {
            centroid_to_midpt->set_sym(prev_midpt_to_centroid);
            midpt_to_centroid->set_sym(first_centroid_to_midpt_edge);
            loop = false;
        }
        else {
            centroid_to_midpt->set_sym(prev_midpt_to_centroid);
            prev_midpt_to_centroid = midpt_to_centroid;
        }
    }
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_glCamera.rotateHori_P(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_glCamera.rotateHori_P(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_glCamera.rotateVert_P(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_glCamera.rotateVert_P(amount);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.zoom_P(-amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.zoom_P(amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.panHori_P(-amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.panHori_P(amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.panVert_P(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.panVert_P(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());
    }

    // Keypresses for iterating through selected edges and vertices
    else if (e->key() == Qt::Key_N) {
        if (m_heDisplay.representedHalfEdge != nullptr) {
            m_heDisplay.setSelected(m_heDisplay.representedHalfEdge->next);
            m_heDisplay.create();
            update();
        }
    } else if (e->key() == Qt::Key_M) {
        if (m_heDisplay.representedHalfEdge != nullptr) {
            m_heDisplay.setSelected(m_heDisplay.representedHalfEdge->sym);
            m_heDisplay.create();
            update();
        }
    } else if (e->key() == Qt::Key_F) {
        if (m_heDisplay.representedHalfEdge != nullptr) {
            m_faceDisplay.setSelected(m_heDisplay.representedHalfEdge->face);
            m_faceDisplay.create();
            update();
        }
    } else if (e->key() == Qt::Key_V) {
        if (m_heDisplay.representedHalfEdge != nullptr) {
            m_vertDisplay.setSelected(m_heDisplay.representedHalfEdge->vertex);
            m_vertDisplay.create();
            update();
        }
    } else if (e->modifiers().testFlag(Qt::ShiftModifier) && e->key() == Qt::Key_H) {
        if (m_faceDisplay.representedFace != nullptr) {
            m_heDisplay.setSelected(m_faceDisplay.representedFace->half_edge);
            m_heDisplay.create();
            update();
        }
    } else if (e->key() == Qt::Key_H) {
        if (m_vertDisplay.representedVertex != nullptr) {
            m_heDisplay.setSelected(m_vertDisplay.representedVertex->half_edge);
            m_heDisplay.create();
            update();
        }
    }

    m_glCamera.RecomputeAttributes();
    update();  // Calls paintGL, among other things
}

void MyGL::slot_setSelectedVertex(QListWidgetItem *i) {
    m_vertDisplay.setSelected(static_cast<Vertex*>(i));
    m_vertDisplay.create();
    update();
}

void MyGL::slot_setSelectedHalfEdge(QListWidgetItem *i) {
    m_heDisplay.setSelected(static_cast<HalfEdge*>(i));
    m_heDisplay.create();
    update();
}

void MyGL::slot_setSelectedFace(QListWidgetItem *i) {
    m_faceDisplay.setSelected(static_cast<Face*>(i));
    m_faceDisplay.create();
    update();
}

void MyGL::slot_setSelectedJoint(QTreeWidgetItem* i) {
    if (selectedJoint != nullptr) {
        // Deselect the joint and recreate it to modify color.
        selectedJoint->selected = false;
        selectedJoint->create();
    }

    // Assign new selected joint and recreate it to modify color.
    selectedJoint = static_cast<Joint*>(i);
    selectedJoint->selected = true;
    selectedJoint->create();
    update();
}

void MyGL::slot_splitEdge() {
    if (!m_heDisplay.isSelected) {
        return;
    }
    splitEdge(m_heDisplay.representedHalfEdge);
    m_heDisplay.create();
    update();
}

void MyGL::slot_triangulateFace() {
    if (!m_faceDisplay.isSelected) {
        return;
    }
    triangulateFace();
    m_mesh.create();
    update();
}

void MyGL::slot_subdivide() {
    CENTROID_MAP cm = createCentroids();
    VPTR_SET vs = createSmoothMidpts(cm);
    smoothOrigVerts(vs, cm);

    // We need to create a copy of the original faces
    // since quadrangulateFace will add new faces.
    std::vector<Face*> face_copy;
    for (auto const &f : m_mesh.faces) {
        face_copy.push_back(f.get());
    }
    for (auto const &f : face_copy) {
        quadrangulateFace(f, cm);
    }

    m_mesh.create();

    if (m_vertDisplay.representedVertex != nullptr) {
        m_vertDisplay.create();
    }
    if (m_heDisplay.representedHalfEdge != nullptr) {
        m_heDisplay.create();
    }
    if (m_faceDisplay.representedFace != nullptr) {
        m_faceDisplay.create();
    }
    update();
}

void MyGL::slot_changeRed(const double &d) {
    if (!m_faceDisplay.isSelected) {
        return;
    }
    m_faceDisplay.representedFace->color.r = d;
    m_mesh.create();
    update();
}

void MyGL::slot_changeGreen(const double &d) {
    if (!m_faceDisplay.isSelected) {
        return;
    }
    m_faceDisplay.representedFace->color.g = d;
    m_mesh.create();
    update();
}

void MyGL::slot_changeBlue(const double &d) {
    if (!m_faceDisplay.isSelected) {
        return;
    }
    m_faceDisplay.representedFace->color.b = d;
    m_mesh.create();
    update();
}

void MyGL::slot_changeX(const double &d) {
    if (!m_vertDisplay.isSelected) {
        return;
    }
    m_vertDisplay.representedVertex->pos.x = d;
    m_vertDisplay.create();
    m_mesh.create();
    update();
}

void MyGL::slot_changeY(const double &d) {
    if (!m_vertDisplay.isSelected) {
        return;
    }
    m_vertDisplay.representedVertex->pos.y = d;
    m_vertDisplay.create();
    m_mesh.create();
    update();
}

void MyGL::slot_changeZ(const double &d) {
    if (!m_vertDisplay.isSelected) {
        return;
    }
    m_vertDisplay.representedVertex->pos.z = d;
    m_vertDisplay.create();
    m_mesh.create();
    update();
}

void MyGL::slot_rotateX() {
    if (selectedJoint != nullptr) {
        glm::mat4 curr_rot = glm::mat4_cast(selectedJoint->rot);
        glm::mat4 rotM = glm::rotate(glm::mat4(1.f), glm::radians(5.0f), glm::vec3(1, 0, 0));

        selectedJoint->rot = glm::quat_cast(rotM * curr_rot);
        traverseCreate(selectedJoint);

        updateUnifMats();
        m_mesh.create();

        update();
    }
}

void MyGL::slot_rotateY() {
    if (selectedJoint != nullptr) {
        glm::mat4 curr_rot = glm::mat4_cast(selectedJoint->rot);
        glm::mat4 rotM = glm::rotate(glm::mat4(1.f), glm::radians(5.0f), glm::vec3(0, 1, 0));

        selectedJoint->rot = glm::quat_cast(curr_rot * rotM);
        traverseCreate(selectedJoint);

        updateUnifMats();
        m_mesh.create();

        update();
    }
}

void MyGL::slot_rotateZ() {
    if (selectedJoint != nullptr) {
        glm::mat4 curr_rot = glm::mat4_cast(selectedJoint->rot);
        glm::mat4 rotM = glm::rotate(glm::mat4(1.f), glm::radians(5.0f), glm::vec3(0, 0, 1));

        selectedJoint->rot = glm::quat_cast(rotM * curr_rot);
        traverseCreate(selectedJoint);

        updateUnifMats();
        m_mesh.create();

        update();
    }
}

void MyGL::slot_skinMesh() {
    if (mesh_loaded && joint_loaded) {
        skinMesh();
    }
}
