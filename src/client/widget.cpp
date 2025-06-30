#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    client = std::make_unique<Client>();
    update();
}

void Widget::paintEvent(QPaintEvent *event) {
    ServerData data = client->run();
    if (data.co2 < 0.f) return;
    ui->lcdNumber->display(data.sleep);
    ui->lcdNumber_2->display(data.co2);
    if (data.turnInfoCount > 0) {
        std::string a;
        ui->lcdNumber_3->display(data.turnInfos[0].distance);
        switch (data.turnInfos[0].mod) {
            case modifier::Left :
                ui->textBrowser_3->setPlainText("Left");
                break;
            case modifier::Right :
                ui->textBrowser_3->setPlainText("Right");
                break;
            case modifier::Straight :
                ui->textBrowser_3->setPlainText("Straight");
                break;
            case modifier::UTurn :
                ui->textBrowser_3->setPlainText("UTurn");
                break;
        }
    }
    QThread::msleep(100);
    update();
}

Widget::~Widget()
{
    delete ui;
}

