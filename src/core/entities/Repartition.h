#ifndef REPARTITION_H
#define REPARTITION_H

#include <QString>
#include <QDateTime>
#include <QDate>
#include <QUuid>

class Repartition
{
public:
    enum class Statut {
        EnAttente,
        EnCours,
        Completee,
        Annulee
    };

    Repartition();
    ~Repartition();

    // Getters
    QUuid getRepartitionId() const { return m_repartitionId; }
    QUuid getEquipeId() const { return m_equipeId; }
    QUuid getRouteId() const { return m_routeId; }
    Statut getStatut() const { return m_statut; }
    QDate getDateRepartition() const { return m_dateRepartition; }
    QDate getDateRetour() const { return m_dateRetour; }
    double getMontantCashAttendu() const { return m_montantCashAttendu; }
    QUuid getCreePar() const { return m_creePar; }
    QDateTime getDateCreation() const { return m_dateCreation; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    // Setters
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    void setRouteId(const QUuid& id) { m_routeId = id; }
    void setStatut(Statut statut) { m_statut = statut; }
    void setDateRepartition(const QDate& date) { m_dateRepartition = date; }
    void setDateRetour(const QDate& date) { m_dateRetour = date; }
    void setMontantCashAttendu(double montant) { m_montantCashAttendu = montant; }
    void setCreePar(const QUuid& id) { m_creePar = id; }
    void setDateCreation(const QDateTime& date) { m_dateCreation = date; }
    void setDateMiseAJour(const QDateTime& date) { m_dateMiseAJour = date; }

    // Utility methods
    static QString statutToString(Statut statut);
    static Statut stringToStatut(const QString& str);
    QString getStatutLabel() const;

private:
    QUuid m_repartitionId;
    QUuid m_equipeId;
    QUuid m_routeId;
    Statut m_statut;
    QDate m_dateRepartition;
    QDate m_dateRetour;
    double m_montantCashAttendu;
    QUuid m_creePar;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
};

#endif // REPARTITION_H