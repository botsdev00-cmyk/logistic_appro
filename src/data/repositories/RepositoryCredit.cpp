#include "RepositoryCredit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryCredit::RepositoryCredit() {}

bool RepositoryCredit::create(const Credit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        INSERT INTO credits 
        (credit_id, vente_id, client_id, montant, date_echeance, statut, notes,
         sync_status, version, created_at, updated_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, 'PENDING', 1, NOW(), NOW())
    )");
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

bool RepositoryCredit::update(const Credit& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE credits SET
          statut = ?, date_paiement = ?, notes = ?,
          updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE credit_id = ? AND deleted_at IS NULL
    )");
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

bool RepositoryCredit::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(R"(
        UPDATE credits 
        SET deleted_at = NOW(), sync_status = 'PENDING', version = version + 1
        WHERE credit_id = ? AND deleted_at IS NULL
    )");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression crédit : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

std::optional<Credit> RepositoryCredit::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM credits WHERE credit_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());

    if (query.exec() && query.next()) {
        Credit credit;
        credit.setCreditId(QUuid(query.value("credit_id").toString()));
        credit.setVenteId(QUuid(query.value("vente_id").toString()));
        credit.setClientId(QUuid(query.value("client_id").toString()));
        credit.setMontant(query.value("montant").toDouble());
        credit.setDateEcheance(query.value("date_echeance").toDate());
        credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
        credit.setDatePaiement(query.value("date_paiement").toDate());
        credit.setNotes(query.value("notes").toString());
        return credit;
    }
    return std::nullopt;
}

QList<Credit> RepositoryCredit::getAll() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    if (query.exec("SELECT * FROM credits WHERE deleted_at IS NULL ORDER BY date_echeance")) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}

QList<Credit> RepositoryCredit::search(const QString& criterion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;
    query.prepare(R"(
        SELECT c.* FROM credits c 
        JOIN clients cl ON c.client_id = cl.client_id 
        WHERE cl.nom ILIKE ? AND c.deleted_at IS NULL ORDER BY c.date_echeance
    )");
    query.addBindValue("%" + criterion + "%");

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}

bool RepositoryCredit::exists(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT 1 FROM credits WHERE credit_id = ? AND deleted_at IS NULL");
    query.addBindValue(id.toString());
    return query.exec() && query.next();
}

QList<Credit> RepositoryCredit::getByClient(const QUuid& clientId) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits WHERE client_id = ? AND deleted_at IS NULL ORDER BY date_echeance DESC");
    query.addBindValue(clientId.toString());

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}

QList<Credit> RepositoryCredit::getOverdueCredits() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    if (query.exec("SELECT * FROM credits WHERE statut = 'en_attente' AND date_echeance < CURRENT_DATE AND deleted_at IS NULL ORDER BY date_echeance")) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}

QList<Credit> RepositoryCredit::getByStatut(const Credit::Statut& statut) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits WHERE statut = ? AND deleted_at IS NULL ORDER BY date_echeance");
    query.addBindValue(Credit::statutToString(statut));

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}

double RepositoryCredit::getTotalAmount(const Credit::Statut& statut) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT SUM(montant) as total FROM credits WHERE statut = ? AND deleted_at IS NULL");
    query.addBindValue(Credit::statutToString(statut));

    if (query.exec() && query.next()) {
        return query.value("total").toDouble();
    }
    return 0.0;
}

QList<Credit> RepositoryCredit::getPendingSync() const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    if (query.exec("SELECT * FROM credits WHERE sync_status = 'PENDING' AND deleted_at IS NULL")) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}

QList<Credit> RepositoryCredit::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<Credit> credits;

    query.prepare("SELECT * FROM credits WHERE version >= ? AND deleted_at IS NULL");
    query.addBindValue(minVersion);

    if (query.exec()) {
        while (query.next()) {
            Credit credit;
            credit.setCreditId(QUuid(query.value("credit_id").toString()));
            credit.setVenteId(QUuid(query.value("vente_id").toString()));
            credit.setClientId(QUuid(query.value("client_id").toString()));
            credit.setMontant(query.value("montant").toDouble());
            credit.setDateEcheance(query.value("date_echeance").toDate());
            credit.setStatut(Credit::stringToStatut(query.value("statut").toString()));
            credit.setDatePaiement(query.value("date_paiement").toDate());
            credit.setNotes(query.value("notes").toString());
            credits.append(credit);
        }
    }
    return credits;
}