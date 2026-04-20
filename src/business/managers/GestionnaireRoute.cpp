#include "GestionnaireRoute.h"
#include "../../data/repositories/RepositoryRoute.h"
#include "../../core/entities/Route.h"

QUuid GestionnaireRoute::creerRoute(const QString& nom, const QString& description)
{
    m_dernierErreur.clear();

    Route route;
    route.setRouteId(QUuid::createUuid());
    route.setNom(nom);
    route.setDescription(description);
    route.setEstActif(true);

    RepositoryRoute repo;
    if (!repo.create(route)) {
        m_dernierErreur = repo.getLastError();
        return QUuid();
    }

    return route.getRouteId();
}