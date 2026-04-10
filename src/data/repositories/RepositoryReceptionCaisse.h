#ifndef REPOSITORYRECEPTIONCAISSE_H
#define REPOSITORYRECEPTIONCAISSE_H

#include "IRepository.h"
#include "../entities/ReceptionCaisse.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryReceptionCaisse : public IRepository<ReceptionCaisse>
{
public:
    RepositoryReceptionCaisse();

    bool create(const ReceptionCaisse& entity) override;
    ReceptionCaisse getById(const QUuid& id) override;
    QList<ReceptionCaisse> getAll() override;
    bool update(const ReceptionCaisse& entity) override;
    bool remove(const QUuid& id) override;

    QList<ReceptionCaisse> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    ReceptionCaisse getByRepartition(const QUuid& repartitionId);
    QList<ReceptionCaisse> getByStatut(const ReceptionCaisse::Statut& statut);
    QList<ReceptionCaisse> getWithDiscrepancies();

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYRECEPTIONCAISSE_H