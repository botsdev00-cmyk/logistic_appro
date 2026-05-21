#ifndef REPOSITORYSTOCKSOLDES_H
#define REPOSITORYSTOCKSOLDES_H

#include "../../core/entities/StockSolde.h"
#include <QList>
#include <QString>
#include <QUuid>
#include <QSqlQuery>
#include <QMap>

class RepositoryStockSoldes
{
public:
    RepositoryStockSoldes();

    // CRUD et gestion de vie
    bool create(const StockSolde& entity);
    bool update(const StockSolde& entity);
    bool logicalDelete(const QUuid& id);
    StockSolde getById(const QUuid& id) const;
    QList<StockSolde> getAll() const;

    // Recherche(s), accès métier
    bool exists(const QUuid& id) const;
    QList<StockSolde> search(const QString& criterion) const;
    StockSolde getByProduit(const QUuid& produitId) const;

    // Quantités/valeurs métier
    int obtenirQuantiteDisponible(const QUuid& produitId) const;
    int obtenirQuantiteTotal(const QUuid& produitId) const;
    int obtenirQuantiteReservee(const QUuid& produitId) const;
    double obtenirValeurProduit(const QUuid& produitId) const;
    double obtenirValeurTotalStock() const;

    // Alertes et catégories
    QList<StockSolde> obtenirStocksBas(int seuil) const;
    QList<StockSolde> obtenirStocksEnRupture() const;
    QList<StockSolde> obtenirStocksParCategorie(const QString& categorie) const;
    QList<StockSolde> obtenirStockDetail() const;

    // Synchronisation/sync
    QList<StockSolde> getPendingSync() const;
    QList<StockSolde> getSinceVersion(int minVersion) const;
    bool synchroniserTousSoldes();
    bool mettreAJourSolde(const QUuid& produitId);

    // Rapports/stats
    QMap<QString, int> obtenirStatistiquesParCategorie() const;
    QMap<QString, double> obtenirValeurParCategorie() const;

    // Erreurs
    QString getLastError() const { return m_dernierErreur; }

private:
    // Mapping record (avec extraction C++ pur du location_historique)
    StockSolde mapRowToStockSolde(const QSqlQuery& query) const;
    int getJsonInt(const QString& str, const char* key) const;

    mutable QString m_dernierErreur;
};

#endif // REPOSITORYSTOCKSOLDES_H
