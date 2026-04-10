#ifndef RECEPTIONCAISSE_H
#define RECEPTIONCAISSE_H

#include <QString>
#include <QDateTime>
#include <QUuid>

class ReceptionCaisse
{
public:
    enum class Statut {
        EnAttente,
        Recu,
        Valide,
        Discrepance
    };

    ReceptionCaisse();
    ~ReceptionCaisse();

    // Getters
    QUuid getReceptionCaisseId() const { return m_receptionCaisseId; }
    QUuid getRepartitionId() const { return m_repartitionId; }
    double getMontantAttendu() const { return m_montantAttendu; }
    double getMontantRecu() const { return m_montantRecu; }
    double getEcart() const { return m_montantAttendu - m_montantRecu; }
    QString getNumeroRecu() const { return m_numeroRecu; }
    Statut getStatut() const { return m_statut; }
    QUuid getCaissierId() const { return m_caissierId; }
    QDateTime getDateReception() const { return m_dateReception; }
    QString getNotes() const { return m_notes; }
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setReceptionCaisseId(const QUuid& id) { m_receptionCaisseId = id; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setMontantAttendu(double montant) { m_montantAttendu = montant; }
    void setMontantRecu(double montant) { m_montantRecu = montant; }
    void setNumeroRecu(const QString& numero) { m_numeroRecu = numero; }
    void setStatut(Statut statut) { m_statut = statut; }
    void setCaissierId(const QUuid& id) { m_caissierId = id; }
    void setDateReception(const QDateTime& date) { m_dateReception = date; }
    void setNotes(const QString& notes) { m_notes = notes; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }

    // Utility methods
    static QString statutToString(Statut statut);
    static Statut stringToStatut(const QString& str);
    QString getStatutLabel() const;
    bool hasDiscrepancy() const;

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
};

#endif // RECEPTIONCAISSE_H