#ifndef ENCRYPTEDSESSION_H
#define ENCRYPTEDSESSION_H

class EncryptedSessionPrivate;
class EncryptedSession
{
public:
  EncryptedSession();
  ~EncryptedSession();

private:
  EncryptedSessionPrivate *d;
};

#endif // ENCRYPTEDSESSION_H
