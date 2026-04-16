#ifndef REPOSITORYSTOCKSOLDES_H
#define REPOSITORYSTOCKSOLDES_H

#include "IRepository.h"
#include "../../core/entities/StockSolde.h"
#include <QList>
#include <QMap>

class RepositoryStockSoldes : public IRepository<StockSolde>
{
public:
    RepositoryStockSoldes();

    // ============ INTERFACE IRepository ============
    bool create(const StockSolde& entity) override;
    StockSolde getById(const QUuid& id) override;
    QList<StockSolde> getAll() override;
    bool update(const StockSolde& entity) override;
    bool remove(const QUuid& id) override;
    QList<StockSolde> search(const QString& criterion) override;
    bool exists(const QUuid& id) override;
    QString getLastError() const override { return m_dernierErreur; }

    // ============ MÉTHODES SPÉCIFIQUES ============
    StockSolde getByProduit(const QUuid& produitId);
    
    // ✅ NOUVELLE MÉTHODE - Retourne les stocks avec tous les détails du produit
    QList<StockSolde> obtenirStockDetail();
    
    // ============ CONSULTATIONS QUANTITÉS ============
    int obtenirQuantiteDisponible(const QUuid& produitId);
    int obtenirQuantiteTotal(const QUuid& produitId);
    int obtenirQuantiteReservee(const QUuid& produitId);

    // ============ CONSULTATIONS VALEURS ============
    double obtenirValeurTotalStock();
    double obtenirValeurProduit(const QUuid& produitId);

    // ============ ALERTES STOCK ============
    QList<StockSolde> obtenirStocksBas(int seuil);
    QList<StockSolde> obtenirStocksEnRupture();
    QList<StockSolde> obtenirStocksParCategorie(const QString& categorie);

    // ============ SYNCHRONISATION ============
    bool synchroniserTousSoldes();
    bool mettreAJourSolde(const QUuid& produitId);
    
    // ============ RAPPORTS ============
    QMap<QString, int> obtenirStatistiquesParCategorie();
    QMap<QString, double> obtenirValeurParCategorie();

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYSTOCKSOLDES_H