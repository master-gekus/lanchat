#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QMainWindow>

namespace Ui {
  class ChatWindow;
}

class ChatWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit ChatWindow(QWidget *parent = 0);
  ~ChatWindow();

private:
  Ui::ChatWindow *ui;
};

#endif // CHAT_WINDOW_H
