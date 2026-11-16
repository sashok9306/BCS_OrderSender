#include "ClassCodeDialog.h"
#include "ui_ClassCodeDialog.h"

ClassCodeDialog::ClassCodeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClassCodeDialog)
{
    ui->setupUi(this);
    ClassCode="SPBFUT";
}

ClassCodeDialog::~ClassCodeDialog()
{
    delete ui;
}

void ClassCodeDialog::on_Ok_clicked()
{
    if(ui->ClassSelector->currentIndex())
        ClassCode="SPBOPT";
    close();
}

