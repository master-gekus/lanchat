#include <QDateTime>

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
    SESSION_NOT_FOUND = 0x03,
    SESSION_DECLINED = 0x04,

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
    quint8 session_id[16];
    quint8 target_id[16];
  };
  #pragma pack(pop)
  static_assert(33 == sizeof(enc_msg_header), "Invalid structure packing!");

  quint8* precompose_data(QByteArray &data, EncryptionSession *session,
                          EncrypedMessageType message_type,
                          size_t addition_size)
  {
    data.resize((int)(sizeof(enc_msg_header) + addition_size));
    enc_msg_header *h = (enc_msg_header*)data.data();
    h->type = message_type;
    memmove(h->session_id, session->id.toRfc4122().constData(),
            sizeof(h->session_id));
    memmove(h->target_id, session->target_uuid.toRfc4122().constData(),
            sizeof(h->target_id));

    return (quint8*)(h + 1);
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
    quint8 requester_uuid[16];
    quint8 p[DiffieHellman::KeySize];
    quint8 q[DiffieHellman::KeySize];
    quint8 A[DiffieHellman::KeySize];
  };
  #pragma pack(pop)

  void initiate_session(EncryptionSession* s)
  {
    DiffieHellman::prepareRequest(s->p, s->q, s->a, s->A);
    QByteArray data;
    ses_req_data* d = (ses_req_data*)precompose_data(data, s,
                                                     SESSION_REQUEST,
                                                     sizeof(ses_req_data));
    memmove(d->requester_uuid, qApp->userUuid().toRfc4122().constData(),
            sizeof(d->requester_uuid));

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

  void initiate_responce(EncryptionSession* s)
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
  struct ses_enc_data
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
    ses_enc_data* d = (ses_enc_data*)precompose_data(data, s,
                                                     ENCRYPED_DATA,
                                                     sizeof(ses_enc_data)
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
                    const ses_enc_data *enc_data, int data_size,
                    int uncompressed_size)
  {
    if (0 < uncompressed_size)
      {
        QByteArray compressed;
        compressed.resize(data_size + sizeof(quint32));
        quint32* d = (quint32*)compressed.data();
        *(d++) = uncompressed_size;
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
  Q_UNUSED(target);

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
      initiate_session(session);
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
      emit
        sendingResult(msg, true, QString());
      break;

    default:
      session->messages_to_send.append(msg);
    }

  return msg;
}

namespace
{
  bool on_SESSION_REQUEST(int msg_size, EncryptionSession *s,
                               const ses_req_data *d,
                               const QUuid& session_id,
                               const QHostAddress& host)
  {
    if (msg_size != (sizeof(enc_msg_header) + sizeof(ses_req_data)))
      {
        qDebug("Invalid size of SESSION_REQUEST message!");
        return false;
      }

    if (0 != s)
      {
        qDebug("Attempt to create existing session!");
        send_simple_session_message(s, SESSION_DECLINED);
        return false;
      }

    // TODO: Need to check existing session to same user!


    QUuid requester_uuid
      = QUuid::fromRfc4122(QByteArray((const char*)d->requester_uuid,
                                      sizeof(d->requester_uuid)));

    s = new EncryptionSession(session_id, requester_uuid, host);
    DiffieHellman::from_raw(s->p, d->p);
    DiffieHellman::from_raw(s->q, d->q);
    DiffieHellman::from_raw(s->A, d->A);
    initiate_responce(s);

    return true;
  }

  bool on_SESSION_CONFIRM(int msg_size, EncryptionSession *s,
                               const ses_resp_data *d)
  {
    if (msg_size != (sizeof(enc_msg_header) + sizeof(ses_resp_data)))
      {
        qDebug("Invalid size of SESSION_CONFIRM message!");
        return false;
      }

    DiffieHellman::from_raw(s->B, d->B);
    DiffieHellman::calculateKey(s->p, s->a, s->B, s->Key);
    s->state = EncryptionSession::WaitForAck;

    return true;
  }

  bool on_ENCRYPED_DATA(int msg_size, EncryptionSession *s,
                             const ses_enc_data *d,
                             int uncompressed_size)
  {
    if (msg_size < (int)(sizeof(enc_msg_header) + sizeof(ses_enc_data)))
      {
        qDebug("Invalid size of ENCRYPED_DATA message!");
        return false;
      }

    QByteArray result;
    quint32 msg_id =  decrypt_data(result, s, d,
                                   msg_size - (sizeof(enc_msg_header)
                                               + sizeof(ses_enc_data)),
                                   uncompressed_size);
    if (0 == msg_id)
      {
        // Bad decrypted
        return false;
      }
    else
      {
        // Good decrypted
      }
    return true;
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
  QUuid session_id = QUuid::fromRfc4122(QByteArray((const char*)h->session_id,
                                                   sizeof(h->session_id)));
  QUuid target_id = QUuid::fromRfc4122(QByteArray((const char*)h->target_id,
                                                  sizeof(h->target_id)));

  if (target_id != qApp->userUuid())
    {
      qDebug("Encrypted datagram has invalid target.");
      return;
    }

  EncryptionSession *s = 0;
  auto it = sessions_by_id.constFind(session_id);
  if (sessions_by_id.constEnd() != it)
    {
      s = it.value();
      s->target_host = host;
    }
  else if (SESSION_REQUEST != h->type)
    {
      qDebug("Access to unexisting session!");
      // TODO: Need to send notification!
      return;
    }

  switch(h->type)
    {
    case SESSION_REQUEST:
      on_SESSION_REQUEST(datagram.size(), s, (const ses_req_data*)(h + 1),
                              session_id, host);
      break;

    case SESSION_CONFIRM:
      if (on_SESSION_CONFIRM(datagram.size(), s,
                                  (const ses_resp_data*)(h + 1)))
        {
          for (const EncryptedMessage& msg : s->messages_to_send)
            {
              send_encrypted_message(s, msg);
              manager_private->emitSendingResult(msg);
            }
          s->messages_to_send.clear();
        }
      break;

    case ENCRYPED_DATA:
      on_ENCRYPED_DATA(datagram.size(), s, (const ses_enc_data*)(h + 1),
                            uncompressed_size);
      break;

    default:
      qDebug("Unknown or unsupported encrypted message type: 0x%02X", h->type);
      return;
    }
}
