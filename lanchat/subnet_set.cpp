#include "subnet_set.h"

class SubnetSetPrivate : public QSharedData
{
private:
  SubnetSetPrivate()
  {
  }

private:
  Q_DISABLE_COPY(SubnetSetPrivate)

private:
  QSet<QNetworkAddressEntry> set_;

  friend class SubnetSet;
  friend class QExplicitlySharedDataPointer<SubnetSetPrivate>;
};

SubnetSet::SubnetSet() :
  d(new SubnetSetPrivate)
{
}

SubnetSet::SubnetSet(const SubnetSet &other) :
  d(other.d)
{
}

SubnetSet::SubnetSet(SubnetSetPrivate* p) :
  d(p)
{
}

SubnetSet&
SubnetSet::operator=(const SubnetSet &other)
{
  if (this != &other)
    d.operator=(other.d);
  return *this;
}

SubnetSet::~SubnetSet()
{
}

SubnetSet::iterator
SubnetSet::begin() const
{
  return d->set_.begin();
}

SubnetSet::iterator
SubnetSet::end() const
{
  return d->set_.end();
}

bool
SubnetSet::operator ==(const SubnetSet& other) const
{
  return d->set_ == other.d->set_;
}

SubnetSet SubnetSet::allSubnets()
{
  SubnetSetPrivate *p = new SubnetSetPrivate();

  for (QNetworkInterface const& nic : QNetworkInterface::allInterfaces())
    {
      if ((!nic.flags().testFlag(QNetworkInterface::IsUp))
          || (!nic.flags().testFlag(QNetworkInterface::IsRunning))
          || nic.flags().testFlag(QNetworkInterface::IsLoopBack)
          || nic.flags().testFlag(QNetworkInterface::IsPointToPoint))
        continue;

      for (QNetworkAddressEntry const& addr : nic.addressEntries())
        {
          if(QAbstractSocket::IPv4Protocol != addr.ip().protocol())
            continue;
          if(addr.broadcast().isNull() || addr.netmask().isNull())
            continue;
          if(addr.broadcast() == addr.ip())
            continue;
          p->set_.insert(addr);
        }
    }

  return SubnetSet(p);
}
