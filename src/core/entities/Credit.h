#ifndef CREDIT_H
#define CREDIT_H

#include <QString>
#include <QDateTime>
#include <QDate>
#include <QUuid>

class Credit
{
public:
    enum class Statut {
        EnAttente,
        Paye,
        EnRetard,
        Annule
    };
    enum class SyncStatus {
        PENDING,
        SYNCED,
        CONFLICT
    };

    Credit();
    ~Credit();

    // Getters
    QUuid getCreditId() const { return m_creditId; }
    QUuid getVenteId() const { return m_venteId; }
    QUuid getClientId() const { return m_clientId; }
    double getMontant() const { return m_montant; }
    QDate getDateEcheance() const { return m_dateEcheance; }
    Statut getStatut() const { return m_statut; }
    QDate getDatePaiement() const { return m_datePaiement; }
    QString getNotes() const { return m_notes; }
    int getVersion() const { return m_version; }
    SyncStatus getSyncStatus() const { return m_syncStatus; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }

    // Setters
    void setCreditId(const QUuid& id) { m_creditId = id; }
    void setVenteId(const QUuid& id) { m_venteId = id; }
    void setClientId(const QUuid& id) { m_clientId = id; }
    void setMontant(double m) { m_montant = m; }
    void setDateEcheance(const QDate& d) { m_dateEcheance = d; }
    void setStatut(Statut s) { m_statut = s; }
    void setDatePaiement(const QDate& d) { m_datePaiement = d; }
    void setNotes(const QString& n) { m_notes = n; }
    void setVersion(int v) { m_version = v; }
    void setSyncStatus(SyncStatus s) { m_syncStatus = s; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }

    // Offline/Utility
    bool isDeleted() const { return m_deletedAt.isValid(); }
    bool needsSync() const { return m_syncStatus != SyncStatus::SYNCED; }

    static QString statutToString(Statut s);
    static Statut stringToStatut(const QString& s);
    QString getStatutLabel() const;
    int getJoursRetard() const;
    bool estEnRetard() const;
    static QString syncStatusToString(SyncStatus s);
    static SyncStatus syncStatusFromString(const QString& s);

private:
    QUuid m_creditId;
    QUuid m_venteId;
    QUuid m_clientId;
    double m_montant;
    QDate m_dateEcheance;
    Statut m_statut;
    QDate m_datePaiement;
    QString m_notes;
    int m_version = 1;
    SyncStatus m_syncStatus = SyncStatus::PENDING;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;
};

#endif // CREDIT_H
