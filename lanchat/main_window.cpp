#include <QNetworkInterface>

#include "app.h"
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

  for (QNetworkInterface const& nic : QNetworkInterface::allInterfaces())
    {
      QStringList flags_str;
      if (nic.flags() & QNetworkInterface::IsUp)
        flags_str.append(QStringLiteral("IsUp"));
      if (nic.flags() & QNetworkInterface::IsRunning)
        flags_str.append(QStringLiteral("IsRunning"));
      if (nic.flags() & QNetworkInterface::CanBroadcast)
        flags_str.append(QStringLiteral("CanBroadcast"));
      if (nic.flags() & QNetworkInterface::IsLoopBack)
        flags_str.append(QStringLiteral("IsLoopBack"));
      if (nic.flags() & QNetworkInterface::IsPointToPoint)
        flags_str.append(QStringLiteral("IsPointToPoint"));
      if (nic.flags() & QNetworkInterface::CanMulticast)
        flags_str.append(QStringLiteral("CanMulticast"));

      QTreeWidgetItem *nic_item = new QTreeWidgetItem();
      nic_item->setText(0, trUtf8("%1 (%2): %3").arg(nic.humanReadableName(),
                                                     nic.name(),
                                                     flags_str.join(',')));
      ui->treeWidget->addTopLevelItem(nic_item);

      for (QNetworkAddressEntry const& addr : nic.addressEntries())
        {
          QTreeWidgetItem *addr_item = new QTreeWidgetItem(nic_item);
          addr_item->setText(0, trUtf8("%1/%2/%3").arg(addr.ip().toString(),
                                                       addr.netmask().toString(),
                                                       addr.broadcast().toString()));
        }

    }
}
