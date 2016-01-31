#include "main_window.h"
#include "app.h"

int main(int argc, char *argv[])
{
  LanChatApp a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
