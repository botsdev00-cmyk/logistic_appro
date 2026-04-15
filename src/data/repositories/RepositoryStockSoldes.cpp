#include "RepositoryStockSoldes.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

RepositoryStockSoldes::RepositoryStockSoldes()
{
}

bool RepositoryStockSoldes::create(const StockSolde& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("INSERT INTO stock_soldes "
                  "(solde_id, produit_id, quantite_total, quantite_reserve, "
                  "valeur_stock, prix_moyen, dernier_mouvement_date, updated_at) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(entity.getSoldeId().toString());
    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getQuantiteTotal());
    query.addBindValue(entity.getQuantiteReservee());
    query.addBindValue(entity.getValeurStock());
    query.addBindValue(entity.getPrixMoyen());
    query.addBindValue(entity.getDernierMouvementDate());
    query.addBindValue(entity.getUpdatedAt());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création solde stock: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] CREATE ERROR:" << m_dernierErreur;
        return false;
    }
    qDebug() << "[REPO-SOLDE] CREATE SUCCESS: produit_id=" << entity.getProduitId();
    return true;
}

StockSolde RepositoryStockSoldes::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM stock_soldes WHERE solde_id = ?");
    query.addBindValue(id.toString());

    StockSolde solde;
    if (query.exec() && query.next()) {
        solde.setSoldeId(QUuid(query.value("solde_id").toString()));
        solde.setProduitId(QUuid(query.value("produit_id").toString()));
        solde.setQuantiteTotal(query.value("quantite_total").toInt());
        solde.setQuantiteReservee(query.value("quantite_reserve").toInt());
        solde.setValeurStock(query.value("valeur_stock").toDouble());
        solde.setPrixMoyen(query.value("prix_moyen").toDouble());
        solde.setDernierMouvementDate(query.value("dernier_mouvement_date").toDateTime());
    } else {
        m_dernierErreur = "Solde stock non trouvé";
        qDebug() << "[REPO-SOLDE] getById: ID not found:" << id.toString();
    }

    return solde;
}

QList<StockSolde> RepositoryStockSoldes::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<StockSolde> soldes;

    query.prepare("SELECT * FROM stock_soldes ORDER BY updated_at DESC");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde;
            solde.setSoldeId(QUuid(query.value("solde_id").toString()));
            solde.setProduitId(QUuid(query.value("produit_id").toString()));
            solde.setQuantiteTotal(query.value("quantite_total").toInt());
            solde.setQuantiteReservee(query.value("quantite_reserve").toInt());
            solde.setValeurStock(query.value("valeur_stock").toDouble());
            solde.setPrixMoyen(query.value("prix_moyen").toDouble());
            solde.setDernierMouvementDate(query.value("dernier_mouvement_date").toDateTime());
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] getAll: returned" << count << "soldes";
    } else {
        m_dernierErreur = "Erreur lecture soldes stock: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] getAll ERROR:" << query.lastError().text();
    }

    return soldes;
}

bool RepositoryStockSoldes::update(const StockSolde& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("UPDATE stock_soldes SET "
                  "quantite_total = ?, quantite_reserve = ?, "
                  "valeur_stock = ?, prix_moyen = ?, "
                  "dernier_mouvement_date = ?, updated_at = CURRENT_TIMESTAMP "
                  "WHERE solde_id = ?");
    
    query.addBindValue(entity.getQuantiteTotal());
    query.addBindValue(entity.getQuantiteReservee());
    query.addBindValue(entity.getValeurStock());
    query.addBindValue(entity.getPrixMoyen());
    query.addBindValue(entity.getDernierMouvementDate());
    query.addBindValue(entity.getSoldeId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour solde stock: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] UPDATE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[REPO-SOLDE] UPDATE: rows affected=" << affected;
    return affected > 0;
}

bool RepositoryStockSoldes::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("DELETE FROM stock_soldes WHERE solde_id = ?");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression solde stock: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] REMOVE ERROR:" << m_dernierErreur;
        return false;
    }
    int affected = query.numRowsAffected();
    qDebug() << "[REPO-SOLDE] REMOVE: rows affected=" << affected;
    return affected > 0;
}

QList<StockSolde> RepositoryStockSoldes::search(const QString& criterion)
{
    QList<StockSolde> all = getAll();
    QList<StockSolde> filtered;
    
    for (const auto& solde : all) {
        if (solde.getProduitId().toString().contains(criterion, Qt::CaseInsensitive)) {
            filtered.append(solde);
        }
    }
    
    return filtered;
}

bool RepositoryStockSoldes::exists(const QUuid& id)
{
    return !getById(id).getSoldeId().isNull();
}

StockSolde RepositoryStockSoldes::getByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM stock_soldes WHERE produit_id = ?");
    query.addBindValue(produitId.toString());

    StockSolde solde;
    if (query.exec() && query.next()) {
        solde = getById(QUuid(query.value("solde_id").toString()));
        qDebug() << "[REPO-SOLDE] getByProduit: produit_id=" << produitId 
                 << "quantite=" << solde.getQuantiteTotal();
    } else {
        qDebug() << "[REPO-SOLDE] getByProduit: NOT FOUND for produit_id=" << produitId;
    }

    return solde;
}

int RepositoryStockSoldes::obtenirQuantiteDisponible(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    int disponible = solde.getQuantiteDisponible();
    qDebug() << "[REPO-SOLDE] Quantité disponible pour" << produitId << "=" << disponible;
    return disponible;
}

int RepositoryStockSoldes::obtenirQuantiteTotal(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    int total = solde.getQuantiteTotal();
    qDebug() << "[REPO-SOLDE] Quantité totale pour" << produitId << "=" << total;
    return total;
}

int RepositoryStockSoldes::obtenirQuantiteReservee(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    int reservee = solde.getQuantiteReservee();
    qDebug() << "[REPO-SOLDE] Quantité réservée pour" << produitId << "=" << reservee;
    return reservee;
}

double RepositoryStockSoldes::obtenirValeurTotalStock()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    double total = 0.0;

    query.prepare("SELECT COALESCE(SUM(valeur_stock), 0.0) as total FROM stock_soldes");

    if (query.exec() && query.next()) {
        total = query.value("total").toDouble();
        qDebug() << "[REPO-SOLDE] Valeur totale du stock =" << total;
    } else {
        qDebug() << "[REPO-SOLDE] Erreur calcul valeur totale:" << query.lastError().text();
    }

    return total;
}

QList<StockSolde> RepositoryStockSoldes::obtenirStocksBas(int seuil)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<StockSolde> soldes;

    query.prepare("SELECT * FROM stock_soldes WHERE quantite_disponible <= ? AND quantite_disponible > 0 ORDER BY quantite_disponible ASC");
    query.addBindValue(seuil);

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde = getById(QUuid(query.value("solde_id").toString()));
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] obtenirStocksBas: trouvé" << count << "produits bas stock";
    } else {
        qDebug() << "[REPO-SOLDE] obtenirStocksBas ERROR:" << query.lastError().text();
    }

    return soldes;
}

QList<StockSolde> RepositoryStockSoldes::obtenirStocksEnRupture()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<StockSolde> soldes;

    query.prepare("SELECT * FROM stock_soldes WHERE quantite_disponible <= 0 ORDER BY quantite_disponible ASC");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde = getById(QUuid(query.value("solde_id").toString()));
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] obtenirStocksEnRupture: trouvé" << count << "produits en rupture";
    } else {
        qDebug() << "[REPO-SOLDE] obtenirStocksEnRupture ERROR:" << query.lastError().text();
    }

    return soldes;
}

bool RepositoryStockSoldes::synchroniserTousSoldes()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-SOLDE] DEBUT synchronisation tous les soldes...";

    // Cette requête recalcule tous les soldes à partir des mouvements
    query.prepare(
        "WITH mouvements_par_produit AS ( "
        "  SELECT produit_id, "
        "    COALESCE(SUM(CASE WHEN type_mouvement='ENTREE' THEN quantite_delta ELSE 0 END), 0) as entrees, "
        "    COALESCE(SUM(CASE WHEN type_mouvement='SORTIE' THEN quantite_delta ELSE 0 END), 0) as sorties, "
        "    COALESCE(SUM(CASE WHEN type_mouvement='RETOUR' THEN quantite_delta ELSE 0 END), 0) as retours "
        "  FROM v_stock_mouvements "
        "  GROUP BY produit_id "
        ") "
        "UPDATE stock_soldes SET "
        "  quantite_total = COALESCE(entrees - sorties + retours, 0), "
        "  dernier_mouvement_date = CURRENT_TIMESTAMP, "
        "  updated_at = CURRENT_TIMESTAMP "
        "FROM mouvements_par_produit "
        "WHERE stock_soldes.produit_id = mouvements_par_produit.produit_id"
    );

    if (!query.exec()) {
        m_dernierErreur = "Erreur synchronisation soldes: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] SYNCHRONISATION ERROR:" << m_dernierErreur;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "[REPO-SOLDE] SYNCHRONISATION OK: " << affected << " soldes mis à jour";
    return true;
}

bool RepositoryStockSoldes::mettreAJourSolde(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-SOLDE] Mise à jour solde pour produit:" << produitId;

    query.prepare(
        "UPDATE stock_soldes SET "
        "  quantite_total = (SELECT COUNT(*) FROM v_stock_mouvements WHERE produit_id = ?), "
        "  dernier_mouvement_date = CURRENT_TIMESTAMP, "
        "  updated_at = CURRENT_TIMESTAMP "
        "WHERE produit_id = ?"
    );
    query.addBindValue(produitId.toString());
    query.addBindValue(produitId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour solde: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] UPDATE ERROR:" << m_dernierErreur;
        return false;
    }

    return query.numRowsAffected() > 0;
}