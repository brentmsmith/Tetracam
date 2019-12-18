#include "ocvworker.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <xyzdata.h>
#include <boost/math/interpolators/cubic_b_spline.hpp>

OpenCvWorker::OpenCvWorker(QObject *parent) :
    QObject(parent),
    status(false),
    toggleStream(false),
    binaryThresholdEnable(false),
    bchan(1),gchan(3),rchan(5),cfrac(0.0),dfact(1.0),
    binaryThreshold(127)
{
    concurrent_queue<Mat> *q;
    concurrent_queue<QImage> *Qq;
    class thread *t;
    Mat temp;
    QImage qtmp;
    for (int i=0; i<7; i++){
        q = new concurrent_queue<Mat>;
        Qq = new concurrent_queue<QImage>;
        queue.push_back(q);
        Qqueue.push_back(Qq);
        Qqueue2.push_back(Qq);
        t = new class thread(&OpenCvWorker::receiveGrabFrame,this,i);
        threads.push_back(t);
    }
    q = new concurrent_queue<Mat>;
    Qq = new concurrent_queue<QImage>;
    queue.push_back(q);
    Qqueue.push_back(Qq);
    Qqueue2.push_back(Qq);
    t = new class thread(&OpenCvWorker::mergeStreams,this);
    threads.push_back(t);
    bgr={0,0,0};uxyz={0,0,0};bxyz={0,0,0};gxyz={0,0,0};rxyz={0,0,0};ixyz={0,0,0};zxyz={0,0,0};txyz={0,0,0};
    uspl.set_points(ufilt[0],ufilt[1]);bspl.set_points(bfilt[0],bfilt[1]);gspl.set_points(gfilt[0],gfilt[1]);rspl.set_points(rfilt[0],rfilt[1]);ispl.set_points(ifilt[0],ifilt[1]);zspl.set_points(zfilt[0],zfilt[1]);tspl.set_points(tfilt[0],tfilt[1]);solspl.set_points(solar[0],solar[1]);
    boost::math::cubic_b_spline<double> tspline(tfilt[1].begin(),tfilt[1].end(),tfilt[0][0],(tfilt[0][tfilt[0].size()-1]-tfilt[0][0])/tfilt[0].size());
    for (int i;i<cmfs[0].size();i++) {
        newu[0].push_back(cmfs[0][i]);
        newu[1].push_back(uspl(cmfs[0][i]));
        newb[0].push_back(cmfs[0][i]);
        newb[1].push_back(bspl(cmfs[0][i]));
        newg[0].push_back(cmfs[0][i]);
        newg[1].push_back(gspl(cmfs[0][i]));
        newr[0].push_back(cmfs[0][i]);
        newr[1].push_back(rspl(cmfs[0][i]));
        newi[0].push_back(cmfs[0][i]);
        newi[1].push_back(ispl(cmfs[0][i]));
        newz[0].push_back(cmfs[0][i]);
        newz[1].push_back(zspl(cmfs[0][i]));
        newt[0].push_back(cmfs[0][i]);
        newt[1].push_back(tspline(cmfs[0][i]));
        newsol[0].push_back(cmfs[0][i]);
        newsol[1].push_back(solspl(cmfs[0][i]));
     }
}

OpenCvWorker::~OpenCvWorker()
{
    for (int i=0; i<7; i++){
    if(cameras[i]->isOpened()) cameras[i]->release();
    delete cameras[i];
    }
}


void OpenCvWorker::process(concurrent_queue<Mat> *q, int i)
{
    Mat proc;
    if(q->try_pop(proc)){
        if(binaryThresholdEnable) {
            threshold(proc, proc, binaryThreshold, 255, cv::THRESH_BINARY);
        }
    }
}

void OpenCvWorker::receiveGrabFrame(int i)
{
    cout<<"Threaad "<<i<<" started"<<endl;
    VideoCapture *capture = new VideoCapture(camstreams[i]);
    cameras.push_back(capture);
    Mat frame,tmp,frames_[3],col[3],outframe;
    QImage output;
    Size size;
    while(true){
        if(!toggleStream) continue;
        (*capture) >> frame;
        if (i==6){
            resize(frame,tmp,Size(0,0),1.33,1.33,INTER_CUBIC);
            frame = tmp(Rect(105,84,640,512));
        }
        cv::split(frame,frames_);
        frames_[0].copyTo(frame);
        queue[i]->push(frame);
        frame.convertTo(col[2],CV_8UC1,bgrvals[i][2]/255,bgrvals[i][2]);
        frame.convertTo(col[1],CV_8UC1,bgrvals[i][1]/255,bgrvals[i][1]);
        frame.convertTo(col[0],CV_8UC1,bgrvals[i][0]/255,bgrvals[i][0]);
        merge(col,3,tmp);
        cvtColor(tmp,outframe,COLOR_BGR2RGB);
        //process(queue[i],i);
        output = QImage((uchar *)outframe.data, outframe.cols, outframe.rows, QImage::Format_RGB888);
        Qqueue[i]->push(output);
        Qqueue2[i]->push(output);
        frame.release();
    }
}

void OpenCvWorker::mergeStreams(){
    cout<<"Thread 7 started"<<endl;
    vector<int> v1 = {bchan,rchan};
    vector<int> v2 = {0,2};
    Mat merged,edges1,refedges,tmp1,tmp2,frame,warpmat,chans[7],temp3[3];
    Point2d offset,offsetprev(-50,10),deltaoffset;
    QImage qmerge;
    while(true){
        v1 = {bchan,rchan};
        if (queue[6]->try_pop(chans[6])){
            for (int k=0;k<6;k++) {
                queue[k]->try_pop(chans[k]);
            }
            if(!toggleStream) continue;
            blur(chans[gchan],tmp1,Size(3,3));
            Canny(tmp1,refedges,50,200,3);
            refedges.convertTo(tmp2,CV_32FC1);
            chans[gchan].copyTo(temp3[1]);
            for (auto zi : zip(v1,v2)){
                unzip(zi, i,j);
                blur(chans[i],tmp1,Size(3,3));
                Canny(tmp1,edges1,50,200,3);
                edges1.convertTo(tmp1,CV_32FC1);
                offset = phaseCorrelate(tmp1,tmp2);
                if (abs(offset.x)<90 && abs(offset.y)<90) {
                    offsetprev = offset;
                    warpmat = (Mat_<double>(2,3) << 1, 0, offset.x, 0, 1, offset.y);
                }
                else warpmat = (Mat_<double>(2,3) << 1, 0, offsetprev.x, 0, 1, offsetprev.y);
                warpAffine(chans[i],temp3[j],warpmat,chans[1].size());
            }
            merge(temp3,3,frame);
            cvtColor(frame,merged,COLOR_BGR2RGB);
            //process(queue[7],7);
            qmerge = QImage((uchar *)merged.data,merged.cols,merged.rows, QImage::Format_RGB888);
            Qqueue[7]->push(qmerge);
            Qqueue2[7]->push(qmerge);
        }
    }
}
vector<double> OpenCvWorker::xyz2bgr(vector<double> xyz){
    bgr[2]=xyz[0]*3.2404542+xyz[1]*-1.5371385+xyz[2]*-0.4985314;
    bgr[1]=xyz[0]*-0.9692660+xyz[1]*1.8760108+xyz[2]*0.0415560;
    bgr[0]=xyz[0]*0.0556434+xyz[1]*-0.2040259+xyz[2]*1.0572252;
    for(int i=0;i<3;i++){
        if(bgr[i]<0){
            bgr[i]=0;
        }
        if(abs(bgr[i])<=0.0031308){
            bgr[i]=12.92*bgr[i];
        }
        else{
            bgr[i]=1.055*pow(bgr[i],0.41666)-0.055;
        }
        if(bgr[i]>255){
            bgr[i]=255;
        }
    }
    return bgr;
}
void OpenCvWorker::calrgb() {
    dfact = sqrt((1+cfrac)/(1-cfrac));
    uxyz=bxyz=gxyz=rxyz=ixyz=zxyz=txyz={0,0,0};
    newu2=newu;newb2=newb;newg2=newg;newr2=newr;newi2=newi;newz2=newz;newt2=newt;newsol2=newsol;
    transform(newu2[0].begin(), newu2[0].end(),newu2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newb2[0].begin(), newb2[0].end(),newb2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newg2[0].begin(), newg2[0].end(),newg2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newr2[0].begin(), newr2[0].end(),newr2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newi2[0].begin(), newi2[0].end(),newi2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newz2[0].begin(), newz2[0].end(),newz2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newt2[0].begin(), newt2[0].end(),newt2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    transform(newsol2[0].begin(), newsol2[0].end(),newsol2[0].begin(),bind(multiplies<double>(),placeholders::_1,dfact));
    boost::math::cubic_b_spline<double> uspline(newu[1].begin(),newu[1].end(),newu2[0][0],(newu2[0][newu2[0].size()-1]-newu2[0][0])/newu2[0].size());
    boost::math::cubic_b_spline<double> bspline(newb[1].begin(),newb[1].end(),newb2[0][0],(newb2[0][newb2[0].size()-1]-newb2[0][0])/newb2[0].size());
    boost::math::cubic_b_spline<double> gspline(newg[1].begin(),newg[1].end(),newg2[0][0],(newg2[0][newg2[0].size()-1]-newg2[0][0])/newg2[0].size());
    boost::math::cubic_b_spline<double> rspline(newr[1].begin(),newr[1].end(),newr2[0][0],(newr2[0][newr2[0].size()-1]-newr2[0][0])/newr2[0].size());
    boost::math::cubic_b_spline<double> ispline(newi[1].begin(),newi[1].end(),newi2[0][0],(newi2[0][newi2[0].size()-1]-newi2[0][0])/newi2[0].size());
    boost::math::cubic_b_spline<double> zspline(newz[1].begin(),newz[1].end(),newz2[0][0],(newz2[0][newz2[0].size()-1]-newz2[0][0])/newz2[0].size());
    boost::math::cubic_b_spline<double> tspline(newt[1].begin(),newt[1].end(),newt2[0][0],(newt2[0][newt2[0].size()-1]-newt2[0][0])/newt2[0].size());
    boost::math::cubic_b_spline<double> solspline(newsol[1].begin(),newsol[1].end(),newsol2[0][0],(newsol2[0][newsol2[0].size()-1]-newsol2[0][0])/newsol2[0].size());
    for (int j=0;j<cmfs[0].size();j++) {
        newu2[1][j]=uspline(cmfs[0][j]);
        if(newu2[1][j]<0) newu2[1][j]=0;
        newb2[1][j]=bspline(cmfs[0][j]);
        if(newb2[1][j]<0) newb2[1][j]=0;
        newg2[1][j]=gspline(cmfs[0][j]);
        if(newg2[1][j]<0) newg2[1][j]=0;
        newr2[1][j]=rspline(cmfs[0][j]);
        if(newr2[1][j]<0) newr2[1][j]=0;
        newi2[1][j]=ispline(cmfs[0][j]);
        if(newi2[1][j]<0) newi2[1][j]=0;
        newz2[1][j]=zspline(cmfs[0][j]);
        if(newz2[1][j]<0) newz2[1][j]=0;
        newt2[1][j]=tspline(cmfs[0][j]);
        if(newt2[1][j]<0) newt2[1][j]=0;
        newsol2[1][j]=solspline(cmfs[0][j]);
        if(newsol2[1][j]<0) newsol2[1][j]=0;
        for (int i=0;i<3;i++) {
            uxyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newu2[1][j]*5;
            bxyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newb2[1][j];
            gxyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newg2[1][j];
            rxyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newr2[1][j];
            ixyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newi2[1][j];
            zxyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newz2[1][j]*100;
            txyz[i]+=cmfs[i+1][j]*newsol2[1][j]*newt2[1][j]*.5;
        }
    }
    bgrvals[0] = xyz2bgr(uxyz);
    bgrvals[1] = xyz2bgr(bxyz);
    bgrvals[2] = xyz2bgr(gxyz);
    bgrvals[3] = xyz2bgr(rxyz);
    bgrvals[4] = xyz2bgr(ixyz);
    bgrvals[5] = xyz2bgr(zxyz);
    bgrvals[6] = xyz2bgr(txyz);
    transform(bgrvals[6].begin(), bgrvals[6].end(),bgrvals[6].begin(),bind(multiplies<double>(),placeholders::_1,1.3));
    //cout<<newt2[0][0]<<" "<<txyz[0]<<" "<<txyz[1]<<" "<<txyz[2]<<" "<<bgrvals[6][0]<<" "<<bgrvals[6][1]<<" "<<bgrvals[6][2]<<endl;
}

void OpenCvWorker::receiveToggleStream() {
    toggleStream = !toggleStream;
}
void OpenCvWorker::receiveEnableBinaryThreshold() {
    binaryThresholdEnable = !binaryThresholdEnable;
}
void OpenCvWorker::receiveBinaryThreshold(int Threshold) {
    binaryThreshold = Threshold;
}
void OpenCvWorker::receiveBchan(int chan) {
    bchan = chan;
}
void OpenCvWorker::receiveGchan(int chan) {
    gchan = chan;
}
void OpenCvWorker::receiveRchan(int chan) {
    rchan = chan;
}
void OpenCvWorker::sendImages() {
    emit sendFrames(Qqueue);
}
void OpenCvWorker::sendImages2() {
    emit sendFrames(Qqueue2);
}
void OpenCvWorker::receiveRedshift(double z) {
    cfrac = z;
}
void OpenCvWorker::receiveFscreen() {
    emit sendFscreen();
}
void OpenCvWorker::receiveIPchange() {
    emit sendIPchange();
}
void OpenCvWorker::receiveIP(int ip1_,int ip2_,int ip3_,int ip4_) {
    ip = to_string(ip1_)+"."+to_string(ip2_)+"."+to_string(ip3_)+"."+to_string(ip4_);
    vector<string> camstreams = {
        "http://"+ip+":8081/?action=stream",
        "http://"+ip+":8082/?action=stream",
        "http://"+ip+":8083/?action=stream",
        "http://"+ip+":8084/?action=stream",
        "http://"+ip+":8080/?action=stream",
        "http://"+ip+":8085/?action=stream",
        "http://"+ip+":8086/?action=stream"
    };
}
