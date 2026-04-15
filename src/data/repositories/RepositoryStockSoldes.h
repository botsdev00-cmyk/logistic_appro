#ifndef REPOSITORYSTOCKSOLDES_H
#define REPOSITORYSTOCKSOLDES_H

#include "IRepository.h"
#include "../../core/entities/StockSolde.h"
#include <QList>

class RepositoryStockSoldes : public IRepository<StockSolde>
{
public:
    RepositoryStockSoldes();

    bool create(const StockSolde& entity) override;
    StockSolde getById(const QUuid& id) override;
    QList<StockSolde> getAll() override;
    bool update(const StockSolde& entity) override;
    bool remove(const QUuid& id) override;
    QList<StockSolde> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;
    QString getLastError() const override { return m_dernierErreur; }

    // Méthodes spécifiques
    StockSolde getByProduit(const QUuid& produitId);
    int obtenirQuantiteDisponible(const QUuid& produitId);
    int obtenirQuantiteTotal(const QUuid& produitId);
    int obtenirQuantiteReservee(const QUuid& produitId);
    double obtenirValeurTotalStock();
    QList<StockSolde> obtenirStocksBas(int seuil);
    QList<StockSolde> obtenirStocksEnRupture();
    bool synchroniserTousSoldes();
    bool mettreAJourSolde(const QUuid& produitId);

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYSTOCKSOLDES_H