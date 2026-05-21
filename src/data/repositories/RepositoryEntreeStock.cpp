#include "RepositoryEntreeStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

// ================== CONSTRUCTEUR ======================
RepositoryEntreeStock::RepositoryEntreeStock() {}

// ================== CRÉATION =========================
bool RepositoryEntreeStock::create(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        INSERT INTO entrees_stock (
            entree_stock_id, produit_id, quantite, source_entree_id, date,
            cree_par, numero_facture, prix_unitaire, numero_lot, date_expiration,
            approuve_par, statut_validation, date_mise_a_jour, cree_par_updated,
            sync_status, version, deleted_at, created_at, updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    // UUIDs : .toString(QUuid::WithoutBraces), ou QVariant() si .isNull()
    auto uuidOrNull = [](const QUuid& id) -> QVariant {
        return id.isNull() ? QVariant() : QVariant(id.toString(QUuid::WithoutBraces));
    };

    query.addBindValue(entity.getEntreeStockId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getProduitId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getSourceEntreeId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getDate());
    query.addBindValue(uuidOrNull(entity.getCreePar()));
    query.addBindValue(entity.getNumeroFacture());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(uuidOrNull(entity.getApprouvePar()));
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDateMiseAJour());
    query.addBindValue(uuidOrNull(entity.getCreeParUpdated()));
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getDeletedAt().isNull() ? QVariant() : entity.getDeletedAt());
    query.addBindValue(entity.getCreatedAt());
    query.addBindValue(entity.getUpdatedAt());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création entrée stock : " + query.lastError().text();
        return false;
    }
    return true;
}

// ================== MISE À JOUR =========================
bool RepositoryEntreeStock::update(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE entrees_stock SET
            produit_id = ?,
            quantite = ?,
            source_entree_id = ?,
            date = ?,
            cree_par = ?,
            numero_facture = ?,
            prix_unitaire = ?,
            numero_lot = ?,
            date_expiration = ?,
            approuve_par = ?,
            statut_validation = ?,
            date_mise_a_jour = ?,
            cree_par_updated = ?,
            sync_status = ?,
            version = ?,
            deleted_at = ?,
            updated_at = ?
        WHERE entree_stock_id = ?
    )");

    auto uuidOrNull = [](const QUuid& id) -> QVariant {
        return id.isNull() ? QVariant() : QVariant(id.toString(QUuid::WithoutBraces));
    };

    query.addBindValue(entity.getProduitId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getSourceEntreeId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getDate());
    query.addBindValue(uuidOrNull(entity.getCreePar()));
    query.addBindValue(entity.getNumeroFacture());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(uuidOrNull(entity.getApprouvePar()));
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDateMiseAJour());
    query.addBindValue(uuidOrNull(entity.getCreeParUpdated()));
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getDeletedAt().isNull() ? QVariant() : entity.getDeletedAt());
    query.addBindValue(entity.getUpdatedAt());
    query.addBindValue(entity.getEntreeStockId().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour entrée stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// ================== SOFT DELETE ========================
bool RepositoryEntreeStock::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("UPDATE entrees_stock SET deleted_at = CURRENT_TIMESTAMP, sync_status = 'PENDING', version = version + 1 WHERE entree_stock_id = ?");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression (soft-delete) entrée stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// =============== LECTURE / RECHERCHE ==================
std::optional<EntreeStock> RepositoryEntreeStock::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE entree_stock_id = ?");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (query.exec() && query.next())
        return mapRowToEntreeStock(query);
    return std::nullopt;
}

QList<EntreeStock> RepositoryEntreeStock::getAll() const
{
    QList<EntreeStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE deleted_at IS NULL");
    if (query.exec()) while (query.next()) list.append(mapRowToEntreeStock(query));
    return list;
}

QList<EntreeStock> RepositoryEntreeStock::getByStatut(const QString& statut) const
{
    QList<EntreeStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE statut_validation = ? AND deleted_at IS NULL");
    query.addBindValue(statut);
    if (query.exec()) while (query.next()) list.append(mapRowToEntreeStock(query));
    return list;
}

QList<EntreeStock> RepositoryEntreeStock::getByProduit(const QUuid& produitId) const
{
    QList<EntreeStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE produit_id = ? AND deleted_at IS NULL");
    query.addBindValue(produitId.toString(QUuid::WithoutBraces));
    if (query.exec()) while (query.next()) list.append(mapRowToEntreeStock(query));
    return list;
}

QList<EntreeStock> RepositoryEntreeStock::getEnAttente() const
{
    return getByStatut("EN_ATTENTE");
}

QList<EntreeStock> RepositoryEntreeStock::getPendingSync() const
{
    QList<EntreeStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE sync_status = 'PENDING' AND deleted_at IS NULL");
    if (query.exec()) while (query.next()) list.append(mapRowToEntreeStock(query));
    return list;
}

QList<EntreeStock> RepositoryEntreeStock::getSinceVersion(int minVersion) const
{
    QList<EntreeStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE version >= ? AND deleted_at IS NULL");
    query.addBindValue(minVersion);
    if (query.exec()) while (query.next()) list.append(mapRowToEntreeStock(query));
    return list;
}

QList<EntreeStock> RepositoryEntreeStock::search(const QString& criterion) const
{
    QList<EntreeStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM entrees_stock WHERE (numero_facture ILIKE ? OR numero_lot ILIKE ?) AND deleted_at IS NULL");
    query.addBindValue("%" + criterion + "%");
    query.addBindValue("%" + criterion + "%");
    if (query.exec()) while (query.next()) list.append(mapRowToEntreeStock(query));
    return list;
}

bool RepositoryEntreeStock::rejeter(const QUuid& id) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE entrees_stock
           SET statut_validation = 'REJETE',
               updated_at = CURRENT_TIMESTAMP
         WHERE entree_stock_id = ?
    )");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur rejet entrée stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool RepositoryEntreeStock::approuver(const QUuid& entreeId, const QUuid& userId) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE entrees_stock
           SET statut_validation = 'APPROUVE',
               approuve_par = ?,
               updated_at = CURRENT_TIMESTAMP
         WHERE entree_stock_id = ?
    )");
    query.addBindValue(userId.toString(QUuid::WithoutBraces));
    query.addBindValue(entreeId.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation entrée stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}



// =================== MAPPING ==========================
EntreeStock RepositoryEntreeStock::mapRowToEntreeStock(const QSqlQuery& query) const
{
    EntreeStock e;
    e.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
    e.setProduitId(QUuid(query.value("produit_id").toString()));
    e.setQuantite(query.value("quantite").toInt());
    e.setSourceEntreeId(QUuid(query.value("source_entree_id").toString()));
    e.setDate(query.value("date").toDateTime());
    e.setCreePar(QUuid(query.value("cree_par").toString()));
    e.setNumeroFacture(query.value("numero_facture").toString());
    e.setPrixUnitaire(query.value("prix_unitaire").toDouble());
    e.setNumeroLot(query.value("numero_lot").toString());
    e.setDateExpiration(query.value("date_expiration").toDate());
    e.setApprouvePar(QUuid(query.value("approuve_par").toString()));
    e.setStatutValidation(query.value("statut_validation").toString());
    e.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
    e.setCreeParUpdated(QUuid(query.value("cree_par_updated").toString()));
    e.setSyncStatus(EntreeStock::stringToSyncStatus(query.value("sync_status").toString()));
    e.setVersion(query.value("version").toInt());
    e.setDeletedAt(query.value("deleted_at").toDateTime());
    e.setCreatedAt(query.value("created_at").toDateTime());
    e.setUpdatedAt(query.value("updated_at").toDateTime());
    return e;
}
