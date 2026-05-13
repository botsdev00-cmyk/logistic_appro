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

    // Identifiants
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }
    QUuid getRepartitionId() const { return m_repartitionId; }

    void setEquipeId(const QUuid& id) { m_equipeId = id; }
    QUuid getEquipeId() const { return m_equipeId; }

    void setRouteId(const QUuid& id) { m_routeId = id; }
    QUuid getRouteId() const { return m_routeId; }

    void setStatutRepartitionId(const QUuid& id) { m_statutRepartitionId = id; }
    QUuid getStatutRepartitionId() const { return m_statutRepartitionId; }

    void setStatut(Statut s) { m_statut = s; }
    Statut getStatut() const { return m_statut; }

    void setDateRepartition(const QDate& d) { m_dateRepartition = d; }
    QDate getDateRepartition() const { return m_dateRepartition; }

    void setMontantCashAttendu(double m) { m_montantCashAttendu = m; }
    double getMontantCashAttendu() const { return m_montantCashAttendu; }

    // Chef d'équipe
    void setChefId(const QUuid& id) { m_chefId = id; }
    QUuid getChefId() const { return m_chefId; }

    // Créé par (audit)
    void setCreePar(const QUuid& id) { m_creePar = id; }
    QUuid getCreePar() const { return m_creePar; }

    void setAnnule(bool v) { m_annule = v; }
    bool getAnnule() const { return m_annule; }

    void setDateRetour(const QDate& d) { m_dateRetour = d; }
    QDate getDateRetour() const { return m_dateRetour; }

    // Champs pour audit/synchronisation
    void setDateMiseAJour(const QDateTime& dt) { m_dateMiseAJour = dt; }
    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }

    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }
    QDateTime getCreatedAt() const { return m_createdAt; }

    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }

    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    QDateTime getDeletedAt() const { return m_deletedAt; }

    void setArticles(const QList<ArticleRepartition>& a) { m_articles = a; }
    QList<ArticleRepartition> getArticles() const { return m_articles; }

private:
    QUuid m_repartitionId;
    QUuid m_equipeId;
    QUuid m_routeId;
    QUuid m_statutRepartitionId;
    Statut m_statut;
    QDate m_dateRepartition;
    double m_montantCashAttendu;
    QUuid m_chefId;
    QUuid m_creePar;
    bool m_annule = false;
    QDate m_dateRetour;
    QDateTime m_dateMiseAJour;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QDateTime m_deletedAt;

    QList<ArticleRepartition> m_articles;
};
