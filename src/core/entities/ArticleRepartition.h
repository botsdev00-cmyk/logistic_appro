#pragma once

#include <QUuid>
#include <QString>
#include <QDateTime>

class ArticleRepartition {
public:
    // SyncState comme la table (Enum string côté offline pour future compatibilité)
    enum class SyncStatus {
        PENDING,
        SYNCED,
        CONFLICT
    };

    ArticleRepartition();
    ~ArticleRepartition();

    // Core fields
    QUuid getArticleRepartitionId() const { return m_articleRepartitionId; }
    void setArticleRepartitionId(const QUuid& id) { m_articleRepartitionId = id; }

    QUuid getRepartitionId() const { return m_repartitionId; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }

    QUuid getProduitId() const { return m_produitId; }
    void setProduitId(const QUuid& id) { m_produitId = id; }

    int getQuantiteVente() const { return m_quantiteVente; }
    void setQuantiteVente(int qty) { m_quantiteVente = qty; }

    int getQuantiteCadeau() const { return m_quantiteCadeau; }
    void setQuantiteCadeau(int qty) { m_quantiteCadeau = qty; }

    int getQuantiteDegustation() const { return m_quantiteDegustation; }
    void setQuantiteDegustation(int qty) { m_quantiteDegustation = qty; }

    int getQuantiteTotale() const { return m_quantiteVente + m_quantiteCadeau + m_quantiteDegustation; }

    QString getObservation() const { return m_observation; }
    void setObservation(const QString& obs) { m_observation = obs; }

    // Offline-first fields
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    void setSyncStatus(SyncStatus s) { m_syncStatus = s; }
    QString syncStatusString() const;
    static SyncStatus stringToSyncStatus(const QString& str);

    int getVersion() const { return m_version; }
    void setVersion(int v) { m_version = v; }

    QDateTime getDeletedAt() const { return m_deletedAt; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    bool isDeleted() const { return !m_deletedAt.isNull(); }

    QDateTime getCreatedAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    QDateTime getUpdatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    QUuid getCreatedBy() const { return m_createdBy; }
    void setCreatedBy(const QUuid& id) { m_createdBy = id; }

    QUuid getUpdatedBy() const { return m_updatedBy; }
    void setUpdatedBy(const QUuid& id) { m_updatedBy = id; }

private:
    QUuid m_articleRepartitionId;
    QUuid m_repartitionId;
    QUuid m_produitId;
    int m_quantiteVente;
    int m_quantiteCadeau;
    int m_quantiteDegustation;
    QString m_observation;

    // Offline-first sync/audit
    SyncStatus m_syncStatus;
    int m_version;
    QDateTime m_deletedAt;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QUuid m_createdBy;
    QUuid m_updatedBy;
};
