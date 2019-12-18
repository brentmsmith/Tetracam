#include "changeIP.h"
#include "ui_changeIP.h"
#include "globals.h"

ChangeIP::ChangeIP(QDialog *parent) :
    QDialog(parent),
    ui3(new Ui::ChangeIP)
{
    ui3->setupUi(this);
    setup();
}

ChangeIP::~ChangeIP()
{
    this->close();
    delete ui3;
}

void ChangeIP::setup()
{
    connect(worker,SIGNAL(sendIPchange()),this,SLOT(receiveIPchange()));
    connect(ui3->buttonBox,SIGNAL(accepted()),this,SLOT(acceptIP()));
    connect(ui3->buttonBox,SIGNAL(rejected()),this,SLOT(receiveQuit()));
    connect(this,SIGNAL(sendIP(int, int, int, int)),worker,SLOT(receiveIP(int, int, int, int)));
}
void ChangeIP::receiveIPchange(){
    this->show();
}
void ChangeIP::acceptIP() {
    ip1 = ui3->ip1->text().toInt();
    ip2 = ui3->ip2->text().toInt();
    ip3 = ui3->ip3->text().toInt();
    ip4 = ui3->ip4->text().toInt();
    emit sendIP(ip1,ip2,ip3,ip4);
    this->close();
}
void ChangeIP::receiveQuit()
{
    this->close();
}
