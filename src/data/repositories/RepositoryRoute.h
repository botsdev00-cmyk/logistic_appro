#ifndef REPOSITORYROUTE_H
#define REPOSITORYROUTE_H

#include "IRepository.h"
#include "../../core/entities/Route.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryRoute : public IRepository<Route>
{
public:
    RepositoryRoute();

    bool create(const Route& entity) override;
    Route getById(const QUuid& id) override;
    QList<Route> getAll() override;
    bool update(const Route& entity) override;
    bool remove(const QUuid& id) override;

    QList<Route> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYROUTE_H