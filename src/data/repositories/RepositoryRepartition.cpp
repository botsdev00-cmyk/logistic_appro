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

    query.prepare(
        "INSERT INTO repartitions (repartition_id, equipe_id, route_id, statut_repartition_id, date_repartition, date_retour, montant_cash_attendu, cree_par, date_creation, date_mise_a_jour) "
        "VALUES (:repartition_id, :equipe_id, :route_id, :statut_repartition_id, :date_repartition, :date_retour, :montant_cash_attendu, :cree_par, :date_creation, :date_mise_a_jour)"
    );

    query.bindValue(":repartition_id", entity.getRepartitionId().toString());
    query.bindValue(":equipe_id", entity.getEquipeId().toString());
    query.bindValue(":route_id", entity.getRouteId().toString());
    query.bindValue(":statut_repartition_id", static_cast<int>(entity.getStatut())); // enum/int attendu
    query.bindValue(":date_repartition", entity.getDateRepartition());
    query.bindValue(":date_retour", entity.getDateRetour());
    query.bindValue(":montant_cash_attendu", entity.getMontantCashAttendu());
    query.bindValue(":cree_par", entity.getCreePar().toString());
    query.bindValue(":date_creation", entity.getDateCreation());
    query.bindValue(":date_mise_a_jour", entity.getDateMiseAJour());

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
    query.bindValue(":id", id.toString());

    Repartition rep;
    if (query.exec() && query.next()) {
        rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
        rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
        rep.setRouteId(QUuid(query.value("route_id").toString()));
        rep.setStatut(Repartition::Statut(query.value("statut_repartition_id").toInt()));
        rep.setDateRepartition(query.value("date_repartition").toDate());
        rep.setDateRetour(query.value("date_retour").toDate());
        rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
        rep.setCreePar(QUuid(query.value("cree_par").toString()));
        rep.setDateCreation(query.value("date_creation").toDateTime());
        rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
        // + articles si tu veux les charger (appel séparé)
    } else {
        m_dernierErreur = "Répartition non trouvée";
    }
    return rep;
}

QList<Repartition> RepositoryRepartition::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;

    query.prepare("SELECT * FROM repartitions ORDER BY date_repartition DESC");

    if (query.exec()) {
        while (query.next()) {
            Repartition rep;
            rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
            rep.setRouteId(QUuid(query.value("route_id").toString()));
            rep.setStatut(Repartition::Statut(query.value("statut_repartition_id").toInt()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setDateRetour(query.value("date_retour").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("cree_par").toString()));
            rep.setDateCreation(query.value("date_creation").toDateTime());
            rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            reps.append(rep);
        }
    }
    return reps;
}

bool RepositoryRepartition::update(const Repartition& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "UPDATE repartitions SET equipe_id = :equipe_id, route_id = :route_id, statut_repartition_id = :statut_repartition_id, "
        "date_repartition = :date_repartition, date_retour = :date_retour, montant_cash_attendu = :montant_cash_attendu, "
        "cree_par = :cree_par, date_creation = :date_creation, date_mise_a_jour = :date_mise_a_jour "
        "WHERE repartition_id = :repartition_id"
    );

    query.bindValue(":equipe_id", entity.getEquipeId().toString());
    query.bindValue(":route_id", entity.getRouteId().toString());
    query.bindValue(":statut_repartition_id", static_cast<int>(entity.getStatut()));
    query.bindValue(":date_repartition", entity.getDateRepartition());
    query.bindValue(":date_retour", entity.getDateRetour());
    query.bindValue(":montant_cash_attendu", entity.getMontantCashAttendu());
    query.bindValue(":cree_par", entity.getCreePar().toString());
    query.bindValue(":date_creation", entity.getDateCreation());
    query.bindValue(":date_mise_a_jour", entity.getDateMiseAJour());
    query.bindValue(":repartition_id", entity.getRepartitionId().toString());

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

    query.prepare("DELETE FROM repartitions WHERE repartition_id = :id");
    query.bindValue(":id", id.toString());

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
    QList<Repartition> reps;

    query.prepare("SELECT * FROM repartitions WHERE CAST(repartition_id AS TEXT) ILIKE :crit OR CAST(equipe_id AS TEXT) ILIKE :crit OR CAST(route_id AS TEXT) ILIKE :crit ORDER BY date_repartition DESC");
    query.bindValue(":crit", "%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Repartition rep;
            rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
            rep.setRouteId(QUuid(query.value("route_id").toString()));
            rep.setStatut(Repartition::Statut(query.value("statut_repartition_id").toInt()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setDateRetour(query.value("date_retour").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("cree_par").toString()));
            rep.setDateCreation(query.value("date_creation").toDateTime());
            rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            reps.append(rep);
        }
    }
    return reps;
}

bool RepositoryRepartition::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM repartitions WHERE repartition_id = :id");
    query.bindValue(":id", id.toString());

    return query.exec() && query.next();
}

QList<Repartition> RepositoryRepartition::getByEquipe(const QUuid& equipeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;

    query.prepare("SELECT * FROM repartitions WHERE equipe_id = :equipe_id ORDER BY date_repartition DESC");
    query.bindValue(":equipe_id", equipeId.toString());

    if (query.exec()) {
        while (query.next()) {
            Repartition rep;
            rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
            rep.setRouteId(QUuid(query.value("route_id").toString()));
            rep.setStatut(Repartition::Statut(query.value("statut_repartition_id").toInt()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setDateRetour(query.value("date_retour").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("cree_par").toString()));
            rep.setDateCreation(query.value("date_creation").toDateTime());
            rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            reps.append(rep);
        }
    }
    return reps;
}

QList<Repartition> RepositoryRepartition::getByRoute(const QUuid& routeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;

    query.prepare("SELECT * FROM repartitions WHERE route_id = :route_id ORDER BY date_repartition DESC");
    query.bindValue(":route_id", routeId.toString());

    if (query.exec()) {
        while (query.next()) {
            Repartition rep;
            rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
            rep.setRouteId(QUuid(query.value("route_id").toString()));
            rep.setStatut(Repartition::Statut(query.value("statut_repartition_id").toInt()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setDateRetour(query.value("date_retour").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("cree_par").toString()));
            rep.setDateCreation(query.value("date_creation").toDateTime());
            rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            reps.append(rep);
        }
    }
    return reps;
}

QList<Repartition> RepositoryRepartition::getByStatut(const Repartition::Statut& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;

    query.prepare("SELECT * FROM repartitions WHERE statut_repartition_id = :statut_id ORDER BY date_repartition DESC");
    query.bindValue(":statut_id", static_cast<int>(statut));

    if (query.exec()) {
        while (query.next()) {
            Repartition rep;
            rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
            rep.setRouteId(QUuid(query.value("route_id").toString()));
            rep.setStatut(Repartition::Statut(query.value("statut_repartition_id").toInt()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setDateRetour(query.value("date_retour").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("cree_par").toString()));
            rep.setDateCreation(query.value("date_creation").toDateTime());
            rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            reps.append(rep);
        }
    }
    return reps;
}