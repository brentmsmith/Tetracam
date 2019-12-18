#include "fscreen.h"
#include "ui_fscreen.h"
#include "globals.h"

FScreen::FScreen(QWidget *parent) :
    QWidget(parent),
    ui2(new Ui::FScreen)
{
    ui2->setupUi(this);
    setup();
}

FScreen::~FScreen()
{
    this->close();
    delete ui2;
}

void FScreen::setup()
{
    qlabels2 = {ui2->frame1,ui2->frame2,ui2->frame3,ui2->frame4,
               ui2->frame5,ui2->frame6,ui2->frame7,ui2->frame8};
    connect(ui2->actionQuit, SIGNAL(triggered()),this,SLOT(receiveQuit()));
    connect(worker,SIGNAL(sendFscreen()),this,SLOT(receiveFscreen()));
}
void FScreen::receiveFscreen(){
    this->show();
}

void FScreen::receiveQuit()
{
    this->close();
}
