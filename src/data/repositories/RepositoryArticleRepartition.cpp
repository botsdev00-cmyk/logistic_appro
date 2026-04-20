#include "RepositoryArticleRepartition.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>

RepositoryArticleRepartition::RepositoryArticleRepartition() {}

bool RepositoryArticleRepartition::create(const ArticleRepartition& a) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    q.prepare("INSERT INTO articles_repartition "
              "(article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau, quantite_degustation, observation) "
              "VALUES (?, ?, ?, ?, ?, ?, ?)");
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

QList<ArticleRepartition> RepositoryArticleRepartition::getByRepartitionId(const QUuid& repartitionId) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(bd.getDatabase());
    QList<ArticleRepartition> res;
    q.prepare("SELECT * FROM articles_repartition WHERE repartition_id = ?");
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