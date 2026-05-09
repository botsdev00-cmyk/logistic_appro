#include "RepositoryArticleRepartition.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryArticleRepartition::RepositoryArticleRepartition()
{
}

ArticleRepartition RepositoryArticleRepartition::mapRowToArticle(const QSqlQuery& query) const
{
    ArticleRepartition article;
    article.setArticleRepartitionId(QUuid(query.value("article_repartition_id").toString()));
    article.setRepartitionId(QUuid(query.value("repartition_id").toString()));
    article.setProduitId(QUuid(query.value("produit_id").toString()));
    article.setQuantiteVente(query.value("quantite_vente").toInt());
    article.setQuantiteCadeau(query.value("quantite_cadeau").toInt());
    article.setQuantiteDegustation(query.value("quantite_degustation").toInt());
    article.setObservation(query.value("observation").toString());
    
    // Sync fields
    article.setSyncStatus(ArticleRepartition::stringToSyncStatus(query.value("sync_status").toString()));
    article.setVersion(query.value("version").toInt());
    article.setCreatedAt(query.value("created_at").toDateTime());
    article.setUpdatedAt(query.value("updated_at").toDateTime());
    article.setDeletedAt(query.value("deleted_at").toDateTime());
    article.setCreatedBy(QUuid(query.value("created_by").toString()));
    article.setUpdatedBy(QUuid(query.value("updated_by").toString()));
    
    return article;
}

bool RepositoryArticleRepartition::create(const ArticleRepartition& article)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "INSERT INTO articles_repartition "
        "(article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau, "
        "quantite_degustation, observation, sync_status, version, created_at, updated_at, "
        "created_by, updated_by) "
        "VALUES (:id, :repId, :prodId, :qtVente, :qtCadeau, :qtDegust, :obs, "
        ":syncStatus, :version, :createdAt, :updatedAt, :createdBy, :updatedBy)"
    );

    query.addBindValue(article.getArticleRepartitionId().toString(QUuid::WithoutBraces));
    query.addBindValue(article.getRepartitionId().toString(QUuid::WithoutBraces));
    query.addBindValue(article.getProduitId().toString(QUuid::WithoutBraces));
    query.addBindValue(article.getQuantiteVente());
    query.addBindValue(article.getQuantiteCadeau());
    query.addBindValue(article.getQuantiteDegustation());
    query.addBindValue(article.getObservation());
    query.addBindValue(article.syncStatusString());
    query.addBindValue(article.getVersion());
    query.addBindValue(article.getCreatedAt());
    query.addBindValue(article.getUpdatedAt());
    query.addBindValue(article.getCreatedBy().toString(QUuid::WithoutBraces));
    query.addBindValue(article.getUpdatedBy().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur création article repartition : " + query.lastError().text();
        qDebug() << "RepositoryArticleRepartition::create error:" << m_dernierErreur;
        return false;
    }

    qDebug() << "RepositoryArticleRepartition::create success";
    return true;
}

bool RepositoryArticleRepartition::update(const ArticleRepartition& article)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "UPDATE articles_repartition SET "
        "quantite_vente = :qtVente, quantite_cadeau = :qtCadeau, "
        "quantite_degustation = :qtDegust, observation = :obs, "
        "sync_status = :syncStatus, version = :version, "
        "updated_at = :updatedAt, updated_by = :updatedBy "
        "WHERE article_repartition_id = :id AND deleted_at IS NULL"
    );

    query.addBindValue(article.getQuantiteVente());
    query.addBindValue(article.getQuantiteCadeau());
    query.addBindValue(article.getQuantiteDegustation());
    query.addBindValue(article.getObservation());
    query.addBindValue(article.syncStatusString());
    query.addBindValue(article.getVersion());
    query.addBindValue(article.getUpdatedAt());
    query.addBindValue(article.getUpdatedBy().toString(QUuid::WithoutBraces));
    query.addBindValue(article.getArticleRepartitionId().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour article repartition : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryArticleRepartition::remove(const QUuid& articleId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    // Soft delete
    query.prepare(
        "UPDATE articles_repartition SET "
        "deleted_at = NOW(), sync_status = :syncStatus, version = version + 1 "
        "WHERE article_repartition_id = :id AND deleted_at IS NULL"
    );
    query.addBindValue("PENDING");
    query.addBindValue(articleId.toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression article repartition : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

ArticleRepartition RepositoryArticleRepartition::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE article_repartition_id = :id AND deleted_at IS NULL"
    );
    query.addBindValue(id.toString(QUuid::WithoutBraces));

    ArticleRepartition article;
    if (query.exec() && query.next()) {
        article = mapRowToArticle(query);
    }

    return article;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getAll() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ArticleRepartition> articles;

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE deleted_at IS NULL ORDER BY created_at DESC"
    );

    if (query.exec()) {
        while (query.next()) {
            articles.append(mapRowToArticle(query));
        }
    }

    return articles;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getByRepartitionId(const QUuid& repartitionId) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ArticleRepartition> articles;

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE repartition_id = :repId AND deleted_at IS NULL "
        "ORDER BY created_at DESC"
    );
    query.addBindValue(repartitionId.toString(QUuid::WithoutBraces));

    if (query.exec()) {
        while (query.next()) {
            articles.append(mapRowToArticle(query));
        }
    }

    return articles;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getPendingSync() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ArticleRepartition> articles;

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE sync_status = 'PENDING' AND deleted_at IS NULL "
        "ORDER BY updated_at ASC"
    );

    if (query.exec()) {
        while (query.next()) {
            articles.append(mapRowToArticle(query));
        }
    }

    return articles;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getSyncedArticles() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ArticleRepartition> articles;

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE sync_status = 'SYNCED' AND deleted_at IS NULL "
        "ORDER BY updated_at DESC"
    );

    if (query.exec()) {
        while (query.next()) {
            articles.append(mapRowToArticle(query));
        }
    }

    return articles;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getConflictSync() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ArticleRepartition> articles;

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE sync_status = 'CONFLICT' AND deleted_at IS NULL "
        "ORDER BY updated_at ASC"
    );

    if (query.exec()) {
        while (query.next()) {
            articles.append(mapRowToArticle(query));
        }
    }

    return articles;
}

QList<ArticleRepartition> RepositoryArticleRepartition::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ArticleRepartition> articles;

    query.prepare(
        "SELECT article_repartition_id, repartition_id, produit_id, quantite_vente, "
        "quantite_cadeau, quantite_degustation, observation, sync_status, version, "
        "created_at, updated_at, deleted_at, created_by, updated_by "
        "FROM articles_repartition WHERE version >= :minVer AND deleted_at IS NULL "
        "ORDER BY version ASC"
    );
    query.addBindValue(minVersion);

    if (query.exec()) {
        while (query.next()) {
            articles.append(mapRowToArticle(query));
        }
    }

    return articles;
}

int RepositoryArticleRepartition::getPendingCount() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT COUNT(*) as count FROM articles_repartition "
        "WHERE sync_status IN ('PENDING', 'CONFLICT') AND deleted_at IS NULL"
    );

    if (query.exec() && query.next()) {
        return query.value("count").toInt();
    }
    return 0;
}

QList<RepositoryArticleRepartition::SyncResult> RepositoryArticleRepartition::syncBatch(
    const QList<ArticleRepartition>& articles, const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QList<SyncResult> results;

    for (const ArticleRepartition& article : articles) {
        QSqlQuery query(bd.getDatabase());
        
        // Vérifier version et conflit
        query.prepare(
            "SELECT version FROM articles_repartition "
            "WHERE article_repartition_id = :id AND deleted_at IS NULL"
        );
        query.addBindValue(article.getArticleRepartitionId().toString(QUuid::WithoutBraces));
        
        SyncResult res;
        res.articleId = article.getArticleRepartitionId();
        res.newVersion = article.getVersion();
        
        if (query.exec() && query.next()) {
            int serverVersion = query.value(0).toInt();
            if (serverVersion > article.getVersion()) {
                // Conflit : version serveur plus récente
                res.success = false;
                res.message = QString("CONFLICT: Server v%1 > Client v%2")
                    .arg(serverVersion).arg(article.getVersion());
                res.newVersion = serverVersion;
                results.append(res);
                continue;
            }
        }
        
        // Upsert
        QSqlQuery upsertQuery(bd.getDatabase());
        upsertQuery.prepare(
            "INSERT INTO articles_repartition "
            "(article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau, "
            "quantite_degustation, observation, sync_status, version, created_at, updated_at, "
            "created_by, updated_by) "
            "VALUES (:id, :repId, :prodId, :qtVente, :qtCadeau, :qtDegust, :obs, "
            "'SYNCED', :version, :createdAt, NOW(), :createdBy, :updatedBy) "
            "ON CONFLICT (article_repartition_id) DO UPDATE SET "
            "quantite_vente = EXCLUDED.quantite_vente, "
            "quantite_cadeau = EXCLUDED.quantite_cadeau, "
            "quantite_degustation = EXCLUDED.quantite_degustation, "
            "observation = EXCLUDED.observation, "
            "sync_status = 'SYNCED', version = :version, updated_at = NOW(), updated_by = :updatedBy"
        );
        upsertQuery.addBindValue(article.getArticleRepartitionId().toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(article.getRepartitionId().toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(article.getProduitId().toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(article.getQuantiteVente());
        upsertQuery.addBindValue(article.getQuantiteCadeau());
        upsertQuery.addBindValue(article.getQuantiteDegustation());
        upsertQuery.addBindValue(article.getObservation());
        upsertQuery.addBindValue(article.getVersion() + 1);
        upsertQuery.addBindValue(article.getCreatedAt());
        upsertQuery.addBindValue(article.getCreatedBy().toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(utilisateurId.toString(QUuid::WithoutBraces));
        
        if (upsertQuery.exec()) {
            res.success = true;
            res.message = "Synced successfully";
            res.newVersion = article.getVersion() + 1;
        } else {
            res.success = false;
            res.message = "Sync failed: " + upsertQuery.lastError().text();
        }
        
        results.append(res);
    }

    return results;
}
