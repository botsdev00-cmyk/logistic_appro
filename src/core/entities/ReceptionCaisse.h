#ifndef RECEPTIONCAISSE_H
#define RECEPTIONCAISSE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class ReceptionCaisse
{
public:
    enum class Statut { EnAttente, Recu, Valide, Discrepance };
    enum class SyncStatus { PENDING, SYNCED, CONFLICT };

    ReceptionCaisse();
    ~ReceptionCaisse();

    // Getters/Setters
    QUuid getReceptionCaisseId() const { return m_receptionCaisseId; }
    void setReceptionCaisseId(const QUuid& id) { m_receptionCaisseId = id; }

    QUuid getRepartitionId() const { return m_repartitionId; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }

    double getMontantAttendu() const { return m_montantAttendu; }
    void setMontantAttendu(double m) { m_montantAttendu = m; }

    double getMontantRecu() const { return m_montantRecu; }
    void setMontantRecu(double m) { m_montantRecu = m; }

    double getEcart() const { return m_montantAttendu - m_montantRecu; }

    QString getNumeroRecu() const { return m_numeroRecu; }
    void setNumeroRecu(const QString& n) { m_numeroRecu = n; }

    Statut getStatut() const { return m_statut; }
    void setStatut(Statut s) { m_statut = s; }

    QUuid getCaissierId() const { return m_caissierId; }
    void setCaissierId(const QUuid& id) { m_caissierId = id; }

    QDateTime getDateReception() const { return m_dateReception; }
    void setDateReception(const QDateTime& d) { m_dateReception = d; }

    QString getNotes() const { return m_notes; }
    void setNotes(const QString& n) { m_notes = n; }

    QDateTime getDateCreation() const { return m_dateCreation; }
    void setDateCreation(const QDateTime& d) { m_dateCreation = d; }

    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    void setDateMiseAJour(const QDateTime& d) { m_dateMiseAJour = d; }

    QDateTime getDeletedAt() const { return m_deletedAt; }
    void setDeletedAt(const QDateTime& d) { m_deletedAt = d; }

    int getVersion() const { return m_version; }
    void setVersion(int v) { m_version = v; }

    SyncStatus getSyncStatus() const { return m_syncStatus; }
    void setSyncStatus(SyncStatus s) { m_syncStatus = s; }

    // Utilitaires
    static QString statutToString(Statut);
    static Statut stringToStatut(const QString&);
    QString getStatutLabel() const;

    static QString syncStatusToString(SyncStatus);
    static SyncStatus stringToSyncStatus(const QString&);

    bool hasDiscrepancy() const { return getEcart() != 0.0; }

private:
    QUuid m_receptionCaisseId;
    QUuid m_repartitionId;
    double m_montantAttendu;
    double m_montantRecu;
    QString m_numeroRecu;
    Statut m_statut;
    QUuid m_caissierId;
    QDateTime m_dateReception;
    QString m_notes;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
    QDateTime m_deletedAt;
    int m_version;
    SyncStatus m_syncStatus;
};

#endif // RECEPTIONCAISSE_H
