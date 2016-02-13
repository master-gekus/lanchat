#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QMainWindow>
#include <QUuid>
#include <QMap>

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
  static void destroyAllWindows();

protected:
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE Q_DECL_FINAL;

private slots:
  void on_btnSend_clicked();

private slots:
  void onSendingResult(EncryptedMessage msg, bool is_ok, QString);
  void onMessageLinkClicked(QString strLink);

private slots:
  void processNewMessage(const GJson& json);

private:
  Ui::ChatWindow *ui;

private:
  QUuid user_uuid_;
  QMap<QUuid,ChatOutgoingMessage*> outgoing_messages_;
  QMap<int,QUuid> sent_messages_;
};

#endif // CHAT_WINDOW_H
