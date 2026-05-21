#ifndef STOCKSOLDE_H
#define STOCKSOLDE_H

#include <QUuid>
#include <QString>
#include <QDateTime>

class StockSolde
{
public:
    enum SyncStatus {
        PENDING = 0,
        SYNCED = 1,
        CONFLICT = 2
    };

    StockSolde();
    ~StockSolde();

    // Identifiants
    QUuid getSoldeId() const { return m_soldeId; }
    QUuid getProduitId() const { return m_produitId; }

    // Quantités
    int getQuantiteTotal() const { return m_quantiteTotal; }
    int getQuantiteReserve() const { return m_quantiteReserve; }
    int getQuantiteDisponible() const { return m_quantiteDisponible; }

    // Valeurs
    double getValeurStock() const { return m_valeurStock; }
    double getPrixMoyen() const { return m_prixMoyen; }

    // Localisation
    QString getLocationId()            const { return m_locationId; }
    QString getDerniereLocationId()    const { return m_derniereLocationId; }

    // Historique : chaque localisation comme entier séparé en C++ pur
    int getHistoryReturned()  const { return m_historyReturned; }
    int getHistoryWarehouse() const { return m_historyWarehouse; }
    int getHistoryInTransit() const { return m_historyInTransit; }
    void setHistoryReturned(int v)  { m_historyReturned = v; }
    void setHistoryWarehouse(int v) { m_historyWarehouse = v; }
    void setHistoryInTransit(int v) { m_historyInTransit = v; }

    // Dates/états métier
    QDateTime getDernierMouvementDate() const { return m_dernierMouvementDate; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    bool getIsDeleted() const { return m_isDeleted; }

    // Version/offline
    SyncStatus getSyncStatus()  const { return m_syncStatus; }
    int getVersion()            const { return m_version; }

    // Setters
    void setSoldeId(const QUuid& id)             { m_soldeId = id; }
    void setProduitId(const QUuid& id)           { m_produitId = id; }
    void setQuantiteTotal(int val)               { m_quantiteTotal = val; }
    void setQuantiteReserve(int val)             { m_quantiteReserve = val; }
    void setQuantiteDisponible(int val)          { m_quantiteDisponible = val; }
    void setValeurStock(double val)              { m_valeurStock = val; }
    void setPrixMoyen(double val)                { m_prixMoyen = val; }
    void setLocationId(const QString& loc)       { m_locationId = loc; }
    void setDerniereLocationId(const QString& l) { m_derniereLocationId = l; }
    void setDernierMouvementDate(const QDateTime& dt) { m_dernierMouvementDate = dt; }
    void setUpdatedAt(const QDateTime& dt)       { m_updatedAt = dt; }
    void setDeletedAt(const QDateTime& dt)       { m_deletedAt = dt; }
    void setCreatedAt(const QDateTime& dt)       { m_createdAt = dt; }
    void setIsDeleted(bool v)                    { m_isDeleted = v; }
    void setSyncStatus(SyncStatus s)             { m_syncStatus = s; }
    void setVersion(int v)                       { m_version = v; }

    // Helpers sync status
    QString syncStatusString() const;
    static SyncStatus stringToSyncStatus(const QString& str);

    // Utils
    bool estEnRupture() const { return m_quantiteDisponible <= 0; }
    bool estBasStock(int seuil) const { return m_quantiteDisponible > 0 && m_quantiteDisponible < seuil; }
    bool estDisponible(int quantiteRequise) const { return m_quantiteDisponible >= quantiteRequise; }

    QString getProduitNom() const { return m_produitNom; }
    QString getCodeSKU() const { return m_codeSKU; }
    QString getCategorie() const { return m_categorie; }
    QString getType() const { return m_type; }
    int getStockMinimum() const { return m_stockMinimum; }

    void setProduitNom(const QString& n) { m_produitNom = n; }
    void setCodeSKU(const QString& s) { m_codeSKU = s; }
    void setCategorie(const QString& c) { m_categorie = c; }
    void setType(const QString& t) { m_type = t; }
    void setStockMinimum(int s) { m_stockMinimum = s; }
    int getQuantiteReservee() const { return getQuantiteReserve(); }

private:
    QString m_produitNom;
    QString m_codeSKU;
    QString m_categorie;
    QString m_type;
    int m_stockMinimum;
    QUuid m_soldeId;
    QUuid m_produitId;
    int m_quantiteTotal;
    int m_quantiteReserve;
    int m_quantiteDisponible;
    double m_valeurStock;
    double m_prixMoyen;
    QString m_locationId;
    QString m_derniereLocationId;
    int m_historyReturned;
    int m_historyWarehouse;
    int m_historyInTransit;
    QDateTime m_dernierMouvementDate;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
    QDateTime m_createdAt;
    SyncStatus m_syncStatus;
    int m_version;
    bool m_isDeleted;
};

#endif // STOCKSOLDE_H
