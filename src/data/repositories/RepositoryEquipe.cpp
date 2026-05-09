#include "RepositoryEquipe.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryEquipe::RepositoryEquipe()
{
}

Equipe RepositoryEquipe::mapRowToEquipe(const QSqlQuery& query) const
{
    Equipe equipe;
    equipe.setEquipeId(QUuid(query.value("equipe_id").toString()));
    equipe.setNom(query.value("nom").toString());
    equipe.setNomChef(query.value("nom_chef").toString());
    equipe.setTelephoneChef(query.value("telephone_chef").toString());
    equipe.setDescription(query.value("description").toString());
    
    // Sync fields
    equipe.setSyncStatus(Equipe::stringToSyncStatus(query.value("sync_status").toString()));
    equipe.setVersion(query.value("version").toInt());
    equipe.setCreatedAt(query.value("created_at").toDateTime());
    equipe.setUpdatedAt(query.value("updated_at").toDateTime());
    equipe.setDeletedAt(query.value("deleted_at").toDateTime());
    equipe.setCreatedBy(QUuid(query.value("created_by").toString()));
    equipe.setUpdatedBy(QUuid(query.value("updated_by").toString()));
    equipe.setEstActif(query.value("est_actif").toBool());
    
    return equipe;
}

bool RepositoryEquipe::create(const Equipe& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "INSERT INTO equipes "
        "(equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, created_by, updated_by, est_actif) "
        "VALUES (:id, :nom, :nomChef, :tel, :desc, :syncStatus, :version, "
        ":createdAt, :updatedAt, :createdBy, :updatedBy, :actif)"
    );
    
    query.addBindValue(entity.getEquipeId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getNomChef());
    query.addBindValue(entity.getTelephoneChef());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getCreatedAt());
    query.addBindValue(entity.getUpdatedAt());
    query.addBindValue(entity.getCreatedBy().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getUpdatedBy().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getEstActif());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création équipe : " + query.lastError().text();
        qDebug() << "RepositoryEquipe::create error:" << m_dernierErreur;
        return false;
    }

    qDebug() << "RepositoryEquipe::create success for" << entity.getNom();
    return true;
}

Equipe RepositoryEquipe::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, deleted_at, "
        "created_by, updated_by, est_actif "
        "FROM equipes WHERE equipe_id = :id AND deleted_at IS NULL"
    );
    query.addBindValue(id.toString(QUuid::WithoutBraces));

    Equipe equipe;
    if (query.exec() && query.next()) {
        equipe = mapRowToEquipe(query);
    } else {
        m_dernierErreur = "Équipe non trouvée";
    }

    return equipe;
}

QList<Equipe> RepositoryEquipe::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare(
        "SELECT equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, deleted_at, "
        "created_by, updated_by, est_actif "
        "FROM equipes WHERE deleted_at IS NULL ORDER BY nom"
    );

    if (query.exec()) {
        while (query.next()) {
            equipes.append(mapRowToEquipe(query));
        }
    }

    return equipes;
}

bool RepositoryEquipe::update(const Equipe& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "UPDATE equipes SET nom = :nom, nom_chef = :nomChef, "
        "telephone_chef = :tel, description = :desc, "
        "sync_status = :syncStatus, version = :version, "
        "updated_at = :updatedAt, updated_by = :updatedBy, est_actif = :actif "
        "WHERE equipe_id = :id AND deleted_at IS NULL"
    );
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getNomChef());
    query.addBindValue(entity.getTelephoneChef());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getUpdatedAt());
    query.addBindValue(entity.getUpdatedBy().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getEstActif());
    query.addBindValue(entity.getEquipeId().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour équipe : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryEquipe::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    // Soft delete
    query.prepare(
        "UPDATE equipes SET deleted_at = NOW(), sync_status = :syncStatus, version = version + 1 "
        "WHERE equipe_id = :id AND deleted_at IS NULL"
    );
    query.addBindValue("PENDING");
    query.addBindValue(id.toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression équipe : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Equipe> RepositoryEquipe::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare(
        "SELECT equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, deleted_at, "
        "created_by, updated_by, est_actif "
        "FROM equipes WHERE (nom ILIKE :criterion OR nom_chef ILIKE :criterion) "
        "AND deleted_at IS NULL ORDER BY nom"
    );
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            equipes.append(mapRowToEquipe(query));
        }
    }

    return equipes;
}

bool RepositoryEquipe::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM equipes WHERE equipe_id = :id AND deleted_at IS NULL");
    query.addBindValue(id.toString(QUuid::WithoutBraces));

    return query.exec() && query.next();
}

// Offline-first methods
QList<Equipe> RepositoryEquipe::getPendingEquipes() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare(
        "SELECT equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, deleted_at, "
        "created_by, updated_by, est_actif "
        "FROM equipes WHERE sync_status = 'PENDING' AND deleted_at IS NULL "
        "ORDER BY updated_at ASC"
    );

    if (query.exec()) {
        while (query.next()) {
            equipes.append(mapRowToEquipe(query));
        }
    }
    return equipes;
}

QList<Equipe> RepositoryEquipe::getSyncedEquipes() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare(
        "SELECT equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, deleted_at, "
        "created_by, updated_by, est_actif "
        "FROM equipes WHERE sync_status = 'SYNCED' AND deleted_at IS NULL "
        "ORDER BY updated_at DESC"
    );

    if (query.exec()) {
        while (query.next()) {
            equipes.append(mapRowToEquipe(query));
        }
    }
    return equipes;
}

QList<Equipe> RepositoryEquipe::getConflictEquipes() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Equipe> equipes;

    query.prepare(
        "SELECT equipe_id, nom, nom_chef, telephone_chef, description, "
        "sync_status, version, created_at, updated_at, deleted_at, "
        "created_by, updated_by, est_actif "
        "FROM equipes WHERE sync_status = 'CONFLICT' AND deleted_at IS NULL "
        "ORDER BY updated_at ASC"
    );

    if (query.exec()) {
        while (query.next()) {
            equipes.append(mapRowToEquipe(query));
        }
    }
    return equipes;
}

int RepositoryEquipe::getPendingCount() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT COUNT(*) as count FROM equipes "
        "WHERE sync_status IN ('PENDING', 'CONFLICT') AND deleted_at IS NULL"
    );

    if (query.exec() && query.next()) {
        return query.value("count").toInt();
    }
    return 0;
}

QList<RepositoryEquipe::SyncResult> RepositoryEquipe::syncBatch(
    const QList<Equipe>& equipes, const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QList<SyncResult> results;

    for (const Equipe& equipe : equipes) {
        QSqlQuery query(bd.getDatabase());
        
        // Vérifier version et conflit
        query.prepare("SELECT version FROM equipes WHERE equipe_id = :id AND deleted_at IS NULL");
        query.addBindValue(equipe.getEquipeId().toString(QUuid::WithoutBraces));
        
        SyncResult res;
        res.equipeId = equipe.getEquipeId();
        res.newVersion = equipe.getVersion();
        
        if (query.exec() && query.next()) {
            int serverVersion = query.value(0).toInt();
            if (serverVersion > equipe.getVersion()) {
                // Conflit : version serveur plus récente
                res.success = false;
                res.message = QString("CONFLICT: Server v%1 > Client v%2")
                    .arg(serverVersion).arg(equipe.getVersion());
                res.newVersion = serverVersion;
                results.append(res);
                continue;
            }
        }
        
        // Upsert
        QSqlQuery upsertQuery(bd.getDatabase());
        upsertQuery.prepare(
            "INSERT INTO equipes "
            "(equipe_id, nom, nom_chef, telephone_chef, description, "
            "sync_status, version, created_at, updated_at, created_by, updated_by, est_actif) "
            "VALUES (:id, :nom, :nomChef, :tel, :desc, 'SYNCED', :version, "
            ":createdAt, NOW(), :createdBy, :updatedBy, :actif) "
            "ON CONFLICT (equipe_id) DO UPDATE SET "
            "nom = EXCLUDED.nom, nom_chef = EXCLUDED.nom_chef, "
            "telephone_chef = EXCLUDED.telephone_chef, description = EXCLUDED.description, "
            "sync_status = 'SYNCED', version = :version, updated_at = NOW(), updated_by = :updatedBy"
        );
        upsertQuery.addBindValue(equipe.getEquipeId().toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(equipe.getNom());
        upsertQuery.addBindValue(equipe.getNomChef());
        upsertQuery.addBindValue(equipe.getTelephoneChef());
        upsertQuery.addBindValue(equipe.getDescription());
        upsertQuery.addBindValue(equipe.getVersion() + 1);
        upsertQuery.addBindValue(equipe.getCreatedAt());
        upsertQuery.addBindValue(equipe.getCreatedBy().toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(utilisateurId.toString(QUuid::WithoutBraces));
        upsertQuery.addBindValue(equipe.getEstActif());
        
        if (upsertQuery.exec()) {
            res.success = true;
            res.message = "Synced successfully";
            res.newVersion = equipe.getVersion() + 1;
        } else {
            res.success = false;
            res.message = "Sync failed: " + upsertQuery.lastError().text();
        }
        
        results.append(res);
    }

    return results;
}
