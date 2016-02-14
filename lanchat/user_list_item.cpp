#include <QSettings>
#include <QDateTime>
#include <QMap>

#include "app.h"
#include "user_list_item.h"

#define KNOWN_USERS_GROUP QStringLiteral("Known Users")
#define USER_NAME QStringLiteral("Name")

class UserListItemPrivate
{
private:
  UserListItemPrivate(UserListItem *owner, const QUuid& uuid,
                      const QString& name, bool is_online) :
    owner_(owner),
    uuid_(uuid),
    name_(name),
    is_online_(is_online),
    last_activity_(QDateTime::currentMSecsSinceEpoch())
  {
    QSettings settings;
    settings.beginGroup(KNOWN_USERS_GROUP);
    settings.beginGroup(uuid_.toString());
    settings.setValue(USER_NAME, name_);

    items.insert(uuid_, owner_);
  }

  ~UserListItemPrivate()
  {
    items.remove(uuid_);
  }

  QIcon icon() const
  {
    return is_online_ ? qApp->iconUserOnline() : qApp->iconUserOffline();
  }

private:
  UserListItem *owner_;
  QUuid uuid_;
  QString name_;
  bool is_online_;
  QHostAddress host_address_;
  qint64 last_activity_;

private:
  static QMap<QUuid, UserListItem*> items;

  friend class UserListItem;
};

QMap<QUuid, UserListItem*> UserListItemPrivate::items;

UserListItem::UserListItem(const QUuid& uuid, const QString& name,
                           bool is_online) :
  d(new UserListItemPrivate(this, uuid, name, is_online))
{
}

UserListItem::~UserListItem()
{
  delete d;
}


QVariant
UserListItem::data(int column, int role) const
{
  if (0 != column)
    return QVariant();

  switch (role)
    {
    case Qt::DisplayRole:
      return d->name_;

    case Qt::DecorationRole:
      return d->icon();

    case Qt::ToolTipRole:
      if (d->is_online_ && (!d->host_address_.isNull()))
        return d->host_address_.toString();
      break;

    case Qt::SizeHintRole:
      return QSize(100, 21);

    default:
      break;
    }

  return QVariant();
}

const QUuid&
UserListItem::uuid() const
{
  return d->uuid_;
}

const QString&
UserListItem::name() const
{
  return d->name_;
}

void
UserListItem::setName(const QString& name)
{
  if (name == d->name_)
    return;

  d->name_ = name;
  updateActivity();
}

bool
UserListItem::isOnline() const
{
  return d->is_online_;
}

void
UserListItem::setOnline(bool is_online)
{
  if (d->is_online_ == is_online)
    return;

  d->is_online_ = is_online;
  updateActivity();
}

QHostAddress UserListItem::hostAddress() const
{
  if (d->is_online_)
    return d->host_address_;

  return QHostAddress();
}

void
UserListItem::setHostAddreess(const QHostAddress& address)
{
  if (d->host_address_ == address)
    return;

  d->host_address_ = address;
  updateActivity();
  emitDataChanged();
}

void UserListItem::updateActivity()
{
  d->last_activity_ = QDateTime::currentMSecsSinceEpoch();
}

int UserListItem::inactivityMilliseconds() const
{
  return (int)(QDateTime::currentMSecsSinceEpoch() - d->last_activity_);
}

UserListItem*
UserListItem::findItem(const QUuid& uuid)
{
  auto it = UserListItemPrivate::items.constFind(uuid);
  return (UserListItemPrivate::items.constEnd() == it) ? 0 : it.value();
}

QList<QPair<QUuid,QString> >
UserListItem::loadItems()
{
  QList<QPair<QUuid,QString> > result;

  QSettings settings;
  settings.beginGroup(KNOWN_USERS_GROUP);
  for (const QString& str_uuid : settings.childGroups())
    {
      QUuid uuid(str_uuid);
      if (uuid.isNull())
        continue;

      settings.beginGroup(str_uuid);
      QString name = settings.value(USER_NAME).toString().trimmed();
      settings.endGroup();

      if (name.isEmpty())
        continue;

      result.append(QPair<QUuid,QString>(uuid, name));
    }

  return result;
}
