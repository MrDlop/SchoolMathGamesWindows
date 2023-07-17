#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "server.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_choosePathButton_clicked()
{
    QDir directory = QFileDialog::getExistingDirectory(nullptr, "Directory Dialog", "");
    ui->choosePathButton->setText(directory.path());
    QStringList tasksFileNames = directory.entryList(
                QStringList() << "task_*.jpg" <<  "task_*.JPG" << "task_*.txt" << "task_*.jpeg",
                QDir::Files);
    foreach(QString filename, tasksFileNames){
        ui->listTaskWidget->addItem(filename);
    }
}

void MainWindow::newUser(QString User)
{
    ui->listCommandWidget->addItem(User);
}

void MainWindow::newSolution(QString path)
{
    ui->listReviewWidget->addItem(path);
}
