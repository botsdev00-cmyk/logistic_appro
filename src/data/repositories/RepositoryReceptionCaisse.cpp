#include "RepositoryReceptionCaisse.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMetaType>

RepositoryReceptionCaisse::RepositoryReceptionCaisse()
{
}

bool RepositoryReceptionCaisse::create(const ReceptionCaisse& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "INSERT INTO receptions_caisse "
        "(reception_caisse_id, repartition_id, montant_attendu, montant_recu, numero_recu, statut, caissier_id, notes, "
        "date_reception, date_creation, date_mise_a_jour, deleted_at, sync_status, version) "
        "VALUES (:id, :repartition_id, :montant_attendu, :montant_recu, :numero_recu, :statut, :caissier_id, :notes, "
        ":date_reception, :date_creation, :date_mise_a_jour, :deleted_at, :sync_status, :version)");
    query.bindValue(":id", entity.getReceptionCaisseId().toString());
    query.bindValue(":repartition_id", entity.getRepartitionId().toString());
    query.bindValue(":montant_attendu", entity.getMontantAttendu());
    query.bindValue(":montant_recu", entity.getMontantRecu());
    query.bindValue(":numero_recu", entity.getNumeroRecu());
    query.bindValue(":statut", ReceptionCaisse::statutToString(entity.getStatut()));

    // Qt6: QVariant(QMetaType(QMetaType::QString)) pour NULL string
    query.bindValue(":caissier_id",
                    entity.getCaissierId().isNull() ? QVariant(QMetaType(QMetaType::QString)) : entity.getCaissierId().toString());

    query.bindValue(":notes", entity.getNotes());
    query.bindValue(":date_reception", entity.getDateReception());
    query.bindValue(":date_creation", entity.getDateCreation());
    query.bindValue(":date_mise_a_jour", entity.getDateMiseAJour());
    query.bindValue(":deleted_at",
                    entity.getDeletedAt().isValid() ? entity.getDeletedAt() : QVariant(QMetaType(QMetaType::QDateTime)));
    query.bindValue(":sync_status", ReceptionCaisse::syncStatusToString(entity.getSyncStatus()));
    query.bindValue(":version", entity.getVersion());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création réception caisse : " + query.lastError().text();
        return false;
    }

    return true;
}

ReceptionCaisse RepositoryReceptionCaisse::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM receptions_caisse WHERE reception_caisse_id = :id");
    query.bindValue(":id", id.toString());

    ReceptionCaisse rc;
    if (query.exec() && query.next()) {
        hydrateFromQuery(rc, query);
    } else {
        m_dernierErreur = "Réception caisse non trouvée";
    }

    return rc;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getAll() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> list;

    query.prepare("SELECT * FROM receptions_caisse WHERE deleted_at IS NULL ORDER BY date_reception DESC");
    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse rc;
            hydrateFromQuery(rc, query);
            list << rc;
        }
    }

    return list;
}

bool RepositoryReceptionCaisse::update(const ReceptionCaisse& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "UPDATE receptions_caisse SET "
        "repartition_id = :repartition_id, montant_attendu = :montant_attendu, "
        "montant_recu = :montant_recu, numero_recu = :numero_recu, statut = :statut, "
        "caissier_id = :caissier_id, notes = :notes, date_reception = :date_reception, "
        "date_mise_a_jour = :date_mise_a_jour, deleted_at = :deleted_at, sync_status = :sync_status, version = :version "
        "WHERE reception_caisse_id = :id"
        );
    query.bindValue(":repartition_id", entity.getRepartitionId().toString());
    query.bindValue(":montant_attendu", entity.getMontantAttendu());
    query.bindValue(":montant_recu", entity.getMontantRecu());
    query.bindValue(":numero_recu", entity.getNumeroRecu());
    query.bindValue(":statut", ReceptionCaisse::statutToString(entity.getStatut()));
    query.bindValue(":caissier_id",
                    entity.getCaissierId().isNull() ? QVariant(QMetaType(QMetaType::QString)) : entity.getCaissierId().toString());
    query.bindValue(":notes", entity.getNotes());
    query.bindValue(":date_reception", entity.getDateReception());
    query.bindValue(":date_mise_a_jour", entity.getDateMiseAJour());
    query.bindValue(":deleted_at",
                    entity.getDeletedAt().isValid() ? entity.getDeletedAt() : QVariant(QMetaType(QMetaType::QDateTime)));
    query.bindValue(":sync_status", ReceptionCaisse::syncStatusToString(entity.getSyncStatus()));
    query.bindValue(":version", entity.getVersion());
    query.bindValue(":id", entity.getReceptionCaisseId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour réception caisse : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// Soft delete (offline-first)
bool RepositoryReceptionCaisse::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QDateTime now = QDateTime::currentDateTime();

    query.prepare(
        "UPDATE receptions_caisse SET deleted_at = :deleted_at, sync_status = 'PENDING', version = version + 1 WHERE reception_caisse_id = :id"
        );
    query.bindValue(":deleted_at", now);
    query.bindValue(":id", id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression (soft) réception caisse : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> list;

    query.prepare("SELECT * FROM receptions_caisse WHERE numero_recu ILIKE :criterion AND deleted_at IS NULL ORDER BY date_reception DESC");
    query.bindValue(":criterion", "%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse rc;
            hydrateFromQuery(rc, query);
            list << rc;
        }
    }
    return list;
}

bool RepositoryReceptionCaisse::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT 1 FROM receptions_caisse WHERE reception_caisse_id = :id AND deleted_at IS NULL");
    query.bindValue(":id", id.toString());
    return query.exec() && query.next();
}

// --- Offline-first ---

QList<ReceptionCaisse> RepositoryReceptionCaisse::getBySyncStatus(ReceptionCaisse::SyncStatus status) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> list;
    query.prepare("SELECT * FROM receptions_caisse WHERE sync_status = :status AND deleted_at IS NULL ORDER BY date_reception DESC");
    query.bindValue(":status", ReceptionCaisse::syncStatusToString(status));
    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse rc;
            hydrateFromQuery(rc, query);
            list << rc;
        }
    }
    return list;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> list;
    query.prepare("SELECT * FROM receptions_caisse WHERE version > :min_version ORDER BY version ASC");
    query.bindValue(":min_version", minVersion);
    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse rc;
            hydrateFromQuery(rc, query);
            list << rc;
        }
    }
    return list;
}

// --- Compatibilité avec ancien métier et UI ---

ReceptionCaisse RepositoryReceptionCaisse::getByRepartition(const QUuid& repartitionId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM receptions_caisse WHERE repartition_id = :rep_id AND deleted_at IS NULL");
    query.bindValue(":rep_id", repartitionId.toString());
    ReceptionCaisse rc;
    if (query.exec() && query.next()) {
        hydrateFromQuery(rc, query);
    }
    return rc;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getByStatut(ReceptionCaisse::Statut statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> list;
    query.prepare("SELECT * FROM receptions_caisse WHERE statut = :statut AND deleted_at IS NULL");
    query.bindValue(":statut", ReceptionCaisse::statutToString(statut));
    if (query.exec()) while (query.next()) {
            ReceptionCaisse rc; hydrateFromQuery(rc, query); list << rc;
        }
    return list;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getWithDiscrepancies()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> list;
    query.prepare("SELECT * FROM receptions_caisse WHERE (statut = :discrep OR montant_attendu != montant_recu) AND deleted_at IS NULL");
    query.bindValue(":discrep", ReceptionCaisse::statutToString(ReceptionCaisse::Statut::Discrepance));
    if (query.exec()) while (query.next()) {
            ReceptionCaisse rc; hydrateFromQuery(rc, query); list << rc;
        }
    return list;
}

// --- Hydrate helper ---

void RepositoryReceptionCaisse::hydrateFromQuery(ReceptionCaisse& rc, const QSqlQuery& query) const
{
    rc.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
    rc.setRepartitionId(QUuid(query.value("repartition_id").toString()));
    rc.setMontantAttendu(query.value("montant_attendu").toDouble());
    rc.setMontantRecu(query.value("montant_recu").toDouble());
    rc.setNumeroRecu(query.value("numero_recu").toString());
    rc.setStatut(ReceptionCaisse::stringToStatut(query.value("statut").toString()));
    rc.setCaissierId(QUuid(query.value("caissier_id").toString()));
    rc.setNotes(query.value("notes").toString());
    rc.setDateReception(query.value("date_reception").toDateTime());
    rc.setDateCreation(query.value("date_creation").toDateTime());
    rc.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
    rc.setDeletedAt(query.value("deleted_at").isNull() ? QDateTime() : query.value("deleted_at").toDateTime());
    rc.setSyncStatus(ReceptionCaisse::stringToSyncStatus(query.value("sync_status").toString()));
    rc.setVersion(query.value("version").toInt());
}
