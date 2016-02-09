#include "app.h"
#include "about_box.h"
#include "settings_dialog.h"
#include "user_list_item.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  online_header_(0),
  offline_header_(0)
{
  ui->setupUi(this);

  setWindowIcon(LanChatApp::getMainIcon());

  // Creating pre-defined ites
  online_header_ = new QTreeWidgetItem();
  QFont font(online_header_->font(0));
  font.setBold(true);
  QBrush brush = online_header_->foreground(0);

  online_header_->setBackgroundColor(0, QColor(224, 224, 224));
  online_header_->setTextAlignment(0, Qt::AlignCenter);
  online_header_->setData(0, Qt::SizeHintRole, QSize(100, 21));
  online_header_->setFont(0, font);
  online_header_->setText(0, QStringLiteral("On-line"));
  online_header_->setFlags(online_header_->flags() & (~Qt::ItemIsEnabled));
  online_header_->setForeground(0, brush);

  offline_header_ = new QTreeWidgetItem();
  offline_header_->setBackgroundColor(0, QColor(224, 224, 224));
  offline_header_->setTextAlignment(0, Qt::AlignCenter);
  offline_header_->setData(0, Qt::SizeHintRole, QSize(100, 21));
  offline_header_->setFont(0, font);
  offline_header_->setText(0, QStringLiteral("Off-line"));
  offline_header_->setFlags(online_header_->flags() & (~Qt::ItemIsEnabled));
  offline_header_->setForeground(0, brush);

  ui->listUsers->addTopLevelItem(online_header_);
  ui->listUsers->addTopLevelItem(offline_header_);

  online_header_->setHidden(true);
  offline_header_->setHidden(true);
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
MainWindow::on_actionSettings_triggered()
{
  SettingsDialog(this).exec();
}

void
MainWindow::upsert_user_item(const QUuid& uuid, const QString& name, bool is_online)
{
  bool is_selected = false;
  UserListItem *item = UserListItem::findItem(uuid);
  if (0 != item)
    {
      if ((item->isOnline() == is_online) && (item->name() == name))
        return;

      is_selected = item->isSelected();

      ui->listUsers->takeTopLevelItem(ui->listUsers->indexOfTopLevelItem(item));
      item->setName(name);
      item->setOnline(is_online);
    }
  else
    {
      item = new UserListItem(uuid, name, is_online);
    }

  int first_index = is_online ? 1
                    : ui->listUsers->indexOfTopLevelItem(offline_header_) + 1;
  int last_index = is_online
                   ? ui->listUsers->indexOfTopLevelItem(offline_header_)
                   : ui->listUsers->topLevelItemCount();

  // It is highly unlikely that we will have a lot of items. It is not
  // necessary to use any algorithms like a binary insertions
  for(; first_index < last_index; ++first_index)
    {
      UserListItem *candidate
        = dynamic_cast<UserListItem*>(ui->listUsers->topLevelItem(first_index));

      Q_ASSERT(0 != candidate);

      if (candidate->name() > item->name())
        break;
    }

  ui->listUsers->insertTopLevelItem(first_index, item);

  online_header_->setHidden(
    1 == ui->listUsers->indexOfTopLevelItem(offline_header_)
  );

  offline_header_->setHidden(
    (ui->listUsers->topLevelItemCount() - 1)
     == ui->listUsers->indexOfTopLevelItem(offline_header_)
  );

  if (is_selected)
    {
      item->setSelected(true);
      ui->listUsers->setCurrentItem(item);
      ui->listUsers->scrollToItem(item);
    }
}
