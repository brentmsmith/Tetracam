#ifndef CHANGEIP_H
#define CHANGEIP_H

#include <QDialog>

using namespace std;

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class ChangeIP; }
QT_END_NAMESPACE

class ChangeIP : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeIP(QDialog *parent = 0);
    ~ChangeIP();
    Ui::ChangeIP *ui3;
    int ip1,ip2,ip3,ip4;

private:
    void setup();

signals:
    void sendIP(int ip1, int ip2, int ip3, int ip4);

private slots:
    void receiveIPchange();
    void receiveQuit();
    void acceptIP();
};

#endif // CHANGEIP_H
