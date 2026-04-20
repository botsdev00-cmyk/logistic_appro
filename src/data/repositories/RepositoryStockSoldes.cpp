#include "RepositoryStockSoldes.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>

RepositoryStockSoldes::RepositoryStockSoldes()
{
    qDebug() << "[REPO STOCK SOLDES] Initialisation";
}

// ============================================================================
// INTERFACE IREPOSITORY
// ============================================================================

bool RepositoryStockSoldes::create(const StockSolde&)
{
    m_dernierErreur = "Modification directe de stock_soldes interdite. Utilisez stock_mouvements.";
    return false;
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
        solde.setUpdatedAt(query.value("updated_at").toDateTime());
        
        qDebug() << "[REPO-SOLDE] getById: Solde trouvé" << id.toString();
    } else {
        m_dernierErreur = "Solde stock non trouvé";
        qDebug() << "[REPO-SOLDE] getById: Solde NOT FOUND:" << id.toString();
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
            solde.setUpdatedAt(query.value("updated_at").toDateTime());
            
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] ✓ getAll: returned" << count << "soldes";
    } else {
        m_dernierErreur = "Erreur lecture soldes stock: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] getAll ERROR:" << query.lastError().text();
    }

    return soldes;
}

bool RepositoryStockSoldes::update(const StockSolde&)
{
    m_dernierErreur = "Modification directe de stock_soldes interdite. Utilisez stock_mouvements.";
    return false;
}


bool RepositoryStockSoldes::remove(const QUuid&)
{
    m_dernierErreur = "Suppression directe de stock_soldes interdite. Utilisez stock_mouvements.";
    return false;
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
    
    qDebug() << "[REPO-SOLDE] search:" << criterion << "=" << filtered.count() << "résultats";
    return filtered;
}

bool RepositoryStockSoldes::exists(const QUuid& id)
{
    return !getById(id).getSoldeId().isNull();
}

// ============================================================================
// MÉTHODES SPÉCIFIQUES
// ============================================================================

StockSolde RepositoryStockSoldes::getByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT * FROM stock_soldes WHERE produit_id = ?");
    query.addBindValue(produitId.toString());

    StockSolde solde;
    if (query.exec() && query.next()) {
        solde = getById(QUuid(query.value("solde_id").toString()));
        qDebug() << "[REPO-SOLDE] ✓ getByProduit: produit_id=" << produitId 
                 << "quantite_total=" << solde.getQuantiteTotal();
    } else {
        qDebug() << "[REPO-SOLDE] getByProduit: NOT FOUND for produit_id=" << produitId;
    }

    return solde;
}

// ✅ MÉTHODE: Obtenir stocks avec tous les détails du produit
QList<StockSolde> RepositoryStockSoldes::obtenirStockDetail()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<StockSolde> soldes;

    query.prepare(
        "SELECT "
        "  ss.solde_id, "
        "  ss.produit_id, "
        "  p.nom AS produit_nom, "
        "  p.code_sku, "
        "  cp.nom AS categorie, "
        "  tp.nom AS type, "
        "  p.stock_minimum, "
        "  ss.quantite_total, "
        "  ss.quantite_reserve, "
        "  ss.prix_moyen, "
        "  ss.valeur_stock, "
        "  ss.dernier_mouvement_date, "
        "  ss.updated_at "
        "FROM stock_soldes ss "
        "JOIN produits p ON ss.produit_id = p.produit_id "
        "LEFT JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id "
        "LEFT JOIN types_produits tp ON p.type_produit_id = tp.type_produit_id "
        "WHERE p.est_actif = true "
        "ORDER BY ss.updated_at DESC"
    );

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde;
            
            // IDs
            solde.setSoldeId(QUuid(query.value("solde_id").toString()));
            solde.setProduitId(QUuid(query.value("produit_id").toString()));
            
            // Données Produit
            solde.setProduitNom(query.value("produit_nom").toString());
            solde.setCodeSKU(query.value("code_sku").toString());
            solde.setCategorie(query.value("categorie").toString());
            solde.setType(query.value("type").toString());
            solde.setStockMinimum(query.value("stock_minimum").toInt());
            
            // Quantités
            solde.setQuantiteTotal(query.value("quantite_total").toInt());
            solde.setQuantiteReservee(query.value("quantite_reserve").toInt());
            
            // Valeurs
            solde.setPrixMoyen(query.value("prix_moyen").toDouble());
            solde.setValeurStock(query.value("valeur_stock").toDouble());
            
            // Dates
            solde.setDernierMouvementDate(query.value("dernier_mouvement_date").toDateTime());
            solde.setUpdatedAt(query.value("updated_at").toDateTime());
            
            soldes.append(solde);
            count++;
            
            qDebug() << "[REPO-SOLDE] Chargé:" << solde.getProduitNom() 
                     << "(" << solde.getCodeSKU() << ") - " << solde.getCategorie();
        }
        qDebug() << "[REPO-SOLDE] ✓ obtenirStockDetail: " << count << "produits chargés";
    } else {
        m_dernierErreur = "Erreur obtenir stock detail: " + query.lastError().text();
        qDebug() << "[REPO-SOLDE] obtenirStockDetail ERROR:" << query.lastError().text();
    }

    return soldes;
}

// ============================================================================
// CONSULTATIONS QUANTITÉS
// ============================================================================

int RepositoryStockSoldes::obtenirQuantiteDisponible(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    int disponible = solde.getQuantiteDisponible();
    qDebug() << "[REPO-SOLDE] Quantité disponible pour" << produitId.toString() << "=" << disponible;
    return disponible;
}

int RepositoryStockSoldes::obtenirQuantiteTotal(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    int total = solde.getQuantiteTotal();
    qDebug() << "[REPO-SOLDE] Quantité totale pour" << produitId.toString() << "=" << total;
    return total;
}

int RepositoryStockSoldes::obtenirQuantiteReservee(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    int reservee = solde.getQuantiteReservee();
    qDebug() << "[REPO-SOLDE] Quantité réservée pour" << produitId.toString() << "=" << reservee;
    return reservee;
}

// ============================================================================
// CONSULTATIONS VALEURS
// ============================================================================

double RepositoryStockSoldes::obtenirValeurTotalStock()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    double total = 0.0;

    query.prepare("SELECT COALESCE(SUM(valeur_stock), 0.0) as total FROM stock_soldes");

    if (query.exec() && query.next()) {
        total = query.value("total").toDouble();
        qDebug() << "[REPO-SOLDE] ✓ Valeur totale du stock =" << total;
    } else {
        qDebug() << "[REPO-SOLDE] Erreur calcul valeur totale:" << query.lastError().text();
    }

    return total;
}

double RepositoryStockSoldes::obtenirValeurProduit(const QUuid& produitId)
{
    StockSolde solde = getByProduit(produitId);
    return solde.getValeurStock();
}

// ============================================================================
// ALERTES STOCK
// ============================================================================

QList<StockSolde> RepositoryStockSoldes::obtenirStocksBas(int seuil)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<StockSolde> soldes;

    query.prepare(
        "SELECT * FROM stock_soldes "
        "WHERE (quantite_total - quantite_reserve) > 0 "
        "AND (quantite_total - quantite_reserve) <= ? "
        "ORDER BY (quantite_total - quantite_reserve) ASC"
    );
    query.addBindValue(seuil);

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde = getById(QUuid(query.value("solde_id").toString()));
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] ✓ obtenirStocksBas: trouvé" << count << "produits bas stock";
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

    query.prepare(
        "SELECT * FROM stock_soldes "
        "WHERE (quantite_total - quantite_reserve) <= 0 "
        "ORDER BY quantite_total ASC"
    );

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde = getById(QUuid(query.value("solde_id").toString()));
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] ✓ obtenirStocksEnRupture: trouvé" << count << "produits en rupture";
    } else {
        qDebug() << "[REPO-SOLDE] obtenirStocksEnRupture ERROR:" << query.lastError().text();
    }

    return soldes;
}

QList<StockSolde> RepositoryStockSoldes::obtenirStocksParCategorie(const QString& categorie)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<StockSolde> soldes;

    query.prepare(
        "SELECT ss.* FROM stock_soldes ss "
        "JOIN produits p ON ss.produit_id = p.produit_id "
        "JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id "
        "WHERE cp.nom = ? "
        "ORDER BY p.nom ASC"
    );
    query.addBindValue(categorie);

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockSolde solde = getById(QUuid(query.value("solde_id").toString()));
            soldes.append(solde);
            count++;
        }
        qDebug() << "[REPO-SOLDE] ✓ obtenirStocksParCategorie:" << categorie << "=" << count;
    } else {
        qDebug() << "[REPO-SOLDE] obtenirStocksParCategorie ERROR:" << query.lastError().text();
    }

    return soldes;
}

// ============================================================================
// SYNCHRONISATION
// ============================================================================

bool RepositoryStockSoldes::synchroniserTousSoldes()
{
    m_dernierErreur = "Synchronisation directe de stock_soldes interdite. Utilisez stock_mouvements.";
    return false;
}

bool RepositoryStockSoldes::mettreAJourSolde(const QUuid&)
{
    m_dernierErreur = "Mise à jour directe de stock_soldes interdite. Utilisez stock_mouvements.";
    return false;
}

// ============================================================================
// RAPPORTS
// ============================================================================

QMap<QString, int> RepositoryStockSoldes::obtenirStatistiquesParCategorie()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QMap<QString, int> stats;

    query.prepare(
        "SELECT cp.nom as categorie, COUNT(ss.solde_id) as nombre "
        "FROM stock_soldes ss "
        "JOIN produits p ON ss.produit_id = p.produit_id "
        "JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id "
        "GROUP BY cp.nom "
        "ORDER BY nombre DESC"
    );

    if (query.exec()) {
        while (query.next()) {
            QString categorie = query.value("categorie").toString();
            int nombre = query.value("nombre").toInt();
            stats[categorie] = nombre;
        }
        qDebug() << "[REPO-SOLDE] ✓ Statistiques par catégorie:" << stats.count() << "catégories";
    } else {
        qDebug() << "[REPO-SOLDE] obtenirStatistiquesParCategorie ERROR:" << query.lastError().text();
    }

    return stats;
}

QMap<QString, double> RepositoryStockSoldes::obtenirValeurParCategorie()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QMap<QString, double> valeurs;

    query.prepare(
        "SELECT cp.nom as categorie, SUM(ss.valeur_stock) as valeur_totale "
        "FROM stock_soldes ss "
        "JOIN produits p ON ss.produit_id = p.produit_id "
        "JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id "
        "GROUP BY cp.nom "
        "ORDER BY valeur_totale DESC"
    );

    if (query.exec()) {
        while (query.next()) {
            QString categorie = query.value("categorie").toString();
            double valeur = query.value("valeur_totale").toDouble();
            valeurs[categorie] = valeur;
        }
        qDebug() << "[REPO-SOLDE] ✓ Valeur par catégorie:" << valeurs.count() << "catégories";
    } else {
        qDebug() << "[REPO-SOLDE] obtenirValeurParCategorie ERROR:" << query.lastError().text();
    }

    return valeurs;
}