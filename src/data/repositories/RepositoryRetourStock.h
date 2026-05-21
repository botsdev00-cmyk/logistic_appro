#ifndef REPOSITORYRETOURSTOCK_H
#define REPOSITORYRETOURSTOCK_H

#include "../../core/entities/RetourStock.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>
#include <QSqlQuery>

class RepositoryRetourStock
{
public:
    RepositoryRetourStock();

    // CRUD & cycle de vie
    bool create(const RetourStock& entity);
    bool update(const RetourStock& entity);
    bool logicalDelete(const QUuid& id);

    // Lecture
    std::optional<RetourStock> getById(const QUuid& id) const;
    QList<RetourStock> getAll() const;

    // Recherches métier
    QList<RetourStock> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;
    QList<RetourStock> getByStatut(const QString& statut) const;
    QList<RetourStock> getByProduit(const QUuid& produitId) const;
    QList<RetourStock> getByRepartition(const QUuid& repartitionId) const;
    QList<RetourStock> getEnAttente() const;
    bool approuver(const QUuid& retourId, const QUuid& utilisateurId);
    bool rejeter(const QUuid& retourId);

    // Offline-first / sync
    QList<RetourStock> getPendingSync() const;
    QList<RetourStock> getSinceVersion(int minVersion) const;

    // Gestion erreurs
    QString getLastError() const { return m_dernierErreur; }
private:
    RetourStock mapRowToRetourStock(const QSqlQuery& query) const;

    QString m_dernierErreur;
};

#endif // REPOSITORYRETOURSTOCK_H
