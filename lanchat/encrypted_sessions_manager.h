#ifndef ENCRYPTEDSESSIONSMANAGER_H
#define ENCRYPTEDSESSIONSMANAGER_H

#include <QObject>
#include <QExplicitlySharedDataPointer>

class EncryptedMessagePrivate;
class EncryptedMessage
{
public:


private:
  QExplicitlySharedDataPointer<EncryptedMessagePrivate> d;
};


class EncryptedSessionsManager : public QObject
{
  Q_OBJECT

public:
  typedef qintptr MessageID;

public:
  explicit EncryptedSessionsManager(QObject *parent = 0);
  ~EncryptedSessionsManager();

public:
  static EncryptedSessionsManager* instance();

public:
  MessageID sendEncrypted(const QUuid& receiver, const QByteArray& data);

signals:
  void sendingResult(EncryptedSessionsManager::MessageID message_id,
                     bool is_ok, QString error_string);

private slots:
};

Q_DECLARE_METATYPE(EncryptedSessionsManager::MessageID)

#define gEsm (EncryptedSessionsManager::instance())

#endif // ENCRYPTEDSESSIONSMANAGER_H
