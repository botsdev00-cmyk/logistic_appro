#include "RepositoryReceptionCaisse.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryReceptionCaisse::RepositoryReceptionCaisse()
{
}

bool RepositoryReceptionCaisse::create(const ReceptionCaisse& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO receptions_caisse "
                  "(reception_caisse_id, repartition_id, montant_attendu, montant_recu, numero_recu, statut, caissier_id, notes) "
                  "VALUES (:id, :rep_id, :attendu, :recu, :numero, :statut, :caissier_id, :notes)");

    query.addBindValue(entity.getReceptionCaisseId().toString());
    query.addBindValue(entity.getRepartitionId().toString());
    query.addBindValue(entity.getMontantAttendu());
    query.addBindValue(entity.getMontantRecu());
    query.addBindValue(entity.getNumeroRecu());
    query.addBindValue(ReceptionCaisse::statutToString(entity.getStatut()));
    query.addBindValue(entity.getCaissierId().toString());
    query.addBindValue(entity.getNotes());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création réception caisse : " + query.lastError().text();
        return false;
    }

    return true;
}

ReceptionCaisse RepositoryReceptionCaisse::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM receptions_caisse WHERE reception_caisse_id = :id");
    query.addBindValue(id.toString());

    ReceptionCaisse reception;
    if (query.exec() && query.next()) {
        reception.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
        reception.setRepartitionId(QUuid(query.value("repartition_id").toString()));
        reception.setMontantAttendu(query.value("montant_attendu").toDouble());
        reception.setMontantRecu(query.value("montant_recu").toDouble());
        reception.setNumeroRecu(query.value("numero_recu").toString());
        reception.setStatut(ReceptionCaisse::stringToStatut(query.value("statut").toString()));
        reception.setCaissierId(QUuid(query.value("caissier_id").toString()));
        reception.setDateReception(query.value("date_reception").toDateTime());
    } else {
        m_dernierErreur = "Réception caisse non trouvée";
    }

    return reception;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> receptions;

    query.prepare("SELECT * FROM receptions_caisse ORDER BY date_reception DESC");

    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse reception;
            reception.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
            reception.setMontantAttendu(query.value("montant_attendu").toDouble());
            reception.setMontantRecu(query.value("montant_recu").toDouble());
            reception.setStatut(ReceptionCaisse::stringToStatut(query.value("statut").toString()));
            receptions.append(reception);
        }
    }

    return receptions;
}

bool RepositoryReceptionCaisse::update(const ReceptionCaisse& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE receptions_caisse SET "
                  "montant_recu = :recu, statut = :statut, date_reception = :date_reception, notes = :notes "
                  "WHERE reception_caisse_id = :id");

    query.addBindValue(entity.getMontantRecu());
    query.addBindValue(ReceptionCaisse::statutToString(entity.getStatut()));
    query.addBindValue(entity.getDateReception());
    query.addBindValue(entity.getNotes());
    query.addBindValue(entity.getReceptionCaisseId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour réception caisse : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryReceptionCaisse::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("DELETE FROM receptions_caisse WHERE reception_caisse_id = :id");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression réception caisse : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> receptions;

    query.prepare("SELECT * FROM receptions_caisse WHERE numero_recu ILIKE :criterion ORDER BY date_reception DESC");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse reception;
            reception.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
            receptions.append(reception);
        }
    }

    return receptions;
}

bool RepositoryReceptionCaisse::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM receptions_caisse WHERE reception_caisse_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

ReceptionCaisse RepositoryReceptionCaisse::getByRepartition(const QUuid& repartitionId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM receptions_caisse WHERE repartition_id = :rep_id");
    query.addBindValue(repartitionId.toString());

    ReceptionCaisse reception;
    if (query.exec() && query.next()) {
        reception.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
        reception.setMontantAttendu(query.value("montant_attendu").toDouble());
        reception.setMontantRecu(query.value("montant_recu").toDouble());
    }

    return reception;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getByStatut(const ReceptionCaisse::Statut& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> receptions;

    query.prepare("SELECT * FROM receptions_caisse WHERE statut = :statut ORDER BY date_reception DESC");
    query.addBindValue(ReceptionCaisse::statutToString(statut));

    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse reception;
            reception.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
            receptions.append(reception);
        }
    }

    return receptions;
}

QList<ReceptionCaisse> RepositoryReceptionCaisse::getWithDiscrepancies()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<ReceptionCaisse> receptions;

    query.prepare("SELECT * FROM receptions_caisse WHERE statut = :statut OR ecart != 0 ORDER BY date_reception DESC");
    query.addBindValue(ReceptionCaisse::statutToString(ReceptionCaisse::Statut::Discrepance));

    if (query.exec()) {
        while (query.next()) {
            ReceptionCaisse reception;
            reception.setReceptionCaisseId(QUuid(query.value("reception_caisse_id").toString()));
            receptions.append(reception);
        }
    }

    return receptions;
}