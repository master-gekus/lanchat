#include <QSet>
#include <time.h>

#include "main_window.h"
#include "app.h"

#include "simple_diffie_hellman.h"

typedef quint32 key_type;
typedef SimpleDiffieHellman<key_type> DiffieHellman;
#define NUMBER_OF_DH_TESTS  10000

int main(int argc, char *argv[])
{
  qsrand(time(0));

  LanChatApp app(argc, argv);

  // Test for sqrt_floor;
  for (key_type i = 101; i < 10001; i++)
    {
      key_type s_i = sqrt_floor(i);
      double s_d = sqrt((double) i);
      key_type s_f = (key_type) s_d;
      if (s_i == s_f)
        continue;
      qDebug("%7u: %7u, %10g, %7u, %s", i, s_i, s_d, s_f,
            (s_i == s_f) ? "Ok" : "FAILED!");
    }

  // Test for SimpleDiffieHellman
  QSet<key_type> keys;
  for (int i = 0; i < NUMBER_OF_DH_TESTS; i++)
    {
      key_type p, q, a, A, b, B, K1, K2;
      DiffieHellman::prepareRequest(p, q, a, A);
      DiffieHellman::prepareResponse(p, q, A, b, B, K1);
      DiffieHellman::calculateKey(p, a, B, K2);
      if (K1 != K2)
        {
          qDebug("DH test FAILED. K1 = 0x%08X, K2 = 0x%08X.", K1, K2);
          continue;
        }
      if (keys.contains(K1))
        {
          qDebug("DH DUPLICATE KEY reached!");
        }
      else
        {
          keys.insert(K1);
        }
    }

  MainWindow w;
  w.show();

  return app.exec();
}
