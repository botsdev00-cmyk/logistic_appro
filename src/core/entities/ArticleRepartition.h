#ifndef ARTICLEREPARTITION_H
#define ARTICLEREPARTITION_H

#include <QUuid>
#include <QString>
#include <QDateTime>

class ArticleRepartition
{
public:
    enum SyncStatus {
        PENDING = 0,
        SYNCED = 1,
        CONFLICT = 2
    };

    ArticleRepartition();
    ~ArticleRepartition();

    // Getters - Core data
    QUuid getArticleRepartitionId() const { return m_articleRepartitionId; }
    QUuid getRepartitionId() const { return m_repartitionId; }
    QUuid getProduitId() const { return m_produitId; }
    int getQuantiteVente() const { return m_quantiteVente; }
    int getQuantiteCadeau() const { return m_quantiteCadeau; }
    int getQuantiteDegustation() const { return m_quantiteDegustation; }
    int getQuantiteTotale() const { return m_quantiteVente + m_quantiteCadeau + m_quantiteDegustation; }
    QString getObservation() const { return m_observation; }

    // Getters - Offline-first / Sync
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    int getVersion() const { return m_version; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QUuid getCreatedBy() const { return m_createdBy; }
    QUuid getUpdatedBy() const { return m_updatedBy; }

    // Setters - Core data
    void setArticleRepartitionId(const QUuid& id) { m_articleRepartitionId = id; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setProduitId(const QUuid& id) { m_produitId = id; }
    void setQuantiteVente(int qty) { m_quantiteVente = qty; }
    void setQuantiteCadeau(int qty) { m_quantiteCadeau = qty; }
    void setQuantiteDegustation(int qty) { m_quantiteDegustation = qty; }
    void setObservation(const QString& obs) { m_observation = obs; }

    // Setters - Offline-first / Sync
    void setSyncStatus(SyncStatus status) { m_syncStatus = status; }
    void setVersion(int version) { m_version = version; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    void setCreatedBy(const QUuid& id) { m_createdBy = id; }
    void setUpdatedBy(const QUuid& id) { m_updatedBy = id; }

    // Convenience methods
    bool isDeleted() const { return !m_deletedAt.isNull(); }
    bool isPending() const { return m_syncStatus == PENDING; }
    bool isConflict() const { return m_syncStatus == CONFLICT; }
    bool needsSync() const { return m_syncStatus == PENDING || m_syncStatus == CONFLICT; }
    QString syncStatusString() const;
    static SyncStatus stringToSyncStatus(const QString& str);

private:
    // Core data
    QUuid m_articleRepartitionId;
    QUuid m_repartitionId;
    QUuid m_produitId;
    int m_quantiteVente;
    int m_quantiteCadeau;
    int m_quantiteDegustation;
    QString m_observation;

    // Offline-first / Sync
    SyncStatus m_syncStatus;
    int m_version;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
    QUuid m_createdBy;
    QUuid m_updatedBy;
};

#endif // ARTICLEREPARTITION_H
