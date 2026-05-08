#include "RepositoryArticleRepartition.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryArticleRepartition::RepositoryArticleRepartition() {}

bool RepositoryArticleRepartition::create(const ArticleRepartition& a) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    q.prepare(R"(
        INSERT INTO articles_repartition 
          (article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau, quantite_degustation, observation,
           sync_status, version, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, 'PENDING', 1, NOW(), NOW())
    )");
    q.addBindValue(a.getArticleRepartitionId().toString());
    q.addBindValue(a.getRepartitionId().toString());
    q.addBindValue(a.getProduitId().toString());
    q.addBindValue(a.getQuantiteVente());
    q.addBindValue(a.getQuantiteCadeau());
    q.addBindValue(a.getQuantiteDegustation());
    q.addBindValue(a.getObservation());
    if (!q.exec()) {
        m_dernierErreur = q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryArticleRepartition::update(const ArticleRepartition& a) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    q.prepare(R"(
        UPDATE articles_repartition SET
          quantite_vente = ?, quantite_cadeau = ?, quantite_degustation = ?, observation = ?,
          updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE article_repartition_id = ? AND deleted_at IS NULL
    )");
    q.addBindValue(a.getQuantiteVente());
    q.addBindValue(a.getQuantiteCadeau());
    q.addBindValue(a.getQuantiteDegustation());
    q.addBindValue(a.getObservation());
    q.addBindValue(a.getArticleRepartitionId().toString());
    if (!q.exec()) {
        m_dernierErreur = q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool RepositoryArticleRepartition::logicalDelete(const QUuid& id) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    q.prepare(R"(
        UPDATE articles_repartition SET deleted_at = NOW(), sync_status = 'PENDING', version = version + 1
        WHERE article_repartition_id = ? AND deleted_at IS NULL
    )");
    q.addBindValue(id.toString());
    if (!q.exec()) {
        m_dernierErreur = q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

std::optional<ArticleRepartition> RepositoryArticleRepartition::getById(const QUuid& id) const {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    q.prepare("SELECT * FROM articles_repartition WHERE article_repartition_id = ? AND deleted_at IS NULL");
    q.addBindValue(id.toString());
    if (q.exec() && q.next()) {
        ArticleRepartition a;
        a.setArticleRepartitionId(QUuid(q.value("article_repartition_id").toString()));
        a.setRepartitionId(QUuid(q.value("repartition_id").toString()));
        a.setProduitId(QUuid(q.value("produit_id").toString()));
        a.setQuantiteVente(q.value("quantite_vente").toInt());
        a.setQuantiteCadeau(q.value("quantite_cadeau").toInt());
        a.setQuantiteDegustation(q.value("quantite_degustation").toInt());
        a.setObservation(q.value("observation").toString());
        return a;
    }
    return std::nullopt;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getAll() const {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    QList<ArticleRepartition> res;
    if (q.exec("SELECT * FROM articles_repartition WHERE deleted_at IS NULL")) {
        while (q.next()) {
            ArticleRepartition a;
            a.setArticleRepartitionId(QUuid(q.value("article_repartition_id").toString()));
            a.setRepartitionId(QUuid(q.value("repartition_id").toString()));
            a.setProduitId(QUuid(q.value("produit_id").toString()));
            a.setQuantiteVente(q.value("quantite_vente").toInt());
            a.setQuantiteCadeau(q.value("quantite_cadeau").toInt());
            a.setQuantiteDegustation(q.value("quantite_degustation").toInt());
            a.setObservation(q.value("observation").toString());
            res.append(a);
        }
    }
    return res;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getByRepartitionId(const QUuid& repartitionId) const {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    QList<ArticleRepartition> res;
    q.prepare("SELECT * FROM articles_repartition WHERE repartition_id = ? AND deleted_at IS NULL");
    q.addBindValue(repartitionId.toString());
    if (q.exec()) {
        while (q.next()) {
            ArticleRepartition a;
            a.setArticleRepartitionId(QUuid(q.value("article_repartition_id").toString()));
            a.setRepartitionId(QUuid(q.value("repartition_id").toString()));
            a.setProduitId(QUuid(q.value("produit_id").toString()));
            a.setQuantiteVente(q.value("quantite_vente").toInt());
            a.setQuantiteCadeau(q.value("quantite_cadeau").toInt());
            a.setQuantiteDegustation(q.value("quantite_degustation").toInt());
            a.setObservation(q.value("observation").toString());
            res.append(a);
        }
    }
    return res;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getPendingSync() const {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    QList<ArticleRepartition> res;
    if (q.exec("SELECT * FROM articles_repartition WHERE sync_status = 'PENDING' AND deleted_at IS NULL")) {
        while (q.next()) {
            ArticleRepartition a;
            a.setArticleRepartitionId(QUuid(q.value("article_repartition_id").toString()));
            a.setRepartitionId(QUuid(q.value("repartition_id").toString()));
            a.setProduitId(QUuid(q.value("produit_id").toString()));
            a.setQuantiteVente(q.value("quantite_vente").toInt());
            a.setQuantiteCadeau(q.value("quantite_cadeau").toInt());
            a.setQuantiteDegustation(q.value("quantite_degustation").toInt());
            a.setObservation(q.value("observation").toString());
            res.append(a);
        }
    }
    return res;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getSinceVersion(int minVersion) const {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    QList<ArticleRepartition> res;
    q.prepare("SELECT * FROM articles_repartition WHERE version >= ? AND deleted_at IS NULL");
    q.addBindValue(minVersion);
    if (q.exec()) {
        while (q.next()) {
            ArticleRepartition a;
            a.setArticleRepartitionId(QUuid(q.value("article_repartition_id").toString()));
            a.setRepartitionId(QUuid(q.value("repartition_id").toString()));
            a.setProduitId(QUuid(q.value("produit_id").toString()));
            a.setQuantiteVente(q.value("quantite_vente").toInt());
            a.setQuantiteCadeau(q.value("quantite_cadeau").toInt());
            a.setQuantiteDegustation(q.value("quantite_degustation").toInt());
            a.setObservation(q.value("observation").toString());
            res.append(a);
        }
    }
    return res;
}