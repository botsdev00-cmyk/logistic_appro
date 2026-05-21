#pragma once

#include <QUuid>
#include <QString>
#include <QList>
#include <QDate>
#include <QDateTime>
#include "ArticleRepartition.h"

class Repartition
{
public:
    enum class Statut { EnAttente, EnCours, Completee, Annulee };

    Repartition();
    ~Repartition();

    // Enum helpers
    static QString statutToString(Statut s);
    static Statut stringToStatut(const QString& str);
    QString getStatutLabel() const;

    // Getters/Setters
    QUuid getRepartitionId() const { return m_repartitionId; }
    void setRepartitionId(const QUuid& id) { m_repartitionId = id; }

    QUuid getEquipeId() const { return m_equipeId; }
    void setEquipeId(const QUuid& id) { m_equipeId = id; }

    QUuid getRouteId() const { return m_routeId; }
    void setRouteId(const QUuid& id) { m_routeId = id; }

    QUuid getStatutRepartitionId() const { return m_statutRepartitionId; }
    void setStatutRepartitionId(const QUuid& id) { m_statutRepartitionId = id; }

    Statut getStatut() const { return m_statut; }
    void setStatut(Statut s) { m_statut = s; }

    QDate getDateRepartition() const { return m_dateRepartition; }
    void setDateRepartition(const QDate& d) { m_dateRepartition = d; }

    double getMontantCashAttendu() const { return m_montantCashAttendu; }
    void setMontantCashAttendu(double val) { m_montantCashAttendu = val; }

    QDateTime getDateMiseAJour() const { return m_dateMiseAJour; }
    void setDateMiseAJour(const QDateTime& dt) { m_dateMiseAJour = dt; }

    QUuid getChefId() const { return m_chefId; }
    void setChefId(const QUuid& id) { m_chefId = id; }

    bool getAnnule() const { return m_annule; }
    void setAnnule(bool val) { m_annule = val; }

    bool getMouvementsGeneres() const { return m_mouvementsGeneres; }
    void setMouvementsGeneres(bool val) { m_mouvementsGeneres = val; }

    // Offline-first sync fields
    QString getSyncStatus() const { return m_syncStatus; }
    void setSyncStatus(const QString& s) { m_syncStatus = s; }

    int getVersion() const { return m_version; }
    void setVersion(int v) { m_version = v; }

    QDateTime getDeletedAt() const { return m_deletedAt; }
    void setDeletedAt(const QDateTime& dt) { m_deletedAt = dt; }
    bool isDeleted() const { return !m_deletedAt.isNull(); }

    QDateTime getCreatedAt() const { return m_createdAt; }
    void setCreatedAt(const QDateTime& dt) { m_createdAt = dt; }

    QDateTime getUpdatedAt() const { return m_updatedAt; }
    void setUpdatedAt(const QDateTime& dt) { m_updatedAt = dt; }

    // Articles
    void setArticles(const QList<ArticleRepartition>& list) { m_articles = list; }
    QList<ArticleRepartition> getArticles() const { return m_articles; }

private:
    QUuid m_repartitionId;
    QUuid m_equipeId;
    QUuid m_routeId;
    QUuid m_statutRepartitionId;
    Statut m_statut;
    QDate m_dateRepartition;
    double m_montantCashAttendu;
    QDateTime m_dateMiseAJour;
    QUuid m_chefId;
    bool m_annule;
    bool m_mouvementsGeneres;

    // Offline-first sync
    QString m_syncStatus; // "PENDING", "SYNCED", ...
    int m_version;
    QDateTime m_deletedAt;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;

    // Relations
    QList<ArticleRepartition> m_articles;
};
