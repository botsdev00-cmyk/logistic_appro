#include "RepositoryProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryProduit::RepositoryProduit()
{
}

bool RepositoryProduit::create(const Produit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO produits "
                  "(produit_id, categorie_produit_id, nom, description, code_sku, prix_unitaire, stock_minimum, est_actif) "
                  "VALUES (:produit_id, :categorie_id, :nom, :description, :sku, :prix, :stock_min, :actif)");

    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getCategorieProduitId().toString());
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getCodeSku());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getStockMinimum());
    query.addBindValue(entity.estActif());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création produit : " + query.lastError().text();
        return false;
    }

    return true;
}

Produit RepositoryProduit::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM produits WHERE produit_id = :id");
    query.addBindValue(id.toString());

    Produit produit;
    if (query.exec() && query.next()) {
        produit.setProduitId(QUuid(query.value("produit_id").toString()));
        produit.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
        produit.setNom(query.value("nom").toString());
        produit.setDescription(query.value("description").toString());
        produit.setCodeSku(query.value("code_sku").toString());
        produit.setPrixUnitaire(query.value("prix_unitaire").toDouble());
        produit.setStockMinimum(query.value("stock_minimum").toInt());
        produit.setEstActif(query.value("est_actif").toBool());
    } else {
        m_dernierErreur = "Produit non trouvé";
    }

    return produit;
}

QList<Produit> RepositoryProduit::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Produit> produits;

    query.prepare("SELECT * FROM produits WHERE est_actif = true ORDER BY nom");

    if (query.exec()) {
        while (query.next()) {
            Produit produit;
            produit.setProduitId(QUuid(query.value("produit_id").toString()));
            produit.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            produit.setNom(query.value("nom").toString());
            produit.setDescription(query.value("description").toString());
            produit.setCodeSku(query.value("code_sku").toString());
            produit.setPrixUnitaire(query.value("prix_unitaire").toDouble());
            produit.setEstActif(query.value("est_actif").toBool());
            produits.append(produit);
        }
    } else {
        m_dernierErreur = "Erreur lecture produits : " + query.lastError().text();
    }

    return produits;
}

bool RepositoryProduit::update(const Produit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE produits SET "
                  "nom = :nom, description = :description, prix_unitaire = :prix, "
                  "stock_minimum = :stock_min, est_actif = :actif "
                  "WHERE produit_id = :id");

    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getStockMinimum());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getProduitId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour produit : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryProduit::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE produits SET est_actif = false WHERE produit_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression produit : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Produit> RepositoryProduit::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Produit> produits;

    query.prepare("SELECT * FROM produits WHERE "
                  "nom ILIKE :criterion OR code_sku ILIKE :criterion "
                  "ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Produit produit;
            produit.setProduitId(QUuid(query.value("produit_id").toString()));
            produit.setNom(query.value("nom").toString());
            produit.setCodeSku(query.value("code_sku").toString());
            produit.setPrixUnitaire(query.value("prix_unitaire").toDouble());
            produits.append(produit);
        }
    }

    return produits;
}

bool RepositoryProduit::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM produits WHERE produit_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

Produit RepositoryProduit::getByCodeSku(const QString& sku)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM produits WHERE code_sku = :sku");
    query.addBindValue(sku);

    Produit produit;
    if (query.exec() && query.next()) {
        produit.setProduitId(QUuid(query.value("produit_id").toString()));
        produit.setNom(query.value("nom").toString());
        produit.setCodeSku(query.value("code_sku").toString());
        produit.setPrixUnitaire(query.value("prix_unitaire").toDouble());
    }

    return produit;
}

QList<Produit> RepositoryProduit::getByCategorie(const QUuid& categorieId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Produit> produits;

    query.prepare("SELECT * FROM produits WHERE categorie_produit_id = :categorie_id AND est_actif = true ORDER BY nom");
    query.addBindValue(categorieId.toString());

    if (query.exec()) {
        while (query.next()) {
            Produit produit;
            produit.setProduitId(QUuid(query.value("produit_id").toString()));
            produit.setNom(query.value("nom").toString());
            produit.setPrixUnitaire(query.value("prix_unitaire").toDouble());
            produits.append(produit);
        }
    }

    return produits;
}