#include "RepositoryRepartition.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryRepartition::RepositoryRepartition()
{
}

bool RepositoryRepartition::create(const Repartition& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO repartitions "
                  "(repartition_id, equipe_id, route_id, statut, date_repartition, date_retour, montant_cash_attendu, cree_par) "
                  "VALUES (:id, :equipe_id, :route_id, :statut, :date_rep, :date_ret, :montant, :cree_par)");

    query.addBindValue(entity.getRepartitionId().toString());
    query.addBindValue(entity.getEquipeId().toString());
    query.addBindValue(entity.getRouteId().toString());
    query.addBindValue(Repartition::statutToString(entity.getStatut()));
    query.addBindValue(entity.getDateRepartition());
    query.addBindValue(entity.getDateRetour());
    query.addBindValue(entity.getMontantCashAttendu());
    query.addBindValue(entity.getCreePar().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création répartition : " + query.lastError().text();
        return false;
    }

    return true;
}

Repartition RepositoryRepartition::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM repartitions WHERE repartition_id = :id");
    query.addBindValue(id.toString());

    Repartition repartition;
    if (query.exec() && query.next()) {
        repartition.setRepartitionId(QUuid(query.value("repartition_id").toString()));
        repartition.setEquipeId(QUuid(query.value("equipe_id").toString()));
        repartition.setRouteId(QUuid(query.value("route_id").toString()));
        repartition.setStatut(Repartition::stringToStatut(query.value("statut").toString()));
        repartition.setDateRepartition(query.value("date_repartition").toDate());
        repartition.setDateRetour(query.value("date_retour").toDate());
        repartition.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
        repartition.setDateCreation(query.value("date_creation").toDateTime());
    } else {
        m_dernierErreur = "Répartition non trouvée";
    }

    return repartition;
}

QList<Repartition> RepositoryRepartition::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> repartitions;

    query.prepare("SELECT * FROM repartitions ORDER BY date_repartition DESC");

    if (query.exec()) {
        while (query.next()) {
            Repartition repartition;
            repartition.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            repartition.setEquipeId(QUuid(query.value("equipe_id").toString()));
            repartition.setRouteId(QUuid(query.value("route_id").toString()));
            repartition.setStatut(Repartition::stringToStatut(query.value("statut").toString()));
            repartition.setDateRepartition(query.value("date_repartition").toDate());
            repartitions.append(repartition);
        }
    }

    return repartitions;
}

bool RepositoryRepartition::update(const Repartition& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE repartitions SET "
                  "statut = :statut, date_retour = :date_ret, montant_cash_attendu = :montant "
                  "WHERE repartition_id = :id");

    query.addBindValue(Repartition::statutToString(entity.getStatut()));
    query.addBindValue(entity.getDateRetour());
    query.addBindValue(entity.getMontantCashAttendu());
    query.addBindValue(entity.getRepartitionId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour répartition : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryRepartition::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE repartitions SET statut = :statut WHERE repartition_id = :id");
    query.addBindValue(Repartition::statutToString(Repartition::Statut::Annulee));
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression répartition : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Repartition> RepositoryRepartition::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> repartitions;

    query.prepare("SELECT r.* FROM repartitions r "
                  "JOIN equipes e ON r.equipe_id = e.equipe_id "
                  "WHERE e.nom ILIKE :criterion ORDER BY r.date_repartition DESC");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Repartition repartition;
            repartition.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            repartition.setEquipeId(QUuid(query.value("equipe_id").toString()));
            repartitions.append(repartition);
        }
    }

    return repartitions;
}

bool RepositoryRepartition::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM repartitions WHERE repartition_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

QList<Repartition> RepositoryRepartition::getByEquipe(const QUuid& equipeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> repartitions;

    query.prepare("SELECT * FROM repartitions WHERE equipe_id = :equipe_id ORDER BY date_repartition DESC");
    query.addBindValue(equipeId.toString());

    if (query.exec()) {
        while (query.next()) {
            Repartition repartition;
            repartition.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            repartitions.append(repartition);
        }
    }

    return repartitions;
}

QList<Repartition> RepositoryRepartition::getByRoute(const QUuid& routeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> repartitions;

    query.prepare("SELECT * FROM repartitions WHERE route_id = :route_id ORDER BY date_repartition DESC");
    query.addBindValue(routeId.toString());

    if (query.exec()) {
        while (query.next()) {
            Repartition repartition;
            repartition.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            repartitions.append(repartition);
        }
    }

    return repartitions;
}

QList<Repartition> RepositoryRepartition::getByStatut(const Repartition::Statut& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> repartitions;

    query.prepare("SELECT * FROM repartitions WHERE statut = :statut ORDER BY date_repartition DESC");
    query.addBindValue(Repartition::statutToString(statut));

    if (query.exec()) {
        while (query.next()) {
            Repartition repartition;
            repartition.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            repartitions.append(repartition);
        }
    }

    return repartitions;
}