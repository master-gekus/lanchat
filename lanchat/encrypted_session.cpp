#include "encrypted_session.h"

class EncryptedSessionPrivate
{
private:
  EncryptedSessionPrivate(EncryptedSession* owner) :
    owner_(owner)
  {
  }

  ~EncryptedSessionPrivate()
  {
  }

private:
  EncryptedSession* owner_;

  friend class EncryptedSession;
};

EncryptedSession::EncryptedSession() :
  d(new EncryptedSessionPrivate(this))
{

}

EncryptedSession::~EncryptedSession()
{
  delete d;
}
