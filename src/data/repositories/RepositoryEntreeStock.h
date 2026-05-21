#ifndef REPOSITORYENTREESTOCK_H
#define REPOSITORYENTREESTOCK_H

#include "../../core/entities/EntreeStock.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>
#include <QSqlQuery>

class RepositoryEntreeStock
{
public:
    RepositoryEntreeStock();

    // CRUD & logique de cycle de vie
    bool create(const EntreeStock& entity);
    bool update(const EntreeStock& entity);
    bool logicalDelete(const QUuid& id);

    // Lecture
    std::optional<EntreeStock> getById(const QUuid& id) const;
    QList<EntreeStock> getAll() const;

    // Recherches métier
    QList<EntreeStock> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;
    QList<EntreeStock> getByStatut(const QString& statut) const;
    QList<EntreeStock> getByProduit(const QUuid& produitId) const;
    QList<EntreeStock> getEnAttente() const;
    bool approuver(const QUuid& entreeId, const QUuid& utilisateurId);
    bool rejeter(const QUuid& entreeId);

    // Offline-first / sync
    QList<EntreeStock> getPendingSync() const;
    QList<EntreeStock> getSinceVersion(int minVersion) const;

    // Gestion erreurs
    QString getLastError() const { return m_dernierErreur; }
private:
    EntreeStock mapRowToEntreeStock(const QSqlQuery& query) const;

    QString m_dernierErreur;
};

#endif // REPOSITORYENTREESTOCK_H
