#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QTreeWidgetItem>
#include <QUuid>
#include <QHostAddress>

class UserListItemPrivate;
class UserListItem : public QTreeWidgetItem
{
public:
  UserListItem(const QUuid& uuid, const QString& name, bool is_online);
  ~UserListItem();

public:
  QVariant data(int column, int role) const Q_DECL_OVERRIDE Q_DECL_FINAL;

public:
  const QUuid& uuid() const;
  const QString& name() const;
  void setName(const QString& name);
  bool isOnline() const;
  void setOnline(bool is_online);

  QHostAddress hostAddress() const;
  void setHostAddreess(const QHostAddress& address);

  void updateActivity();
  int inactivityMilliseconds() const;

  /**
   * @brief updateIcon
   * @return true, if item blinking (has unread messages)
   */
  bool updateIcon();

public:
  static UserListItem* findItem(const QUuid& uuid);
  static QList<QPair<QUuid,QString> > loadItems();

private:
  UserListItemPrivate *d;
};

#endif // USERLISTITEM_H
