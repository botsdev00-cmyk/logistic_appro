#ifndef REPOSITORY_STOCK_H
#define REPOSITORY_STOCK_H

#include "IRepository.h"
#include "../../core/entities/EntreeStock.h"
#include <QList>
#include <QDate>

class RepositoryStock : public IRepository<EntreeStock>
{
public:
    RepositoryStock();

    // ====== CRUD ======
    bool create(const EntreeStock& entity) override;
    EntreeStock getById(const QUuid& id) override;
    QList<EntreeStock> getAll() override;
    bool update(const EntreeStock& entity) override;
    bool remove(const QUuid& id) override;
    QList<EntreeStock> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;

    // ====== GETTERS D'ERREUR ======
    QString getLastError() const override { return m_dernierErreur; }

    // ====== SPÉCIFIQUES - FILTRAGE ======
    QList<EntreeStock> getByProduit(const QUuid& produitId);
    QList<EntreeStock> getByStatut(const QString& statut);
    QList<EntreeStock> getEnAttente();
    QList<EntreeStock> getBySource(const QUuid& sourceId);
    QList<EntreeStock> getByDateRange(const QDate& dateDebut, const QDate& dateFin);

    // ====== SPÉCIFIQUES - APPROBATION ======
    bool approuver(const QUuid& entreeId, const QUuid& utilisateurId);
    bool rejeter(const QUuid& entreeId);
    bool approuverTous(const QUuid& utilisateurId);

    // ====== SPÉCIFIQUES - AGRÉGATION ======
    int count();
    int countByStatut(const QString& statut);
    int sumQuantite(const QUuid& produitId);
    double sumValeur(const QUuid& produitId);

    // ====== SPÉCIFIQUES - PAGINATION ======
    QList<EntreeStock> getPagine(int page, int pageSize);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORY_STOCK_H