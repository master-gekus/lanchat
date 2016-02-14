#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QMainWindow>
#include <QUuid>
#include <QMap>
#include <QSet>

#include "encrypted_message.h"

namespace Ui {
  class ChatWindow;
}

class MainWindow;
class GJson;
class ChatOutgoingMessage;

class ChatWindow : public QMainWindow
{
  Q_OBJECT

private:
  explicit ChatWindow(const QUuid& uuid);
  ~ChatWindow();

public:
  static ChatWindow* createWindow(MainWindow* parent, const QUuid& uuid,
                                  bool show_window);
  static ChatWindow* findWindow(const QUuid& uuid);
  static void destroyAllWindows();

public:
  void updateUserState();
  bool hasUnreadMessages();

protected:
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE Q_DECL_FINAL;
  void changeEvent(QEvent *event) Q_DECL_OVERRIDE Q_DECL_FINAL;

private slots:
  void on_btnSend_clicked();
  void clear_unread_messages();
  void icon_blinks();

private slots:
  void onSendingResult(EncryptedMessage msg, bool is_ok, QString);
  void onMessageLinkClicked(QString strLink);

private slots:
  void processNewMessage(const GJson& json);
  void processMessageViewed(const GJson& json);

signals:
  void unreadMessageListChanged();

// Helpers
private:
  void send_new_message(const QUuid& msg_uuid, const QString& msg);
  void send_message_viewed(const QUuid& uuid);
  void set_icon_blinks();

private:
  Ui::ChatWindow *ui;

private:
  bool is_on_top_;
  bool icon_blinks_;
  QUuid user_uuid_;
  QMap<QUuid,ChatOutgoingMessage*> outgoing_messages_;
  QMap<int,QUuid> sent_messages_;
  QSet<QUuid> unread_messages_;
};

#endif // CHAT_WINDOW_H
