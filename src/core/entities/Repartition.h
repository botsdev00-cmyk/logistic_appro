#ifndef REPARTITION_H
#define REPARTITION_H

#include <QString>
#include <QDateTime>
#include <QDate>
#include <QUuid>
#include <QList>
#include "ArticleRepartition.h"

class Repartition
{
public:
    enum class Statut { EnAttente, EnCours, Completee, Annulee };

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
    QList<ArticleRepartition> getArticles() const { return m_articles; }

    // Setters
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    void setRouteId(const QUuid& id) { m_routeId = id; }
    void setStatut(Statut s) { m_statut = s; }
    void setDateRepartition(const QDate& d) { m_dateRepartition = d; }
    void setDateRetour(const QDate& d) { m_dateRetour = d; }
    void setMontantCashAttendu(double m) { m_montantCashAttendu = m; }
    void setCreePar(const QUuid& c) { m_creePar = c; }
    void setDateCreation(const QDateTime& d) { m_dateCreation = d; }
    void setDateMiseAJour(const QDateTime& d) { m_dateMiseAJour = d; }
    void setArticles(const QList<ArticleRepartition>& a) { m_articles = a; }

    // Utilitaire
    static QString statutToString(Statut s);
    static Statut stringToStatut(const QString& str);
    QString getStatutLabel() const;

private:
    QUuid m_repartitionId;
    QUuid m_equipeId;
    QUuid m_routeId;
    Statut m_statut;
    QDate m_dateRepartition;
    QDate m_dateRetour;
    double m_montantCashAttendu = 0.0;
    QUuid m_creePar;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
    QList<ArticleRepartition> m_articles;
};

#endif // REPARTITION_H