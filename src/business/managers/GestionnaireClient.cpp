#include "GestionnaireClient.h"
#include "../exceptions/ValidationException.h"
#include "../services/ServiceValidation.h"
#include "../../data/repositories/RepositoryClient.h"
#include "../../data/repositories/RepositorySales.h"
#include "../../core/entities/Client.h"
#include <QDebug>

GestionnaireClient::GestionnaireClient()
{
}

QUuid GestionnaireClient::creerClient(const QString& nom, const QString& adresse, 
                                      const QString& telephone, const QString& email,
                                      const QUuid& routeId, const QString& conditionsPaiement)
{
    try {
        QStringList erreurs;
        
        if (!ServiceValidation::validateNotEmpty(nom)) {
            erreurs << "Nom requis";
        }
        
        if (!email.isEmpty() && !ServiceValidation::validateEmail(email)) {
            erreurs << "Email invalide";
        }
        
        if (!telephone.isEmpty() && !ServiceValidation::validatePhone(telephone)) {
            erreurs << "Téléphone invalide";
        }
        
        if (!erreurs.isEmpty()) {
            throw ValidationException("Erreurs de validation", erreurs);
        }
        
        Client client;
        client.setNom(nom);
        client.setAdresse(adresse);
        client.setTelephone(telephone);
        client.setEmail(email);
        client.setRouteId(routeId);
        client.setConditionsPaiement(Client::stringToConditionsPaiement(conditionsPaiement));
        client.setEstActif(true);
        
        RepositoryClient repo;
        if (!repo.create(client)) {
            m_dernierErreur = "Erreur lors de la création : " + repo.getLastError();
            return QUuid();
        }
        
        qDebug() << "Client créé :" << client.getClientId().toString();
        return client.getClientId();
        
    } catch (const ValidationException& e) {
        m_dernierErreur = e.getMessage();
        return QUuid();
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return QUuid();
    }
}

bool GestionnaireClient::modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse)
{
    try {
        if (clientId.isNull() || !ServiceValidation::validateNotEmpty(nom)) {
            m_dernierErreur = "Paramètres invalides";
            return false;
        }
        
        RepositoryClient repo;
        Client client = repo.getById(clientId);
        
        if (client.getClientId().isNull()) {
            m_dernierErreur = "Client non trouvé";
            return false;
        }
        
        client.setNom(nom);
        client.setAdresse(adresse);
        
        return repo.update(client);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

bool GestionnaireClient::desactiverClient(const QUuid& clientId)
{
    try {
        RepositoryClient repo;
        return repo.remove(clientId);
        
    } catch (const std::exception& e) {
        m_dernierErreur = QString::fromStdString(e.what());
        return false;
    }
}

Client GestionnaireClient::obtenirClient(const QUuid& clientId)
{
    RepositoryClient repo;
    return repo.getById(clientId);
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

double GestionnaireClient::obtenirChiffreAffairesClient(const QUuid& clientId)
{
    RepositorySales repo;
    QList<Vente> ventes = repo.getByClient(clientId);
    
    double total = 0.0;
    for (const auto& vente : ventes) {
        total += vente.getMontantTotal();
    }
    
    return total;
}

double GestionnaireClient::obtenirTotalCreditsClient(const QUuid& clientId)
{
    // À implémenter avec les crédits
    return 0.0;
}

int GestionnaireClient::obtenirNombreVentesClient(const QUuid& clientId)
{
    RepositorySales repo;
    return repo.getByClient(clientId).count();
}

bool GestionnaireClient::validerContact(const QString& email, const QString& telephone)
{
    bool emailValide = email.isEmpty() || ServiceValidation::validateEmail(email);
    bool telephoneValide = telephone.isEmpty() || ServiceValidation::validatePhone(telephone);
    
    return emailValide && telephoneValide;
}