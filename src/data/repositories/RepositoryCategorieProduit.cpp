#include "RepositoryCategorieProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

RepositoryCategorieProduit::RepositoryCategorieProduit() {}

bool RepositoryCategorieProduit::create(const CategorieProduit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        INSERT INTO categories_produits 
        (categorie_produit_id, nom, description, code_categorie, est_actif, ordre_affichage, 
         sync_status, version, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, 'PENDING', 1, NOW(), NOW())
    )");
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

bool RepositoryCategorieProduit::update(const CategorieProduit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE categories_produits SET
            nom = ?, description = ?, code_categorie = ?,
            ordre_affichage = ?, est_actif = ?,
            updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE categorie_produit_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.getCodeCategorie());
    query.addBindValue(entity.getOrdreAffichage());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getCategorieProduitId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour catégorie : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool RepositoryCategorieProduit::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE categories_produits 
        SET deleted_at = NOW(), sync_status = 'PENDING', version = version + 1
        WHERE categorie_produit_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression catégorie : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

std::optional<CategorieProduit> RepositoryCategorieProduit::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM categories_produits WHERE categorie_produit_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());

    if (query.exec() && query.next()) {
        CategorieProduit c;
        c.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
        c.setNom(query.value("nom").toString());
        c.setDescription(query.value("description").toString());
        c.setCodeCategorie(query.value("code_categorie").toString());
        c.setEstActif(query.value("est_actif").toBool());
        c.setOrdreAffichage(query.value("ordre_affichage").toInt());
        return c;
    }
    return std::nullopt;
}

QList<CategorieProduit> RepositoryCategorieProduit::getAll() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    if (query.exec("SELECT * FROM categories_produits WHERE deleted_at IS NULL ORDER BY ordre_affichage")) {
        while (query.next()) {
            CategorieProduit c;
            c.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            c.setNom(query.value("nom").toString());
            c.setDescription(query.value("description").toString());
            c.setCodeCategorie(query.value("code_categorie").toString());
            c.setEstActif(query.value("est_actif").toBool());
            c.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(c);
        }
    }
    return categories;
}

QList<CategorieProduit> RepositoryCategorieProduit::search(const QString& criterion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    query.prepare("SELECT * FROM categories_produits WHERE nom ILIKE ? AND deleted_at IS NULL ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            CategorieProduit c;
            c.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            c.setNom(query.value("nom").toString());
            c.setDescription(query.value("description").toString());
            c.setCodeCategorie(query.value("code_categorie").toString());
            c.setEstActif(query.value("est_actif").toBool());
            c.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(c);
        }
    }
    return categories;
}

bool RepositoryCategorieProduit::exists(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM categories_produits WHERE categorie_produit_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

std::optional<CategorieProduit> RepositoryCategorieProduit::getByCode(const QString& code) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    QString codeTrim = code.trimmed();
    query.prepare("SELECT * FROM categories_produits WHERE code_categorie ILIKE ? AND deleted_at IS NULL LIMIT 1");
    query.addBindValue(codeTrim);

    if (query.exec() && query.next()) {
        CategorieProduit c;
        c.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
        c.setNom(query.value("nom").toString());
        c.setDescription(query.value("description").toString());
        c.setCodeCategorie(query.value("code_categorie").toString());
        c.setEstActif(query.value("est_actif").toBool());
        c.setOrdreAffichage(query.value("ordre_affichage").toInt());
        return c;
    }
    return std::nullopt;
}

QList<CategorieProduit> RepositoryCategorieProduit::getPendingSync() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    if (query.exec("SELECT * FROM categories_produits WHERE sync_status = 'PENDING' AND deleted_at IS NULL")) {
        while (query.next()) {
            CategorieProduit c;
            c.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            c.setNom(query.value("nom").toString());
            c.setDescription(query.value("description").toString());
            c.setCodeCategorie(query.value("code_categorie").toString());
            c.setEstActif(query.value("est_actif").toBool());
            c.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(c);
        }
    }
    return categories;
}

QList<CategorieProduit> RepositoryCategorieProduit::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<CategorieProduit> categories;

    query.prepare("SELECT * FROM categories_produits WHERE version >= ? AND deleted_at IS NULL");
    query.addBindValue(minVersion);

    if (query.exec()) {
        while (query.next()) {
            CategorieProduit c;
            c.setCategorieProduitId(QUuid(query.value("categorie_produit_id").toString()));
            c.setNom(query.value("nom").toString());
            c.setDescription(query.value("description").toString());
            c.setCodeCategorie(query.value("code_categorie").toString());
            c.setEstActif(query.value("est_actif").toBool());
            c.setOrdreAffichage(query.value("ordre_affichage").toInt());
            categories.append(c);
        }
    }
    return categories;
}