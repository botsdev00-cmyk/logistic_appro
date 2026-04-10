#ifndef REPOSITORYCLIENT_H
#define REPOSITORYCLIENT_H

#include "IRepository.h"
#include "../../core/entities/Client.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryClient : public IRepository<Client>
{
public:
    RepositoryClient();

    bool create(const Client& entity) override;
    Client getById(const QUuid& id) override;
    QList<Client> getAll() override;
    bool update(const Client& entity) override;
    bool remove(const QUuid& id) override;

    QList<Client> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<Client> getByRoute(const QUuid& routeId);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYCLIENT_H