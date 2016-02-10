#include <QMessageBox>

#include "app.h"
#include "about_box.h"
#include "settings_dialog.h"
#include "user_list_item.h"

#include "send_message_dialog.h"

#include "main_window.h"
#include "ui_main_window.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  online_header_(0),
  offline_header_(0),
  check_inactivity_timer_(this)
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

  for (const auto& user_pair : UserListItem::loadItems())
    upsert_user_item(user_pair.first, user_pair.second, false);

  connect(qApp, SIGNAL(userIsOnLine(QUuid,QString,QHostAddress)),
          SLOT(onUserIsOnLine(QUuid,QString,QHostAddress)),
          Qt::QueuedConnection);
  connect(qApp, SIGNAL(userIsOffLine(QUuid)), SLOT(onUserIsOffLine(QUuid)),
          Qt::QueuedConnection);
  connect(&check_inactivity_timer_, SIGNAL(timeout()), SLOT(checkInactivity()),
          Qt::QueuedConnection);
  connect(gEmm, SIGNAL(sendingResult(EncryptedMessage,bool,QString)),
          SLOT(onSendingResult(EncryptedMessage,bool,QString)),
          Qt::QueuedConnection);
  connect(gEmm, SIGNAL(messageReceived(QUuid,QByteArray)),
          SLOT(onMessageReceived(QUuid,QByteArray)), Qt::QueuedConnection);

  check_inactivity_timer_.start(1000);
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
MainWindow::on_actionSendMessage_triggered()
{
  auto items = ui->listUsers->selectedItems();
  UserListItem *item = (1 != items.size()) ? 0
                       : dynamic_cast<UserListItem*>(items.first());
  if (0 == item)
    return;

  SendMessageDialog smd(this);
  if (QDialog::Accepted != smd.exec())
    return;

  gEmm->sendMessage(item->uuid(), smd.messageText());
}

void
MainWindow::onSendingResult(EncryptedMessage msg, bool is_ok,
                            QString error_string)
{
  if (is_ok)
    {
      QMessageBox::information(this, windowTitle(),
        QStringLiteral("Message with id #%1 successfully sent.").arg(msg.id()));
    }
  else
    {
      QMessageBox::critical(this, windowTitle(),
        QStringLiteral("Error sendig message with id #%1: %2").arg(msg.id())
          .arg(error_string));
    }
}

void
MainWindow::onMessageReceived(QUuid sender_uuid, QByteArray msg)
{
  UserListItem *item = UserListItem::findItem(sender_uuid);

  QMessageBox::information(this, QStringLiteral("Message received"),
    QStringLiteral("<B>Sender:</B>&nbsp;%1<BR><B>Message:</B><BR>%2")
      .arg((0 == item) ? sender_uuid.toString() : item->name(),
           QString::fromUtf8(msg)));
}

void
MainWindow::onUserIsOnLine(QUuid uuid, QString name, QHostAddress host)
{
  UserListItem *item = upsert_user_item(uuid, name, true);
  if (0 != item)
    item->setHostAddreess(host);
}

void
MainWindow::onUserIsOffLine(QUuid uuid)
{
  upsert_user_item(uuid, QString(), false);
}

void
MainWindow::checkInactivity()
{
  QList<QUuid> uuids;
  int inactivity_limit = qApp->notifyInternal() * 3 + 1000;
  int last_index = ui->listUsers->indexOfTopLevelItem(offline_header_);
  for (int i = 1; i < last_index; i++)
    {
      UserListItem *item
        = dynamic_cast<UserListItem*>(ui->listUsers->topLevelItem(i));

      Q_ASSERT(0 != item);
      if (item->inactivityMilliseconds() > inactivity_limit)
        uuids.append(item->uuid());
    }

  if (uuids.isEmpty())
    return;

  qDebug("Items became inactive!");

  for (const QUuid& uuid : uuids)
    upsert_user_item(uuid, QString(), false);
}

UserListItem *
MainWindow::upsert_user_item(const QUuid& uuid, const QString& name, bool is_online)
{
  bool is_selected = false;
  UserListItem *item = UserListItem::findItem(uuid);
  if (0 != item)
    {
      item->updateActivity();
      if (name.isEmpty())
        {
          is_online = false;
        }
      else
        {
          if ((item->isOnline() == is_online) && (item->name() == name))
            return item;

          item->setName(name);
        }

      is_selected = item->isSelected();

      ui->listUsers->takeTopLevelItem(ui->listUsers->indexOfTopLevelItem(item));
      item->setOnline(is_online);
    }
  else
    {
      if (name.isEmpty())
        return 0;

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

  return item;
}
