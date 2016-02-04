#ifndef SUBNETSET_H
#define SUBNETSET_H

#include <QExplicitlySharedDataPointer>
#include <QSet>
#include <QNetworkAddressEntry>

class SubnetSetPrivate;
class SubnetSet
{
public:
  SubnetSet();
  SubnetSet(const SubnetSet&);
  SubnetSet &operator=(const SubnetSet &);
  ~SubnetSet();

private:
  SubnetSet(SubnetSetPrivate* p);

public:
  typedef QSet<QNetworkAddressEntry>::iterator iterator;
  iterator begin() const;
  iterator end() const;

public:
  static SubnetSet allSubnets();

private:
  QExplicitlySharedDataPointer<SubnetSetPrivate> d;
};

inline uint
qHash(const QNetworkAddressEntry &addr, uint seed = 0)
{
  return qHash(addr.ip().toIPv4Address(), seed);
}

#endif // SUBNETSET_H
