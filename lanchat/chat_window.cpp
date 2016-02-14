#include <QMap>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QDateTime>

#include "GUiHelpers.h"

#include "app.h"
#include "encrypted_message.h"
#include "main_window.h"
#include "chat_outgoing_message.h"
#include "chat_incoming_message.h"
#include "user_list_item.h"

#include "chat_window.h"
#include "ui_chat_window.h"

#define CHAT_WINDOW_GROUP QStringLiteral("Chat Windows")
#define CHAT_WINDOW_GEOMETRY_KEY QStringLiteral("Geometry")
#define CHAT_WINDOW_STATE_KEY QStringLiteral("State")

namespace
{
  QMap<QUuid,ChatWindow*> chat_windows;
}

ChatWindow::ChatWindow(const QUuid& uuid) :
  QMainWindow(0),
  ui(new Ui::ChatWindow),
  is_on_top_(false),
  icon_blinks_(false),
  user_uuid_(uuid)
{
  Q_ASSERT(!chat_windows.contains(user_uuid_));
  chat_windows.insert(user_uuid_, this);

  ui->setupUi(this);
  ui->splitter->setSizes(QList<int>() << 300 << 20);
  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);

  updateUserState();

  connect(gEmm, SIGNAL(sendingResult(EncryptedMessage,bool,QString)),
          SLOT(onSendingResult(EncryptedMessage,bool,QString)),
          Qt::QueuedConnection);
  connect(qApp, SIGNAL(iconBlinks()), SLOT(icon_blinks()),
          Qt::QueuedConnection);
}

ChatWindow::~ChatWindow()
{
  delete ui;

  Q_ASSERT(chat_windows.contains(user_uuid_));
  chat_windows.remove(user_uuid_);
}

void
ChatWindow::updateUserState()
{
  UserListItem *item = UserListItem::findItem(user_uuid_);
  Q_ASSERT(0 != item);

  setWindowIcon(icon_blinks_ ? qApp->iconMessageBlinkCurrent()
                : item->isOnline() ? qApp->iconUserOnline()
                : qApp->iconUserOffline());

  setWindowTitle(QStringLiteral("Chat with %1%2 :: Lan Chat")
    .arg(item->name(), item->isOnline() ? QString()
                                        : QStringLiteral(" [off-line]")));
}

bool
ChatWindow::hasUnreadMessages()
{
  return !unread_messages_.isEmpty();
}

void
ChatWindow::icon_blinks()
{
  if (!icon_blinks_)
    return;

  setWindowIcon(qApp->iconMessageBlinkCurrent());
}

ChatWindow*
ChatWindow::createWindow(MainWindow* parent, const QUuid& uuid, bool show_window)
{
  ChatWindow *window = 0;

  auto it = chat_windows.constFind(uuid);
  if (chat_windows.constEnd() == it)
    {
      window = new ChatWindow(uuid);

      connect(window, SIGNAL(unreadMessageListChanged()), parent,
              SLOT(onIconBlinks()), Qt::QueuedConnection);
    }
  else
    {
      window = it.value();
    }

  if (show_window)
    {
      if (!window->isVisible())
        {
          window->show();

          QSettings settings;
          settings.beginGroup(CHAT_WINDOW_GROUP);
          settings.beginGroup(window->user_uuid_.toString());

          window->restoreGeometry(settings.value(CHAT_WINDOW_GEOMETRY_KEY).toByteArray());
          GUiHelpers::restoreElementsState(window, settings);
        }

      if (window->isMinimized())
        window->showNormal();

      window->activateWindow();
      window->raise();
    }

  return window;
}

ChatWindow*
ChatWindow::findWindow(const QUuid& uuid)
{
  auto it = chat_windows.constFind(uuid);
  if (chat_windows.constEnd() == it)
    return 0;

  return it.value();
}

void
ChatWindow::destroyAllWindows()
{
  for (ChatWindow *window : chat_windows.values())
    {
      window->close();
      delete window;
    }
}

void
ChatWindow::closeEvent(QCloseEvent* event)
{
  QSettings settings;
  settings.beginGroup(CHAT_WINDOW_GROUP);
  settings.beginGroup(user_uuid_.toString());

  settings.setValue(CHAT_WINDOW_GEOMETRY_KEY, saveGeometry());
  GUiHelpers::saveElementsState(this, settings);

  QMainWindow::closeEvent(event);
}

void
ChatWindow::changeEvent(QEvent *event)
{
  QMainWindow::changeEvent(event);

  if (QEvent::ActivationChange != event->type())
    return;

  bool is_active = (isVisible() && isActiveWindow() &&
                   (this == QApplication::activeWindow()));
  if (is_on_top_ == is_active)
    return;
  is_on_top_ = is_active;

//  UserListItem *item = UserListItem::findItem(user_uuid_);
//  Q_ASSERT(0 != item);
//  qDebug("Window \"%s\" became %s.", item->name().toUtf8().constData(),
//         is_on_top_ ? "ACTIVE" : "inactive");

  if (is_on_top_)
    QMetaObject::invokeMethod(this, "clear_unread_messages",
                              Qt::QueuedConnection);
}

void
ChatWindow::on_btnSend_clicked()
{
  QString msg = ui->editMessage->toPlainText().trimmed();
  if (msg.isEmpty())
    return;
  ui->editMessage->clear();
  ui->editMessage->setFocus();

  QUuid msg_uuid = QUuid::createUuid();
  send_new_message(msg_uuid, msg);

  ChatOutgoingMessage *msg_widget = new ChatOutgoingMessage(msg);
  outgoing_messages_.insert(msg_uuid, msg_widget);

  connect(msg_widget->labelStatus(), SIGNAL(linkActivated(QString)),
          SLOT(onMessageLinkClicked(QString)), Qt::QueuedConnection);

  QTreeWidgetItem *item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  item->setFlags(item->flags() & (~Qt::ItemIsEnabled));
  ui->listHistory->setItemWidget(item, 0, msg_widget);
  ui->listHistory->scrollToItem(item);
}

void
ChatWindow::onSendingResult(EncryptedMessage msg, bool is_ok, QString)
{
  if (!sent_messages_.contains(msg.id()))
    return;

  QUuid msg_uuid = sent_messages_.take(msg.id());

  if (!outgoing_messages_.contains(msg_uuid))
    {
      qDebug("ChatWindow::onSendingResult(): Message widget not found.");
      return;
    }

  ChatOutgoingMessage *msg_widget = outgoing_messages_[msg_uuid];

  if (is_ok)
    {
      msg_widget->labelStatus()->setText(QDateTime::currentDateTime().toString(
        QStringLiteral("'Delivered at 'dd.MM.yy HH:mm")));
    }
  else
    {
      msg_widget->labelStatus()->setText(
        QStringLiteral("Error sending message!&nbsp;&nbsp;<A HREF=\"%1\">Retry</A>")
          .arg(msg_uuid.toString()));
    }
}

void
ChatWindow::processNewMessage(const GJson& json)
{
  QUuid msg_uuid = QUuid::fromRfc4122(json["MessageId"].toByteArray());
  if (msg_uuid.isNull())
    {
      qDebug("ChatWindow::processNewMessage(): Invalid message uuid.");
      return;
    }
  QTreeWidgetItem *item = new QTreeWidgetItem();
  ui->listHistory->addTopLevelItem(item);
  item->setFlags(item->flags() & (~Qt::ItemIsEnabled));
  ui->listHistory->setItemWidget(item, 0, new ChatIncomingMessage(json["Message"].toString()));
  ui->listHistory->scrollToItem(item);

  if (is_on_top_)
    send_message_viewed(msg_uuid);
  else
    unread_messages_.insert(msg_uuid);

  set_icon_blinks();
}

void
ChatWindow::processMessageViewed(const GJson& json)
{
  QUuid msg_uuid = QUuid::fromRfc4122(json["MessageId"].toByteArray());
  if (msg_uuid.isNull())
    {
      qDebug("ChatWindow::processMessageViewed(): Invalid message uuid.");
      return;
    }

  if (!outgoing_messages_.contains(msg_uuid))
    {
      qDebug("ChatWindow::processMessageViewed(): unknown message guid.");
      return;
    }

  outgoing_messages_[msg_uuid]->labelStatus()->setText(
    QDateTime::currentDateTime().toString(
      QStringLiteral("'Viewed at 'dd.MM.yy HH:mm")));
}

void
ChatWindow::onMessageLinkClicked(QString strLink)
{
  QUuid msg_uuid(strLink);
  if (!outgoing_messages_.contains(msg_uuid))
    {
      qDebug("ChatWindow::onMessageLinkClicked(): unknown message guid.");
      return;
    }

  ChatOutgoingMessage *msg_widget = outgoing_messages_[msg_uuid];
  msg_widget->labelStatus()->setText(QStringLiteral("Sending..."));

  send_new_message(msg_uuid, msg_widget->messageText());
}

void
ChatWindow::clear_unread_messages()
{
  for (const QUuid& uuid : unread_messages_)
    send_message_viewed(uuid);
  unread_messages_.clear();
  set_icon_blinks();
}

void
ChatWindow::send_new_message(const QUuid& msg_uuid, const QString& msg)
{
  GJson json;
  json["Action"] = "NewMessage";
  json["MessageId"] = msg_uuid.toRfc4122();
  json["Message"] = msg;
  sent_messages_.insert(gEmm->sendMessage(user_uuid_, json).id(), msg_uuid);
}

void
ChatWindow::send_message_viewed(const QUuid& uuid)
{
  GJson json;
  json["Action"] = "MessageViewed";
  json["MessageId"] = uuid.toRfc4122();
  gEmm->sendMessage(user_uuid_, json);
}

void
ChatWindow::set_icon_blinks()
{
  bool blinks = !unread_messages_.isEmpty();
  if (icon_blinks_ == blinks)
    return;

  icon_blinks_ = blinks;
  updateUserState();

  if (icon_blinks_ && isVisible())
    QApplication::alert(this);

  emit
    unreadMessageListChanged();
}
