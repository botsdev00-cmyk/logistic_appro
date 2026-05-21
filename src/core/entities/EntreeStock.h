#ifndef ENTREESTOCK_H
#define ENTREESTOCK_H

#include <QUuid>
#include <QString>
#include <QDate>
#include <QDateTime>

class EntreeStock
{
public:
    enum SyncStatus {
        PENDING = 0,
        SYNCED = 1,
        CONFLICT = 2
    };

    EntreeStock();
    ~EntreeStock();

    // Getters
    QUuid getEntreeStockId() const { return m_entreeStockId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantite() const { return m_quantite; }
    QUuid getSourceEntreeId() const { return m_sourceEntreeId; }
    QDateTime getDate() const { return m_date; }
    QUuid getCreePar() const { return m_creePar; }
    QString getNumeroFacture() const { return m_numeroFacture; }
    double getPrixUnitaire() const { return m_prixUnitaire; }
    QString getNumeroLot() const { return m_numeroLot; }
    QDate getDateExpiration() const { return m_dateExpiration; }
    QUuid getApprouvePar() const { return m_approuvePar; }
    QString getStatutValidation() const { return m_statutValidation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    QUuid getCreeParUpdated() const { return m_creeParUpdated; }

    // Offline-first/sync stuff
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    int getVersion() const { return m_version; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }

    // Setters
    void setEntreeStockId(const QUuid& id) { m_entreeStockId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantite(int q) { m_quantite = q; }
    void setSourceEntreeId(const QUuid& id) { m_sourceEntreeId = id; }
    void setDate(const QDateTime& dt) { m_date = dt; }
    void setCreePar(const QUuid& id) { m_creePar = id; }
    void setNumeroFacture(const QString& num) { m_numeroFacture = num; }
    void setPrixUnitaire(double prix) { m_prixUnitaire = prix; }
    void setNumeroLot(const QString& lot) { m_numeroLot = lot; }
    void setDateExpiration(const QDate& date) { m_dateExpiration = date; }
    void setApprouvePar(const QUuid& id) { m_approuvePar = id; }
    void setStatutValidation(const QString& statut) { m_statutValidation = statut; }
    void setDateMiseAJour(const QDateTime& dt) { m_dateMiseAJour = dt; }
    void setCreeParUpdated(const QUuid& id) { m_creeParUpdated = id; }

    // Offline-first/sync
    void setSyncStatus(SyncStatus s) { m_syncStatus = s; }
    void setVersion(int v) { m_version = v; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    // Validation
    bool estValide() const;

    // Helpers for sync status as string
    QString syncStatusString() const;
    static SyncStatus stringToSyncStatus(const QString& str);

private:
    QUuid m_entreeStockId;
    QUuid m_produitId;
    int m_quantite;
    QUuid m_sourceEntreeId;
    QDateTime m_date;
    QUuid m_creePar;
    QString m_numeroFacture;
    double m_prixUnitaire;
    QString m_numeroLot;
    QDate m_dateExpiration;
    QUuid m_approuvePar;
    QString m_statutValidation;
    QDateTime m_dateMiseAJour;
    QUuid m_creeParUpdated;

    // Offline-first/sync
    SyncStatus m_syncStatus;
    int m_version;
    QDateTime m_deletedAt;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

#endif // ENTREESTOCK_H
