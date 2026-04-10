#include "RepositoryClient.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryClient::RepositoryClient()
{
}

bool RepositoryClient::create(const Client& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO clients "
                  "(client_id, nom, adresse, telephone, email, route_id, personne_contact, conditions_paiement, est_actif) "
                  "VALUES (:id, :nom, :adresse, :tel, :email, :route_id, :personne, :conditions, :actif)");

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

Client RepositoryClient::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM clients WHERE client_id = :id");
    query.addBindValue(id.toString());

    Client client;
    if (query.exec() && query.next()) {
        client.setClientId(QUuid(query.value("client_id").toString()));
        client.setNom(query.value("nom").toString());
        client.setAdresse(query.value("adresse").toString());
        client.setTelephone(query.value("telephone").toString());
        client.setEmail(query.value("email").toString());
        client.setRouteId(QUuid(query.value("route_id").toString()));
        client.setPersonneContact(query.value("personne_contact").toString());
        client.setConditionsPaiement(Client::stringToConditionsPaiement(query.value("conditions_paiement").toString()));
        client.setEstActif(query.value("est_actif").toBool());
    } else {
        m_dernierErreur = "Client non trouvé";
    }

    return client;
}

QList<Client> RepositoryClient::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    query.prepare("SELECT * FROM clients WHERE est_actif = true ORDER BY nom");

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            client.setTelephone(query.value("telephone").toString());
            client.setEmail(query.value("email").toString());
            client.setRouteId(QUuid(query.value("route_id").toString()));
            clients.append(client);
        }
    }

    return clients;
}

bool RepositoryClient::update(const Client& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE clients SET "
                  "nom = :nom, adresse = :adresse, telephone = :tel, email = :email, "
                  "route_id = :route_id, personne_contact = :personne, conditions_paiement = :conditions, est_actif = :actif "
                  "WHERE client_id = :id");

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

bool RepositoryClient::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE clients SET est_actif = false WHERE client_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression client : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Client> RepositoryClient::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    query.prepare("SELECT * FROM clients WHERE nom ILIKE :criterion ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            clients.append(client);
        }
    }

    return clients;
}

bool RepositoryClient::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM clients WHERE client_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

QList<Client> RepositoryClient::getByRoute(const QUuid& routeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Client> clients;

    query.prepare("SELECT * FROM clients WHERE route_id = :route_id AND est_actif = true ORDER BY nom");
    query.addBindValue(routeId.toString());

    if (query.exec()) {
        while (query.next()) {
            Client client;
            client.setClientId(QUuid(query.value("client_id").toString()));
            client.setNom(query.value("nom").toString());
            client.setAdresse(query.value("adresse").toString());
            clients.append(client);
        }
    }

    return clients;
}