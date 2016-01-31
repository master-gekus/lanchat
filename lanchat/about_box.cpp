#include "about_box.h"
#include "ui_about_box.h"

AboutBox::AboutBox(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AboutBox)
{
  ui->setupUi(this);
  ui->labelVersion->setText(QApplication::applicationVersion());
}

AboutBox::~AboutBox()
{
  delete ui;
}
