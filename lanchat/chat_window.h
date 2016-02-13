#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QMainWindow>
#include <QUuid>

namespace Ui {
  class ChatWindow;
}

class MainWindow;

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

private:
  Ui::ChatWindow *ui;

private:
  QUuid user_uuid_;
};

#endif // CHAT_WINDOW_H
