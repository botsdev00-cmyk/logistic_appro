#ifndef REPOSITORYRECEPTIONCAISSE_H
#define REPOSITORYRECEPTIONCAISSE_H

#include "IRepository.h"
#include "../../core/entities/ReceptionCaisse.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <QSqlQuery>

class RepositoryReceptionCaisse : public IRepository<ReceptionCaisse>
{
public:
    RepositoryReceptionCaisse();

    bool create(const ReceptionCaisse& entity) override;
    ReceptionCaisse getById(const QUuid& id) override;
    QList<ReceptionCaisse> getAll() const override;
    bool update(const ReceptionCaisse& entity) override;
    bool remove(const QUuid& id) override;

    QList<ReceptionCaisse> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;
    QString getLastError() const override { return m_dernierErreur; }

    // Offline-first
    QList<ReceptionCaisse> getBySyncStatus(ReceptionCaisse::SyncStatus status) const;
    QList<ReceptionCaisse> getSinceVersion(int minVersion) const;

    // Anciennes méthodes pour compatibilité métier
    ReceptionCaisse getByRepartition(const QUuid& repartitionId);
    QList<ReceptionCaisse> getByStatut(ReceptionCaisse::Statut statut);
    QList<ReceptionCaisse> getWithDiscrepancies();

private:
    void hydrateFromQuery(ReceptionCaisse& rc, const QSqlQuery& query) const;
    mutable QString m_dernierErreur;
};

#endif // REPOSITORYRECEPTIONCAISSE_H
