#ifndef REPOSITORYREPARTITION_H
#define REPOSITORYREPARTITION_H

#include "IRepository.h"
#include "../../core/entities/Repartition.h"
#include <QList>
#include <QString>
#include <QUuid>

class RepositoryRepartition : public IRepository<Repartition>
{
public:
    RepositoryRepartition();

    bool create(const Repartition& entity) override;
    Repartition getById(const QUuid& id) override;
    QList<Repartition> getAll() override;
    bool update(const Repartition& entity) override;
    bool remove(const QUuid& id) override;

    QList<Repartition> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<Repartition> getByEquipe(const QUuid& equipeId);
    QList<Repartition> getByRoute(const QUuid& routeId);
    QList<Repartition> getByStatut(const Repartition::Statut& statut);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYREPARTITION_H