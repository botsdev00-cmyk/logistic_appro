#include "RepositoryEntreeStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>

RepositoryEntreeStock::RepositoryEntreeStock() {}

bool RepositoryEntreeStock::create(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        INSERT INTO entrees_stock
          (entree_stock_id, produit_id, quantite, source_entree_id, numero_facture,
           prix_unitaire, numero_lot, date_expiration, cree_par, statut_validation,
           date, date_mise_a_jour,
           sync_status, version, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 'PENDING', 1, NOW(), NOW())
    )");
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
        return false;
    }
    return true;
}

bool RepositoryEntreeStock::update(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE entrees_stock SET
           quantite = ?, prix_unitaire = ?, numero_lot = ?,
           date_expiration = ?, approuve_par = ?, statut_validation = ?,
           date_mise_a_jour = CURRENT_TIMESTAMP,
           updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE entree_stock_id = ? AND deleted_at IS NULL
    )");

    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(entity.getApprouvePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getEntreeStockId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour entrée stock: " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool RepositoryEntreeStock::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE entrees_stock
        SET deleted_at = NOW(), sync_status = 'PENDING', version = version + 1
        WHERE entree_stock_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression entrée stock: " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

std::optional<EntreeStock> RepositoryEntreeStock::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM entrees_stock WHERE entree_stock_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());

    if (query.exec() && query.next()) {
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
        entree.setDate(query.value("date").toDateTime());
        entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
        return entree;
    }
    return std::nullopt;
}

QList<EntreeStock> RepositoryEntreeStock::getAll() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    if (query.exec("SELECT * FROM entrees_stock WHERE deleted_at IS NULL ORDER BY date DESC")) {
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
            entree.setDate(query.value("date").toDateTime());
            entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            entrees.append(entree);
        }
    }
    return entrees;
}

QList<EntreeStock> RepositoryEntreeStock::search(const QString& criterion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE deleted_at IS NULL AND (numero_facture ILIKE ? OR numero_lot ILIKE ?) ORDER BY date DESC");
    QString crit = "%" + criterion + "%";
    query.addBindValue(crit);
    query.addBindValue(crit);

    if (query.exec()) {
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
            entree.setDate(query.value("date").toDateTime());
            entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            entrees.append(entree);
        }
    }
    return entrees;
}

bool RepositoryEntreeStock::exists(const QUuid& id) const
{
    return getById(id).has_value();
}

QList<EntreeStock> RepositoryEntreeStock::getByStatut(const QString& statut) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE statut_validation = ? AND deleted_at IS NULL ORDER BY date DESC");
    query.addBindValue(statut);

    if (query.exec()) {
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
            entree.setDate(query.value("date").toDateTime());
            entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            entrees.append(entree);
        }
    }
    return entrees;
}

QList<EntreeStock> RepositoryEntreeStock::getByProduit(const QUuid& produitId) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE produit_id = ? AND deleted_at IS NULL ORDER BY date DESC");
    query.addBindValue(produitId.toString());

    if (query.exec()) {
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
            entree.setDate(query.value("date").toDateTime());
            entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            entrees.append(entree);
        }
    }
    return entrees;
}

QList<EntreeStock> RepositoryEntreeStock::getEnAttente() const
{
    return getByStatut("EN_ATTENTE");
}

bool RepositoryEntreeStock::approuver(const QUuid& entreeId, const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE entrees_stock 
        SET statut_validation = 'APPROUVE', approuve_par = ?, 
            date_mise_a_jour = CURRENT_TIMESTAMP, updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE entree_stock_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(utilisateurId.toString());
    query.addBindValue(entreeId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation entrée stock: " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool RepositoryEntreeStock::rejeter(const QUuid& entreeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE entrees_stock 
        SET statut_validation = 'REJETE', 
            date_mise_a_jour = CURRENT_TIMESTAMP, updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE entree_stock_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(entreeId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur rejet entrée stock: " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<EntreeStock> RepositoryEntreeStock::getPendingSync() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    if (query.exec("SELECT * FROM entrees_stock WHERE sync_status = 'PENDING' AND deleted_at IS NULL")) {
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
            entree.setDate(query.value("date").toDateTime());
            entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            entrees.append(entree);
        }
    }
    return entrees;
}

QList<EntreeStock> RepositoryEntreeStock::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    query.prepare("SELECT * FROM entrees_stock WHERE version >= ? AND deleted_at IS NULL");
    query.addBindValue(minVersion);

    if (query.exec()) {
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
            entree.setDate(query.value("date").toDateTime());
            entree.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            entrees.append(entree);
        }
    }
    return entrees;
}