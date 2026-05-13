#include "RepositoryProduit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryProduit::RepositoryProduit() {}

Produit RepositoryProduit::mapRowToProduit(const QSqlQuery& q) const {
    Produit prod;
    prod.setProduitId(QUuid(q.value("produit_id").toString()));
    prod.setCategorieProduitId(QUuid(q.value("categorie_produit_id").toString()));
    prod.setTypeProduitId(QUuid(q.value("type_produit_id").toString()));
    prod.setNom(q.value("nom").toString());
    prod.setDescription(q.value("description").toString());
    prod.setCodeSku(q.value("code_sku").toString());
    prod.setPrixUnitaire(q.value("prix_unitaire").toDouble());
    prod.setStockMinimum(q.value("stock_minimum").toInt());
    prod.setEstActif(q.value("est_actif").toBool());
    prod.setDateCreation(q.value("date_creation").toDateTime());
    prod.setDateMiseAJour(q.value("date_mise_a_jour").toDateTime());
    prod.setVersion(q.value("version").toInt());
    prod.setSyncStatus(Produit::fromString(q.value("sync_status").toString()));
    prod.setDeletedAt(q.value("deleted_at").toDateTime());
    return prod;
}

bool RepositoryProduit::create(const Produit& entity) {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        INSERT INTO produits (
            produit_id, categorie_produit_id, type_produit_id,
            nom, description, code_sku, prix_unitaire, stock_minimum, est_actif,
            date_creation, date_mise_a_jour, version, sync_status
        ) VALUES (:id, :catid, :typeid, :nom, :desc, :sku, :prix, :stock, :actif, :datecr, :datemaj, :version, :sync)
    )");
    q.bindValue(":id", entity.getProduitId().toString(QUuid::WithoutBraces));
    q.bindValue(":catid", entity.getCategorieProduitId().toString(QUuid::WithoutBraces));
    q.bindValue(":typeid", entity.getTypeProduitId().toString(QUuid::WithoutBraces));
    q.bindValue(":nom", entity.getNom());
    q.bindValue(":desc", entity.getDescription());
    q.bindValue(":sku", entity.getCodeSku());
    q.bindValue(":prix", entity.getPrixUnitaire());
    q.bindValue(":stock", entity.getStockMinimum());
    q.bindValue(":actif", entity.estActif());
    q.bindValue(":datecr", entity.getDateCreation());
    q.bindValue(":datemaj", entity.getDateMiseAJour());
    q.bindValue(":version", entity.getVersion());
    q.bindValue(":sync", entity.syncStatusString());
    if (!q.exec()) {
        m_dernierErreur = "Erreur création produit : " + q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryProduit::update(const Produit& entity) {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE produits SET
            categorie_produit_id=:catid, type_produit_id=:typeid,
            nom=:nom, description=:desc, code_sku=:sku, prix_unitaire=:prix, stock_minimum=:stock, est_actif=:actif,
            date_mise_a_jour=:datemaj, version=:version, sync_status=:sync
        WHERE produit_id=:id AND deleted_at IS NULL
    )");
    q.bindValue(":catid", entity.getCategorieProduitId().toString(QUuid::WithoutBraces));
    q.bindValue(":typeid", entity.getTypeProduitId().toString(QUuid::WithoutBraces));
    q.bindValue(":nom", entity.getNom());
    q.bindValue(":desc", entity.getDescription());
    q.bindValue(":sku", entity.getCodeSku());
    q.bindValue(":prix", entity.getPrixUnitaire());
    q.bindValue(":stock", entity.getStockMinimum());
    q.bindValue(":actif", entity.estActif());
    q.bindValue(":datemaj", entity.getDateMiseAJour());
    q.bindValue(":version", entity.getVersion());
    q.bindValue(":sync", entity.syncStatusString());
    q.bindValue(":id", entity.getProduitId().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur update produit : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool RepositoryProduit::logicalDelete(const QUuid& id) {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("UPDATE produits SET deleted_at=NOW(), sync_status='PENDING', version=version+1 WHERE produit_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur suppression produit : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

std::optional<Produit> RepositoryProduit::getById(const QUuid& id) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM produits WHERE produit_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRowToProduit(q);
}

std::optional<Produit> RepositoryProduit::getByCodeSku(const QString& sku) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM produits WHERE code_sku=:sku AND deleted_at IS NULL");
    q.bindValue(":sku", sku);
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRowToProduit(q);
}

QList<Produit> RepositoryProduit::getAll() const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Produit> res;
    if (q.exec("SELECT * FROM produits WHERE deleted_at IS NULL ORDER BY nom")) {
        while (q.next()) res.append(mapRowToProduit(q));
    }
    return res;
}

QList<Produit> RepositoryProduit::getByCategorie(const QUuid& categorieId) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Produit> res;
    q.prepare("SELECT * FROM produits WHERE categorie_produit_id=:catid AND deleted_at IS NULL");
    q.bindValue(":catid", categorieId.toString(QUuid::WithoutBraces));
    if (q.exec()) while (q.next()) res.append(mapRowToProduit(q));
    return res;
}

QList<Produit> RepositoryProduit::search(const QString& criterion) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Produit> res;
    q.prepare("SELECT * FROM produits WHERE (nom ILIKE :crit OR code_sku ILIKE :crit) AND deleted_at IS NULL");
    q.bindValue(":crit", "%" + criterion + "%");
    if (q.exec()) while (q.next()) res.append(mapRowToProduit(q));
    return res;
}

bool RepositoryProduit::exists(const QUuid& id) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT 1 FROM produits WHERE produit_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    return q.exec() && q.next();
}

QList<Produit> RepositoryProduit::getPendingSync() const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Produit> res;
    if (q.exec("SELECT * FROM produits WHERE sync_status='PENDING' AND deleted_at IS NULL")) {
        while (q.next()) res.append(mapRowToProduit(q));
    }
    return res;
}

QList<Produit> RepositoryProduit::getSinceVersion(int minVersion) const {
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Produit> res;
    q.prepare("SELECT * FROM produits WHERE version>=:v AND deleted_at IS NULL");
    q.bindValue(":v", minVersion);
    if (q.exec()) while (q.next()) res.append(mapRowToProduit(q));
    return res;
}
