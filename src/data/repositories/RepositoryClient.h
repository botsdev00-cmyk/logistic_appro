#ifndef REPOSITORYCLIENT_H
#define REPOSITORYCLIENT_H

#include "../../core/entities/Client.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>
#include <QSqlQuery>

class RepositoryClient
{
public:
    RepositoryClient();

    // CRUD & offline-first
    bool create(const Client& entity);
    bool update(const Client& entity);
    bool logicalDelete(const QUuid& id);
    std::optional<Client> getById(const QUuid& id) const;
    QList<Client> getAll() const;

    // Recherche et offline
    QList<Client> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;
    QList<Client> getByRoute(const QUuid& routeId) const;

    // Offline sync management
    QList<Client> getPendingSync() const;
    QList<Client> getSinceVersion(int minVersion) const;

    QString getLastError() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    Client mapRowToClient(const QSqlQuery& q) const;
};

#endif // REPOSITORYCLIENT_H
