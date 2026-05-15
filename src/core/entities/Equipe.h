#ifndef EQUIPE_H
#define EQUIPE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class Equipe
{
public:
    enum class SyncStatus {
        PENDING,
        SYNCED,
        CONFLICT
    };

    Equipe();
    ~Equipe();

    // Getters
    QUuid getEquipeId() const { return m_equipeId; }
    QString getNom() const { return m_nom; }
    QString getNomChef() const { return m_nomChef; }
    bool getEstActif() const { return m_estActif; }

    SyncStatus getSyncStatus() const { return m_syncStatus; }
    int getVersion() const { return m_version; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QUuid getCreatedBy() const { return m_createdBy; }
    QUuid getUpdatedBy() const { return m_updatedBy; }

    // Setters
    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    void setNom(const QString& v) { m_nom = v; }
    void setNomChef(const QString& v) { m_nomChef = v; }
    void setEstActif(bool v) { m_estActif = v; }
    void setSyncStatus(SyncStatus st) { m_syncStatus = st; }
    void setVersion(int v) { m_version = v; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    void setCreatedBy(const QUuid& id) { m_createdBy = id; }
    void setUpdatedBy(const QUuid& id) { m_updatedBy = id; }

    // Utilitaires
    bool isDeleted() const { return m_deletedAt.isValid(); }
    bool needsSync() const { return m_syncStatus != SyncStatus::SYNCED; }

    QString syncStatusString() const;
    static SyncStatus syncStatusFromString(const QString& str);

private:
    QUuid m_equipeId;
    QString m_nom;
    QString m_nomChef;
    bool m_estActif = true;

    SyncStatus m_syncStatus = SyncStatus::PENDING;
    int m_version = 1;

    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
    QUuid m_createdBy;
    QUuid m_updatedBy;
};

#endif // EQUIPE_H
