#include "GestionnaireClient.h"
#include "../../data/repositories/RepositoryClient.h"
#include "../../core/entities/Client.h"
#include <QDebug>

GestionnaireClient::GestionnaireClient() {}

QUuid GestionnaireClient::creerClient(const QString& nom, const QString& adresse, const QString& telephone,
                                      const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId)
{
    try {
        Client client;
        client.setNom(nom);
        client.setAdresse(adresse);
        client.setTelephone(telephone);
        client.setEmail(email);
        client.setRouteId(routeId);
        client.setConditionPaiementId(conditionPaiementId);
        client.setGrilleId(grilleId);
        client.setSyncState(SyncState::PENDING);
        client.setVersion(1);
        // created_at/updated_at auto

        RepositoryClient repo;
        if (!repo.create(client)) {
            m_dernierErreur = repo.getLastError();
            return QUuid();
        }
        return client.getClientId();
    } catch (...) {
        m_dernierErreur = "Erreur imprévue lors de la création client";
        return QUuid();
    }
}

bool GestionnaireClient::modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse, const QString& telephone,
                                        const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId)
{
    try {
        RepositoryClient repo;
        auto optClient = repo.getById(clientId);
        if (!optClient) {
            m_dernierErreur = "Client introuvable";
            return false;
        }
        Client client = *optClient;
        client.setNom(nom);
        client.setAdresse(adresse);
        client.setTelephone(telephone);
        client.setEmail(email);
        client.setRouteId(routeId);
        client.setConditionPaiementId(conditionPaiementId);
        client.setGrilleId(grilleId);
        client.setUpdatedAt(QDateTime::currentDateTime());
        client.setSyncState(SyncState::PENDING);
        client.setVersion(client.getVersion() + 1);
        if (!repo.update(client)) {
            m_dernierErreur = repo.getLastError();
            return false;
        }
        return true;
    } catch (...) {
        m_dernierErreur = "Erreur inattendue lors de la modification";
        return false;
    }
}

bool GestionnaireClient::desactiverClient(const QUuid& clientId)
{
    RepositoryClient repo;
    return repo.logicalDelete(clientId);
}

Client GestionnaireClient::obtenirClient(const QUuid& clientId)
{
    RepositoryClient repo;
    auto opt = repo.getById(clientId);
    return opt.value_or(Client());
}

QList<Client> GestionnaireClient::obtenirClientsRoute(const QUuid& routeId)
{
    RepositoryClient repo;
    return repo.getByRoute(routeId);
}

QList<Client> GestionnaireClient::obtenirTousClients()
{
    RepositoryClient repo;
    return repo.getAll();
}

QList<Client> GestionnaireClient::rechercherClient(const QString& nom)
{
    RepositoryClient repo;
    return repo.search(nom);
}

QList<Client> GestionnaireClient::clientsEnAttenteSync()
{
    RepositoryClient repo;
    return repo.getPendingSync();
}

QList<Client> GestionnaireClient::clientsDepuisVersion(int version)
{
    RepositoryClient repo;
    return repo.getSinceVersion(version);
}
