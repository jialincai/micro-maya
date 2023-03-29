#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTreeWidgetItem>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();
    void on_actionLoad_OBJ_triggered();
    void on_actionLoad_JSON_triggered();
    void on_actionCamera_Controls_triggered();

    void slot_addVertexToListWidget(QListWidgetItem*);
    void slot_addEdgeToListWidget(QListWidgetItem*);
    void slot_addFaceToListWidget(QListWidgetItem*);

    // Adds the root joint to the Tree Widget
    void slot_addJointToTreeWidget(QTreeWidgetItem*);

    void slot_clearListWidgets();
    void slot_clearTreeWidget();


private:
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
