#include "GestionnaireStock.h"
#include "../../data/repositories/RepositoryEntreeStock.h"
#include "../../data/repositories/RepositoryRetourStock.h"
#include "../../data/repositories/RepositoryStockSoldes.h"
#include "../../data/repositories/RepositoryStockMouvements.h"
#include "../../data/repositories/RepositoryProduit.h"
#include "../services/ServicePermissions.h"
#include "../../core/entities/EntreeStock.h"
#include "../../core/entities/RetourStock.h"
#include "../../core/entities/StockSolde.h"
#include "../../data/database/ConnexionBaseDonnees.h"
#include <QDebug>
#include <QDate>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <algorithm>

GestionnaireStock::GestionnaireStock()
    : m_repoEntrees(nullptr),
      m_repoRetours(nullptr),
      m_repoSoldes(nullptr),
      m_repoMouvements(nullptr),
      m_servicePermissions(nullptr)
{
    qDebug() << "[GESTIONNAIRE STOCK] Initialisation";
}

GestionnaireStock::~GestionnaireStock()
{
    qDebug() << "[GESTIONNAIRE STOCK] Destruction";
}

// ============================================================================
// SETTERS REPOSITORIES
// ============================================================================

void GestionnaireStock::setRepositoryEntreeStock(RepositoryEntreeStock* repo)
{
    m_repoEntrees = repo;
    qDebug() << "[GESTIONNAIRE STOCK] Repository Entrees défini";
}

void GestionnaireStock::setRepositoryRetourStock(RepositoryRetourStock* repo)
{
    m_repoRetours = repo;
    qDebug() << "[GESTIONNAIRE STOCK] Repository Retours défini";
}

void GestionnaireStock::setRepositoryStockSoldes(RepositoryStockSoldes* repo)
{
    m_repoSoldes = repo;
    qDebug() << "[GESTIONNAIRE STOCK] Repository Soldes défini";
}

void GestionnaireStock::setRepositoryStockMouvements(RepositoryStockMouvements* repo)
{
    m_repoMouvements = repo;
    qDebug() << "[GESTIONNAIRE STOCK] Repository Mouvements défini";
}

void GestionnaireStock::setServicePermissions(ServicePermissions* service)
{
    m_servicePermissions = service;
    qDebug() << "[GESTIONNAIRE STOCK] Service permissions défini";
}

// ============================================================================
// ✅ NOUVEAU: STOCKS PAR LOCATION
// ============================================================================

StockParLocation GestionnaireStock::obtenirStockParLocation(const QUuid& produitId)
{
    StockParLocation stock;
    stock.produitId = produitId;
    stock.warehouse = 0;
    stock.inTransit = 0;
    stock.returned = 0;
    stock.total = 0;

    if (!m_repoSoldes) return stock;

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT warehouse, in_transit, returned "
        "FROM fn_get_stock_by_location(?)"
    );
    query.addBindValue(produitId.toString());

    if (query.exec() && query.next()) {
        stock.warehouse = query.value("warehouse").toInt();
        stock.inTransit = query.value("in_transit").toInt();
        stock.returned = query.value("returned").toInt();
        stock.total = stock.warehouse + stock.inTransit + stock.returned;

        // Récupérer le nom du produit
        StockInfo info = obtenirStockDetail(produitId);
        stock.produitNom = info.produitNom;

        qDebug() << "[STOCK LOCATION]" << stock.produitNom
                 << "| WAREHOUSE:" << stock.warehouse
                 << "| IN_TRANSIT:" << stock.inTransit
                 << "| RETURNED:" << stock.returned
                 << "| TOTAL:" << stock.total;
    } else {
        qDebug() << "[STOCK LOCATION] Erreur:" << query.lastError().text();
    }

    return stock;
}

QList<StockParLocation> GestionnaireStock::obtenirTousStocksParLocation()
{
    QList<StockParLocation> stocks;

    if (!m_repoSoldes) return stocks;

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT p.produit_id, p.nom, "
        "       COALESCE(SUM(CASE WHEN sm.location_id = 'WAREHOUSE' THEN sm.quantite_delta ELSE 0 END), 0) AS warehouse, "
        "       COALESCE(SUM(CASE WHEN sm.location_id = 'IN_TRANSIT' THEN sm.quantite_delta ELSE 0 END), 0) AS in_transit, "
        "       COALESCE(SUM(CASE WHEN sm.location_id = 'RETURNED' THEN sm.quantite_delta ELSE 0 END), 0) AS returned "
        "FROM produits p "
        "LEFT JOIN stock_mouvements sm ON p.produit_id = sm.produit_id "
        "WHERE p.est_actif = true "
        "GROUP BY p.produit_id, p.nom"
    );

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            StockParLocation stock;
            stock.produitId = QUuid(query.value("produit_id").toString());
            stock.produitNom = query.value("nom").toString();
            stock.warehouse = query.value("warehouse").toInt();
            stock.inTransit = query.value("in_transit").toInt();
            stock.returned = query.value("returned").toInt();
            stock.total = stock.warehouse + stock.inTransit + stock.returned;

            stocks.append(stock);
            count++;
        }
        qDebug() << "[STOCK LOCATION] ✅ Tous stocks:" << count << "produits";
    } else {
        qDebug() << "[STOCK LOCATION] Erreur:" << query.lastError().text();
    }

    return stocks;
}

// ============================================================================
// ✅ NOUVEAU: RÉCONCILIATION STOCK
// ============================================================================

QList<ReconciliationResult> GestionnaireStock::verifierIntegriteStock()
{
    QList<ReconciliationResult> resultats;

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT * FROM v_stock_reconciliation"
    );

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            ReconciliationResult res;
            res.produitId = QUuid(query.value("produit_id").toString());
            res.produitNom = query.value("nom").toString();
            res.stockFromMovements = query.value("stock_from_movements").toInt();
            res.stockInSoldes = query.value("stock_in_soldes").toInt();
            res.difference = query.value("difference").toInt();
            res.status = query.value("status").toString();
            res.isConsistent = (res.difference == 0);

            resultats.append(res);
            count++;

            if (res.isConsistent) {
                qDebug() << "[RÉCONCILIATION ✓]" << res.produitNom
                         << "- Stock:" << res.stockFromMovements;
            } else {
                qDebug() << "[RÉCONCILIATION ❌]" << res.produitNom
                         << "- DISCREPANCY:" << res.difference;
            }
        }
        qDebug() << "[RÉCONCILIATION] ✅ Vérification terminée:" << count << "produits";
    } else {
        qDebug() << "[RÉCONCILIATION] Erreur SQL:" << query.lastError().text();
    }

    return resultats;
}

ReconciliationResult GestionnaireStock::verifierIntegriteProduit(const QUuid& produitId)
{
    ReconciliationResult res;
    res.produitId = produitId;
    res.isConsistent = false;

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare(
        "SELECT * FROM v_stock_reconciliation WHERE produit_id = ?"
    );
    query.addBindValue(produitId.toString());

    if (query.exec() && query.next()) {
        res.produitNom = query.value("nom").toString();
        res.stockFromMovements = query.value("stock_from_movements").toInt();
        res.stockInSoldes = query.value("stock_in_soldes").toInt();
        res.difference = query.value("difference").toInt();
        res.status = query.value("status").toString();
        res.isConsistent = (res.difference == 0);

        qDebug() << "[RÉCONCILIATION PRODUIT]" << res.produitNom
                 << "| Mouvements:" << res.stockFromMovements
                 << "| Soldes:" << res.stockInSoldes
                 << "| Différence:" << res.difference;
    } else {
        qDebug() << "[RÉCONCILIATION PRODUIT] Erreur:" << query.lastError().text();
    }

    return res;
}

bool GestionnaireStock::repairerIntegriteStock()
{
    if (!m_repoSoldes) {
        m_dernierErreur = "Repository soldes non initialisé";
        return false;
    }

    qDebug() << "[RÉPARATION STOCK] Début...";

    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT fn_repair_stock_integrity()");

    if (query.exec()) {
        qDebug() << "[RÉPARATION STOCK] ✅ Intégrité réparée";
        return true;
    } else {
        m_dernierErreur = query.lastError().text();
        qDebug() << "[RÉPARATION STOCK] ❌ Erreur:" << m_dernierErreur;
        return false;
    }
}

// ============================================================================
// GESTION DES ENTRÉES (Existant - Inchangé)
// ============================================================================

bool GestionnaireStock::creerEntreeStock(const EntreeStock& entree)
{
    if (!m_repoEntrees) {
        m_dernierErreur = "Repository entrees non initialisé";
        qDebug() << "[GESTIONNAIRE STOCK] ERREUR:" << m_dernierErreur;
        return false;
    }

    if (!validerEntree(entree)) {
        m_dernierErreur = "Entrée stock invalide";
        return false;
    }

    if (m_repoEntrees->create(entree)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Entrée créée:" << entree.getEntreeStockId();
        synchroniserStockSoldes();
        return true;
    }

    m_dernierErreur = m_repoEntrees->getLastError();
    qDebug() << "[GESTIONNAIRE STOCK] ERREUR création:" << m_dernierErreur;
    return false;
}

bool GestionnaireStock::creerEntreeStockAvecPermission(const EntreeStock& entree, const QUuid& utilisateurId)
{
    if (!m_servicePermissions) {
        m_dernierErreur = "Service permissions non initialisé";
        return false;
    }

    if (!m_servicePermissions->peutModifierStock(utilisateurId)) {
        m_dernierErreur = "Permission refusée: STOCK_EDIT requis";
        qDebug() << "[GESTIONNAIRE STOCK] Permission refusée pour" << utilisateurId;
        return false;
    }

    return creerEntreeStock(entree);
}

bool GestionnaireStock::approuverEntree(const QUuid& entreeId, const QUuid& utilisateurId)
{
    if (!m_repoEntrees) {
        m_dernierErreur = "Repository entrees non initialisé";
        return false;
    }

    if (m_repoEntrees->approuver(entreeId, utilisateurId)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Entrée approuvée:" << entreeId;
        synchroniserStockSoldes();
        return true;
    }

    m_dernierErreur = m_repoEntrees->getLastError();
    return false;
}

bool GestionnaireStock::approuverEntreeAvecPermission(const QUuid& entreeId, const QUuid& utilisateurId)
{
    if (!m_servicePermissions) {
        m_dernierErreur = "Service permissions non initialisé";
        return false;
    }

    if (!m_servicePermissions->peutApprouverStock(utilisateurId)) {
        m_dernierErreur = "Permission refusée: STOCK_APPROVE requis";
        qDebug() << "[GESTIONNAIRE STOCK] Permission refusée pour" << utilisateurId;
        return false;
    }

    return approuverEntree(entreeId, utilisateurId);
}

bool GestionnaireStock::rejeterEntree(const QUuid& entreeId)
{
    if (!m_repoEntrees) {
        m_dernierErreur = "Repository entrees non initialisé";
        return false;
    }

    if (m_repoEntrees->rejeter(entreeId)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Entrée rejetée:" << entreeId;
        return true;
    }

    m_dernierErreur = m_repoEntrees->getLastError();
    return false;
}

QList<EntreeStock> GestionnaireStock::obtenirEntreesEnAttente()
{
    if (!m_repoEntrees) return QList<EntreeStock>();
    auto entrees = m_repoEntrees->getEnAttente();
    qDebug() << "[GESTIONNAIRE STOCK] Entrées en attente:" << entrees.count();
    return entrees;
}

QList<EntreeStock> GestionnaireStock::obtenirHistoriqueEntrees(const QUuid& produitId, int joursArriere)
{
    if (!m_repoEntrees) return QList<EntreeStock>();
    
    if (!produitId.isNull()) {
        auto entrees = m_repoEntrees->getByProduit(produitId);
        qDebug() << "[GESTIONNAIRE STOCK] Historique entrees pour produit" << produitId << "=" << entrees.count();
        return entrees;
    }
    
    auto entrees = m_repoEntrees->getAll();
    qDebug() << "[GESTIONNAIRE STOCK] Historique entrees total =" << entrees.count();
    return entrees;
}

EntreeStock GestionnaireStock::obtenirEntree(const QUuid& entreeId)
{
    if (!m_repoEntrees) return EntreeStock();
    return m_repoEntrees->getById(entreeId);
}

bool GestionnaireStock::supprimerEntree(const QUuid& entreeId)
{
    if (!m_repoEntrees) {
        m_dernierErreur = "Repository entrees non initialisé";
        return false;
    }

    if (m_repoEntrees->remove(entreeId)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Entrée supprimée:" << entreeId;
        synchroniserStockSoldes();
        return true;
    }

    m_dernierErreur = m_repoEntrees->getLastError();
    return false;
}

// ============================================================================
// GESTION DES RETOURS (Existant - Inchangé)
// ============================================================================

bool GestionnaireStock::creerRetourStock(const RetourStock& retour)
{
    if (!m_repoRetours) {
        m_dernierErreur = "Repository retours non initialisé";
        return false;
    }

    if (!validerRetour(retour)) {
        m_dernierErreur = "Retour stock invalide";
        return false;
    }

    if (m_repoRetours->create(retour)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Retour créé:" << retour.getRetourStockId();
        synchroniserStockSoldes();
        return true;
    }

    m_dernierErreur = m_repoRetours->getLastError();
    return false;
}

bool GestionnaireStock::creerRetourStockAvecPermission(const RetourStock& retour, const QUuid& utilisateurId)
{
    if (!m_servicePermissions) {
        m_dernierErreur = "Service permissions non initialisé";
        return false;
    }

    if (!m_servicePermissions->peutModifierStock(utilisateurId)) {
        m_dernierErreur = "Permission refusée: STOCK_EDIT requis";
        return false;
    }

    return creerRetourStock(retour);
}

bool GestionnaireStock::approuverRetour(const QUuid& retourId, const QUuid& utilisateurId)
{
    if (!m_repoRetours) {
        m_dernierErreur = "Repository retours non initialisé";
        return false;
    }

    if (m_repoRetours->approuver(retourId, utilisateurId)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Retour approuvé:" << retourId;
        synchroniserStockSoldes();
        return true;
    }

    m_dernierErreur = m_repoRetours->getLastError();
    return false;
}

bool GestionnaireStock::approuverRetourAvecPermission(const QUuid& retourId, const QUuid& utilisateurId)
{
    if (!m_servicePermissions) {
        m_dernierErreur = "Service permissions non initialisé";
        return false;
    }

    if (!m_servicePermissions->peutApprouverStock(utilisateurId)) {
        m_dernierErreur = "Permission refusée: STOCK_APPROVE requis";
        return false;
    }

    return approuverRetour(retourId, utilisateurId);
}

bool GestionnaireStock::rejeterRetour(const QUuid& retourId)
{
    if (!m_repoRetours) {
        m_dernierErreur = "Repository retours non initialisé";
        return false;
    }

    if (m_repoRetours->rejeter(retourId)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Retour rejeté:" << retourId;
        return true;
    }

    m_dernierErreur = m_repoRetours->getLastError();
    return false;
}

QList<RetourStock> GestionnaireStock::obtenirRetoursEnAttente()
{
    if (!m_repoRetours) return QList<RetourStock>();
    return m_repoRetours->getEnAttente();
}

QList<RetourStock> GestionnaireStock::obtenirHistoriqueRetours(const QUuid& produitId, int joursArriere)
{
    if (!m_repoRetours) return QList<RetourStock>();
    
    if (!produitId.isNull()) {
        return m_repoRetours->getByProduit(produitId);
    }
    
    return m_repoRetours->getAll();
}

RetourStock GestionnaireStock::obtenirRetour(const QUuid& retourId)
{
    if (!m_repoRetours) return RetourStock();
    return m_repoRetours->getById(retourId);
}

bool GestionnaireStock::supprimerRetour(const QUuid& retourId)
{
    if (!m_repoRetours) {
        m_dernierErreur = "Repository retours non initialisé";
        return false;
    }

    if (m_repoRetours->remove(retourId)) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Retour supprimé:" << retourId;
        synchroniserStockSoldes();
        return true;
    }

    m_dernierErreur = m_repoRetours->getLastError();
    return false;
}

// ============================================================================
// CONSULTATION STOCK (Existant - Simplifié)
// ============================================================================

int GestionnaireStock::obtenirQuantiteDisponible(const QUuid& produitId)
{
    if (!m_repoSoldes) return 0;
    return m_repoSoldes->obtenirQuantiteDisponible(produitId);
}

int GestionnaireStock::obtenirQuantiteReservee(const QUuid& produitId)
{
    if (!m_repoSoldes) return 0;
    return m_repoSoldes->obtenirQuantiteReservee(produitId);
}

int GestionnaireStock::obtenirQuantiteTotal(const QUuid& produitId)
{
    if (!m_repoSoldes) return 0;
    return m_repoSoldes->obtenirQuantiteTotal(produitId);
}

StockInfo GestionnaireStock::obtenirStockDetail(const QUuid& produitId)
{
    StockInfo info;
    if (!m_repoSoldes) return info;
    
    StockSolde solde = m_repoSoldes->getByProduit(produitId);
    
    info.produitId = solde.getProduitId();
    info.produitNom = solde.getProduitNom();
    info.codeSKU = solde.getCodeSKU();
    info.categorie = solde.getCategorie();
    info.type = solde.getType();
    info.quantiteTotal = solde.getQuantiteTotal();
    info.quantiteReservee = solde.getQuantiteReservee();
    info.quantiteDisponible = solde.getQuantiteDisponible();
    info.prixMoyen = solde.getPrixMoyen();
    info.valeurStock = solde.getValeurStock();
    info.stockMinimum = solde.getStockMinimum();
    info.dernierMouvement = solde.getDernierMouvementDate();
    info.statut = determinerStatutStock(info.quantiteDisponible, info.stockMinimum);
    
    return info;
}

QList<StockInfo> GestionnaireStock::obtenirTousLesStocks()
{
    QList<StockInfo> stocks;
    
    if (!m_repoSoldes) {
        qWarning() << "[GESTIONNAIRE STOCK] ❌ Repository soldes non initialisé!";
        return stocks;
    }
    
    auto soldes = m_repoSoldes->obtenirStockDetail();
    qDebug() << "[GESTIONNAIRE STOCK] Nombre de stocks à charger:" << soldes.count();
    
    for (const auto& solde : soldes) {
        StockInfo info;
        info.produitId = solde.getProduitId();
        info.produitNom = solde.getProduitNom();
        info.codeSKU = solde.getCodeSKU();
        info.categorie = solde.getCategorie();
        info.type = solde.getType();
        info.quantiteTotal = solde.getQuantiteTotal();
        info.quantiteReservee = solde.getQuantiteReservee();
        info.quantiteDisponible = solde.getQuantiteDisponible();
        info.prixMoyen = solde.getPrixMoyen();
        info.valeurStock = solde.getValeurStock();
        info.stockMinimum = solde.getStockMinimum();
        info.dernierMouvement = solde.getDernierMouvementDate();
        info.statut = determinerStatutStock(info.quantiteDisponible, info.stockMinimum);
        
        stocks.append(info);
        
        qDebug() << "[GESTIONNAIRE STOCK]" << info.produitNom << "-" << info.codeSKU 
                 << "(" << info.categorie << ") - Dispo:" << info.quantiteDisponible;
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] ✓ Stocks chargés:" << stocks.count();
    return stocks;
}

QList<Mouvement> GestionnaireStock::obtenirMouvementsRecents(const QUuid& produitId, int jours)
{
    QList<Mouvement> mouvements;
    
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    QString sqlQuery =
        "SELECT "
        "  sm.type_mouvement, "
        "  p.nom AS produit_nom, "
        "  p.code_sku, "
        "  sm.quantite_delta, "
        "  sm.raison, "
        "  u.nom_complet AS nom_utilisateur, "
        "  sm.created_at, "
        "  COALESCE(se.nom, 'N/A') AS source "
        "FROM stock_mouvements sm "
        "JOIN produits p ON sm.produit_id = p.produit_id "
        "JOIN utilisateurs u ON sm.utilisateur_id = u.utilisateur_id "
        "LEFT JOIN sources_entree se ON sm.reference_type = 'ENTREE_STOCK' "
        "WHERE sm.created_at >= CURRENT_TIMESTAMP - INTERVAL '" + QString::number(jours) + " days' ";
    
    if (!produitId.isNull()) {
        sqlQuery += " AND sm.produit_id = '" + produitId.toString() + "' ";
    }
    
    sqlQuery += " ORDER BY sm.created_at DESC LIMIT 500";

    if (query.exec(sqlQuery)) {
        int count = 0;
        while (query.next()) {
            Mouvement mvt;
            mvt.type = query.value("type_mouvement").toString();
            mvt.nomProduit = query.value("produit_nom").toString();
            mvt.codeSKU = query.value("code_sku").toString();
            mvt.quantiteDelta = query.value("quantite_delta").toInt();
            mvt.raison = query.value("raison").toString();
            mvt.nomUtilisateur = query.value("nom_utilisateur").toString();
            mvt.dateCreation = query.value("created_at").toDateTime().toString("dd/MM/yyyy HH:mm:ss");
            mvt.source = query.value("source").toString();
            
            mouvements.append(mvt);
            count++;
        }
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Mouvements:" << count;
    } else {
        qDebug() << "[GESTIONNAIRE STOCK] ❌ Erreur SQL:" << query.lastError().text();
        m_dernierErreur = query.lastError().text();
    }
    
    return mouvements;
}

QList<StockInfo> GestionnaireStock::obtenirStocksPaginee(int page, int parPage)
{
    auto tousStocks = obtenirTousLesStocks();
    QList<StockInfo> resultat;

    int debut = (page - 1) * parPage;
    int fin = std::min(debut + parPage, (int)tousStocks.count());

    for (int i = debut; i < fin; ++i) {
        resultat.append(tousStocks[i]);
    }

    qDebug() << "[GESTIONNAIRE STOCK] Page" << page << "(" << resultat.count() << "articles)";
    return resultat;
}

QList<StockInfo> GestionnaireStock::rechercherStocks(const QString& critere)
{
    auto tousStocks = obtenirTousLesStocks();
    QList<StockInfo> filtered;
    
    for (const auto& stock : tousStocks) {
        if (stock.produitNom.contains(critere, Qt::CaseInsensitive) ||
            stock.codeSKU.contains(critere, Qt::CaseInsensitive) ||
            stock.categorie.contains(critere, Qt::CaseInsensitive)) {
            filtered.append(stock);
        }
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Recherche" << critere << "=" << filtered.count() << "résultats";
    return filtered;
}

QList<StockInfo> GestionnaireStock::filtrerStocksParStatut(const QString& statut)
{
    auto tousStocks = obtenirTousLesStocks();
    QList<StockInfo> filtered;
    
    for (const auto& stock : tousStocks) {
        if (stock.statut == statut) {
            filtered.append(stock);
        }
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Statut" << statut << "=" << filtered.count() << "produits";
    return filtered;
}

QList<StockInfo> GestionnaireStock::filtrerStocksParCategorie(const QUuid& categorieId)
{
    auto tousStocks = obtenirTousLesStocks();
    QList<StockInfo> filtered;
    
    for (const auto& stock : tousStocks) {
        filtered.append(stock);
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Catégorie" << categorieId.toString() << "=" << filtered.count();
    return filtered;
}

// ============================================================================
// MOUVEMENTS (Existant - Simplifié)
// ============================================================================

QList<Mouvement> GestionnaireStock::obtenirHistoriqueProduit(const QUuid& produitId)
{
    qDebug() << "[GESTIONNAIRE STOCK] Récupération historique produit:" << produitId.toString();
    return obtenirMouvementsRecents(produitId, 365);
}

QList<Mouvement> GestionnaireStock::obtenirMouvementsParType(const QString& type)
{
    QList<Mouvement> mouvements;
    
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    
    QString sqlQuery = 
        "SELECT sm.type_mouvement, p.nom AS produit_nom, p.code_sku, "
        "       sm.quantite_delta, sm.raison, u.nom_complet AS nom_utilisateur, "
        "       sm.created_at, COALESCE(se.nom, 'N/A') AS source "
        "FROM stock_mouvements sm "
        "JOIN produits p ON sm.produit_id = p.produit_id "
        "JOIN utilisateurs u ON sm.utilisateur_id = u.utilisateur_id "
        "LEFT JOIN sources_entree se ON sm.reference_type = 'ENTREE_STOCK' "
        "WHERE sm.type_mouvement = '" + type + "' "
        "ORDER BY sm.created_at DESC LIMIT 500";
    
    if (query.exec(sqlQuery)) {
        while (query.next()) {
            Mouvement mvt;
            mvt.type = query.value("type_mouvement").toString();
            mvt.nomProduit = query.value("produit_nom").toString();
            mvt.codeSKU = query.value("code_sku").toString();
            mvt.quantiteDelta = query.value("quantite_delta").toInt();
            mvt.raison = query.value("raison").toString();
            mvt.nomUtilisateur = query.value("nom_utilisateur").toString();
            mvt.dateCreation = query.value("created_at").toDateTime().toString("dd/MM/yyyy HH:mm:ss");
            mvt.source = query.value("source").toString();
            
            mouvements.append(mvt);
        }
        qDebug() << "[GESTIONNAIRE STOCK] Mouvements par type" << type << "=" << mouvements.count();
    }
    
    return mouvements;
}

QList<Mouvement> GestionnaireStock::obtenirMouvementsParDateRange(const QDate& dateDebut, const QDate& dateFin)
{
    QList<Mouvement> mouvements;
    
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    
    QString sqlQuery = QString(
        "SELECT sm.type_mouvement, p.nom AS produit_nom, p.code_sku, "
        "       sm.quantite_delta, sm.raison, u.nom_complet AS nom_utilisateur, "
        "       sm.created_at, COALESCE(se.nom, 'N/A') AS source "
        "FROM stock_mouvements sm "
        "JOIN produits p ON sm.produit_id = p.produit_id "
        "JOIN utilisateurs u ON sm.utilisateur_id = u.utilisateur_id "
        "LEFT JOIN sources_entree se ON sm.reference_type = 'ENTREE_STOCK' "
        "WHERE DATE(sm.created_at) >= '%1' AND DATE(sm.created_at) <= '%2' "
        "ORDER BY sm.created_at DESC LIMIT 500"
    ).arg(dateDebut.toString("yyyy-MM-dd"), dateFin.toString("yyyy-MM-dd"));
    
    if (query.exec(sqlQuery)) {
        while (query.next()) {
            Mouvement mvt;
            mvt.type = query.value("type_mouvement").toString();
            mvt.nomProduit = query.value("produit_nom").toString();
            mvt.codeSKU = query.value("code_sku").toString();
            mvt.quantiteDelta = query.value("quantite_delta").toInt();
            mvt.raison = query.value("raison").toString();
            mvt.nomUtilisateur = query.value("nom_utilisateur").toString();
            mvt.dateCreation = query.value("created_at").toDateTime().toString("dd/MM/yyyy HH:mm:ss");
            mvt.source = query.value("source").toString();
            
            mouvements.append(mvt);
        }
        qDebug() << "[GESTIONNAIRE STOCK] Mouvements date range =" << mouvements.count();
    }
    
    return mouvements;
}

int GestionnaireStock::obtenirNombreMouvements(const QUuid& produitId)
{
    auto mouvements = obtenirMouvementsRecents(produitId);
    return mouvements.count();
}

// ============================================================================
// ALERTES (Existant - Inchangé)
// ============================================================================

QList<Alerte> GestionnaireStock::obtenirAlertes()
{
    QList<Alerte> alertes;
    auto stocks = obtenirTousLesStocks();
    
    for (const auto& stock : stocks) {
        if (stock.quantiteDisponible <= stock.stockMinimum) {
            Alerte alerte;
            alerte.produitId = stock.produitId;
            alerte.produitNom = stock.produitNom;
            alerte.codeSKU = stock.codeSKU;
            alerte.quantiteDisponible = stock.quantiteDisponible;
            alerte.stockMinimum = stock.stockMinimum;
            alerte.valeurStock = stock.valeurStock;
            
            if (stock.quantiteDisponible <= 0) {
                alerte.severite = "RUPTURE";
                alerte.action = "URGENT: Commander immédiatement";
            } else if (stock.quantiteDisponible < stock.stockMinimum / 2) {
                alerte.severite = "CRITICAL";
                alerte.action = "Commander en priorité";
            } else {
                alerte.severite = "LOW";
                alerte.action = "Prévoir réapprovisionnement";
            }
            
            alertes.append(alerte);
        }
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Alertes totales:" << alertes.count();
    return alertes;
}

QList<Alerte> GestionnaireStock::obtenirAlertesCritiques()
{
    auto alertes = obtenirAlertes();
    QList<Alerte> critiques;
    
    for (const auto& alerte : alertes) {
        if (alerte.severite == "CRITICAL" || alerte.severite == "RUPTURE") {
            critiques.append(alerte);
        }
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Alertes critiques:" << critiques.count();
    return critiques;
}

QList<Alerte> GestionnaireStock::obtenirAlertesBasStock()
{
    auto alertes = obtenirAlertes();
    QList<Alerte> basStock;
    
    for (const auto& alerte : alertes) {
        if (alerte.severite == "LOW") {
            basStock.append(alerte);
        }
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Alertes bas stock:" << basStock.count();
    return basStock;
}

QList<Alerte> GestionnaireStock::obtenirAlerteRupture()
{
    auto alertes = obtenirAlertes();
    QList<Alerte> rupture;
    
    for (const auto& alerte : alertes) {
        if (alerte.severite == "RUPTURE") {
            rupture.append(alerte);
        }
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Alertes rupture:" << rupture.count();
    return rupture;
}

bool GestionnaireStock::genererAlertes()
{
    auto alertes = obtenirAlertesCritiques();
    
    qDebug() << "[GESTIONNAIRE STOCK] Génération d'alertes:" << alertes.count() << "alerte(s) critique(s)";
    
    for (const auto& alerte : alertes) {
        if (alerte.severite == "RUPTURE") {
            qDebug() << "[ALERTE-RUPTURE]" << alerte.produitNom << "- Qté:" << alerte.quantiteDisponible;
        } else {
            qDebug() << "[ALERTE-CRITICAL]" << alerte.produitNom << "- Qté:" << alerte.quantiteDisponible;
        }
    }
    
    return true;
}

int GestionnaireStock::obtenirNombreAlertes()
{
    return obtenirAlertes().count();
}

int GestionnaireStock::obtenirNombreAlertesComme(const QString& severite)
{
    auto alertes = obtenirAlertes();
    int count = 0;
    
    for (const auto& alerte : alertes) {
        if (alerte.severite == severite) {
            count++;
        }
    }
    
    return count;
}

// ============================================================================
// VALIDATION (Existant - Inchangé)
// ============================================================================

bool GestionnaireStock::validerDisponibilite(const QUuid& produitId, int quantiteRequise)
{
    int quantiteDisponible = obtenirQuantiteDisponible(produitId);
    
    if (quantiteDisponible < quantiteRequise) {
        m_dernierErreur = QString("Stock insuffisant. Disponible: %1, Requis: %2")
                         .arg(quantiteDisponible).arg(quantiteRequise);
        qDebug() << "[VALIDATION STOCK]" << m_dernierErreur;
        return false;
    }
    
    return true;
}

bool GestionnaireStock::validerEntree(const EntreeStock& entree)
{
    if (!validerQuantite(entree.getQuantite())) {
        m_dernierErreur = "Quantité invalide pour l'entrée (doit être > 0)";
        return false;
    }
    
    if (!validerProduit(entree.getProduitId())) {
        m_dernierErreur = "Produit inexistant";
        return false;
    }
    
    if (entree.getCreePar().isNull()) {
        m_dernierErreur = "Utilisateur créateur non défini";
        return false;
    }
    
    return true;
}

bool GestionnaireStock::validerRetour(const RetourStock& retour)
{
    if (!validerQuantite(retour.getQuantite())) {
        m_dernierErreur = "Quantité invalide pour le retour (doit être > 0)";
        return false;
    }
    
    if (!validerProduit(retour.getProduitId())) {
        m_dernierErreur = "Produit inexistant";
        return false;
    }
    
    if (retour.getCreePar().isNull()) {
        m_dernierErreur = "Utilisateur créateur non défini";
        return false;
    }
    
    return true;
}

QString GestionnaireStock::obtenirErreurValidation()
{
    return m_dernierErreur;
}

// ============================================================================
// TRANSACTIONS (Existant - Inchangé)
// ============================================================================

bool GestionnaireStock::reserverStock(const QUuid& repartitionId, const QUuid& produitId, int quantite)
{
    if (!validerDisponibilite(produitId, quantite)) {
        return false;
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Réservation:" << quantite << "unités de produit" << produitId;
    return true;
}

bool GestionnaireStock::confirmerConsommation(const QUuid& repartitionId, const QUuid& produitId, int quantite)
{
    qDebug() << "[GESTIONNAIRE STOCK] Consommation confirmée:" << quantite << "unités";
    synchroniserStockSoldes();
    return true;
}

bool GestionnaireStock::annulerReservation(const QUuid& repartitionId, const QUuid& produitId)
{
    qDebug() << "[GESTIONNAIRE STOCK] Annulation réservation:" << repartitionId << produitId;
    return true;
}

bool GestionnaireStock::synchroniserStockSoldes()
{
    if (!m_repoSoldes) {
        m_dernierErreur = "Repository soldes non initialisé";
        return false;
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Synchronisation des soldes...";
    
    if (m_repoSoldes->synchroniserTousSoldes()) {
        qDebug() << "[GESTIONNAIRE STOCK] ✓ Synchronisation OK";
        return true;
    }
    
    m_dernierErreur = "Erreur lors de la synchronisation";
    return false;
}

// ============================================================================
// STATISTIQUES (Existant - Inchangé)
// ============================================================================

StatistiquesStock GestionnaireStock::obtenirStatistiques()
{
    StatistiquesStock stats;
    auto stocks = obtenirTousLesStocks();
    
    stats.nombreProduitsTotal = stocks.count();
    stats.nombreProduitsEnStock = 0;
    stats.nombreProduitsEnRupture = 0;
    stats.nombreProduitsBasStock = 0;
    stats.valeurTotalStock = 0.0;
    stats.quantiteTotalUnites = 0;
    stats.quantiteReserveeUnites = 0;
    stats.quantiteDisponibleUnites = 0;
    
    for (const auto& stock : stocks) {
        stats.valeurTotalStock += stock.valeurStock;
        stats.quantiteTotalUnites += stock.quantiteTotal;
        stats.quantiteReserveeUnites += stock.quantiteReservee;
        stats.quantiteDisponibleUnites += stock.quantiteDisponible;
        
        if (stock.quantiteDisponible > 0) {
            stats.nombreProduitsEnStock++;
        }
        
        if (stock.quantiteDisponible <= 0) {
            stats.nombreProduitsEnRupture++;
        } else if (stock.quantiteDisponible < stock.stockMinimum) {
            stats.nombreProduitsBasStock++;
        }
    }
    
    if (stats.nombreProduitsTotal > 0) {
        stats.valeurMoyenneProduit = stats.valeurTotalStock / stats.nombreProduitsTotal;
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Statistiques:"
             << "Total=" << stats.nombreProduitsTotal
             << "EnStock=" << stats.nombreProduitsEnStock
             << "Rupture=" << stats.nombreProduitsEnRupture
             << "Valeur=" << stats.valeurTotalStock;
    
    return stats;
}

QMap<QString, int> GestionnaireStock::obtenirStatutistiquesParCategorie()
{
    QMap<QString, int> stats;
    auto stocks = obtenirTousLesStocks();
    
    for (const auto& stock : stocks) {
        stats[stock.categorie]++;
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Statistiques par catégorie:" << stats.count();
    return stats;
}

QMap<QString, double> GestionnaireStock::obtenirValeurParCategorie()
{
    QMap<QString, double> valeurs;
    auto stocks = obtenirTousLesStocks();
    
    for (const auto& stock : stocks) {
        valeurs[stock.categorie] += stock.valeurStock;
    }
    
    qDebug() << "[GESTIONNAIRE STOCK] Valeur par catégorie:" << valeurs.count();
    return valeurs;
}

double GestionnaireStock::obtenirValeurTotalStock()
{
    if (!m_repoSoldes) return 0.0;
    return m_repoSoldes->obtenirValeurTotalStock();
}

double GestionnaireStock::obtenirValeurMoyenneProduit()
{
    auto stats = obtenirStatistiques();
    return stats.valeurMoyenneProduit;
}

int GestionnaireStock::obtenirRotationStock(const QUuid& produitId, int jours)
{
    auto mouvements = obtenirMouvementsParDateRange(
        QDate::currentDate().addDays(-jours),
        QDate::currentDate()
    );
    
    return mouvements.count();
}

// ============================================================================
// ✅ NOUVEAU: RAPPORT RÉCONCILIATION
// ============================================================================

QString GestionnaireStock::genererRapportReconciliation()
{
    auto resultats = verifierIntegriteStock();
    
    QString rapport = "═══════════════════════════════════════════\n";
    rapport += "RAPPORT RÉCONCILIATION STOCK - " + QDate::currentDate().toString("dd/MM/yyyy") + "\n";
    rapport += "═══════════════════════════════════════════\n\n";
    
    int ok = 0, erreurs = 0;
    
    for (const auto& res : resultats) {
        if (res.isConsistent) {
            ok++;
        } else {
            erreurs++;
        }
    }
    
    rapport += QString("Produits vérifiés: %1\n").arg(resultats.count());
    rapport += QString("Stocks valides: %1 ✓\n").arg(ok);
    rapport += QString("Stocks invalides: %1 ❌\n\n").arg(erreurs);
    
    if (erreurs > 0) {
        rapport += "DISCREPANCIES DÉTECTÉES:\n";
        rapport += "──────────────────────\n";
        
        for (const auto& res : resultats) {
            if (!res.isConsistent) {
                rapport += QString("• %1 (%2)\n").arg(res.produitNom, res.status);
                rapport += QString("  Mouvements: %1, Soldes: %2, Différence: %3\n")
                    .arg(res.stockFromMovements)
                    .arg(res.stockInSoldes)
                    .arg(res.difference);
            }
        }
    }
    
    return rapport;
}

// ============================================================================
// RAPPORTS (Existant - Enrichi)
// ============================================================================

QString GestionnaireStock::genererRapportStock()
{
    auto stats = obtenirStatistiques();
    
    QString rapport = "═══════════════════════════════════════════\n";
    rapport += "RAPPORT DE STOCK - " + QDate::currentDate().toString("dd/MM/yyyy") + "\n";
    rapport += "═══════════════════════════════════════════\n\n";
    
    rapport += QString("Produits totaux: %1\n").arg(stats.nombreProduitsTotal);
    rapport += QString("Produits en stock: %1\n").arg(stats.nombreProduitsEnStock);
    rapport += QString("Produits en rupture: %1\n").arg(stats.nombreProduitsEnRupture);
    rapport += QString("Produits bas stock: %1\n\n").arg(stats.nombreProduitsBasStock);
    
    rapport += QString("Quantités:\n");
    rapport += QString("  Total: %1 unités\n").arg(stats.quantiteTotalUnites);
    rapport += QString("  Réservée: %1 unités\n").arg(stats.quantiteReserveeUnites);
    rapport += QString("  Disponible: %1 unités\n\n").arg(stats.quantiteDisponibleUnites);
    
    rapport += QString("Valeur:\n");
    rapport += QString("  Total: %1 €\n").arg(stats.valeurTotalStock, 0, 'f', 2);
    rapport += QString("  Moyenne par produit: %1 €\n").arg(stats.valeurMoyenneProduit, 0, 'f', 2);
    
    return rapport;
}

QString GestionnaireStock::genererRapportAlertes()
{
    auto alertes = obtenirAlertes();
    
    QString rapport = "═══════════════════════════════════════════\n";
    rapport += "RAPPORT D'ALERTES STOCK\n";
    rapport += "═══════════════════════════════════════════\n\n";
    
    rapport += QString("Total d'alertes: %1\n\n").arg(alertes.count());
    
    for (const auto& alerte : alertes) {
        rapport += QString("• %1 (%2)\n").arg(alerte.produitNom, alerte.codeSKU);
        rapport += QString("  Sévérité: %1\n").arg(alerte.severite);
        rapport += QString("  Quantité: %1 (min: %2)\n").arg(alerte.quantiteDisponible).arg(alerte.stockMinimum);
        rapport += QString("  Action: %1\n\n").arg(alerte.action);
    }
    
    return rapport;
}

QString GestionnaireStock::genererRapportMouvements(const QDate& dateDebut, const QDate& dateFin)
{
    auto mouvements = obtenirMouvementsParDateRange(dateDebut, dateFin);
    
    QString rapport = "═══════════════════════════════════════════\n";
    rapport += QString("RAPPORT MOUVEMENTS %1 à %2\n").arg(dateDebut.toString("dd/MM/yyyy"), dateFin.toString("dd/MM/yyyy"));
    rapport += "═══════════════════════════════════════════\n\n";
    
    rapport += QString("Total mouvements: %1\n\n").arg(mouvements.count());
    
    for (const auto& mvt : mouvements) {
        rapport += QString("• %1 | Type: %2 | Qté: %3\n").arg(mvt.dateCreation, mvt.type).arg(mvt.quantiteDelta);
    }
    
    return rapport;
}

// ============================================================================
// MÉTHODES INTERNES (Existant - Inchangé)
// ============================================================================

bool GestionnaireStock::validerQuantite(int quantite)
{
    return quantite > 0;
}

bool GestionnaireStock::validerProduit(const QUuid& produitId)
{
    return !produitId.isNull();
}

QString GestionnaireStock::determinerStatutStock(int quantiteDisponible, int stockMinimum)
{
    if (quantiteDisponible <= 0) return "RUPTURE";
    if (quantiteDisponible < stockMinimum) return "CRITIQUE";
    if (quantiteDisponible < stockMinimum * 1.5) return "FAIBLE";
    return "OK";
}

double GestionnaireStock::calculerValeurStock(int quantite, double prixMoyen)
{
    return quantite * prixMoyen;
}

QString GestionnaireStock::genererRecommandation(const Alerte& alerte)
{
    if (alerte.severite == "RUPTURE") return "URGENT: Commander immédiatement";
    if (alerte.severite == "CRITICAL") return "Commander en priorité";
    return "Prévoir réapprovisionnement";
}