#include "RepositoryClient.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryClient::RepositoryClient() {}

Client RepositoryClient::mapRowToClient(const QSqlQuery& q) const
{
    Client c;
    c.setClientId(QUuid(q.value("client_id").toString()));
    c.setNom(q.value("nom").toString());
    c.setRouteId(QUuid(q.value("route_id").toString()));
    c.setConditionPaiementId(QUuid(q.value("condition_paiement_id").toString()));
    c.setAdresse(q.value("adresse").toString());
    c.setTelephone(q.value("telephone").toString());
    c.setEmail(q.value("email").toString());
    c.setDateMiseAJour(q.value("date_mise_a_jour").toDateTime());
    c.setCreatedAt(q.value("created_at").toDateTime());
    c.setUpdatedAt(q.value("updated_at").toDateTime());
    c.setVersion(q.value("version").toInt());
    c.setSyncState(Client::syncStateFromString(q.value("sync_status").toString()));
    c.setDeletedAt(q.value("deleted_at").toDateTime());
    c.setGrilleId(QUuid(q.value("grille_id").toString()));
    // Optionnel : charger le nom du paiement si jointure
    return c;
}

bool RepositoryClient::create(const Client& e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        INSERT INTO clients (
            client_id, nom, route_id, condition_paiement_id,
            adresse, telephone, email,
            date_mise_a_jour, version, sync_status, created_at, updated_at, grille_id
        ) VALUES
            (:id, :nom, :route, :cond, :adr, :tel, :em, :dmj, :version, :sync, :created, :updated, :grille)
    )");
    q.bindValue(":id", e.getClientId().toString(QUuid::WithoutBraces));
    q.bindValue(":nom", e.getNom());
    q.bindValue(":route", e.getRouteId().toString(QUuid::WithoutBraces));
    q.bindValue(":cond", e.getConditionPaiementId().toString(QUuid::WithoutBraces));
    q.bindValue(":adr", e.getAdresse());
    q.bindValue(":tel", e.getTelephone());
    q.bindValue(":em", e.getEmail());
    q.bindValue(":dmj", e.getDateMiseAJour());
    q.bindValue(":version", e.getVersion());
    q.bindValue(":sync", e.syncStateString());
    q.bindValue(":created", e.getCreatedAt());
    q.bindValue(":updated", e.getUpdatedAt());
    q.bindValue(":grille", e.getGrilleId().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur création client : " + q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryClient::update(const Client& e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE clients set
            nom=:nom, route_id=:route, condition_paiement_id=:cond, adresse=:adr, telephone=:tel, email=:em,
            date_mise_a_jour=:dmj, version=:version, sync_status=:sync, updated_at=:updated, grille_id=:grille
        WHERE client_id=:id AND deleted_at IS NULL
    )");
    q.bindValue(":nom", e.getNom());
    q.bindValue(":route", e.getRouteId().toString(QUuid::WithoutBraces));
    q.bindValue(":cond", e.getConditionPaiementId().toString(QUuid::WithoutBraces));
    q.bindValue(":adr", e.getAdresse());
    q.bindValue(":tel", e.getTelephone());
    q.bindValue(":em", e.getEmail());
    q.bindValue(":dmj", e.getDateMiseAJour());
    q.bindValue(":version", e.getVersion());
    q.bindValue(":sync", e.syncStateString());
    q.bindValue(":updated", e.getUpdatedAt());
    q.bindValue(":grille", e.getGrilleId().toString(QUuid::WithoutBraces));
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
    q.prepare(R"(UPDATE clients SET deleted_at=NOW(), sync_status='PENDING', version=version+1 WHERE client_id=:id AND deleted_at IS NULL)");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur suppression client : " + q.lastError().text();
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
