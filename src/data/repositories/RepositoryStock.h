#ifndef REPOSITORYSTOCK_H
#define REPOSITORYSTOCK_H

#include "IRepository.h"
#include "../entities/EntreeStock.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryStock : public IRepository<EntreeStock>
{
public:
    RepositoryStock();

    bool create(const EntreeStock& entity) override;
    EntreeStock getById(const QUuid& id) override;
    QList<EntreeStock> getAll() override;
    bool update(const EntreeStock& entity) override;
    bool remove(const QUuid& id) override;

    QList<EntreeStock> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<EntreeStock> getByProduit(const QUuid& produitId);
    int getTotalQuantiteByProduit(const QUuid& produitId);
    QList<EntreeStock> getBySource(const EntreeStock::Source& source);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYSTOCK_H