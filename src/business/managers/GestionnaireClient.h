#ifndef GESTIONNAIRECLIENT_H
#define GESTIONNAIRECLIENT_H

#include <QString>
#include <QList>
#include <QUuid>
#include "../../core/entities/Client.h"

class GestionnaireClient
{
public:
    GestionnaireClient();

    // CRUD et offline-first
    QUuid creerClient(const QString& nom, const QString& adresse, const QString& telephone,
                      const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId = QUuid());
    bool modifierClient(const QUuid& clientId, const QString& nom, const QString& adresse, const QString& telephone,
                        const QString& email, const QUuid& routeId, const QUuid& conditionPaiementId, const QUuid& grilleId = QUuid());
    bool desactiverClient(const QUuid& clientId);

    // Accès
    Client obtenirClient(const QUuid& clientId);
    QList<Client> obtenirClientsRoute(const QUuid& routeId);
    QList<Client> obtenirTousClients();
    QList<Client> rechercherClient(const QString& nom);

    // SYNC offline
    QList<Client> clientsEnAttenteSync();
    QList<Client> clientsDepuisVersion(int version);

    // Erreur :
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIRECLIENT_H
