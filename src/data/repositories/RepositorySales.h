#ifndef REPOSITORYSALES_H
#define REPOSITORYSALES_H

#include "IRepository.h"
#include "../../core/entities/Vente.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositorySales : public IRepository<Vente>
{
public:
    RepositorySales();

    bool create(const Vente& entity) override;
    Vente getById(const QUuid& id) override;
    QList<Vente> getAll() override;
    bool update(const Vente& entity) override;
    bool remove(const QUuid& id) override;

    QList<Vente> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<Vente> getByRepartition(const QUuid& repartitionId);
    QList<Vente> getByClient(const QUuid& clientId);
    double getTotalVentes(const QUuid& repartitionId);
    double getTotalCash(const QUuid& repartitionId);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYSALES_H