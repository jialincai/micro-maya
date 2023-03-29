#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    // Add mesh components to List Widget
    connect(ui->mygl,
            // Signal name
            SIGNAL(sig_sendVertex(QListWidgetItem*)),
            // Widget with the slot that receives the signal
            this,
            // Slot name
            SLOT(slot_addVertexToListWidget(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(sig_sendEdge(QListWidgetItem*)), this, SLOT(slot_addEdgeToListWidget(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(sig_sendFace(QListWidgetItem*)), this, SLOT(slot_addFaceToListWidget(QListWidgetItem*)));

    // Add joints to Tree Widget
    connect(ui->mygl, SIGNAL(sig_sendJoint(QTreeWidgetItem*)), this, SLOT(slot_addJointToTreeWidget(QTreeWidgetItem*)));

    // Clears List Widgets when a new OBJ file is loaded.
    connect(ui->mygl, SIGNAL(sig_clearListWidgets()), this, SLOT(slot_clearListWidgets()));

    // Clears Tree Widgets when a new JSON file is loaded.
    connect(ui->mygl, SIGNAL(sig_clearTreeWidget()), this, SLOT(slot_clearTreeWidget()));

    // Select a component
    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedVertex(QListWidgetItem*)));

    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedHalfEdge(QListWidgetItem*)));

    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            ui->mygl, SLOT(slot_setSelectedFace(QListWidgetItem*)));

    // Select a joint
    connect(ui->jointsTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            ui->mygl, SLOT(slot_setSelectedJoint(QTreeWidgetItem*)));

    // Split the currently selected edge
    connect(ui->splitEdgeButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_splitEdge()));

    // Triangulate the currently selected face
    connect(ui->triangulateFaceButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_triangulateFace()));

    // Subdivide the mesh
    connect(ui->subdivideButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_subdivide()));

    // When any of the face color spin boxes are clicked,
    // change the color of the face.
    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_changeRed(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_changeGreen(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_changeBlue(double)));

    // When any of the vertex position spin boxes are clicked,
    // change the color of the face.
    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_changeX(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_changeY(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)),
            ui->mygl, SLOT(slot_changeZ(double)));

    // Rotate a joint along an axis
    connect(ui->rotateXButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_rotateX()));
    connect(ui->rotateYButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_rotateY()));
    connect(ui->rotateZButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_rotateZ()));

    // Skin the mesh
    connect(ui->skinMeshButton, SIGNAL(clicked()),
            ui->mygl, SLOT(slot_skinMesh()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionLoad_OBJ_triggered()
{
    QString OBJ_file = QFileDialog::getOpenFileName(this, tr("Load OBJ"),
                                                    "../obj_files",
                                                    tr("OBJ Files (*.obj)"));
    if (OBJ_file != "") {
        ui->mygl->load_OBJ(OBJ_file);
    }
}

void MainWindow::on_actionLoad_JSON_triggered()
{
    QString JSON_file = QFileDialog::getOpenFileName(this, tr("Load JSON"),
                                                    "../jsons",
                                                    tr("JSON Files (*.json)"));
    if (JSON_file != "") {
        ui->mygl->load_JSON(JSON_file);
    }
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

void MainWindow::slot_addVertexToListWidget(QListWidgetItem  *i) {
    ui->vertsListWidget->addItem(i);
}

void MainWindow::slot_addEdgeToListWidget(QListWidgetItem  *i) {
    ui->halfEdgesListWidget->addItem(i);
}

void MainWindow::slot_addFaceToListWidget(QListWidgetItem  *i) {
    ui->facesListWidget->addItem(i);
}

void MainWindow::slot_addJointToTreeWidget(QTreeWidgetItem* i) {
    ui->jointsTreeWidget->addTopLevelItem(i);
}

void MainWindow::slot_clearListWidgets() {
    ui->mygl->m_vertDisplay = VertexDisplay(ui->mygl);
    ui->mygl->m_heDisplay = HalfEdgeDisplay(ui->mygl);
    ui->mygl->m_faceDisplay = FaceDisplay(ui->mygl);
    ui->vertsListWidget->clear();
    ui->halfEdgesListWidget->clear();
    ui->facesListWidget->clear();
}

void MainWindow::slot_clearTreeWidget() {
    ui->mygl->selectedJoint = nullptr;
    ui->mygl->joint = mkU<Joint>(Joint(ui->mygl));
    ui->jointsTreeWidget->clear();
}
