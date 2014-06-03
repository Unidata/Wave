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

    // Create a widget container for the view and add to a layout. The container
    // takes ownership of the view.
    auto view = new DataCanvas;
    auto container = createWindowContainer(view, this);
//    auto layout = new QHBoxLayout(this);
//    layout->setMargin(20);
//    layout->addWidget(container);

    setCentralWidget(container);
}

MainWindow::~MainWindow()
{
    delete ui;
}
