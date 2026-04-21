#pragma once

#include <QUuid>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QList>
#include "ArticleRepartition.h"

class Repartition
{
public:
    enum class Statut { EnAttente, EnCours, Completee, Annulee };

    Repartition();
    ~Repartition();

    static QString statutToString(Statut s);
    static Statut stringToStatut(const QString& str);
    QString getStatutLabel() const;

    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    QUuid getRepartitionId() const { return m_repartitionId; }

    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    QUuid getEquipeId() const { return m_equipeId; }

    void setRouteId(const QUuid& id) { m_routeId = id; }
    QUuid getRouteId() const { return m_routeId; }

    void setStatut(Statut s) { m_statut = s; }
    Statut getStatut() const { return m_statut; }

    void setDateRepartition(const QDate& d) { m_dateRepartition = d; }
    QDate getDateRepartition() const { return m_dateRepartition; }

    void setMontantCashAttendu(double m) { m_montantCashAttendu = m; }
    double getMontantCashAttendu() const { return m_montantCashAttendu; }

    void setCreePar(const QUuid& id) { m_chefId = id; }
    QUuid getCreePar() const { return m_chefId; }

    void setDateRetour(const QDate& d) { m_dateRetour = d; }
    QDate getDateRetour() const { return m_dateRetour; }

    void setDateCreation(const QDateTime& dt) { m_dateCreation = dt; }
    QDateTime getDateCreation() const { return m_dateCreation; }

    void setDateMiseAJour(const QDateTime& dt) { m_dateMiseAJour = dt; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    void setArticles(const QList<ArticleRepartition>& a) { m_articles = a; }
    QList<ArticleRepartition> getArticles() const { return m_articles; }

private:
    QUuid m_repartitionId;
    QUuid m_equipeId;
    QUuid m_routeId;
    Statut m_statut;
    QDate m_dateRepartition;
    double m_montantCashAttendu;
    QUuid m_chefId;
    QDate m_dateRetour;
    QDateTime m_dateCreation;
    QDateTime m_dateMiseAJour;
    QList<ArticleRepartition> m_articles;
};