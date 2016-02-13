#include <QDateTime>
#include <QtEndian>

#include "app.h"
#include "user_list_item.h"
#include "message_composer.h"
#include "simple_diffie_hellman.h"

#include "encrypted_message_p.h"

#define SESSION_CONFIRM_WAIT_TIME 5000

typedef SimpleDiffieHellman<quint32> DiffieHellman;


// ////////////////////////////////////////////////////////////////////////////
namespace
{
  EncryptedMessageManager *manager = 0;
  EncryptedMessageManagerPrivate *manager_private = 0;

  struct EncryptionSession;
  QMap<QUuid,EncryptionSession*> sessions_by_id;
  QMap<QUuid,EncryptionSession*> sessions_by_target;

  struct EncryptionSession
  {
    enum State
    {
      WaitForConfirm,
      WaitForAck,     // After this state we can accept messages
      Active,
    };

    EncryptionSession(const QUuid& target, const QHostAddress& host) :
      id(QUuid::createUuid()),
      target_uuid(target),
      target_host(host),
      state(WaitForConfirm)
    {
      sessions_by_id.insert(id, this);
      sessions_by_target.insert(target_uuid, this);
    }

    EncryptionSession(const QUuid& session_id, const QUuid& target,
                      const QHostAddress& host) :
      id(session_id),
      target_uuid(target),
      target_host(host),
      state(WaitForAck)
    {
      sessions_by_id.insert(id, this);
      sessions_by_target.insert(target_uuid, this);
    }

    ~EncryptionSession()
    {
      sessions_by_id.remove(id);
      sessions_by_target.remove(target_uuid);
    }

    QUuid id, target_uuid;
    QHostAddress target_host;
    State state;
    quint64 last_activity_time = QDateTime::currentMSecsSinceEpoch();
    QList<EncryptedMessage> messages_to_send;
    QMap<int,EncryptedMessage> messages_to_confirm;

    DiffieHellman::KeyType  p, q, a, A, b, B, Key;

  private:
    EncryptionSession() = delete;
    Q_DISABLE_COPY(EncryptionSession)
  };

  enum EncrypedMessageType
  {
    SESSION_REQUEST = 0x01,
    SESSION_CONFIRM = 0x02,
    SESSION_ACK = 0x03,
    SESSION_NOT_FOUND = 0x04,
    SESSION_DECLINED = 0x05,

    ENCRYPED_DATA = 0x11,
    ENCRYPED_DATA_ACK = 0x12,
  };

  /* All encryped payload has following header:
   * +0 MessageType 1 bype
   * +1 SessionUuid 16 bytes
   * +17 TargetUserUuid 16 bytes
   *
   * futher payload depends of MessageType
   */
  #pragma pack(push, 1)
  struct enc_msg_header
  {
    quint8 type;
    quint8 session_uuid[16];
    quint8 sended_uuid[16];
    quint8 target_uuid[16];
  };
  #pragma pack(pop)
  static_assert(49 == sizeof(enc_msg_header), "Invalid structure packing!");

  #define fromQUuid(d,s) memmove(d,s.toRfc4122().constData(), sizeof(d))
  #define toQUuid(s) QUuid::fromRfc4122(QByteArray((const char*)s, sizeof(s)))

  quint8* precompose_data(QByteArray &data, EncryptionSession *session,
                          EncrypedMessageType message_type,
                          size_t addition_size)
  {
    data.resize((int)(sizeof(enc_msg_header) + addition_size));
    enc_msg_header *h = (enc_msg_header*)data.data();
    h->type = message_type;
    fromQUuid(h->session_uuid, session->id);
    fromQUuid(h->sended_uuid, qApp->userUuid());
    fromQUuid(h->target_uuid, session->target_uuid);
    return (quint8*)(h + 1);
  }

  void send_simple_message(EncrypedMessageType message_type,
                           const QUuid& session_uuid, const QUuid& target_uuid,
                           const QHostAddress& host)
  {
    QByteArray data;
    data.resize(sizeof(enc_msg_header));
    enc_msg_header *h = (enc_msg_header*)data.data();
    h->type = message_type;
    fromQUuid(h->session_uuid, session_uuid);
    fromQUuid(h->sended_uuid, qApp->userUuid());
    fromQUuid(h->target_uuid, target_uuid);

    qApp->sendDatagram(host, MessageComposer::composeEncrypted(data, 0));
  }

  void send_simple_session_message(EncryptionSession* s,
                                   EncrypedMessageType message_type)
  {
    QByteArray data;
    precompose_data(data, s, message_type, 0);
    qApp->sendDatagram(s->target_host,
                       MessageComposer::composeEncrypted(data, 0));
  }

  /* SESSION_REQUEST additional payload
   * + 0 Requestor uuid
   * + 16 p
   * + 16 + KeySize q
   * + 16 + KeySize*2 A = q^a mod p
   */
  #pragma pack(push, 1)
  struct ses_req_data
  {
    quint8 p[DiffieHellman::KeySize];
    quint8 q[DiffieHellman::KeySize];
    quint8 A[DiffieHellman::KeySize];
  };
  #pragma pack(pop)

  void send_session_request(EncryptionSession* s)
  {
    DiffieHellman::prepareRequest(s->p, s->q, s->a, s->A);
    QByteArray data;
    ses_req_data* d = (ses_req_data*)precompose_data(data, s,
                                                     SESSION_REQUEST,
                                                     sizeof(ses_req_data));
    memmove(d->p, DiffieHellman::raw_data(s->p), sizeof(d->p));
    memmove(d->q, DiffieHellman::raw_data(s->q), sizeof(d->q));
    memmove(d->A, DiffieHellman::raw_data(s->A), sizeof(d->A));

    qApp->sendDatagram(s->target_host,
                       MessageComposer::composeEncrypted(data, 0));
  }

  /* SESSION_CONFIRM additional payload
   * + 0 B
   */
  #pragma pack(push, 1)
  struct ses_resp_data
  {
    quint8 B[DiffieHellman::KeySize];
  };
  #pragma pack(pop)

  void send_session_confirm(EncryptionSession* s)
  {
    DiffieHellman::prepareResponse(s->p, s->q, s->A, s->b, s->B, s->Key);
    QByteArray data;
    ses_resp_data* d = (ses_resp_data*)precompose_data(data, s,
                                                       SESSION_CONFIRM,
                                                       sizeof(ses_resp_data));
    memmove(d->B, DiffieHellman::raw_data(s->B), sizeof(d->B));
    qApp->sendDatagram(s->target_host,
                       MessageComposer::composeEncrypted(data, 0));
  }

  /* ENCRYPED_DATA additional payload
   * + 0 4 bytes CRC32 of data
   * + 4 4 bytes Message id
   */
  #pragma pack(push, 1)
  #ifdef Q_CC_MSVC
    #pragma warning(disable: 4200) // nonstandard extension used : zero-sized array in struct/union
  #endif
  struct enc_data
  {
    quint32 crc32;
    quint32 id;
    quint8 data[];
  };
  #ifdef Q_CC_MSVC
    #pragma warning(default: 4200)
  #endif
  #pragma pack(pop)

  void enc_dec_buf(void *buf, size_t cb_buf, const void* key)
  {
    quint8* b = (quint8*)buf;
    const quint8* k = (const quint8*)key;
    for (size_t i = 0; i < cb_buf; i++)
      *(b++) ^= k[i % DiffieHellman::KeySize];
  }

  void send_encrypted_message(EncryptionSession* s, const EncryptedMessage& msg)
  {
    int uncompressed_size = 0;
    QByteArray to_encrypt(msg.data());
    QByteArray compressed = qCompress(msg.data(), 9);
    if ((compressed.size() - 4) < msg.data().size())
      {
        uncompressed_size = msg.data().size();
        to_encrypt = compressed.mid(4);
      }

    QByteArray data;
    enc_data* d = (enc_data*)precompose_data(data, s,
                                                     ENCRYPED_DATA,
                                                     sizeof(enc_data)
                                                     + to_encrypt.size());
    d->crc32 = MessageComposer::crc32(msg.data());
    d->id = msg.id();
    Q_ASSERT(0 < d->id);
    memmove(d->data, to_encrypt.constData(), to_encrypt.size());
    enc_dec_buf(d->data, to_encrypt.size(), DiffieHellman::raw_data(s->Key));
    qApp->sendDatagram(s->target_host,
                       MessageComposer::composeEncrypted(data,
                                                         uncompressed_size));
    s->messages_to_confirm.insert(msg.id(), msg);
  }

  quint32 decrypt_data(QByteArray& result, EncryptionSession* s,
                    const enc_data *enc_data, int data_size,
                    int uncompressed_size)
  {
    if (0 < uncompressed_size)
      {
        QByteArray compressed;
        compressed.resize(data_size + sizeof(quint32));
        quint32* d = (quint32*)compressed.data();
        *(d++) = qToBigEndian(uncompressed_size);
        memmove(d, enc_data->data, data_size);
        enc_dec_buf(d, data_size, DiffieHellman::raw_data(s->Key));
        result = qUncompress(compressed);
      }
    else
      {
        result.resize(data_size);
        memmove(result.data(), enc_data->data, data_size);
        enc_dec_buf(result.data(), data_size, DiffieHellman::raw_data(s->Key));
      }

    if (MessageComposer::crc32(result) != enc_data->crc32)
      {
        qDebug("Invalid CRC32 inside encrypted data.");
        return 0;
      }

    return enc_data->id;
  }

  /* ENCRYPED_DATA_ACK additional payload
   * + 0 4 bytes Message Id
   */
  #pragma pack(push, 1)
  struct enc_data_ack
  {
    quint32 id;
  };
  #pragma pack(pop)

  void send_enc_data_ack(EncryptionSession* s, quint32 msg_id)
  {
    QByteArray data;
    enc_data_ack* d
      = (enc_data_ack*)precompose_data(data, s, ENCRYPED_DATA_ACK,
                                           sizeof(enc_data_ack));
    d->id = msg_id;
    qApp->sendDatagram(s->target_host,
                       MessageComposer::composeEncrypted(data, 0));
  }

}

// ////////////////////////////////////////////////////////////////////////////
class EncryptedMessagePrivate : public QSharedData
{
private:
  EncryptedMessagePrivate(const QByteArray& data) :
    id_(++last_used_id_),
    data_(data)
  {
  }

  ~EncryptedMessagePrivate()
  {
  }

private:
  Q_DISABLE_COPY(EncryptedMessagePrivate)

private:
  int id_;
  QByteArray data_;

private:
  static int last_used_id_;

  friend class EncryptedMessage;
  friend class QExplicitlySharedDataPointer<EncryptedMessagePrivate>;
};

int EncryptedMessagePrivate::last_used_id_ = 0;

// ////////////////////////////////////////////////////////////////////////////
EncryptedMessage::EncryptedMessage() :
  d(0)
{
}

EncryptedMessage::EncryptedMessage(const EncryptedMessage &other) :
  d(other.d)
{
}

EncryptedMessage::EncryptedMessage(const QByteArray& data) :
  d(new EncryptedMessagePrivate(data))
{
}

EncryptedMessage&
EncryptedMessage::operator=(const EncryptedMessage &other)
{
  if (this != &other)
    d.operator=(other.d);
  return *this;
}

EncryptedMessage::~EncryptedMessage()
{
}

bool
EncryptedMessage::isValid() const
{
  return (0 != d.data());
}

int
EncryptedMessage::id() const
{
  auto p = d.data();
  if (0 == p)
    return (-1);
  return p->id_;
}

QByteArray
EncryptedMessage::data() const
{
  auto p = d.data();
  if (0 == p)
    return QByteArray();
  return p->data_;
}

// ////////////////////////////////////////////////////////////////////////////
EncryptedMessageManagerPrivate::EncryptedMessageManagerPrivate(EncryptedMessageManager *owner,
                                                               LanChatApp *app) :
  owner_(owner)
{
  connect(app, SIGNAL(encryptedDatagram(QHostAddress,QByteArray,int)),
          SLOT(onEncrypedDatagram(QHostAddress,QByteArray,int)),
          Qt::QueuedConnection);

  connect(&check_expired_timer_, SIGNAL(timeout()), SLOT(check_expired()),
          Qt::QueuedConnection);
  check_expired_timer_.start(1000);
}

EncryptedMessageManagerPrivate::~EncryptedMessageManagerPrivate()
{
}

void
EncryptedMessageManagerPrivate::emitSendingResult(const EncryptedMessage& msg,
                                                  bool is_ok,
                                                  const QString& error_string)
{
  emit
    owner_->sendingResult(msg, is_ok, error_string);
}

void
EncryptedMessageManagerPrivate::emitSendingResult(const EncryptedMessage& msg)
{
  emit
    owner_->sendingResult(msg, true, QString());
}

void
EncryptedMessageManagerPrivate::emitMessageReceived(const QUuid& sender_uuid,
                                                    const QByteArray& msg)
{
  emit
    owner_->messageReceived(sender_uuid, msg);
}

// ////////////////////////////////////////////////////////////////////////////
EncryptedMessageManager::EncryptedMessageManager(LanChatApp* app)
{
  Q_ASSERT(0 == manager);
  manager = this;

  qRegisterMetaType<QHostAddress>("QHostAddress");
  qRegisterMetaType<EncryptedMessage>("EncryptedMessage");

  manager_private = new EncryptedMessageManagerPrivate(this, app);
}

EncryptedMessageManager::~EncryptedMessageManager()
{
  Q_ASSERT(this == manager);
  manager = 0;
  delete manager_private;
  manager_private = 0;
}

EncryptedMessageManager*
EncryptedMessageManager::instance()
{
  return manager;
}

void
EncryptedMessageManagerPrivate::check_expired()
{
  quint64 cur_time = QDateTime::currentMSecsSinceEpoch();
  QList<EncryptionSession*> to_delete;
  for (EncryptionSession *s : sessions_by_id)
    {
      if (s->state == EncryptionSession::Active)
        continue;

      if ((cur_time - s->last_activity_time) < SESSION_CONFIRM_WAIT_TIME)
        continue;

      for (EncryptedMessage const& msg : s->messages_to_send)
        {
          emitSendingResult(msg, false,
                            QStringLiteral("Session initiation timeout."));
        }
      to_delete.append(s);
    }
  for (EncryptionSession *s : to_delete)
    delete s;
}

EncryptedMessage
EncryptedMessageManager::sendMessage(const QUuid& target,
                                     const QByteArray& data)
{
  EncryptedMessage msg(data);
  EncryptionSession *session = 0;

  auto it = sessions_by_target.constFind(target);
  if (sessions_by_target.constEnd() == it)
    {
      UserListItem *item = UserListItem::findItem(target);
      if ((0 == item) || item->hostAddress().isNull())
        {
          emit
            sendingResult(msg, false,
                          QStringLiteral("Can not determine host address of recepient."));
          return msg;
        }
      session = new EncryptionSession(target, item->hostAddress());
      send_session_request(session);
    }
  else
    {
      session = it.value();
    }

  Q_ASSERT(0 != session);

  switch(session->state)
    {
    case EncryptionSession::Active:
      send_encrypted_message(session, msg);
      break;

    default:
      session->messages_to_send.append(msg);
    }

  return msg;
}

namespace
{
  void on_SESSION_REQUEST(int msg_size, EncryptionSession *s,
                          const ses_req_data *d,
                          const QUuid& session_uuid,
                          const QUuid& requester_uuid,
                          const QHostAddress& host)
  {
    if (msg_size != (sizeof(enc_msg_header) + sizeof(ses_req_data)))
      {
        qDebug("Invalid size of SESSION_REQUEST message!");
        return;
      }

    if (0 != s)
      {
        qDebug("Attempt to create existing session!");
        send_simple_session_message(s, SESSION_DECLINED);
        return;
      }

    s = new EncryptionSession(session_uuid, requester_uuid, host);
    DiffieHellman::from_raw(s->p, d->p);
    DiffieHellman::from_raw(s->q, d->q);
    DiffieHellman::from_raw(s->A, d->A);
    s->state = EncryptionSession::WaitForAck;
    send_session_confirm(s);
  }

  void on_SESSION_CONFIRM(int msg_size, EncryptionSession *s,
                          const ses_resp_data *d)
  {
    if (msg_size != (sizeof(enc_msg_header) + sizeof(ses_resp_data)))
      {
        qDebug("Invalid size of SESSION_CONFIRM message!");
        return;
      }

    DiffieHellman::from_raw(s->B, d->B);
    DiffieHellman::calculateKey(s->p, s->a, s->B, s->Key);
    s->state = EncryptionSession::Active;
    send_simple_session_message(s, SESSION_ACK);

    while (!s->messages_to_send.isEmpty())
      send_encrypted_message(s, s->messages_to_send.takeFirst());
  }

  void on_SESSION_ACK(int msg_size, EncryptionSession *s)
  {
    if (msg_size != sizeof(enc_msg_header))
      {
        qDebug("Invalid size of SESSION_CONFIRM message!");
        return;
      }

    s->state = EncryptionSession::Active;

    while (!s->messages_to_send.isEmpty())
      send_encrypted_message(s, s->messages_to_send.takeFirst());
  }

  void on_SESSION_NOT_FOUND(int msg_size, EncryptionSession *s)
  {
    if (msg_size != sizeof(enc_msg_header))
      {
        qDebug("Invalid size of SESSION_CONFIRM message!");
        return;
      }

    // Let's restart session!
    QList<int> ids = s->messages_to_confirm.keys();
    qSort(ids);
    for (int i = (ids.size() - 1); i >= 0; i--)
      s->messages_to_send.prepend(s->messages_to_confirm.take(ids[i]));
    s->state = EncryptionSession::WaitForConfirm;

    send_session_request(s);
  }

  void on_ENCRYPED_DATA(int msg_size, EncryptionSession *s,
                        const enc_data *d,
                        int uncompressed_size)
  {
    if (msg_size < (int)(sizeof(enc_msg_header) + sizeof(enc_data)))
      {
        qDebug("Invalid size of ENCRYPED_DATA message!");
        return;
      }

    QByteArray result;
    quint32 msg_id =  decrypt_data(result, s, d,
                                   msg_size - (sizeof(enc_msg_header)
                                               + sizeof(enc_data)),
                                   uncompressed_size);
    if (0 == msg_id)
      {
        // Bad decrypted
        return;
      }
    else
      {
        // Good decrypted
        send_enc_data_ack(s, msg_id);
        manager_private->emitMessageReceived(s->target_uuid, result);
      }
  }

  void on_ENCRYPED_DATA_ACK(int msg_size, EncryptionSession *s,
                            const enc_data_ack *d)
  {
    if (msg_size < (int)(sizeof(enc_msg_header) + sizeof(enc_data_ack)))
      {
        qDebug("Invalid size of ENCRYPED_DATA_ACK message!");
        return;
      }

    if (s->messages_to_confirm.contains(d->id))
      {
        EncryptedMessage msg = s->messages_to_confirm.take(d->id);
        manager_private->emitSendingResult(msg);
      }
    else
      {
        qDebug("Confirmation of unexisting message!");
      }
  }
}

void
EncryptedMessageManagerPrivate::onEncrypedDatagram(QHostAddress host,
                                                   QByteArray datagram,
                                                   int uncompressed_size)
{
  if (datagram.size() < (int)sizeof(enc_msg_header))
    {
      qDebug("Invalid encryped datagram.");
      return;
    }

  const enc_msg_header *h = (const enc_msg_header*)datagram.constData();
  QUuid session_uuid = toQUuid(h->session_uuid);
  QUuid target_uuid = toQUuid(h->target_uuid);

  if (target_uuid != qApp->userUuid())
    {
      qDebug("Encrypted datagram has invalid target.");
      return;
    }

  EncryptionSession *s = 0;
  auto it = sessions_by_id.constFind(session_uuid);
  if (sessions_by_id.constEnd() != it)
    {
      s = it.value();
      s->target_host = host;
      s->last_activity_time = QDateTime::currentMSecsSinceEpoch();
    }
  else if (SESSION_REQUEST != h->type)
    {
      if (SESSION_NOT_FOUND != h->type)
        {
          qDebug("Access to unexisting session. sending SESSION_NOT_FOUND.");
          send_simple_message(SESSION_NOT_FOUND, session_uuid,
                              toQUuid(h->sended_uuid), host);
        }
      else
        {
          qDebug("A vicious circle of SESSION_NOT_FOUND messages was broken!");
        }
      return;
    }

  switch(h->type)
    {
    case SESSION_REQUEST:
      on_SESSION_REQUEST(datagram.size(), s, (const ses_req_data*)(h + 1),
                         session_uuid, toQUuid(h->sended_uuid), host);
      break;

    case SESSION_CONFIRM:
      on_SESSION_CONFIRM(datagram.size(), s, (const ses_resp_data*)(h + 1));
      break;

    case SESSION_ACK:
      on_SESSION_ACK(datagram.size(), s);
      break;

    case SESSION_NOT_FOUND:
      on_SESSION_NOT_FOUND(datagram.size(), s);
      break;

    case ENCRYPED_DATA:
      on_ENCRYPED_DATA(datagram.size(), s, (const enc_data*)(h + 1),
                       uncompressed_size);
      break;

    case ENCRYPED_DATA_ACK:
      on_ENCRYPED_DATA_ACK(datagram.size(), s, (const enc_data_ack*)(h + 1));
      break;

    default:
      qDebug("Unknown or unsupported encrypted message type: 0x%02X", h->type);
      return;
    }
}
