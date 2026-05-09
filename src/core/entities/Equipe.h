#ifndef EQUIPE_H
#define EQUIPE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Equipe
{
public:
    enum SyncStatus {
        PENDING = 0,
        SYNCED = 1,
        CONFLICT = 2
    };

    Equipe();
    ~Equipe();

    // Getters - Core data
    QUuid getEquipeId() const { return m_equipeId; }
    QString getNom() const { return m_nom; }
    QString getNomChef() const { return m_nomChef; }
    QString getTelephoneChef() const { return m_telephoneChef; }
    QString getDescription() const { return m_description; }
    bool getEstActif() const { return m_estActif; }

    // Getters - Offline-first / Sync
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    int getVersion() const { return m_version; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QUuid getCreatedBy() const { return m_createdBy; }
    QUuid getUpdatedBy() const { return m_updatedBy; }

    // Setters - Core data
    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    void setNom(const QString& nom) { m_nom = nom; }
    void setNomChef(const QString& nom) { m_nomChef = nom; }
    void setTelephoneChef(const QString& tel) { m_telephoneChef = tel; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setEstActif(bool actif) { m_estActif = actif; }

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
    QUuid m_equipeId;
    QString m_nom;
    QString m_nomChef;
    QString m_telephoneChef;
    QString m_description;
    bool m_estActif;

    // Offline-first / Sync
    SyncStatus m_syncStatus;
    int m_version;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
    QUuid m_createdBy;
    QUuid m_updatedBy;
};

#endif // EQUIPE_H
