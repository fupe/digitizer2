#include <QDebug>
#include "InfoDialog.h"
#include "ui_InfoDialog.h"

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::set_num_of_points(int number)
{
    QString num_of_point_String = QString::number(number);
    ui->num_points->setText(num_of_point_String);

}

void InfoDialog::closeEvent(QCloseEvent *event)
{
    qDebug()<<"info::closeEvent " << &event;
    this->deleteLater();



}

void InfoDialog::on_close_clicked()
{
    //this->hide();
    qDebug()<<"Info on_close_clicked" ;
    this->close(); // Vyvolá closeEvent automaticky

}
