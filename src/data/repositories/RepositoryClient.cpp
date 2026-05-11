#include "RepositoryClient.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

RepositoryClient::RepositoryClient() {}

Client RepositoryClient::mapRowToClient(const QSqlQuery& q) const
{
    Client c;
    c.setClientId(QUuid(q.value("client_id").toString()));
    c.setNom(q.value("nom").toString());
    c.setAdresse(q.value("adresse").toString());
    c.setTelephone(q.value("telephone").toString());
    c.setEmail(q.value("email").toString());
    c.setRouteId(QUuid(q.value("route_id").toString()));
    c.setPersonneContact(q.value("personne_contact").toString());
    c.setConditionsPaiement(Client::stringToConditionsPaiement(q.value("conditions_paiement").toString()));
    c.setEstActif(q.value("est_actif").toBool());
    c.setDateCreation(q.value("date_creation").toDateTime());
    c.setDateMiseAJour(q.value("date_mise_a_jour").toDateTime());
    c.setVersion(q.value("version").toInt());
    c.setSyncStatus(Client::syncStatusFromString(q.value("sync_status").toString()));
    c.setDeletedAt(q.value("deleted_at").toDateTime());
    return c;
}

bool RepositoryClient::create(const Client& e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        INSERT INTO clients (
            client_id, nom, adresse, telephone, email, route_id,
            personne_contact, conditions_paiement, est_actif, date_creation, date_mise_a_jour, version, sync_status
        ) VALUES (:id, :n, :adr, :tel, :em, :route, :pc, :cp, :act, :dc, :dmj, :v, :sync)
    )");
    q.bindValue(":id", e.getClientId().toString(QUuid::WithoutBraces));
    q.bindValue(":n", e.getNom());
    q.bindValue(":adr", e.getAdresse());
    q.bindValue(":tel", e.getTelephone());
    q.bindValue(":em", e.getEmail());
    q.bindValue(":route", e.getRouteId().toString(QUuid::WithoutBraces));
    q.bindValue(":pc", e.getPersonneContact());
    q.bindValue(":cp", Client::conditionsPaiementToString(e.getConditionsPaiement()));
    q.bindValue(":act", e.estActif());
    q.bindValue(":dc", e.getDateCreation());
    q.bindValue(":dmj", e.getDateMiseAJour());
    q.bindValue(":v", e.getVersion());
    q.bindValue(":sync", e.syncStatusString());
    if (!q.exec()) {
        m_dernierErreur = "Erreur création client : " + q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryClient::update(const Client& e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE clients SET
            nom=:n, adresse=:adr, telephone=:tel, email=:em, route_id=:route, personne_contact=:pc,
            conditions_paiement=:cp, est_actif=:act, date_mise_a_jour=:dmj,
            version=:v, sync_status=:sync
        WHERE client_id=:id AND deleted_at IS NULL
    )");
    q.bindValue(":n", e.getNom());
    q.bindValue(":adr", e.getAdresse());
    q.bindValue(":tel", e.getTelephone());
    q.bindValue(":em", e.getEmail());
    q.bindValue(":route", e.getRouteId().toString(QUuid::WithoutBraces));
    q.bindValue(":pc", e.getPersonneContact());
    q.bindValue(":cp", Client::conditionsPaiementToString(e.getConditionsPaiement()));
    q.bindValue(":act", e.estActif());
    q.bindValue(":dmj", e.getDateMiseAJour());
    q.bindValue(":v", e.getVersion());
    q.bindValue(":sync", e.syncStatusString());
    q.bindValue(":id", e.getClientId().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur update client : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool RepositoryClient::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("UPDATE clients SET deleted_at=NOW(), sync_status='PENDING', version=version+1 WHERE client_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur suppression client : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

std::optional<Client> RepositoryClient::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM clients WHERE client_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRowToClient(q);
}

QList<Client> RepositoryClient::getAll() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Client> res;
    if (q.exec("SELECT * FROM clients WHERE deleted_at IS NULL ORDER BY nom")) {
        while (q.next())
            res.append(mapRowToClient(q));
    }
    return res;
}

QList<Client> RepositoryClient::search(const QString& crit) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Client> res;
    q.prepare("SELECT * FROM clients WHERE (nom ILIKE :c OR email ILIKE :c) AND deleted_at IS NULL");
    q.bindValue(":c", "%" + crit + "%");
    if (q.exec()) {
        while (q.next())
            res.append(mapRowToClient(q));
    }
    return res;
}

bool RepositoryClient::exists(const QUuid& id) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT 1 FROM clients WHERE client_id=:id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    return q.exec() && q.next();
}

QList<Client> RepositoryClient::getByRoute(const QUuid& routeId) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Client> res;
    q.prepare("SELECT * FROM clients WHERE route_id=:rid AND deleted_at IS NULL");
    q.bindValue(":rid", routeId.toString(QUuid::WithoutBraces));
    if (q.exec()) while (q.next()) res.append(mapRowToClient(q));
    return res;
}

QList<Client> RepositoryClient::getPendingSync() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Client> res;
    if (q.exec("SELECT * FROM clients WHERE sync_status='PENDING' AND deleted_at IS NULL")) {
        while (q.next())
            res.append(mapRowToClient(q));
    }
    return res;
}

QList<Client> RepositoryClient::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Client> res;
    q.prepare("SELECT * FROM clients WHERE version>=:v AND deleted_at IS NULL");
    q.bindValue(":v", minVersion);
    if (q.exec()) while (q.next()) res.append(mapRowToClient(q));
    return res;
}