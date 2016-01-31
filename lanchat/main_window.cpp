#include "app.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  setWindowIcon(LanChatApp::getMainIcon());
}

MainWindow::~MainWindow()
{
  delete ui;
}
