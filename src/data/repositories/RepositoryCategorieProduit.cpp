#include "RepositoryCategorieProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

RepositoryCategorieProduit::RepositoryCategorieProduit() {}

CategorieProduit RepositoryCategorieProduit::mapRowToCategorie(const QSqlQuery& q) const {
    CategorieProduit cat;
    cat.setCategorieProduitId(QUuid(q.value("categorie_produit_id").toString()));
    cat.setNom(q.value("nom").toString());
    cat.setDescription(q.value("description").toString());
    cat.setCodeCategorie(q.value("code_categorie").toString());
    cat.setEstActif(q.value("est_actif").toBool());
    cat.setOrdreAffichage(q.value("ordre_affichage").toInt());
    cat.setDateCreation(q.value("date_creation").toDateTime());
    cat.setDateMiseAJour(q.value("date_mise_a_jour").toDateTime());
    cat.setVersion(q.value("version").toInt());
    cat.setSyncStatus(CategorieProduit::fromString(q.value("sync_status").toString()));
    cat.setDeletedAt(q.value("deleted_at").toDateTime());
    return cat;
}

bool RepositoryCategorieProduit::create(const CategorieProduit& entity) {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        INSERT INTO categories_produit (
            categorie_produit_id, nom, description, code_categorie,
            est_actif, ordre_affichage, date_creation, date_mise_a_jour,
            version, sync_status
        ) VALUES (:id, :nom, :descr, :code, :actif, :ordre, :datecr, :datemaj, :version, :sync)
    )");
    q.bindValue(":id", entity.getCategorieProduitId().toString(QUuid::WithoutBraces));
    q.bindValue(":nom", entity.getNom());
    q.bindValue(":descr", entity.getDescription());
    q.bindValue(":code", entity.getCodeCategorie());
    q.bindValue(":actif", entity.estActif());
    q.bindValue(":ordre", entity.getOrdreAffichage());
    q.bindValue(":datecr", entity.getDateCreation());
    q.bindValue(":datemaj", entity.getDateMiseAJour());
    q.bindValue(":version", entity.getVersion());
    q.bindValue(":sync", entity.syncStatusString());
    if (!q.exec()) {
        m_dernierErreur = "Erreur création cat: " + q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryCategorieProduit::update(const CategorieProduit& entity) {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE categories_produit SET
            nom=:nom, description=:descr, code_categorie=:code,
            est_actif=:actif, ordre_affichage=:ordre, date_mise_a_jour=:datemaj,
            version=:version, sync_status=:sync
        WHERE categorie_produit_id=:id AND deleted_at IS NULL
    )");
    q.bindValue(":nom", entity.getNom());
    q.bindValue(":descr", entity.getDescription());
    q.bindValue(":code", entity.getCodeCategorie());
    q.bindValue(":actif", entity.estActif());
    q.bindValue(":ordre", entity.getOrdreAffichage());
    q.bindValue(":datemaj", entity.getDateMiseAJour());
    q.bindValue(":version", entity.getVersion());
    q.bindValue(":sync", entity.syncStatusString());
    q.bindValue(":id", entity.getCategorieProduitId().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur update cat: " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool RepositoryCategorieProduit::logicalDelete(const QUuid& id) {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("UPDATE categories_produit SET deleted_at=NOW(), sync_status='PENDING', version=version+1 WHERE categorie_produit_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur suppression cat: " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

std::optional<CategorieProduit> RepositoryCategorieProduit::getById(const QUuid& id) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM categories_produit WHERE categorie_produit_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRowToCategorie(q);
}

QList<CategorieProduit> RepositoryCategorieProduit::getAll() const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<CategorieProduit> res;
    if (q.exec("SELECT * FROM categories_produit WHERE deleted_at IS NULL ORDER BY nom")) {
        while (q.next())
            res.append(mapRowToCategorie(q));
    }
    return res;
}

QList<CategorieProduit> RepositoryCategorieProduit::search(const QString& crit) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<CategorieProduit> res;
    q.prepare("SELECT * FROM categories_produit WHERE (nom ILIKE :c OR code_categorie ILIKE :c) AND deleted_at IS NULL");
    q.bindValue(":c", "%" + crit + "%");
    if (q.exec()) {
        while (q.next())
            res.append(mapRowToCategorie(q));
    }
    return res;
}

bool RepositoryCategorieProduit::exists(const QUuid& id) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT 1 FROM categories_produit WHERE categorie_produit_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    return q.exec() && q.next();
}

std::optional<CategorieProduit> RepositoryCategorieProduit::getByCode(const QString& code) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM categories_produit WHERE code_categorie=:c AND deleted_at IS NULL");
    q.bindValue(":c", code);
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRowToCategorie(q);
}

QList<CategorieProduit> RepositoryCategorieProduit::getPendingSync() const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<CategorieProduit> res;
    if (q.exec("SELECT * FROM categories_produit WHERE sync_status='PENDING' AND deleted_at IS NULL")) {
        while (q.next())
            res.append(mapRowToCategorie(q));
    }
    return res;
}

QList<CategorieProduit> RepositoryCategorieProduit::getSinceVersion(int minVersion) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<CategorieProduit> res;
    q.prepare("SELECT * FROM categories_produit WHERE version>=:v AND deleted_at IS NULL");
    q.bindValue(":v", minVersion);
    if (q.exec()) while (q.next()) res.append(mapRowToCategorie(q));
    return res;
}