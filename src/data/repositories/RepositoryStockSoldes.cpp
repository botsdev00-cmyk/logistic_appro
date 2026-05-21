#include "RepositoryStockSoldes.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

RepositoryStockSoldes::RepositoryStockSoldes()
{
    qDebug() << "[REPO STOCK SOLDES] Initialisation";
}

// Création stock_soldes
bool RepositoryStockSoldes::create(const StockSolde& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        INSERT INTO stock_soldes (
            solde_id, produit_id, quantite_total, quantite_reserve, quantite_disponible,
            valeur_stock, prix_moyen, location_id, location_historique, derniere_location_id,
            dernier_mouvement_date, updated_at, sync_status, version, deleted_at, is_deleted, created_at
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    // For location_historique, serialize the three ints as a JSON string (compatible SQL jsonb)
    QString locationHist = QString(
                               "{\"RETURNED\": %1, \"WAREHOUSE\": %2, \"IN_TRANSIT\": %3}")
                               .arg(entity.getHistoryReturned())
                               .arg(entity.getHistoryWarehouse())
                               .arg(entity.getHistoryInTransit());

    query.addBindValue(entity.getSoldeId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getProduitId().toString(QUuid::WithoutBraces));
    query.addBindValue(entity.getQuantiteTotal());
    query.addBindValue(entity.getQuantiteReserve());
    query.addBindValue(entity.getQuantiteDisponible());
    query.addBindValue(entity.getValeurStock());
    query.addBindValue(entity.getPrixMoyen());
    query.addBindValue(entity.getLocationId());
    query.addBindValue(locationHist);
    query.addBindValue(entity.getDerniereLocationId());
    query.addBindValue(entity.getDernierMouvementDate());
    query.addBindValue(entity.getUpdatedAt());
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getDeletedAt().isNull() ? QVariant() : entity.getDeletedAt());
    query.addBindValue(entity.getIsDeleted());
    query.addBindValue(entity.getCreatedAt());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création stock solde : " + query.lastError().text();
        return false;
    }
    return true;
}

// Mise à jour stock_soldes
bool RepositoryStockSoldes::update(const StockSolde& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    QString locationHist = QString(
                               "{\"RETURNED\": %1, \"WAREHOUSE\": %2, \"IN_TRANSIT\": %3}")
                               .arg(entity.getHistoryReturned())
                               .arg(entity.getHistoryWarehouse())
                               .arg(entity.getHistoryInTransit());

    query.prepare(R"(
        UPDATE stock_soldes SET
            quantite_total = ?, quantite_reserve = ?, quantite_disponible = ?,
            valeur_stock = ?, prix_moyen = ?, location_id = ?, location_historique = ?,
            derniere_location_id = ?, dernier_mouvement_date = ?, updated_at = ?,
            sync_status = ?, version = ?, deleted_at = ?, is_deleted = ?, created_at = ?
        WHERE solde_id = ?
    )");
    query.addBindValue(entity.getQuantiteTotal());
    query.addBindValue(entity.getQuantiteReserve());
    query.addBindValue(entity.getQuantiteDisponible());
    query.addBindValue(entity.getValeurStock());
    query.addBindValue(entity.getPrixMoyen());
    query.addBindValue(entity.getLocationId());
    query.addBindValue(locationHist);
    query.addBindValue(entity.getDerniereLocationId());
    query.addBindValue(entity.getDernierMouvementDate());
    query.addBindValue(entity.getUpdatedAt());
    query.addBindValue(entity.syncStatusString());
    query.addBindValue(entity.getVersion());
    query.addBindValue(entity.getDeletedAt().isNull() ? QVariant() : entity.getDeletedAt());
    query.addBindValue(entity.getIsDeleted());
    query.addBindValue(entity.getCreatedAt());
    query.addBindValue(entity.getSoldeId().toString(QUuid::WithoutBraces));

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour stock solde : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// Soft Delete (logique)
bool RepositoryStockSoldes::logicalDelete(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE stock_soldes SET deleted_at = CURRENT_TIMESTAMP, is_deleted = true, sync_status = 'PENDING', version = version + 1 WHERE solde_id = ?
    )");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression (soft-delete) stock solde : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

StockSolde RepositoryStockSoldes::getById(const QUuid& id) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM stock_soldes WHERE solde_id = ?");
    query.addBindValue(id.toString(QUuid::WithoutBraces));
    if (query.exec() && query.next())
        return mapRowToStockSolde(query);
    return StockSolde();
}

QList<StockSolde> RepositoryStockSoldes::getAll() const
{
    QList<StockSolde> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM stock_soldes WHERE is_deleted = false OR is_deleted IS NULL");
    if (query.exec()) while (query.next()) list.append(mapRowToStockSolde(query));
    return list;
}

bool RepositoryStockSoldes::exists(const QUuid& id) const
{
    return !getById(id).getSoldeId().isNull();
}

QList<StockSolde> RepositoryStockSoldes::search(const QString& criterion) const
{
    QList<StockSolde> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM stock_soldes WHERE (location_id ILIKE ? OR derniere_location_id ILIKE ?) AND (is_deleted = false OR is_deleted IS NULL)");
    query.addBindValue("%" + criterion + "%");
    query.addBindValue("%" + criterion + "%");
    if (query.exec()) while (query.next()) list.append(mapRowToStockSolde(query));
    return list;
}

StockSolde RepositoryStockSoldes::getByProduit(const QUuid& produitId) const
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM stock_soldes WHERE produit_id = ? AND (is_deleted = false OR is_deleted IS NULL)");
    query.addBindValue(produitId.toString(QUuid::WithoutBraces));
    if (query.exec() && query.next())
        return mapRowToStockSolde(query);
    return StockSolde();
}

QList<StockSolde> RepositoryStockSoldes::getPendingSync() const
{
    QList<StockSolde> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM stock_soldes WHERE sync_status = 'PENDING' AND (is_deleted = false OR is_deleted IS NULL)");
    if (query.exec()) while (query.next()) list.append(mapRowToStockSolde(query));
    return list;
}

QList<StockSolde> RepositoryStockSoldes::getSinceVersion(int minVersion) const
{
    QList<StockSolde> list;
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare("SELECT * FROM stock_soldes WHERE version >= ? AND (is_deleted = false OR is_deleted IS NULL)");
    query.addBindValue(minVersion);
    if (query.exec()) while (query.next()) list.append(mapRowToStockSolde(query));
    return list;
}

QList<StockSolde> RepositoryStockSoldes::obtenirStocksBas(int seuil) const
{
    QList<StockSolde> res;
    for (const StockSolde& s : getAll())
        if (s.estBasStock(seuil))
            res << s;
    return res;
}

QList<StockSolde> RepositoryStockSoldes::obtenirStocksEnRupture() const
{
    QList<StockSolde> res;
    for (const StockSolde& s : getAll())
        if (s.estEnRupture())
            res << s;
    return res;
}

QList<StockSolde> RepositoryStockSoldes::obtenirStocksParCategorie(const QString& categorie) const
{
    QList<StockSolde> res;
    for (const StockSolde& s : getAll())
        if (s.getDerniereLocationId() == categorie)
            res << s;
    return res;
}

int RepositoryStockSoldes::obtenirQuantiteDisponible(const QUuid& produitId) const
{
    return getByProduit(produitId).getQuantiteDisponible();
}
int RepositoryStockSoldes::obtenirQuantiteTotal(const QUuid& produitId) const
{
    return getByProduit(produitId).getQuantiteTotal();
}
int RepositoryStockSoldes::obtenirQuantiteReservee(const QUuid& produitId) const
{
    return getByProduit(produitId).getQuantiteReserve();
}
double RepositoryStockSoldes::obtenirValeurProduit(const QUuid& produitId) const
{
    return getByProduit(produitId).getValeurStock();
}
double RepositoryStockSoldes::obtenirValeurTotalStock() const
{
    double sum = 0;
    for (const StockSolde& s : getAll())
        sum += s.getValeurStock();
    return sum;
}

// Statistiques (rapides, sans performances optimisées)
QMap<QString, int> RepositoryStockSoldes::obtenirStatistiquesParCategorie() const
{
    QMap<QString, int> m;
    for (const StockSolde& s : getAll())
        m[s.getDerniereLocationId()] += s.getQuantiteDisponible();
    return m;
}

QMap<QString, double> RepositoryStockSoldes::obtenirValeurParCategorie() const
{
    QMap<QString, double> m;
    for (const StockSolde& s : getAll())
        m[s.getDerniereLocationId()] += s.getValeurStock();
    return m;
}

// Méthode - mapping SQL -> objet StockSolde (y compris historique simple)
StockSolde RepositoryStockSoldes::mapRowToStockSolde(const QSqlQuery& q) const
{
    StockSolde s;
    s.setSoldeId(QUuid(q.value("solde_id").toString()));
    s.setProduitId(QUuid(q.value("produit_id").toString()));
    s.setQuantiteTotal(q.value("quantite_total").toInt());
    s.setQuantiteReserve(q.value("quantite_reserve").toInt());
    s.setQuantiteDisponible(q.value("quantite_disponible").toInt());
    s.setValeurStock(q.value("valeur_stock").toDouble());
    s.setPrixMoyen(q.value("prix_moyen").toDouble());
    s.setLocationId(q.value("location_id").toString());
    s.setDerniereLocationId(q.value("derniere_location_id").toString());
    s.setDernierMouvementDate(q.value("dernier_mouvement_date").toDateTime());
    s.setUpdatedAt(q.value("updated_at").toDateTime());
    s.setDeletedAt(q.value("deleted_at").toDateTime());
    s.setCreatedAt(q.value("created_at").toDateTime());
    s.setSyncStatus(StockSolde::stringToSyncStatus(q.value("sync_status").toString()));
    s.setVersion(q.value("version").toInt());
    s.setIsDeleted(q.value("is_deleted").toBool());

    // Extraction C++ pur du JSONB (ex : {"RETURNED": 0, "WAREHOUSE": 12, "IN_TRANSIT": 3})
    QString hist = q.value("location_historique").toString();
    // Extraction manuelle/primitive (tu peux l'améliorer) :
    s.setHistoryReturned(hist.contains("\"RETURNED\": 0") ? 0 : getJsonInt(hist, "RETURNED"));
    s.setHistoryWarehouse(hist.contains("\"WAREHOUSE\": 0") ? 0 : getJsonInt(hist, "WAREHOUSE"));
    s.setHistoryInTransit(hist.contains("\"IN_TRANSIT\": 0") ? 0 : getJsonInt(hist, "IN_TRANSIT"));

    return s;
}

// Helper C++ pur pour extraire un champ entier d’une string style {"RETURNED": 0, ...}
int RepositoryStockSoldes::getJsonInt(const QString& str, const char* key) const
{
    QString k = QString("\"") + key + "\":";
    int i = str.indexOf(k);
    if (i < 0) return 0;
    i += k.length();
    while (i < str.size() && str[i] == ' ') ++i;
    int end = i;
    while (end < str.size() && str[end].isDigit()) ++end;
    return str.mid(i, end - i).toInt();
}

QList<StockSolde> RepositoryStockSoldes::obtenirStockDetail() const
{
    return getAll();
}

bool RepositoryStockSoldes::synchroniserTousSoldes() {
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    query.prepare(R"(
        UPDATE stock_soldes
           SET sync_status = 'SYNCED',
               updated_at = CURRENT_TIMESTAMP
         WHERE sync_status = 'PENDING'
           AND (is_deleted = false OR is_deleted IS NULL)
    )");
    if (!query.exec()) {
        m_dernierErreur = "Erreur synchronisation stock soldes : " + query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}
