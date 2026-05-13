#ifndef REPOSITORYREPARTITION_H
#define REPOSITORYREPARTITION_H

#include "IRepository.h"
#include "../../core/entities/Repartition.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <QSqlQuery>

class RepositoryRepartition : public IRepository<Repartition>
{
public:
    RepositoryRepartition();

    bool create(const Repartition& entity) override;
    Repartition getById(const QUuid& id) override;
    QList<Repartition> getAll() const override;
    bool update(const Repartition& entity) override;
    bool remove(const QUuid& id) override;

    QList<Repartition> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<Repartition> getByEquipe(const QUuid& equipeId);
    QList<Repartition> getByRoute(const QUuid& routeId);
    QList<Repartition> getByStatut(const Repartition::Statut& statut);
    void setStatutRepartitionId(const QUuid& id) { m_statutRepartitionId = id; }
    QUuid getStatutRepartitionId() const { return m_statutRepartitionId; }

    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    QDateTime getCreatedAt() const { return m_createdAt; }

    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }

    void setDateMiseAJour(const QDateTime& dt) { m_dateMiseAJour = dt; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }

    void setChefId(const QUuid& id) { m_chefId = id; }
    QUuid getChefId() const { return m_chefId; }

    void setAnnule(bool annule) { m_annule = annule; }
    bool getAnnule() const { return m_annule; }


private:
    Repartition mapRowToRepartition(const QSqlQuery& query) const;
    QUuid m_statutRepartitionId;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_dateMiseAJour;
    QDateTime m_deletedAt;
    QUuid m_chefId;
    bool m_annule;
    mutable QString m_dernierErreur;
};

#endif // REPOSITORYREPARTITION_H
