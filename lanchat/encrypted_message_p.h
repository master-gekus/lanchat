#ifndef ENCRYPTED_MESSAGE_P_H
#define ENCRYPTED_MESSAGE_P_H

#include <QTimer>
#include <QHostAddress>

#include "encrypted_message.h"

/**
 * @brief The EncryptedMessageManagerPrivate class
 *
 * The main purpose of a class - hide slots from public class and
 * emits signals
 */
class EncryptedMessageManagerPrivate : public QObject
{
  Q_OBJECT

private:
  EncryptedMessageManagerPrivate(EncryptedMessageManager *owner,
                                 LanChatApp *app);
  ~EncryptedMessageManagerPrivate();

private:
  Q_DISABLE_COPY(EncryptedMessageManagerPrivate)

public:
  void emitSendingResult(const EncryptedMessage& msg, bool is_ok,
                         const QString& error_string);
  void emitSendingResult(const EncryptedMessage& msg);
  void emitMessageReceived(const QUuid& sender_uuid, const QByteArray& msg);

private:
  EncryptedMessageManager *owner_;
  QTimer check_expired_timer_;

private slots:
  void check_expired();
  void onEncrypedDatagram(QHostAddress host, QByteArray datagram,
                          int uncompressed_size);

  friend class EncryptedMessageManager;
};



#endif // ENCRYPTED_MESSAGE_P_H

