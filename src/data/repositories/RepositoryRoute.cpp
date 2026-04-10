#include "RepositoryRoute.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryRoute::RepositoryRoute()
{
}

bool RepositoryRoute::create(const Route& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO routes "
                  "(route_id, nom, description, est_actif) "
                  "VALUES (:id, :nom, :description, :actif)");

    query.addBindValue(entity.getRouteId().toString());
    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.estActif());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création route : " + query.lastError().text();
        return false;
    }

    return true;
}

Route RepositoryRoute::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM routes WHERE route_id = :id");
    query.addBindValue(id.toString());

    Route route;
    if (query.exec() && query.next()) {
        route.setRouteId(QUuid(query.value("route_id").toString()));
        route.setNom(query.value("nom").toString());
        route.setDescription(query.value("description").toString());
        route.setEstActif(query.value("est_actif").toBool());
    } else {
        m_dernierErreur = "Route non trouvée";
    }

    return route;
}

QList<Route> RepositoryRoute::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Route> routes;

    query.prepare("SELECT * FROM routes WHERE est_actif = true ORDER BY nom");

    if (query.exec()) {
        while (query.next()) {
            Route route;
            route.setRouteId(QUuid(query.value("route_id").toString()));
            route.setNom(query.value("nom").toString());
            route.setDescription(query.value("description").toString());
            routes.append(route);
        }
    }

    return routes;
}

bool RepositoryRoute::update(const Route& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE routes SET "
                  "nom = :nom, description = :description, est_actif = :actif "
                  "WHERE route_id = :id");

    query.addBindValue(entity.getNom());
    query.addBindValue(entity.getDescription());
    query.addBindValue(entity.estActif());
    query.addBindValue(entity.getRouteId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour route : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryRoute::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE routes SET est_actif = false WHERE route_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression route : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Route> RepositoryRoute::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Route> routes;

    query.prepare("SELECT * FROM routes WHERE nom ILIKE :criterion ORDER BY nom");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Route route;
            route.setRouteId(QUuid(query.value("route_id").toString()));
            route.setNom(query.value("nom").toString());
            routes.append(route);
        }
    }

    return routes;
}

bool RepositoryRoute::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM routes WHERE route_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}