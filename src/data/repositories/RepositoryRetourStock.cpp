#include "RepositoryRetourStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

// ============ CONSTRUCTEUR ========================
RepositoryRetourStock::RepositoryRetourStock() {}

// ============= CRÉATION ===========================
bool RepositoryRetourStock::create(const RetourStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        INSERT INTO retours_stock (
            retour_stock_id, produit_id, quantite, raison_retour_id, date,
            repartition_id, observations, cree_par, approuve_par,
            statut_validation, date_mise_a_jour,
            sync_status, version, deleted_at, created_at, updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    auto uuidOrNull = [](const QUuid& id) -> QVariant {
        return id.isNull() ? QVariant() : QVariant(id.toString(QUuid::WithoutBraces));
    };

    query.addBindValue(entity.getRetourStockId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getProduitId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getRaisonRetourId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getDate());
    query.addBindValue(uuidOrNull(entity.getRepartitionId()));
    query.addBindValue(entity.getObservations());
    query.addBindValue(entity.getCreePar().toString(QUuid::WithoutBraces));
    query.addBindValue(uuidOrNull(entity.getApprouvePar()));
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDateMiseAJour());
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getDeletedAt().isNull() ? QVariant() : entity.getDeletedAt());
    query.addBindValue(entity.getCreatedAt());
    query.addBindValue(entity.getUpdatedAt());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création retour stock : " + query.lastError().text();
        return false;
    }
    return true;
}

// ============= MISE À JOUR =========================
bool RepositoryRetourStock::update(const RetourStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE retours_stock SET
            produit_id = ?,
            quantite = ?,
            raison_retour_id = ?,
            date = ?,
            repartition_id = ?,
            observations = ?,
            cree_par = ?,
            approuve_par = ?,
            statut_validation = ?,
            date_mise_a_jour = ?,
            sync_status = ?,
            version = ?,
            deleted_at = ?,
            updated_at = ?
        WHERE retour_stock_id = ?
    )");

    auto uuidOrNull = [](const QUuid& id) -> QVariant {
        return id.isNull() ? QVariant() : QVariant(id.toString(QUuid::WithoutBraces));
    };

    query.addBindValue(entity.getProduitId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getRaisonRetourId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getDate());
    query.addBindValue(uuidOrNull(entity.getRepartitionId()));
    query.addBindValue(entity.getObservations());
    query.addBindValue(entity.getCreePar().toString(QUuid::WithoutBraces));
    query.addBindValue(uuidOrNull(entity.getApprouvePar()));
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDateMiseAJour());
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getDeletedAt().isNull() ? QVariant() : entity.getDeletedAt());
    query.addBindValue(entity.getUpdatedAt());
    query.addBindValue(entity.getRetourStockId().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour retour stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// ============= SOFT DELETE ========================
bool RepositoryRetourStock::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("UPDATE retours_stock SET deleted_at = CURRENT_TIMESTAMP, sync_status = 'PENDING', version = version + 1 WHERE retour_stock_id = ?");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression (soft-delete) retour stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// ============== LECTURE / RECHERCHE ===================
std::optional<RetourStock> RepositoryRetourStock::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE retour_stock_id = ?");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (query.exec() && query.next())
        return mapRowToRetourStock(query);
    return std::nullopt;
}

QList<RetourStock> RepositoryRetourStock::getAll() const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE deleted_at IS NULL");
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

QList<RetourStock> RepositoryRetourStock::getByStatut(const QString& statut) const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE statut_validation = ? AND deleted_at IS NULL");
    query.addBindValue(statut);
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

QList<RetourStock> RepositoryRetourStock::getByProduit(const QUuid& produitId) const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE produit_id = ? AND deleted_at IS NULL");
    query.addBindValue(produitId.toString(QUuid::WithoutBraces));
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

QList<RetourStock> RepositoryRetourStock::getByRepartition(const QUuid& repartitionId) const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE repartition_id = ? AND deleted_at IS NULL");
    query.addBindValue(repartitionId.toString(QUuid::WithoutBraces));
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

QList<RetourStock> RepositoryRetourStock::getEnAttente() const
{
    return getByStatut("EN_ATTENTE");
}

QList<RetourStock> RepositoryRetourStock::getPendingSync() const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE sync_status = 'PENDING' AND deleted_at IS NULL");
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

QList<RetourStock> RepositoryRetourStock::getSinceVersion(int minVersion) const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE version >= ? AND deleted_at IS NULL");
    query.addBindValue(minVersion);
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

QList<RetourStock> RepositoryRetourStock::search(const QString& criterion) const
{
    QList<RetourStock> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM retours_stock WHERE observations ILIKE ? AND deleted_at IS NULL");
    query.addBindValue("%" + criterion + "%");
    if (query.exec()) while (query.next()) list.append(mapRowToRetourStock(query));
    return list;
}

bool RepositoryRetourStock::rejeter(const QUuid& id) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE retours_stock
           SET statut_validation = 'REJETE',
               updated_at = CURRENT_TIMESTAMP
         WHERE retour_stock_id = ?
    )");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur rejet retour stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool RepositoryRetourStock::approuver(const QUuid& retourId, const QUuid& userId) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE retours_stock
           SET statut_validation = 'APPROUVE',
               approuve_par = ?,
               updated_at = CURRENT_TIMESTAMP
         WHERE retour_stock_id = ?
    )");
    query.addBindValue(userId.toString(QUuid::WithoutBraces));
    query.addBindValue(retourId.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation retour stock : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// ============== MAPPING ===================
RetourStock RepositoryRetourStock::mapRowToRetourStock(const QSqlQuery& query) const
{
    RetourStock r;
    r.setRetourStockId(QUuid(query.value("retour_stock_id").toString()));
    r.setProduitId(QUuid(query.value("produit_id").toString()));
    r.setQuantite(query.value("quantite").toInt());
    r.setRaisonRetourId(QUuid(query.value("raison_retour_id").toString()));
    r.setDate(query.value("date").toDateTime());
    r.setRepartitionId(QUuid(query.value("repartition_id").toString()));
    r.setObservations(query.value("observations").toString());
    r.setCreePar(QUuid(query.value("cree_par").toString()));
    r.setApprouvePar(QUuid(query.value("approuve_par").toString()));
    r.setStatutValidation(query.value("statut_validation").toString());
    r.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
    r.setSyncStatus(RetourStock::stringToSyncStatus(query.value("sync_status").toString()));
    r.setVersion(query.value("version").toInt());
    r.setDeletedAt(query.value("deleted_at").toDateTime());
    r.setCreatedAt(query.value("created_at").toDateTime());
    r.setUpdatedAt(query.value("updated_at").toDateTime());
    return r;
}
