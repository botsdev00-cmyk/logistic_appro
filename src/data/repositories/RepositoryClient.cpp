#include "RepositoryClient.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryClient::RepositoryClient() {}

bool RepositoryClient::create(const Client& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        INSERT INTO clients
          (client_id, nom, adresse, telephone, email, route_id, personne_contact, conditions_paiement, est_actif, 
           sync_status, version, created_at, updated_at)
        VALUES 
          (?, ?, ?, ?, ?, ?, ?, ?, ?, 'PENDING', 1, NOW(), NOW())
    )");

    query.addBindValue(entity.getClientId().toString());
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getAdresse());
    query.addBindValue(entity.getTelephone());
    query.addBindValue(entity.getEmail());
    query.addBindValue(entity.getRouteId().toString());
    query.addBindValue(entity.getPersonneContact());
    query.addBindValue(Client::conditionsPaiementToString(entity.getConditionsPaiement()));
    query.addBindValue(entity.estActif());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création client : " + query.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryClient::update(const Client& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE clients SET
            nom = ?, adresse = ?, telephone = ?, email = ?,
            route_id = ?, personne_contact = ?, conditions_paiement = ?, est_actif = ?,
            updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE client_id = ? AND deleted_at IS NULL
    )");

    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getAdresse());
    query.addBindValue(entity.getTelephone());
    query.addBindValue(entity.getEmail());
    query.addBindValue(entity.getRouteId().toString());
    query.addBindValue(entity.getPersonneContact());
    query.addBindValue(Client::conditionsPaiementToString(entity.getConditionsPaiement()));
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getClientId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour client : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool RepositoryClient::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE clients 
        SET deleted_at = NOW(), sync_status = 'PENDING', version = version + 1
        WHERE client_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression client : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

std::optional<Client> RepositoryClient::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM clients WHERE client_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());

    if (query.exec() && query.next()) {
        Client client;
        client.setClientId(QUuid(query.value("client_id").toString()));
        client.setNom(query.value("nom").toString());
        client.setAdresse(query.value("adresse").toString());
        client.setTelephone(query.value("telephone").toString());
        client.setEmail(query.value("email").toString());
        client.setRouteId(QUuid(query.value("route_id").toString()));
        client.setPersonneContact(query.value("personne_contact").toString());
        client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
        client.setEstActif(query.value("est_actif").toBool());
        return client;
    }
    return std::nullopt;
}

QList<Client> RepositoryClient::getAll() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    if (query.exec("SELECT * FROM clients WHERE deleted_at IS NULL ORDER BY nom")) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            client.setTelephone(query.value("telephone").toString());
            client.setEmail(query.value("email").toString());
            client.setRouteId(QUuid(query.value("route_id").toString()));
            client.setPersonneContact(query.value("personne_contact").toString());
            client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
            client.setEstActif(query.value("est_actif").toBool());
            clients.append(client);
        }
    }
    return clients;
}

QList<Client> RepositoryClient::search(const QString& criterion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    query.prepare("SELECT * FROM clients WHERE nom ILIKE ? AND deleted_at IS NULL ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            client.setTelephone(query.value("telephone").toString());
            client.setEmail(query.value("email").toString());
            client.setRouteId(QUuid(query.value("route_id").toString()));
            client.setPersonneContact(query.value("personne_contact").toString());
            client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
            client.setEstActif(query.value("est_actif").toBool());
            clients.append(client);
        }
    }
    return clients;
}

bool RepositoryClient::exists(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM clients WHERE client_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

QList<Client> RepositoryClient::getByRoute(const QUuid& routeId) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    query.prepare("SELECT * FROM clients WHERE route_id = ? AND deleted_at IS NULL ORDER BY nom");
    query.addBindValue(routeId.toString());

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            client.setTelephone(query.value("telephone").toString());
            client.setEmail(query.value("email").toString());
            client.setRouteId(QUuid(query.value("route_id").toString()));
            client.setPersonneContact(query.value("personne_contact").toString());
            client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
            client.setEstActif(query.value("est_actif").toBool());
            clients.append(client);
        }
    }
    return clients;
}

QList<Client> RepositoryClient::getPendingSync() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    if (query.exec("SELECT * FROM clients WHERE sync_status = 'PENDING' AND deleted_at IS NULL")) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            client.setTelephone(query.value("telephone").toString());
            client.setEmail(query.value("email").toString());
            client.setRouteId(QUuid(query.value("route_id").toString()));
            client.setPersonneContact(query.value("personne_contact").toString());
            client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
            client.setEstActif(query.value("est_actif").toBool());
            clients.append(client);
        }
    }
    return clients;
}

QList<Client> RepositoryClient::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    query.prepare("SELECT * FROM clients WHERE version >= ? AND deleted_at IS NULL");
    query.addBindValue(minVersion);

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            client.setTelephone(query.value("telephone").toString());
            client.setEmail(query.value("email").toString());
            client.setRouteId(QUuid(query.value("route_id").toString()));
            client.setPersonneContact(query.value("personne_contact").toString());
            client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
            client.setEstActif(query.value("est_actif").toBool());
            clients.append(client);
        }
    }
    return clients;
}