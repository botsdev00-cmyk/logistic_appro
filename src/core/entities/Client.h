#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QDateTime>
#include <QUuid>

// Les enums pour la table
enum class SyncState {
    PENDING,
    SYNCED,
    CONFLICT
};

class Client
{
public:
    Client();
    ~Client();

    // Accesseurs
    QUuid getClientId() const { return m_clientId; }
    QString getNom() const { return m_nom; }
    QUuid getRouteId() const { return m_routeId; }
    QUuid getConditionPaiementId() const { return m_conditionPaiementId; }
    QString getConditionPaiementNom() const { return m_conditionPaiementNom; }
    QString getAdresse() const { return m_adresse; }
    QString getTelephone() const { return m_telephone; }
    QString getEmail() const { return m_email; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    int getVersion() const { return m_version; }
    SyncState getSyncState() const { return m_syncState; }
    QDateTime getDeletedAt() const { return m_deletedAt; }
    QUuid getGrilleId() const { return m_grilleId; }

    // Mutateurs
    void setClientId(const QUuid& v) { m_clientId = v; }
    void setNom(const QString& v) { m_nom = v; }
    void setRouteId(const QUuid& v) { m_routeId = v; }
    void setConditionPaiementId(const QUuid& v) { m_conditionPaiementId = v; }
    void setConditionPaiementNom(const QString& v) { m_conditionPaiementNom = v; }
    void setAdresse(const QString& v) { m_adresse = v; }
    void setTelephone(const QString& v) { m_telephone = v; }
    void setEmail(const QString& v) { m_email = v; }
    void setDateMiseAJour(const QDateTime& v) { m_dateMiseAJour = v; }
    void setCreatedAt(const QDateTime& v) { m_createdAt = v; }
    void setUpdatedAt(const QDateTime& v) { m_updatedAt = v; }
    void setVersion(int v) { m_version = v; }
    void setSyncState(SyncState v) { m_syncState = v; }
    void setDeletedAt(const QDateTime& v) { m_deletedAt = v; }
    void setGrilleId(const QUuid& v) { m_grilleId = v; }

    // Utilitaires
    bool isDeleted() const { return m_deletedAt.isValid(); }
    bool needsSync() const { return m_syncState != SyncState::SYNCED; }
    QString syncStateString() const;
    static SyncState syncStateFromString(const QString& val);

private:
    QUuid m_clientId;
    QString m_nom;
    QUuid m_routeId;
    QUuid m_conditionPaiementId;
    QString m_conditionPaiementNom; // Optionnel, pour affichage
    QString m_adresse;
    QString m_telephone;
    QString m_email;
    QDateTime m_dateMiseAJour;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    int m_version = 1;
    SyncState m_syncState = SyncState::PENDING;
    QDateTime m_deletedAt;
    QUuid m_grilleId;
};

#endif // CLIENT_H
