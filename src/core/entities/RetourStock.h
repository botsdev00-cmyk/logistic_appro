#ifndef RETOURSTOCK_H
#define RETOURSTOCK_H

#include <QUuid>
#include <QString>
#include <QDateTime>

class RetourStock
{
public:
    enum SyncStatus {
        PENDING = 0,
        SYNCED = 1,
        CONFLICT = 2
    };

    RetourStock();
    ~RetourStock();

    // Getters
    QUuid getRetourStockId() const { return m_retourStockId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantite() const { return m_quantite; }
    QUuid getRaisonRetourId() const { return m_raisonRetourId; }
    QDateTime getDate() const { return m_date; }
    QUuid getRepartitionId() const { return m_repartitionId; }
    QString getObservations() const { return m_observations; }
    QUuid getCreePar() const { return m_creePar; }
    QUuid getApprouvePar() const { return m_approuvePar; }
    QString getStatutValidation() const { return m_statutValidation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    int getVersion() const { return m_version; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }

    // Setters
    void setRetourStockId(const QUuid& id) { m_retourStockId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantite(int q) { m_quantite = q; }
    void setRaisonRetourId(const QUuid& id) { m_raisonRetourId = id; }
    void setDate(const QDateTime& dt) { m_date = dt; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setObservations(const QString& obs) { m_observations = obs; }
    void setCreePar(const QUuid& id) { m_creePar = id; }
    void setApprouvePar(const QUuid& id) { m_approuvePar = id; }
    void setStatutValidation(const QString& statut) { m_statutValidation = statut; }
    void setDateMiseAJour(const QDateTime& dt) { m_dateMiseAJour = dt; }
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
    QUuid m_retourStockId;
    QUuid m_produitId;
    int m_quantite;
    QUuid m_raisonRetourId;
    QDateTime m_date;
    QUuid m_repartitionId;
    QString m_observations;
    QUuid m_creePar;
    QUuid m_approuvePar;
    QString m_statutValidation;
    QDateTime m_dateMiseAJour;
    SyncStatus m_syncStatus;
    int m_version;
    QDateTime m_deletedAt;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

#endif // RETOURSTOCK_H
