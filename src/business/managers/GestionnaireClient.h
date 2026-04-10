#ifndef GESTIONNAIRECLIENT_H
#define GESTIONNAIRECLIENT_H

#include <QString>
#include <QList>
#include <QUuid>

class Client;

class GestionnaireClient
{
public:
    GestionnaireClient();

    // Client operations
    QUuid creerClient(const QString& nom, const QString& adresse, const QString& telephone,
                     const QString& email, const QUuid& routeId, const QString& conditionsPaiement);
    bool modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse);
    bool desactiverClient(const QUuid& clientId);
    
    // Client queries
    Client obtenirClient(const QUuid& clientId);
    QList<Client> obtenirClientsRoute(const QUuid& routeId);
    QList<Client> obtenirTousClients();
    QList<Client> rechercherClient(const QString& nom);
    
    // Client statistics
    double obtenirChiffreAffairesClient(const QUuid& clientId);
    double obtenirTotalCreditsClient(const QUuid& clientId);
    int obtenirNombreVentesClient(const QUuid& clientId);
    
    // Validation
    bool validerContact(const QString& email, const QString& telephone);
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIRECLIENT_H