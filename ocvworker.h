#ifndef OPENCVWORKER_H
#define OPENCVWORKER_H

#include <QObject>
#include <QImage>
#include <string>
#include <vector>
#include <QThread>
#include <concurrent_queue.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
using namespace tbb;

class OpenCvWorker : public QObject
{
    Q_OBJECT

private:
    bool status,toggleStream,binaryThresholdEnable;
    int binaryThreshold,bchan,gchan,rchan,index,ip1,ip2,ip3,ip4;
    double cfrac,dfact;
    vector<double> uxyz,bxyz,gxyz,rxyz,ixyz,zxyz,txyz,bgr;
    vector<vector<double>> bgrvals = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};

    void checkIfDeviceAlreadyOpened(const vector<std::string> devices);
    void process(concurrent_queue<Mat> *q, int i);
    void align(Mat BGR[3]);
    void receiveGrabFrame(int i);
    void mergeStreams();
    vector<double> xyz2bgr(vector<double> xyz);



public:
    explicit OpenCvWorker(QObject *parent = nullptr);
    ~OpenCvWorker();
    vector<VideoCapture*> cameras;
    vector<concurrent_queue<Mat>*> queue;
    vector<concurrent_queue<QImage>*> Qqueue, Qqueue2;
    vector<class thread*> threads;
    string ip = "192.168.122.1";
    vector<string> camstreams = {
        "http://"+ip+":8081/?action=stream",
        "http://"+ip+":8082/?action=stream",
        "http://"+ip+":8083/?action=stream",
        "http://"+ip+":8084/?action=stream",
        "http://"+ip+":8080/?action=stream",
        "http://"+ip+":8085/?action=stream",
        "http://"+ip+":8086/?action=stream"
    };
    vector<vector<double> > newu = {{},{}};
    vector<vector<double> > newb = {{},{}};
    vector<vector<double> > newg = {{},{}};
    vector<vector<double> > newr = {{},{}};
    vector<vector<double> > newi = {{},{}};
    vector<vector<double> > newz = {{},{}};
    vector<vector<double> > newt = {{},{}};
    vector<vector<double> > newsol = {{},{}};
    vector<vector<double>> newu2,newb2,newg2,newr2,newi2,newz2,newt2,newsol2;

signals:
    void sendFrames(vector<concurrent_queue<QImage>*> Qqueue);
    void sendFrames2(vector<concurrent_queue<QImage>*> Qqueue2);
    void sendFscreen();
    void sendIPchange();

public slots:
    void receiveToggleStream();
    void receiveFscreen();
    void receiveIPchange();
    void sendImages();
    void sendImages2();
    void calrgb();

    void receiveEnableBinaryThreshold();
    void receiveBinaryThreshold(int threshold);
    void receiveRchan(int chan);
    void receiveGchan(int chan);
    void receiveBchan(int chan);
    void receiveRedshift(double z);
    void receiveIP(int ip1_, int ip2_, int ip3_, int ip4_);
};

#endif // OPENCVWORKER_H
