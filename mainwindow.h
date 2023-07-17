#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_choosePathButton_clicked();

public slots:
    void newUser(QString User);
    void newSolution(QString path);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
