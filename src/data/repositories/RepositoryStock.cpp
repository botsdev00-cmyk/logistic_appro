#include "RepositoryStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryStock::RepositoryStock()
{
}

bool RepositoryStock::create(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO entrees_stock "
                  "(entree_stock_id, produit_id, quantite, source, identifiant_reference, cree_par, notes) "
                  "VALUES (:id, :prod_id, :qty, :source, :ref_id, :cree_par, :notes)");

    query.addBindValue(entity.getEntreeStockId().toString());
    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getQuantite());
    query.addBindValue(EntreeStock::sourceToString(entity.getSource()));
    query.addBindValue(entity.getIdentifiantReference().toString());
    query.addBindValue(entity.getCreePar().toString());
    query.addBindValue(entity.getNotes());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création entrée stock : " + query.lastError().text();
        return false;
    }

    return true;
}

EntreeStock RepositoryStock::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM entrees_stock WHERE entree_stock_id = :id");
    query.addBindValue(id.toString());

    EntreeStock entree;
    if (query.exec() && query.next()) {
        entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
        entree.setProduitId(QUuid(query.value("produit_id").toString()));
        entree.setQuantite(query.value("quantite").toInt());
        entree.setSource(EntreeStock::stringToSource(query.value("source").toString()));
        entree.setDate(query.value("date").toDateTime());
    } else {
        m_dernierErreur = "Entrée stock non trouvée";
    }

    return entree;
}

QList<EntreeStock> RepositoryStock::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock ORDER BY date DESC");

    if (query.exec()) {
        while (query.next()) {
            EntreeStock entree;
            entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
            entree.setProduitId(QUuid(query.value("produit_id").toString()));
            entree.setQuantite(query.value("quantite").toInt());
            entrees.append(entree);
        }
    }

    return entrees;
}

bool RepositoryStock::update(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE entrees_stock SET "
                  "quantite = :qty, source = :source, notes = :notes "
                  "WHERE entree_stock_id = :id");

    query.addBindValue(entity.getQuantite());
    query.addBindValue(EntreeStock::sourceToString(entity.getSource()));
    query.addBindValue(entity.getNotes());
    query.addBindValue(entity.getEntreeStockId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour entrée stock : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryStock::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("DELETE FROM entrees_stock WHERE entree_stock_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression entrée stock : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<EntreeStock> RepositoryStock::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT e.* FROM entrees_stock e "
                  "JOIN produits p ON e.produit_id = p.produit_id "
                  "WHERE p.nom ILIKE :criterion ORDER BY e.date DESC");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            EntreeStock entree;
            entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
        }
    }

    return entrees;
}

bool RepositoryStock::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM entrees_stock WHERE entree_stock_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

QList<EntreeStock> RepositoryStock::getByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE produit_id = :prod_id ORDER BY date DESC");
    query.addBindValue(produitId.toString());

    if (query.exec()) {
        while (query.next()) {
            EntreeStock entree;
            entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
            entree.setQuantite(query.value("quantite").toInt());
            entrees.append(entree);
        }
    }

    return entrees;
}

int RepositoryStock::getTotalQuantiteByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT SUM(quantite) as total FROM entrees_stock WHERE produit_id = :prod_id");
    query.addBindValue(produitId.toString());

    if (query.exec() && query.next()) {
        return query.value("total").toInt();
    }

    return 0;
}

QList<EntreeStock> RepositoryStock::getBySource(const EntreeStock::Source& source)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE source = :source ORDER BY date DESC");
    query.addBindValue(EntreeStock::sourceToString(source));

    if (query.exec()) {
        while (query.next()) {
            EntreeStock entree;
            entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
        }
    }

    return entrees;
}