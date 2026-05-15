#include "RepositoryEquipe.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryEquipe::RepositoryEquipe() {}

Equipe RepositoryEquipe::mapRowToEquipe(const QSqlQuery& q) const
{
    Equipe equipe;
    equipe.setEquipeId(QUuid(q.value("equipe_id").toString()));
    equipe.setNom(q.value("nom").toString());
    equipe.setNomChef(q.value("nom_chef").toString());
    equipe.setEstActif(q.value("est_actif").toBool());
    equipe.setSyncStatus(Equipe::syncStatusFromString(q.value("sync_status").toString()));
    equipe.setVersion(q.value("version").toInt());
    equipe.setCreatedAt(q.value("created_at").toDateTime());
    equipe.setUpdatedAt(q.value("updated_at").toDateTime());
    equipe.setDeletedAt(q.value("deleted_at").toDateTime());
    equipe.setCreatedBy(QUuid(q.value("created_by").toString()));
    equipe.setUpdatedBy(QUuid(q.value("updated_by").toString()));
    return equipe;
}

bool RepositoryEquipe::create(const Equipe& e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        INSERT INTO equipes (
            equipe_id, nom, nom_chef, est_actif,
            sync_status, version, created_at, updated_at, created_by, updated_by
        ) VALUES (
            :id, :nom, :chef, :actif,
            'PENDING', 1, NOW(), NOW(), :cb, :ub
        )
    )");
    q.bindValue(":id", e.getEquipeId().toString(QUuid::WithoutBraces));
    q.bindValue(":nom", e.getNom());
    q.bindValue(":chef", e.getNomChef());
    q.bindValue(":actif", e.getEstActif());
    q.bindValue(":cb", e.getCreatedBy().toString(QUuid::WithoutBraces));
    q.bindValue(":ub", e.getUpdatedBy().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur création équipe : " + q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryEquipe::update(const Equipe& e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE equipes
        SET nom=:nom, nom_chef=:chef, est_actif=:actif,
            updated_at=NOW(), updated_by=:ub,
            version=version+1, sync_status='PENDING'
        WHERE equipe_id=:id AND deleted_at IS NULL
    )");
    q.bindValue(":nom", e.getNom());
    q.bindValue(":chef", e.getNomChef());
    q.bindValue(":actif", e.getEstActif());
    q.bindValue(":ub", e.getUpdatedBy().toString(QUuid::WithoutBraces));
    q.bindValue(":id", e.getEquipeId().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur update équipe : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool RepositoryEquipe::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE equipes SET deleted_at=NOW(), sync_status='PENDING', version=version+1
        WHERE equipe_id=:id AND deleted_at IS NULL
    )");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur suppression équipe : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

std::optional<Equipe> RepositoryEquipe::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM equipes WHERE equipe_id = :id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec() || !q.next())
        return std::nullopt;
    return mapRowToEquipe(q);
}

QList<Equipe> RepositoryEquipe::getAll() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Equipe> res;
    if (q.exec("SELECT * FROM equipes WHERE deleted_at IS NULL ORDER BY nom")) {
        while (q.next())
            res.append(mapRowToEquipe(q));
    }
    return res;
}

QList<Equipe> RepositoryEquipe::search(const QString& crit) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Equipe> res;
    q.prepare("SELECT * FROM equipes WHERE nom ILIKE :c AND deleted_at IS NULL");
    q.bindValue(":c", "%" + crit + "%");
    if (q.exec()) while (q.next()) res.append(mapRowToEquipe(q));
    return res;
}

bool RepositoryEquipe::exists(const QUuid& id) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT 1 FROM equipes WHERE equipe_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    return q.exec() && q.next();
}

QList<Equipe> RepositoryEquipe::getPendingEquipes() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Equipe> res;
    if (q.exec("SELECT * FROM equipes WHERE sync_status='PENDING' AND deleted_at IS NULL")) {
        while (q.next())
            res.append(mapRowToEquipe(q));
    }
    return res;
}
QList<Equipe> RepositoryEquipe::getConflictEquipes() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Equipe> res;
    if (q.exec("SELECT * FROM equipes WHERE sync_status='CONFLICT' AND deleted_at IS NULL")) {
        while (q.next())
            res.append(mapRowToEquipe(q));
    }
    return res;
}
QList<Equipe> RepositoryEquipe::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Equipe> res;
    q.prepare("SELECT * FROM equipes WHERE version >= :v AND deleted_at IS NULL");
    q.bindValue(":v", minVersion);
    if (q.exec()) while (q.next()) res.append(mapRowToEquipe(q));
    return res;
}
