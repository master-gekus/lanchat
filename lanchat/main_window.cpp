#include <QNetworkInterface>

#include "app.h"
#include "subnet_set.h"
#include "about_box.h"

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

void
MainWindow::on_actionAbout_triggered()
{
  AboutBox(this).exec();
}

void
MainWindow::on_actionCheckInterfaces_triggered()
{
  ui->treeWidget->clear();

  for (QNetworkAddressEntry const& entry : SubnetSet::allSubnets())
    {
      QTreeWidgetItem *item = new QTreeWidgetItem();
      item->setText(0, trUtf8("%1 / %2 / %3")
        .arg(entry.ip().toString(), entry.netmask().toString(),
             entry.broadcast().toString()));
      ui->treeWidget->addTopLevelItem(item);
    }
}
