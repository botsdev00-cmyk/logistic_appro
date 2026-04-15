#ifndef REPOSITORYENTREESTOCK_H
#define REPOSITORYENTREESTOCK_H

#include "IRepository.h"
#include "../../core/entities/EntreeStock.h"
#include <QList>

class RepositoryEntreeStock : public IRepository<EntreeStock>
{
public:
    RepositoryEntreeStock();

    bool create(const EntreeStock& entity) override;
    EntreeStock getById(const QUuid& id) override;
    QList<EntreeStock> getAll() override;
    bool update(const EntreeStock& entity) override;
    bool remove(const QUuid& id) override;
    QList<EntreeStock> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;
    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    QList<EntreeStock> getByStatut(const QString& statut);
    QList<EntreeStock> getByProduit(const QUuid& produitId);
    QList<EntreeStock> getEnAttente();
    bool approuver(const QUuid& entreeId, const QUuid& utilisateurId);
    bool rejeter(const QUuid& entreeId);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYENTREESTOCK_H