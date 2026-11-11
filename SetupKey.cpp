#include "SetupKey.h"
#include "ui_SetupKey.h"

SetupKey::SetupKey(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetupKey)
{
    ui->setupUi(this);
}

SetupKey::~SetupKey()
{
    delete ui;
}

void SetupKey::on_Ok_clicked()
{
    Key=ui->KeyEdit->toPlainText();
    close();
}


void SetupKey::on_Cancel_clicked()
{
    close();
}

