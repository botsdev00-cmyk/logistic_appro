#include "RepositoryCredit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryCredit::RepositoryCredit()
{
}

bool RepositoryCredit::create(const Credit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO credits "
                  "(credit_id, vente_id, client_id, montant, date_echeance, statut, notes) "
                  "VALUES (:id, :vente_id, :client_id, :montant, :echeance, :statut, :notes)");

    query.addBindValue(entity.getCreditId().toString());
    query.addBindValue(entity.getVenteId().toString());
    query.addBindValue(entity.getClientId().toString());
    query.addBindValue(entity.getMontant());
    query.addBindValue(entity.getDateEcheance());
    query.addBindValue(Credit::statutToString(entity.getStatut()));
    query.addBindValue(entity.getNotes());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création crédit : " + query.lastError().text();
        return false;
    }

    return true;
}

Credit RepositoryCredit::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM credits WHERE credit_id = :id");
    query.addBindValue(id.toString());

    Credit credit;
    if (query.exec() && query.next()) {
        credit.setCreditId(QUuid(query.value("credit_id").toString()));
        credit.setVenteId(QUuid(query.value("vente_id").toString()));
        credit.setClientId(QUuid(query.value("client_id").toString()));
        credit.setMontant(query.value("montant").toDouble());
        credit.setDateEcheance(query.value("date_echeance").toDate());
        credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
        credit.setDatePaiement(query.value("date_paiement").toDate());
    } else {
        m_dernierErreur = "Crédit non trouvé";
    }

    return credit;
}

QList<Credit> RepositoryCredit::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits ORDER BY date_echeance");

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credits.append(credit);
        }
    }

    return credits;
}

bool RepositoryCredit::update(const Credit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE credits SET "
                  "statut = :statut, date_paiement = :date_paiement, notes = :notes "
                  "WHERE credit_id = :id");

    query.addBindValue(Credit::statutToString(entity.getStatut()));
    query.addBindValue(entity.getDatePaiement());
    query.addBindValue(entity.getNotes());
    query.addBindValue(entity.getCreditId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour crédit : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool RepositoryCredit::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE credits SET statut = :statut WHERE credit_id = :id");
    query.addBindValue(Credit::statutToString(Credit::Statut::Annule));
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression crédit : " + query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Credit> RepositoryCredit::search(const QString& criterion)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT c.* FROM credits c "
                  "JOIN clients cl ON c.client_id = cl.client_id "
                  "WHERE cl.nom ILIKE :criterion ORDER BY c.date_echeance");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credits.append(credit);
        }
    }

    return credits;
}

bool RepositoryCredit::exists(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT 1 FROM credits WHERE credit_id = :id");
    query.addBindValue(id.toString());

    return query.exec() && query.next();
}

QList<Credit> RepositoryCredit::getByClient(const QUuid& clientId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits WHERE client_id = :client_id ORDER BY date_echeance DESC");
    query.addBindValue(clientId.toString());

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credits.append(credit);
        }
    }

    return credits;
}

QList<Credit> RepositoryCredit::getOverdueCredits()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits WHERE statut = 'en_attente' AND date_echeance < CURRENT_DATE ORDER BY date_echeance");

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credits.append(credit);
        }
    }

    return credits;
}

QList<Credit> RepositoryCredit::getByStatut(const Credit::Statut& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits WHERE statut = :statut ORDER BY date_echeance");
    query.addBindValue(Credit::statutToString(statut));

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credits.append(credit);
        }
    }

    return credits;
}

double RepositoryCredit::getTotalAmount(const Credit::Statut& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT SUM(montant) as total FROM credits WHERE statut = :statut");
    query.addBindValue(Credit::statutToString(statut));

    if (query.exec() && query.next()) {
        return query.value("total").toDouble();
    }

    return 0.0;
}