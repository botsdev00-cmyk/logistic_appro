#include "RepositoryCategorieProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

RepositoryCategorieProduit::RepositoryCategorieProduit()
{
}

bool RepositoryCategorieProduit::create(const CategorieProduit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO categories_produits "
                  "(categorie_produit_id, nom, description, code_categorie, est_actif, ordre_affichage) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(entity.getCategorieProduitId().toString());
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getCodeCategorie());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getOrdreAffichage());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création catégorie : " + query.lastError().text();
        qDebug() << "[DEBUG-REPO-CAT] CREATE ERROR:" << m_dernierErreur;
        return false;
    }
    qDebug() << "[DEBUG-REPO-CAT] CREATE SUCCESS: categorie_id=" << entity.getCategorieProduitId();
    return true;
}

CategorieProduit RepositoryCategorieProduit::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM categories_produits WHERE categorie_produit_id = ?");
    query.addBindValue(id.toString());

    CategorieProduit categorie;
    
    // CORRECTION : Forcer l'ID à Null pour indiquer "Non trouvé" par défaut
    categorie.setCategorieProduitId(QUuid());

    if (query.exec() && query.next()) {
        categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
        categorie.setNom(query.value("nom").toString());
        categorie.setDescription(query.value("description").toString());
        categorie.setCodeCategorie(query.value("code_categorie").toString());
        categorie.setEstActif(query.value("est_actif").toBool());
        categorie.setOrdreAffichage(query.value("ordre_affichage").toInt());
    } else {
        m_dernierErreur = "Catégorie non trouvée";
        qDebug() << "[DEBUG-REPO-CAT] getById: ID not found:" << id.toString();
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
        int count = 0;
        while (query.next()) {
            CategorieProduit categorie;
            categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            categorie.setNom(query.value("nom").toString());
            categorie.setDescription(query.value("description").toString());
            categorie.setCodeCategorie(query.value("code_categorie").toString());
            categorie.setEstActif(query.value("est_actif").toBool());
            categorie.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(categorie);
            count++;
        }
        qDebug() << "[DEBUG-REPO-CAT] getAll: returned" << count << "categories";
    } else {
        m_dernierErreur = "Erreur lecture catégories : " + query.lastError().text();
        qDebug() << "[DEBUG-REPO-CAT] getAll ERROR:" << query.lastError().text();
    }

    return categories;
}

bool RepositoryCategorieProduit::update(const CategorieProduit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE categories_produits SET "
                  "nom = ?, description = ?, ordre_affichage = ?, est_actif = ? "
                  "WHERE categorie_produit_id = ?");
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getOrdreAffichage());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getCategorieProduitId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour catégorie : " + query.lastError().text();
        qDebug() << "[DEBUG-REPO-CAT] UPDATE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[DEBUG-REPO-CAT] UPDATE: rows affected=" << affected;
    return affected > 0;
}

bool RepositoryCategorieProduit::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE categories_produits SET est_actif = false WHERE categorie_produit_id = ?");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression catégorie : " + query.lastError().text();
        qDebug() << "[DEBUG-REPO-CAT] REMOVE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[DEBUG-REPO-CAT] REMOVE: rows affected=" << affected;
    return affected > 0;
}

QList<CategorieProduit> RepositoryCategorieProduit::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    query.prepare("SELECT * FROM categories_produits WHERE nom ILIKE ? AND est_actif = true ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            CategorieProduit categorie;
            categorie.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            categorie.setNom(query.value("nom").toString());
            categorie.setDescription(query.value("description").toString());
            categorie.setCodeCategorie(query.value("code_categorie").toString());
            categorie.setEstActif(query.value("est_actif").toBool());
            categorie.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(categorie);
            count++;
        }
        qDebug() << "[DEBUG-REPO-CAT] search: criterion='" << criterion << "' returned" << count << "results";
    } else {
        qDebug() << "[DEBUG-REPO-CAT] search ERROR:" << query.lastError().text();
    }

    return categories;
}

bool RepositoryCategorieProduit::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM categories_produits WHERE categorie_produit_id = ?");
    query.addBindValue(id.toString());

    bool result = query.exec() && query.next();
    qDebug() << "[DEBUG-REPO-CAT] exists: id=" << id.toString() << "result=" << result;
    return result;
}

CategorieProduit RepositoryCategorieProduit::getByCode(const QString& code)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    QString codeTrim = code.trimmed();
    qDebug() << "\n[DEBUG-REPO-CAT] ========== getByCode START ==========";
    qDebug() << "[DEBUG-REPO-CAT] Input code:" << codeTrim;

    query.prepare("SELECT * FROM categories_produits WHERE code_categorie ILIKE ? AND est_actif = true LIMIT 1");
    query.addBindValue(codeTrim);

    CategorieProduit categorie;
    
    // CORRECTION : Forcer l'ID à Null pour indiquer "Non trouvé" par défaut
    categorie.setCategorieProduitId(QUuid()); 

    if (!query.exec()) {
        qDebug() << "[DEBUG-REPO-CAT] ❌ SQL EXECUTION FAILED";
        qDebug() << "[DEBUG-REPO-CAT] Error:" << query.lastError().text();
        return categorie;
    }

    if (query.next()) {
        QString codeDb = query.value("code_categorie").toString();
        QString idDb = query.value("categorie_produit_id").toString();
        
        categorie.setCategorieProduitId(QUuid(idDb)); // Écrase le Null par l'ID réel
        categorie.setNom(query.value("nom").toString());
        categorie.setDescription(query.value("description").toString());
        categorie.setCodeCategorie(codeDb);
        categorie.setEstActif(query.value("est_actif").toBool());
        categorie.setOrdreAffichage(query.value("ordre_affichage").toInt());
        
        qDebug() << "[DEBUG-REPO-CAT] ✓ FOUND IN DATABASE, id_db:" << idDb;
    } else {
        qDebug() << "[DEBUG-REPO-CAT] ❌ NO ROWS RETURNED from database";
    }
    
    qDebug() << "[DEBUG-REPO-CAT] ========== getByCode END ==========\n";
    return categorie;
}