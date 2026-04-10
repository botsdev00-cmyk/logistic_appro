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
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setCreditId(const QUuid& id) { m_creditId = id; }
    void setVenteId(const QUuid& id) { m_venteId = id; }
    void setClientId(const QUuid& id) { m_clientId = id; }
    void setMontant(double montant) { m_montant = montant; }
    void setDateEcheance(const QDate& date) { m_dateEcheance = date; }
    void setStatut(Statut statut) { m_statut = statut; }
    void setDatePaiement(const QDate& date) { m_datePaiement = date; }
    void setNotes(const QString& notes) { m_notes = notes; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }

    // Utility methods
    static QString statutToString(Statut statut);
    static Statut stringToStatut(const QString& str);
    QString getStatutLabel() const;
    int getJoursRetard() const;
    bool estEnRetard() const;

private:
    QUuid m_creditId;
    QUuid m_venteId;
    QUuid m_clientId;
    double m_montant;
    QDate m_dateEcheance;
    Statut m_statut;
    QDate m_datePaiement;
    QString m_notes;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
};

#endif // CREDIT_H