#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QMainWindow>
#include <QUuid>

namespace Ui {
  class ChatWindow;
}

class MainWindow;
class GJson;

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

public:
  void processJson(const GJson& json);

protected:
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE Q_DECL_FINAL;

private slots:
  void on_btnSend_clicked();

private:
  Ui::ChatWindow *ui;

private:
  QUuid user_uuid_;
};

#endif // CHAT_WINDOW_H
