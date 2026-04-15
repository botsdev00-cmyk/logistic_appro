#include "RepositoryEntreeStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryEntreeStock::RepositoryEntreeStock()
{
}

bool RepositoryEntreeStock::create(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO entrees_stock "
                  "(entree_stock_id, produit_id, quantite, source_entree_id, "
                  "numero_facture, prix_unitaire, numero_lot, date_expiration, "
                  "cree_par, statut_validation, date, date_mise_a_jour) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(entity.getEntreeStockId().toString());
    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getSourceEntreeId().toString());
    query.addBindValue(entity.getNumeroFacture());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(entity.getCreePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDate());
    query.addBindValue(entity.getDateMiseAJour());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création entrée stock: " + query.lastError().text();
        qDebug() << "[REPO-ENTREE] CREATE ERROR:" << m_dernierErreur;
        return false;
    }
    qDebug() << "[REPO-ENTREE] CREATE SUCCESS: entree_id=" << entity.getEntreeStockId();
    return true;
}

EntreeStock RepositoryEntreeStock::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM entrees_stock WHERE entree_stock_id = ?");
    query.addBindValue(id.toString());

    EntreeStock entree;
    if (query.exec() && query.next()) {
        entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
        entree.setProduitId(QUuid(query.value("produit_id").toString()));
        entree.setQuantite(query.value("quantite").toInt());
        entree.setSourceEntreeId(QUuid(query.value("source_entree_id").toString()));
        entree.setNumeroFacture(query.value("numero_facture").toString());
        entree.setPrixUnitaire(query.value("prix_unitaire").toDouble());
        entree.setNumeroLot(query.value("numero_lot").toString());
        entree.setDateExpiration(query.value("date_expiration").toDate());
        entree.setCreePar(QUuid(query.value("cree_par").toString()));
        entree.setApprouvePar(QUuid(query.value("approuve_par").toString()));
        entree.setStatutValidation(query.value("statut_validation").toString());
    } else {
        m_dernierErreur = "Entrée stock non trouvée";
        qDebug() << "[REPO-ENTREE] getById: ID not found:" << id.toString();
    }

    return entree;
}

QList<EntreeStock> RepositoryEntreeStock::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock ORDER BY date DESC");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree;
            entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
            entree.setProduitId(QUuid(query.value("produit_id").toString()));
            entree.setQuantite(query.value("quantite").toInt());
            entree.setSourceEntreeId(QUuid(query.value("source_entree_id").toString()));
            entree.setNumeroFacture(query.value("numero_facture").toString());
            entree.setPrixUnitaire(query.value("prix_unitaire").toDouble());
            entree.setNumeroLot(query.value("numero_lot").toString());
            entree.setDateExpiration(query.value("date_expiration").toDate());
            entree.setCreePar(QUuid(query.value("cree_par").toString()));
            entree.setApprouvePar(QUuid(query.value("approuve_par").toString()));
            entree.setStatutValidation(query.value("statut_validation").toString());
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-ENTREE] getAll: returned" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture entrées stock: " + query.lastError().text();
        qDebug() << "[REPO-ENTREE] getAll ERROR:" << query.lastError().text();
    }

    return entrees;
}

bool RepositoryEntreeStock::update(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE entrees_stock SET "
                  "quantite = ?, prix_unitaire = ?, numero_lot = ?, "
                  "date_expiration = ?, approuve_par = ?, statut_validation = ?, "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE entree_stock_id = ?");
    
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(entity.getApprouvePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getEntreeStockId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour entrée stock: " + query.lastError().text();
        qDebug() << "[REPO-ENTREE] UPDATE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[REPO-ENTREE] UPDATE: rows affected=" << affected;
    return affected > 0;
}

bool RepositoryEntreeStock::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("DELETE FROM entrees_stock WHERE entree_stock_id = ?");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression entrée stock: " + query.lastError().text();
        qDebug() << "[REPO-ENTREE] REMOVE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[REPO-ENTREE] REMOVE: rows affected=" << affected;
    return affected > 0;
}

QList<EntreeStock> RepositoryEntreeStock::search(const QString& criterion)
{
    QList<EntreeStock> all = getAll();
    QList<EntreeStock> filtered;
    
    for (const auto& entree : all) {
        if (entree.getNumeroFacture().contains(criterion, Qt::CaseInsensitive) ||
            entree.getNumeroLot().contains(criterion, Qt::CaseInsensitive)) {
            filtered.append(entree);
        }
    }
    
    return filtered;
}

bool RepositoryEntreeStock::exists(const QUuid& id)
{
    return !getById(id).getEntreeStockId().isNull();
}

QList<EntreeStock> RepositoryEntreeStock::getByStatut(const QString& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE statut_validation = ? ORDER BY date DESC");
    query.addBindValue(statut);

    if (query.exec()) {
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
        }
    }

    return entrees;
}

QList<EntreeStock> RepositoryEntreeStock::getByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE produit_id = ? ORDER BY date DESC");
    query.addBindValue(produitId.toString());

    if (query.exec()) {
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
        }
    }

    return entrees;
}

QList<EntreeStock> RepositoryEntreeStock::getEnAttente()
{
    return getByStatut("EN_ATTENTE");
}

bool RepositoryEntreeStock::approuver(const QUuid& entreeId, const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE entrees_stock SET "
                  "statut_validation = 'APPROUVE', "
                  "approuve_par = ?, "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE entree_stock_id = ?");
    query.addBindValue(utilisateurId.toString());
    query.addBindValue(entreeId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation entrée stock: " + query.lastError().text();
        return false;
    }
    
    qDebug() << "[REPO-ENTREE] APPROUVE: entree_id=" << entreeId << "par=" << utilisateurId;
    return query.numRowsAffected() > 0;
}

bool RepositoryEntreeStock::rejeter(const QUuid& entreeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE entrees_stock SET "
                  "statut_validation = 'REJETE', "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE entree_stock_id = ?");
    query.addBindValue(entreeId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur rejet entrée stock: " + query.lastError().text();
        return false;
    }
    
    qDebug() << "[REPO-ENTREE] REJETE: entree_id=" << entreeId;
    return query.numRowsAffected() > 0;
}