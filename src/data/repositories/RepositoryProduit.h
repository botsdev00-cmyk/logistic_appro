#ifndef REPOSITORYPRODUIT_H
#define REPOSITORYPRODUIT_H

#include "../../core/entities/Produit.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>
#include <QSqlQuery>

class RepositoryProduit {
public:
    RepositoryProduit();

    bool create(const Produit& entity);
    bool update(const Produit& entity);
    bool logicalDelete(const QUuid& id);
    std::optional<Produit> getById(const QUuid& id) const;
    std::optional<Produit> getByCodeSku(const QString& sku) const;
    QList<Produit> getAll() const;
    QList<Produit> getByCategorie(const QUuid& categorieId) const;
    QList<Produit> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;

    QList<Produit> getPendingSync() const;
    QList<Produit> getSinceVersion(int minVersion) const;

    QString getLastError() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    Produit mapRowToProduit(const QSqlQuery& query) const;
};

#endif // REPOSITORYPRODUIT_H
