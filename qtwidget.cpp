#include "qtwidget.h"
#include "ui_qtwidget.h"
#include "ui_fscreen.h"
#include <QTimer>
#include <QString>
#include <math.h>
#include <QDesktopServices>
#include <QUrl>
#include <QMenuBar>
#include <globals.h>
#include <QScreen>
#include <QRect>

OpenCvWorker *worker = new OpenCvWorker();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //QDesktopServices::openUrl(QUrl("http://192.168.122.1/"));
    //QDesktopServices::openUrl(QUrl("http://192.168.122.1/english/main9.php"));
    ui->setupUi(this);
    ui->frame1->setScaledContents(true);
    ui->frame2->setScaledContents(true);
    ui->frame3->setScaledContents(true);
    ui->frame4->setScaledContents(true);
    ui->frame5->setScaledContents(true);
    ui->frame6->setScaledContents(true);
    ui->frame7->setScaledContents(true);
    ui->frame8->setScaledContents(true);
    for(int i=0;i<7;i++){
    ui->bchan->addItem(chanlbls[i]);
    ui->gchan->addItem(chanlbls[i]);
    ui->rchan->addItem(chanlbls[i]);
    }
    ui->bchan->setCurrentIndex(1);
    ui->gchan->setCurrentIndex(3);
    ui->rchan->setCurrentIndex(5);
    setup();
}

MainWindow::~MainWindow()
{
    qthread->quit();
    while(!qthread->isFinished());
    delete qthread;
    delete ui;
}

void MainWindow::setup()
{

    qthread = new QThread;
    workerTrigger = new QTimer;
    qthread2 = new QThread;
    workerTrigger2 = new QTimer;
    qthread3 = new QThread;
    workerTrigger3 = new QTimer;

    workerTrigger->setInterval(1);
    workerTrigger2->setInterval(1);
    workerTrigger3->setInterval(1);
    qRegisterMetaType<vector<std::string>>();
    qRegisterMetaType<vector<concurrent_queue<QImage>*>>();

    qlabels = {ui->frame1,ui->frame2,ui->frame3,ui->frame4,
               ui->frame5,ui->frame6,ui->frame7,ui->frame8};

    connect(ui->actionQuit, SIGNAL(triggered()),this,SLOT(receiveQuit()));
    connect(this, SIGNAL(sendToggleStream()), worker, SLOT(receiveToggleStream()));
    connect(ui->pushButtonPlay, SIGNAL(clicked(bool)), this, SLOT(receiveToggleStream()));
    connect(ui->checkBoxEnableBinaryThreshold, SIGNAL(toggled(bool)), worker, SLOT(receiveEnableBinaryThreshold()));
    connect(ui->spinBoxBinaryThreshold, SIGNAL(valueChanged(int)), worker, SLOT(receiveBinaryThreshold(int)));
    connect(ui->rchan,SIGNAL(currentIndexChanged(int)),worker,SLOT(receiveRchan(int)));
    connect(ui->gchan,SIGNAL(currentIndexChanged(int)),worker,SLOT(receiveGchan(int)));
    connect(ui->bchan,SIGNAL(currentIndexChanged(int)),worker,SLOT(receiveBchan(int)));
    connect(ui->zslider,SIGNAL(valueChanged(int)),this,SLOT(receiveRedshift(int)));
    connect(this, SIGNAL(sendRedshift(double)),worker,SLOT(receiveRedshift(double)));
    connect(worker, SIGNAL(sendFrames(vector<concurrent_queue<QImage>*>)), this, SLOT(receiveFrames(vector<concurrent_queue<QImage>*>)));
    connect(workerTrigger2, SIGNAL(timeout()), worker, SLOT(sendImages()));
    connect(workerTrigger3, SIGNAL(timeout()), worker, SLOT(calrgb()));
    connect(qthread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(qthread, SIGNAL(finished()), workerTrigger, SLOT(deleteLater()));
    connect(ui->actionFull_Screen, SIGNAL(triggered()),this,SLOT(receiveFscreen()));
    connect(ui->actionChange_Camera_IP, SIGNAL(triggered()),this,SLOT(receiveIPchange()));
    connect(ui->actionExit_Full_Screen,SIGNAL(triggered()),this,SLOT(receiveExitFscreen()));
    connect(this,SIGNAL(sendFscreen()),worker, SLOT(receiveFscreen()));
    connect(this,SIGNAL(sendIPchange()),worker, SLOT(receiveIPchange()));


    workerTrigger->start();
    worker->moveToThread(qthread);
    workerTrigger2->start();
    workerTrigger3->start();
//    worker->deleteLater();
    workerTrigger->moveToThread(qthread);
    workerTrigger2->moveToThread(qthread2);
    workerTrigger3->moveToThread(qthread3);
//    workerTrigger->deleteLater();
    qthread->start();
    qthread2->start();
    qthread3->start();
    QScreen *screen = QGuiApplication::screens()[1];
    QRect  screenSize = screen->geometry();
    f.setGeometry(screenSize);
    int x=0,y=0,dx=screenSize.width()/4,dy=screenSize.height()/2;
    for(int i=0; i<8; i++) {
        f.qlabels2[i]->setGeometry(x,y,dx,dy);
        x+=dx;
        if(x==screenSize.x()*3/4) {
            x=0;
            y=dy;
        }
    }
    f.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

}

void MainWindow::receiveFrames(vector<concurrent_queue<QImage>*> Qqueue)
{
    for (int i=0;i<8;i++) {
        if (Qqueue[i]->try_pop(tmpQImg)){
        qlabels[i]->setPixmap(QPixmap::fromImage(tmpQImg));
        f.qlabels2[i]->setPixmap(QPixmap::fromImage(tmpQImg));
        }
    }
}
void MainWindow::receiveToggleStream()
{
    if(!ui->pushButtonPlay->text().compare(">")) ui->pushButtonPlay->setText("||");
    else ui->pushButtonPlay->setText(">");

    emit sendToggleStream();
}
void MainWindow::receiveRedshift(int zshift)
{
    double z = zshift/100.0;
    QString disp = "v="+QString::number(z)+" c\nv="+QString::number(int(z*670616629))+" mph";
    ui->vtext->setText(disp);
    emit sendRedshift(z);
}
void MainWindow::receiveQuit()
{
    this->close();
}
void MainWindow::receiveFscreen()
{
    emit sendFscreen();
}
void MainWindow::receiveIPchange()
{
    emit sendIPchange();
    delete worker;
    worker = new OpenCvWorker();
}
void MainWindow::receiveExitFscreen()
{
    f.close();
}
