#include "RepositoryCredit.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

RepositoryCredit::RepositoryCredit() {}

Credit RepositoryCredit::mapRowToCredit(const QSqlQuery& q) const
{
    Credit credit;
    credit.setCreditId(QUuid(q.value("credit_id").toString()));
    credit.setVenteId(QUuid(q.value("vente_id").toString()));
    credit.setClientId(QUuid(q.value("client_id").toString()));
    credit.setMontant(q.value("montant").toDouble());
    credit.setDateEcheance(q.value("date_echeance").toDate());
    credit.setStatut(Credit::stringToStatut(q.value("statut").toString()));
    credit.setDatePaiement(q.value("date_paiement").toDate());
    credit.setNotes(q.value("notes").toString());
    credit.setVersion(q.value("version").toInt());
    credit.setSyncStatus(Credit::syncStatusFromString(q.value("sync_status").toString()));
    credit.setCreatedAt(q.value("created_at").toDateTime());
    credit.setUpdatedAt(q.value("updated_at").toDateTime());
    credit.setDeletedAt(q.value("deleted_at").toDateTime());
    return credit;
}

bool RepositoryCredit::create(Credit e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        INSERT INTO credits (
            credit_id, vente_id, client_id, montant, date_echeance, statut, date_paiement, notes,
            sync_status, version, created_at, updated_at
        ) VALUES (:id, :vente, :client, :montant, :echeance, :statut, :paiement, :notes, 'PENDING', 1, NOW(), NOW())
    )");
    q.bindValue(":id", e.getCreditId().toString(QUuid::WithoutBraces));
    q.bindValue(":vente", e.getVenteId().toString(QUuid::WithoutBraces));
    q.bindValue(":client", e.getClientId().toString(QUuid::WithoutBraces));
    q.bindValue(":montant", e.getMontant());
    q.bindValue(":echeance", e.getDateEcheance());
    q.bindValue(":statut", Credit::statutToString(e.getStatut()));
    q.bindValue(":paiement", e.getDatePaiement());
    q.bindValue(":notes", e.getNotes());
    if (!q.exec()) {
        m_dernierErreur = "Erreur création crédit : " + q.lastError().text();
        return false;
    }
    return true;
}

bool RepositoryCredit::update(Credit e)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE credits SET
          vente_id = :vente, client_id = :client, montant = :montant, date_echeance = :echeance,
          statut = :statut, date_paiement = :paiement, notes = :notes,
          updated_at = NOW(), version = version + 1, sync_status = 'PENDING'
        WHERE credit_id = :id AND deleted_at IS NULL
    )");
    q.bindValue(":vente", e.getVenteId().toString(QUuid::WithoutBraces));
    q.bindValue(":client", e.getClientId().toString(QUuid::WithoutBraces));
    q.bindValue(":montant", e.getMontant());
    q.bindValue(":echeance", e.getDateEcheance());
    q.bindValue(":statut", Credit::statutToString(e.getStatut()));
    q.bindValue(":paiement", e.getDatePaiement());
    q.bindValue(":notes", e.getNotes());
    q.bindValue(":id", e.getCreditId().toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur update crédit : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool RepositoryCredit::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare(R"(
        UPDATE credits
        SET deleted_at = NOW(), sync_status = 'PENDING', version = version + 1
        WHERE credit_id = :id AND deleted_at IS NULL
    )");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec()) {
        m_dernierErreur = "Erreur suppression crédit : " + q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

std::optional<Credit> RepositoryCredit::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT * FROM credits WHERE credit_id = :id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    if (!q.exec() || !q.next()) return std::nullopt;
    return mapRowToCredit(q);
}

QList<Credit> RepositoryCredit::getAll() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    if (q.exec("SELECT * FROM credits WHERE deleted_at IS NULL ORDER BY date_echeance")) {
        while (q.next())
            res.append(mapRowToCredit(q));
    }
    return res;
}

QList<Credit> RepositoryCredit::search(const QString& crit) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    q.prepare(R"(
        SELECT c.* FROM credits c
        JOIN clients cl ON c.client_id = cl.client_id
        WHERE cl.nom ILIKE :c AND c.deleted_at IS NULL
        ORDER BY c.date_echeance desc
    )");
    q.bindValue(":c", "%" + crit + "%");
    if (q.exec()) while (q.next()) res.append(mapRowToCredit(q));
    return res;
}

bool RepositoryCredit::exists(const QUuid& id) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT 1 FROM credits WHERE credit_id = :id AND deleted_at IS NULL");
    q.bindValue(":id", id.toString(QUuid::WithoutBraces));
    return q.exec() && q.next();
}

QList<Credit> RepositoryCredit::getByClient(const QUuid& clientId) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    q.prepare("SELECT * FROM credits WHERE client_id = :c AND deleted_at IS NULL ORDER BY date_echeance desc");
    q.bindValue(":c", clientId.toString(QUuid::WithoutBraces));
    if (q.exec()) while (q.next()) res.append(mapRowToCredit(q));
    return res;
}

QList<Credit> RepositoryCredit::getOverdueCredits() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    if (q.exec("SELECT * FROM credits WHERE statut = 'EN_ATTENTE' AND date_echeance < CURRENT_DATE AND deleted_at IS NULL ORDER BY date_echeance")) {
        while (q.next())
            res.append(mapRowToCredit(q));
    }
    return res;
}

QList<Credit> RepositoryCredit::getByStatut(const Credit::Statut& statut) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    q.prepare("SELECT * FROM credits WHERE statut = :statut AND deleted_at IS NULL ORDER BY date_echeance");
    q.bindValue(":statut", Credit::statutToString(statut));
    if (q.exec()) while (q.next()) res.append(mapRowToCredit(q));
    return res;
}

double RepositoryCredit::getTotalAmount(const Credit::Statut& statut) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    q.prepare("SELECT SUM(montant) as total FROM credits WHERE statut = :statut AND deleted_at IS NULL");
    q.bindValue(":statut", Credit::statutToString(statut));
    if (q.exec() && q.next()) {
        return q.value("total").toDouble();
    }
    return 0.0;
}

// --- offline-first
QList<Credit> RepositoryCredit::getPendingSync() const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    if (q.exec("SELECT * FROM credits WHERE sync_status = 'PENDING' AND deleted_at IS NULL")) {
        while (q.next())
            res.append(mapRowToCredit(q));
    }
    return res;
}
QList<Credit> RepositoryCredit::getSinceVersion(int minVersion) const
{
    ConnexionBaseDonnees& db = ConnexionBaseDonnees::getInstance();
    QSqlQuery q(db.getDatabase());
    QList<Credit> res;
    q.prepare("SELECT * FROM credits WHERE version >= :v AND deleted_at IS NULL");
    q.bindValue(":v", minVersion);
    if (q.exec()) while (q.next()) res.append(mapRowToCredit(q));
    return res;
}
