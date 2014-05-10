#include <QApplication>

#include "datacanvas.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionQuit, &QAction::triggered, qApp, &QApplication::quit);
    auto view = new DataCanvas(this);
    setCentralWidget(view);
}

MainWindow::~MainWindow()
{
    delete ui;
}
