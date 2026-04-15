#ifndef REPOSITORYRETOURSTOCK_H
#define REPOSITORYRETOURSTOCK_H

#include "IRepository.h"
#include "../../core/entities/RetourStock.h"
#include <QList>

class RepositoryRetourStock : public IRepository<RetourStock>
{
public:
    RepositoryRetourStock();

    bool create(const RetourStock& entity) override;
    RetourStock getById(const QUuid& id) override;
    QList<RetourStock> getAll() override;
    bool update(const RetourStock& entity) override;
    bool remove(const QUuid& id) override;
    QList<RetourStock> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;
    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<RetourStock> getByStatut(const QString& statut);
    QList<RetourStock> getByProduit(const QUuid& produitId);
    QList<RetourStock> getByRepartition(const QUuid& repartitionId);
    QList<RetourStock> getEnAttente();
    bool approuver(const QUuid& retourId, const QUuid& utilisateurId);
    bool rejeter(const QUuid& retourId);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYRETOURSTOCK_H