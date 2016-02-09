#include <QSettings>

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
    is_online_(is_online)
  {
    QSettings settings;
    settings.beginGroup(KNOWN_USERS_GROUP);
    settings.beginGroup(uuid_.toString());
    settings.setValue(USER_NAME, name_);
  }

  ~UserListItemPrivate()
  {
  }

  QIcon icon() const
  {
    static QIcon online;
    static QIcon offline;

    if (online.isNull())
      online.addFile(QStringLiteral(":/res/icons16/user-online.png"));

    if (offline.isNull())
      offline.addFile(QStringLiteral(":/res/icons16/user-offline.png"));

    return is_online_ ? online : offline;
  }

private:
  UserListItem *owner_;
  QUuid uuid_;
  QString name_;
  bool is_online_;

  friend class UserListItem;
};


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

    case Qt::SizeHintRole:
      return QSize(100, 21);

    default:
      return QVariant();
    }
}
