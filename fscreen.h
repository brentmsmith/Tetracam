#ifndef FSCREEN_H
#define FSCREEN_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <vector>
#include <concurrent_queue.h>
#include <QTimer>

using namespace std;
using namespace tbb;

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class FScreen; }
QT_END_NAMESPACE

class FScreen : public QWidget
{
    Q_OBJECT

public:
    explicit FScreen(QWidget *parent = 0);
    ~FScreen();
    Ui::FScreen *ui2;
    vector<QLabel*> qlabels2;

private:
    void setup();

//signals:


private slots:
    void receiveFscreen();
    void receiveQuit();
};

#endif // FSCREEN_H
