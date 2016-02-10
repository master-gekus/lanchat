#ifndef ENCRYPEDMESSAGE_H
#define ENCRYPEDMESSAGE_H

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QUuid>

class EncryptedMessagePrivate;

/**
 * @brief The EncryptedMessage class
 */
class EncryptedMessage
{
public:
  EncryptedMessage();
  EncryptedMessage(const EncryptedMessage&);
  EncryptedMessage &operator=(const EncryptedMessage&);
  ~EncryptedMessage();

public:
  bool isValid() const;
  int id() const;
  QByteArray data() const;

private:
  EncryptedMessage(const QByteArray& data);

private:
  QExplicitlySharedDataPointer<EncryptedMessagePrivate> d;

  friend class EncryptedMessageManager;
};

Q_DECLARE_METATYPE(EncryptedMessage)

/**
 * @brief The EncryptedMessageManager class
 */
class EncryptedMessageManager : public QObject
{
  Q_OBJECT

public:
  EncryptedMessageManager();
  ~EncryptedMessageManager();

public:
  static EncryptedMessageManager* instance();

public:
  EncryptedMessage sendMessage(const QUuid& target, const QByteArray& data);

signals:
  void sendingResult(EncryptedMessage msg, bool is_ok, QString error_string);

private slots:

};

#define gEmm (EncryptedMessageManager::instance())

#endif // ENCRYPEDMESSAGE_H
