#include "encrypted_message.h"

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
namespace
{
  EncryptedMessageManager manager;
}

EncryptedMessageManager::EncryptedMessageManager()
{
  Q_ASSERT(this == &manager);

  qRegisterMetaType<EncryptedMessage>("EncryptedMessage");
}

EncryptedMessageManager::~EncryptedMessageManager()
{
  Q_ASSERT(this == &manager);
}

EncryptedMessageManager*
EncryptedMessageManager::instance()
{
  return &manager;
}

EncryptedMessage
EncryptedMessageManager::sendMessage(const QUuid& target,
                                     const QByteArray& data)
{
  Q_UNUSED(target);

  EncryptedMessage msg(data);

  emit
    sendingResult(msg, false, QStringLiteral("Not implemented now!"));

  return msg;
}
