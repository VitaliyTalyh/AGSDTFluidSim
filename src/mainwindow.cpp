#include "mainwindow.h"
#include <QFileDialog>
#include <QPushButton>
#include <QDesktopServices>
#include "FluidPropDockWidget.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    QGroupBox *gb = new QGroupBox(this);
    setCentralWidget(gb);
    m_gridLayout = new QGridLayout(gb);
    gb->setLayout(m_gridLayout);

    QGLFormat format;
    format.setVersion(4,1);
    format.setProfile(QGLFormat::CoreProfile);

    //do this so everything isnt so bunched up
    resize(1000,600);

    //add our openGL context to our scene
    m_openGLWidget = new OpenGLWidget(format,this);
    m_openGLWidget->hide();
    m_gridLayout->addWidget(m_openGLWidget,0,0,7,1);

    //Group box for our general UI buttons
    QGroupBox *docGrb = new QGroupBox("General:",this);
    m_gridLayout->addWidget(docGrb,8,0,1,1);
    QGridLayout *docLayout = new QGridLayout(docGrb);
    docGrb->setLayout(docLayout);

    //button to add fluid simulations
    QPushButton *addFluidSimBtn = new QPushButton("Add Fluid Simulation",docGrb);
    connect(addFluidSimBtn,SIGNAL(clicked()),this,SLOT(addFluidSim()));
    docLayout->addWidget(addFluidSimBtn,0,0,1,1);

    //open Documation button
    QPushButton *openDocBtn = new QPushButton("Open Documentation",docGrb);
    connect(openDocBtn,SIGNAL(pressed()),this,SLOT(openDoc()));
    docLayout->addWidget(openDocBtn,1,0,1,1);

}



void MainWindow::openDoc(){
    QDesktopServices::openUrl(QUrl(QDir::currentPath() + "/doc/html/index.html"));
}

void MainWindow::addFluidSim(){
    m_openGLWidget->show();
    this->addDockWidget(Qt::RightDockWidgetArea, new FluidPropDockWidget(m_openGLWidget,this));
}

MainWindow::~MainWindow(){
    delete m_openGLWidget;

}
