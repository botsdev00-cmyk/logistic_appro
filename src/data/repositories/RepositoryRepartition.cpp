#include "RepositoryRepartition.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

// ===== Helpers for statut mapping (UUID <=> enum) ===================================
namespace {
QString getStatutUuid(Repartition::Statut statut) {
    if (statut == Repartition::Statut::EnAttente) return "d6f4c36d-9951-44ba-b5e1-1e0609968275";
    if (statut == Repartition::Statut::EnCours)   return "e0059987-5a9f-44bd-b806-18434792491d";
    if (statut == Repartition::Statut::Completee) return "936c875a-f441-410d-9098-98531a60c073";
    if (statut == Repartition::Statut::Annulee)   return "d8a91671-fd79-4af7-a75e-ea7e39b8be24";
    return "d6f4c36d-9951-44ba-b5e1-1e0609968275";
}
Repartition::Statut getStatutFromUuid(const QString& uuidStr) {
    if (uuidStr == "e0059987-5a9f-44bd-b806-18434792491d") return Repartition::Statut::EnCours;
    if (uuidStr == "936c875a-f441-410d-9098-98531a60c073") return Repartition::Statut::Completee;
    if (uuidStr == "d8a91671-fd79-4af7-a75e-ea7e39b8be24") return Repartition::Statut::Annulee;
    return Repartition::Statut::EnAttente;
}
}

RepositoryRepartition::RepositoryRepartition() {}

bool RepositoryRepartition::create(const Repartition& entity) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("INSERT INTO repartitions "
                  "(repartition_id, equipe_id, route_id, statut_repartition_id, date_repartition, montant_cash_attendu, chef_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(entity.getRepartitionId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getEquipeId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getRouteId().toString(QUuid::WithoutBraces));
    query.addBindValue(getStatutUuid(entity.getStatut()));
    query.addBindValue(entity.getDateRepartition());
    query.addBindValue(entity.getMontantCashAttendu());

    // ⚠️ Correction pour champ nullable chef_id
    QUuid chefId = entity.getChefId();
    if (chefId.isNull())
        query.addBindValue(QVariant(QVariant::String)); // génère NULL SQL
    else
        query.addBindValue(chefId.toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur création repartition : " + query.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryRepartition::update(const Repartition& entity) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(
        "UPDATE repartitions SET equipe_id=?, route_id=?, statut_repartition_id=?, date_repartition=?, montant_cash_attendu=?, "
        "chef_id=?, date_mise_a_jour=? WHERE repartition_id=?"
        );
    query.addBindValue(entity.getEquipeId().toString());
    query.addBindValue(entity.getRouteId().toString());
    query.addBindValue(getStatutUuid(entity.getStatut()));
    query.addBindValue(entity.getDateRepartition());
    query.addBindValue(entity.getMontantCashAttendu());

    // ⚠️ Correction pour champ nullable chef_id
    QUuid chefId = entity.getChefId();
    if (chefId.isNull())
        query.addBindValue(QVariant(QVariant::String));
    else
        query.addBindValue(chefId.toString());

    query.addBindValue(entity.getDateMiseAJour());
    query.addBindValue(entity.getRepartitionId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour repartition : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

Repartition RepositoryRepartition::getById(const QUuid& id) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM repartitions WHERE repartition_id = ?");
    query.addBindValue(id.toString());

    Repartition rep;
    if (query.exec() && query.next()) {
        rep = mapRowToRepartition(query);
        rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
    } else {
        m_dernierErreur = "Répartition non trouvée";
    }
    return rep;
}

QList<Repartition> RepositoryRepartition::getAll() const {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;
    if (query.exec("SELECT * FROM repartitions ORDER BY date_repartition DESC")) {
        while (query.next()) {
            Repartition rep = mapRowToRepartition(query);
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            reps.append(rep);
        }
    }
    return reps;
}

bool RepositoryRepartition::remove(const QUuid& id) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("DELETE FROM repartitions WHERE repartition_id = ?");
    query.addBindValue(id.toString());
    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression repartition : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<Repartition> RepositoryRepartition::search(const QString& criterion) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;
    query.prepare("SELECT * FROM repartitions WHERE CAST(repartition_id AS TEXT) ILIKE ? OR CAST(equipe_id AS TEXT) ILIKE ? OR CAST(route_id AS TEXT) ILIKE ? ORDER BY date_repartition DESC");
    QString c = "%" + criterion + "%";
    query.addBindValue(c); query.addBindValue(c); query.addBindValue(c);
    if (query.exec()) {
        while (query.next()) {
            Repartition rep = mapRowToRepartition(query);
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            reps.append(rep);
        }
    }
    return reps;
}

bool RepositoryRepartition::exists(const QUuid& id) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT 1 FROM repartitions WHERE repartition_id = ?");
    query.addBindValue(id.toString());
    return query.exec() && query.next();
}

QList<Repartition> RepositoryRepartition::getByEquipe(const QUuid& equipeId) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;
    query.prepare("SELECT * FROM repartitions WHERE equipe_id = ? ORDER BY date_repartition DESC");
    query.addBindValue(equipeId.toString());
    if (query.exec()) {
        while (query.next()) {
            Repartition rep = mapRowToRepartition(query);
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            reps.append(rep);
        }
    }
    return reps;
}

QList<Repartition> RepositoryRepartition::getByRoute(const QUuid& routeId) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;
    query.prepare("SELECT * FROM repartitions WHERE route_id = ? ORDER BY date_repartition DESC");
    query.addBindValue(routeId.toString());
    if (query.exec()) {
        while (query.next()) {
            Repartition rep = mapRowToRepartition(query);
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            reps.append(rep);
        }
    }
    return reps;
}

QList<Repartition> RepositoryRepartition::getByStatut(const Repartition::Statut& statut) {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Repartition> reps;
    query.prepare("SELECT * FROM repartitions WHERE statut_repartition_id = ? ORDER BY date_repartition DESC");
    query.addBindValue(getStatutUuid(statut));
    if (query.exec()) {
        while (query.next()) {
            Repartition rep = mapRowToRepartition(query);
            rep.setStatut(getStatutFromUuid(query.value("statut_repartition_id").toString()));
            reps.append(rep);
        }
    }
    return reps;
}

Repartition RepositoryRepartition::mapRowToRepartition(const QSqlQuery& query) const {
    Repartition r;
    r.setRepartitionId(QUuid(query.value("repartition_id").toString()));
    r.setEquipeId(QUuid(query.value("equipe_id").toString()));
    r.setRouteId(QUuid(query.value("route_id").toString()));
    if (query.record().indexOf("statut_repartition_id") >= 0)
        r.setStatutRepartitionId(QUuid(query.value("statut_repartition_id").toString()));
    if (query.record().indexOf("date_repartition") >= 0)
        r.setDateRepartition(query.value("date_repartition").toDate());
    if (query.record().indexOf("montant_cash_attendu") >= 0)
        r.setMontantCashAttendu(query.value("montant_cash_attendu").toDouble());
    if (query.record().indexOf("chef_id") >= 0)
        r.setChefId(QUuid(query.value("chef_id").toString()));
    if (query.record().indexOf("date_mise_a_jour") >= 0)
        r.setDateMiseAJour(query.value("date_mise_a_jour").toDateTime());
    if (query.record().indexOf("annule") >= 0)
        r.setAnnule(query.value("annule").toBool());
    if (query.record().indexOf("created_at") >= 0)
        r.setCreatedAt(query.value("created_at").toDateTime());
    if (query.record().indexOf("updated_at") >= 0)
        r.setUpdatedAt(query.value("updated_at").toDateTime());
    if (query.record().indexOf("deleted_at") >= 0)
        r.setDeletedAt(query.value("deleted_at").toDateTime());
    return r;
}
