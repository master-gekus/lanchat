#include "encrypted_sessions_manager.h"

namespace
{
  EncryptedSessionsManager manager;
}

EncryptedSessionsManager::EncryptedSessionsManager(QObject *parent) :
  QObject(parent)
{
  Q_ASSERT(this == &manager);
  qRegisterMetaType<EncryptedSessionsManager::MessageID>("EncryptedSessionsManager::MessageID");
}

EncryptedSessionsManager::~EncryptedSessionsManager()
{
  Q_ASSERT(this == &manager);
}

EncryptedSessionsManager* EncryptedSessionsManager::instance()
{
  return &manager;
}

EncryptedSessionsManager::MessageID
EncryptedSessionsManager::sendEncrypted(const QUuid& receiver,
                                        const QByteArray& data)
{
  Q_UNUSED(data);
  Q_UNUSED(receiver);

  emit
    sendingResult(0, false, QStringLiteral("Not implemented now!"));

  return 0;
}
