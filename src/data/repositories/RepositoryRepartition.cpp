#include "RepositoryRepartition.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================================
// HELPERS POUR MAPPER L'ENUM C++ VERS LES UUIDS DE LA BASE DE DONNEES
// Ces UUIDs proviennent de ta table statuts_repartition dans structure.sql
// ============================================================================
namespace {
    QString getStatutUuid(Repartition::Statut statut) {
        switch (statut) {
            case Repartition::Statut::EnAttente: return "d6f4c36d-9951-44ba-b5e1-1e0609968275"; // BROUILLON
            case Repartition::Statut::EnCours:   return "e0059987-5a9f-44bd-b806-18434792491d"; // EN_COURS
            case Repartition::Statut::Completee: return "936c875a-f441-410d-9098-98531a60c073"; // COMPLETEE
            case Repartition::Statut::Annulee:   return "d8a91671-fd79-4af7-a75e-ea7e39b8be24"; // ANNULEE
            default:                             return "d6f4c36d-9951-44ba-b5e1-1e0609968275";
        }
    }

    Repartition::Statut getStatutFromUuid(const QString& uuidStr) {
        if (uuidStr == "e0059987-5a9f-44bd-b806-18434792491d") return Repartition::Statut::EnCours;
        if (uuidStr == "936c875a-f441-410d-9098-98531a60c073") return Repartition::Statut::Completee;
        if (uuidStr == "d8a91671-fd79-4af7-a75e-ea7e39b8be24") return Repartition::Statut::Annulee;
        return Repartition::Statut::EnAttente; // Par défaut (Brouillon)
    }
}

RepositoryRepartition::RepositoryRepartition()
{
}

bool RepositoryRepartition::create(const Repartition& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO repartitions (repartition_id, equipe_id, route_id, "
                  "statut_repartition_id, date_repartition, montant_cash_attendu, chef_id) "
                  "VALUES (:id, :equipe, :route, :statut, :date, :montant, :chef)");

    query.bindValue(":id", entity.getRepartitionId().toString(QUuid::WithoutBraces));
    query.bindValue(":equipe", entity.getEquipeId().toString(QUuid::WithoutBraces));
    query.bindValue(":route", entity.getRouteId().toString(QUuid::WithoutBraces));
    
    // C'EST ICI QUE LE NULL ARRIVAIT : On utilise le helper getStatutUuid
    query.bindValue(":statut", getStatutUuid(entity.getStatut()));
    
    query.bindValue(":date", entity.getDateRepartition());
    query.bindValue(":montant", entity.getMontantCashAttendu());
    query.bindValue(":chef", entity.getCreePar().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur SQL : " + query.lastError().text();
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
        rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString())); // Utilisation du Helper
        rep.setDateRepartition(query.value("date_repartition").toDate());
        rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
        rep.setCreePar(QUuid(query.value("chef_id").toString()));
        rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
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
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("chef_id").toString()));
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

    // CORRECTION : Suppression des colonnes inexistantes
    query.prepare(
        "UPDATE repartitions SET equipe_id = :equipe_id, route_id = :route_id, statut_repartition_id = :statut_repartition_id, "
        "date_repartition = :date_repartition, montant_cash_attendu = :montant_cash_attendu, "
        "chef_id = :chef_id, date_mise_a_jour = :date_mise_a_jour "
        "WHERE repartition_id = :repartition_id"
    );

    query.bindValue(":equipe_id", entity.getEquipeId().toString());
    query.bindValue(":route_id", entity.getRouteId().toString());
    query.bindValue(":statut_repartition_id", getStatutUuid(entity.getStatut())); // Utilisation du Helper
    query.bindValue(":date_repartition", entity.getDateRepartition());
    query.bindValue(":montant_cash_attendu", entity.getMontantCashAttendu());
    query.bindValue(":chef_id", entity.getCreePar().toString());
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
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("chef_id").toString()));
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
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("chef_id").toString()));
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
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("chef_id").toString()));
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
    // CORRECTION : On bind l'UUID exact et pas l'entier
    query.bindValue(":statut_id", getStatutUuid(statut));

    if (query.exec()) {
        while (query.next()) {
            Repartition rep;
            rep.setRepartitionId(QUuid(query.value("repartition_id").toString()));
            rep.setEquipeId(QUuid(query.value("equipe_id").toString()));
            rep.setRouteId(QUuid(query.value("route_id").toString()));
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            rep.setDateRepartition(query.value("date_repartition").toDate());
            rep.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
            rep.setCreePar(QUuid(query.value("chef_id").toString()));
            rep.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
            reps.append(rep);
        }
    }
    return reps;
}