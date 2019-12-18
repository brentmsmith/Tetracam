#ifndef QCVWIDGET_H
#define QCVWIDGET_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QVector>
#include <string>
#include <vector>
#include <concurrent_queue.h>
#include <fscreen.h>
#include <changeIP.h>

using namespace std;
using namespace tbb;

Q_DECLARE_METATYPE(vector<string>);
Q_DECLARE_METATYPE(vector<concurrent_queue<QImage>*>);
Q_DECLARE_METATYPE(int);
QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    vector<QLabel*> qlabels;
    QThread *qthread,*qthread2,*qthread3;
    QTimer *workerTrigger,*workerTrigger2,*workerTrigger3;
    QImage tmpQImg;
    vector<QString> chanlbls={
        "400 nm","500 nm","600 nm","700 nm",
        "800 nm", "900 nm", "Thermal"
    };
    FScreen f;
    ChangeIP IPdialog;

    void setup();

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


signals:
    void sendSetup(vector<std::string> device);
    void sendToggleStream();
    void sendRedshift(double z);
    void sendFscreen();
    void sendIPchange();

private slots:
    void receiveFrames(vector<concurrent_queue<QImage>*> Qqueue);
    void receiveToggleStream();
    void receiveRedshift(int zshift);
    void receiveQuit();
    void receiveFscreen();
    void receiveIPchange();
    void receiveExitFscreen();
};

#endif // QCVWIDGET_H
