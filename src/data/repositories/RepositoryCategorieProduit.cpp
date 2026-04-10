#include "RepositoryCategorieProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryCategorieProduit::RepositoryCategorieProduit()
{
}

bool RepositoryCategorieProduit::create(const CategorieProduit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO categories_produits "
                  "(categorie_produit_id, nom, description, code_categorie, est_actif, ordre_affichage) "
                  "VALUES (:id, :nom, :description, :code, :actif, :ordre)");

    query.addBindValue(entity.getCategorieProduitId().toString());
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getCodeCategorie());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getOrdreAffichage());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création catégorie : " + query.lastError().text();
        return false;
    }

    return true;
}

CategorieProduit RepositoryCategorieProduit::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM categories_produits WHERE categorie_produit_id = :id");
    query.addBindValue(id.toString());

    CategorieProduit categorie;
    if (query.exec() && query.next()) {
        categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
        categorie.setNom(query.value("nom").toString());
        categorie.setDescription(query.value("description").toString());
        categorie.setCodeCategorie(query.value("code_categorie").toString());
        categorie.setEstActif(query.value("est_actif").toBool());
        categorie.setOrdreAffichage(query.value("ordre_affichage").toInt());
    } else {
        m_dernierErreur = "Catégorie non trouvée";
    }

    return categorie;
}

QList<CategorieProduit> RepositoryCategorieProduit::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    query.prepare("SELECT * FROM categories_produits WHERE est_actif = true ORDER BY ordre_affichage");

    if (query.exec()) {
        while (query.next()) {
            CategorieProduit categorie;
            categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            categorie.setNom(query.value("nom").toString());
            categorie.setDescription(query.value("description").toString());
            categorie.setCodeCategorie(query.value("code_categorie").toString());
            categorie.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(categorie);
        }
    } else {
        m_dernierErreur = "Erreur lecture catégories : " + query.lastError().text();
    }

    return categories;
}

bool RepositoryCategorieProduit::update(const CategorieProduit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE categories_produits SET "
                  "nom = :nom, description = :description, ordre_affichage = :ordre, est_actif = :actif "
                  "WHERE categorie_produit_id = :id");

    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getOrdreAffichage());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getCategorieProduitId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour catégorie : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryCategorieProduit::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE categories_produits SET est_actif = false WHERE categorie_produit_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression catégorie : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<CategorieProduit> RepositoryCategorieProduit::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    query.prepare("SELECT * FROM categories_produits WHERE nom ILIKE :criterion ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            CategorieProduit categorie;
            categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            categorie.setNom(query.value("nom").toString());
            categories.append(categorie);
        }
    }

    return categories;
}

bool RepositoryCategorieProduit::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM categories_produits WHERE categorie_produit_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

CategorieProduit RepositoryCategorieProduit::getByCode(const QString& code)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM categories_produits WHERE code_categorie = :code");
    query.addBindValue(code);

    CategorieProduit categorie;
    if (query.exec() && query.next()) {
        categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
        categorie.setNom(query.value("nom").toString());
        categorie.setCodeCategorie(query.value("code_categorie").toString());
    }

    return categorie;
}