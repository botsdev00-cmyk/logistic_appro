#include "RepositoryRetourStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryRetourStock::RepositoryRetourStock()
{
}

bool RepositoryRetourStock::create(const RetourStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO retours_stock "
                  "(retour_stock_id, produit_id, quantite, raison_retour_id, "
                  "repartition_id, observations, cree_par, statut_validation, date, date_mise_a_jour) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(entity.getRetourStockId().toString());
    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getRaisonRetourId().toString());
    query.addBindValue(entity.getRepartitionId().toString());
    query.addBindValue(entity.getObservations());
    query.addBindValue(entity.getCreePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDate());
    query.addBindValue(entity.getDateMiseAJour());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création retour stock: " + query.lastError().text();
        qDebug() << "[REPO-RETOUR] CREATE ERROR:" << m_dernierErreur;
        return false;
    }
    qDebug() << "[REPO-RETOUR] CREATE SUCCESS: retour_id=" << entity.getRetourStockId();
    return true;
}

RetourStock RepositoryRetourStock::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM retours_stock WHERE retour_stock_id = ?");
    query.addBindValue(id.toString());

    RetourStock retour;
    if (query.exec() && query.next()) {
        retour.setRetourStockId(QUuid(query.value("retour_stock_id").toString()));
        retour.setProduitId(QUuid(query.value("produit_id").toString()));
        retour.setQuantite(query.value("quantite").toInt());
        retour.setRaisonRetourId(QUuid(query.value("raison_retour_id").toString()));
        retour.setRepartitionId(QUuid(query.value("repartition_id").toString()));
        retour.setObservations(query.value("observations").toString());
        retour.setCreePar(QUuid(query.value("cree_par").toString()));
        retour.setApprouvePar(QUuid(query.value("approuve_par").toString()));
        retour.setStatutValidation(query.value("statut_validation").toString());
    } else {
        m_dernierErreur = "Retour stock non trouvé";
        qDebug() << "[REPO-RETOUR] getById: ID not found:" << id.toString();
    }

    return retour;
}

QList<RetourStock> RepositoryRetourStock::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<RetourStock> retours;

    query.prepare("SELECT * FROM retours_stock ORDER BY date DESC");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            RetourStock retour;
            retour.setRetourStockId(QUuid(query.value("retour_stock_id").toString()));
            retour.setProduitId(QUuid(query.value("produit_id").toString()));
            retour.setQuantite(query.value("quantite").toInt());
            retour.setRaisonRetourId(QUuid(query.value("raison_retour_id").toString()));
            retour.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            retour.setObservations(query.value("observations").toString());
            retour.setCreePar(QUuid(query.value("cree_par").toString()));
            retour.setApprouvePar(QUuid(query.value("approuve_par").toString()));
            retour.setStatutValidation(query.value("statut_validation").toString());
            retours.append(retour);
            count++;
        }
        qDebug() << "[REPO-RETOUR] getAll: returned" << count << "returns";
    } else {
        m_dernierErreur = "Erreur lecture retours stock: " + query.lastError().text();
        qDebug() << "[REPO-RETOUR] getAll ERROR:" << query.lastError().text();
    }

    return retours;
}

bool RepositoryRetourStock::update(const RetourStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE retours_stock SET "
                  "quantite = ?, observations = ?, approuve_par = ?, "
                  "statut_validation = ?, date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE retour_stock_id = ?");
    
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getObservations());
    query.addBindValue(entity.getApprouvePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getRetourStockId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour retour stock: " + query.lastError().text();
        qDebug() << "[REPO-RETOUR] UPDATE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[REPO-RETOUR] UPDATE: rows affected=" << affected;
    return affected > 0;
}

bool RepositoryRetourStock::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("DELETE FROM retours_stock WHERE retour_stock_id = ?");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression retour stock: " + query.lastError().text();
        qDebug() << "[REPO-RETOUR] REMOVE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[REPO-RETOUR] REMOVE: rows affected=" << affected;
    return affected > 0;
}

QList<RetourStock> RepositoryRetourStock::search(const QString& criterion)
{
    QList<RetourStock> all = getAll();
    QList<RetourStock> filtered;
    
    for (const auto& retour : all) {
        if (retour.getObservations().contains(criterion, Qt::CaseInsensitive)) {
            filtered.append(retour);
        }
    }
    
    return filtered;
}

bool RepositoryRetourStock::exists(const QUuid& id)
{
    return !getById(id).getRetourStockId().isNull();
}

QList<RetourStock> RepositoryRetourStock::getByStatut(const QString& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<RetourStock> retours;

    query.prepare("SELECT * FROM retours_stock WHERE statut_validation = ? ORDER BY date DESC");
    query.addBindValue(statut);

    if (query.exec()) {
        while (query.next()) {
            RetourStock retour = getById(QUuid(query.value("retour_stock_id").toString()));
            retours.append(retour);
        }
    }

    return retours;
}

QList<RetourStock> RepositoryRetourStock::getByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<RetourStock> retours;

    query.prepare("SELECT * FROM retours_stock WHERE produit_id = ? ORDER BY date DESC");
    query.addBindValue(produitId.toString());

    if (query.exec()) {
        while (query.next()) {
            RetourStock retour = getById(QUuid(query.value("retour_stock_id").toString()));
            retours.append(retour);
        }
    }

    return retours;
}

QList<RetourStock> RepositoryRetourStock::getByRepartition(const QUuid& repartitionId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<RetourStock> retours;

    query.prepare("SELECT * FROM retours_stock WHERE repartition_id = ? ORDER BY date DESC");
    query.addBindValue(repartitionId.toString());

    if (query.exec()) {
        while (query.next()) {
            RetourStock retour = getById(QUuid(query.value("retour_stock_id").toString()));
            retours.append(retour);
        }
    }

    return retours;
}

QList<RetourStock> RepositoryRetourStock::getEnAttente()
{
    return getByStatut("EN_ATTENTE");
}

bool RepositoryRetourStock::approuver(const QUuid& retourId, const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE retours_stock SET "
                  "statut_validation = 'APPROUVE', "
                  "approuve_par = ?, "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE retour_stock_id = ?");
    query.addBindValue(utilisateurId.toString());
    query.addBindValue(retourId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation retour stock: " + query.lastError().text();
        return false;
    }
    
    qDebug() << "[REPO-RETOUR] APPROUVE: retour_id=" << retourId << "par=" << utilisateurId;
    return query.numRowsAffected() > 0;
}

bool RepositoryRetourStock::rejeter(const QUuid& retourId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE retours_stock SET "
                  "statut_validation = 'REJETE', "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE retour_stock_id = ?");
    query.addBindValue(retourId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur rejet retour stock: " + query.lastError().text();
        return false;
    }
    
    qDebug() << "[REPO-RETOUR] REJETE: retour_id=" << retourId;
    return query.numRowsAffected() > 0;
}