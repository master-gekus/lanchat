#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QTreeWidgetItem>
#include <QUuid>

class UserListItemPrivate;
class UserListItem : public QTreeWidgetItem
{
public:
  UserListItem(const QUuid& uuid, const QString& name, bool is_online);
  ~UserListItem();

public:
  QVariant data(int column, int role) const Q_DECL_OVERRIDE Q_DECL_FINAL;

private:
  UserListItemPrivate *d;
};

#endif // USERLISTITEM_H
