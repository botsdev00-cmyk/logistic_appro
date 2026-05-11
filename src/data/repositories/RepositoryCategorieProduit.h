#ifndef REPOSITORYCATEGORIEPRODUIT_H
#define REPOSITORYCATEGORIEPRODUIT_H

#include "../../core/entities/CategorieProduit.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <optional>
#include <QSqlQuery>

class RepositoryCategorieProduit
{
public:
    RepositoryCategorieProduit();

    // CRUD offline-first
    bool create(const CategorieProduit& entity);               // Ajout : sync_status PENDING, version 1
    bool update(const CategorieProduit& entity);               // Maj (maj version, sync_status)
    bool logicalDelete(const QUuid& id);                       // Soft delete (never SQL DELETE)
    std::optional<CategorieProduit> getById(const QUuid& id) const;
    QList<CategorieProduit> getAll() const;

    // Recherche avancée
    QList<CategorieProduit> search(const QString& criterion) const;
    bool exists(const QUuid& id) const;
    std::optional<CategorieProduit> getByCode(const QString& code) const;

    // OFFLINE-FIRST
    QList<CategorieProduit> getPendingSync() const;
    QList<CategorieProduit> getSinceVersion(int minVersion) const;

    QString getLastError() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    CategorieProduit mapRowToCategorie(const QSqlQuery& query) const;
};

#endif // REPOSITORYCATEGORIEPRODUIT_H
