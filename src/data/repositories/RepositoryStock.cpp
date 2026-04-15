#include "RepositoryStock.h"
#include "../database/ConnexionBaseDonnees.h"
#include "../../core/entities/EntreeStock.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>

// ============================================================================
// CONSTRUCTEUR
// ============================================================================

RepositoryStock::RepositoryStock()
{
    qDebug() << "[REPOSITORY STOCK] Initialisation";
}

// ============================================================================
// CREATE
// ============================================================================

bool RepositoryStock::create(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] CREATE: entree_id=" << entity.getEntreeStockId();

    query.prepare("INSERT INTO entrees_stock "
                  "(entree_stock_id, produit_id, quantite, source_entree_id, "
                  "numero_facture, prix_unitaire, numero_lot, date_expiration, "
                  "cree_par, statut_validation, date, date_mise_a_jour) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(entity.getEntreeStockId().toString());
    query.addBindValue(entity.getProduitId().toString());
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getSourceEntreeId().toString());
    query.addBindValue(entity.getNumeroFacture());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(entity.getCreePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getDate());
    query.addBindValue(entity.getDateMiseAJour());

    if (!query.exec()) {
        m_dernierErreur = "Erreur création entrée stock: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] CREATE ERROR:" << m_dernierErreur;
        return false;
    }

    qDebug() << "[REPO-STOCK] ✓ CREATE SUCCESS";
    return true;
}

// ============================================================================
// READ - BY ID
// ============================================================================

EntreeStock RepositoryStock::getById(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] getById: id=" << id.toString();

    query.prepare("SELECT * FROM entrees_stock WHERE entree_stock_id = ?");
    query.addBindValue(id.toString());

    EntreeStock entree;

    if (query.exec() && query.next()) {
        entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
        entree.setProduitId(QUuid(query.value("produit_id").toString()));
        entree.setQuantite(query.value("quantite").toInt());
        entree.setSourceEntreeId(QUuid(query.value("source_entree_id").toString()));
        entree.setNumeroFacture(query.value("numero_facture").toString());
        entree.setPrixUnitaire(query.value("prix_unitaire").toDouble());
        entree.setNumeroLot(query.value("numero_lot").toString());
        entree.setDateExpiration(query.value("date_expiration").toDate());
        entree.setCreePar(QUuid(query.value("cree_par").toString()));
        entree.setApprouvePar(QUuid(query.value("approuve_par").toString()));
        entree.setStatutValidation(query.value("statut_validation").toString());

        qDebug() << "[REPO-STOCK] ✓ getById found: produit=" << entree.getProduitId() 
                 << "qty=" << entree.getQuantite();
    } else {
        m_dernierErreur = "Entrée stock non trouvée";
        qDebug() << "[REPO-STOCK] getById ERROR: NOT FOUND";
    }

    return entree;
}

// ============================================================================
// READ - ALL
// ============================================================================

QList<EntreeStock> RepositoryStock::getAll()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    qDebug() << "[REPO-STOCK] getAll";

    query.prepare("SELECT * FROM entrees_stock ORDER BY date DESC");

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree;
            entree.setEntreeStockId(QUuid(query.value("entree_stock_id").toString()));
            entree.setProduitId(QUuid(query.value("produit_id").toString()));
            entree.setQuantite(query.value("quantite").toInt());
            entree.setSourceEntreeId(QUuid(query.value("source_entree_id").toString()));
            entree.setNumeroFacture(query.value("numero_facture").toString());
            entree.setPrixUnitaire(query.value("prix_unitaire").toDouble());
            entree.setNumeroLot(query.value("numero_lot").toString());
            entree.setDateExpiration(query.value("date_expiration").toDate());
            entree.setCreePar(QUuid(query.value("cree_par").toString()));
            entree.setApprouvePar(QUuid(query.value("approuve_par").toString()));
            entree.setStatutValidation(query.value("statut_validation").toString());
            
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-STOCK] ✓ getAll: returned" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture entrees stock: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] getAll ERROR:" << query.lastError().text();
    }

    return entrees;
}

// ============================================================================
// UPDATE
// ============================================================================

bool RepositoryStock::update(const EntreeStock& entity)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] UPDATE: id=" << entity.getEntreeStockId();

    query.prepare("UPDATE entrees_stock SET "
                  "quantite = ?, "
                  "prix_unitaire = ?, "
                  "numero_lot = ?, "
                  "date_expiration = ?, "
                  "approuve_par = ?, "
                  "statut_validation = ?, "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE entree_stock_id = ?");
    
    query.addBindValue(entity.getQuantite());
    query.addBindValue(entity.getPrixUnitaire());
    query.addBindValue(entity.getNumeroLot());
    query.addBindValue(entity.getDateExpiration());
    query.addBindValue(entity.getApprouvePar().toString());
    query.addBindValue(entity.getStatutValidation());
    query.addBindValue(entity.getEntreeStockId().toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur mise à jour entree stock: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] UPDATE ERROR:" << m_dernierErreur;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "[REPO-STOCK] ✓ UPDATE: rows affected=" << affected;
    return affected > 0;
}

// ============================================================================
// DELETE
// ============================================================================

bool RepositoryStock::remove(const QUuid& id)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] REMOVE: id=" << id.toString();

    query.prepare("DELETE FROM entrees_stock WHERE entree_stock_id = ?");
    query.addBindValue(id.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur suppression entree stock: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] REMOVE ERROR:" << m_dernierErreur;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "[REPO-STOCK] ✓ REMOVE: rows affected=" << affected;
    return affected > 0;
}

// ============================================================================
// SEARCH
// ============================================================================

QList<EntreeStock> RepositoryStock::search(const QString& criterion)
{
    QList<EntreeStock> all = getAll();
    QList<EntreeStock> filtered;
    
    qDebug() << "[REPO-STOCK] search criterion=" << criterion;

    for (const auto& entree : all) {
        if (entree.getNumeroFacture().contains(criterion, Qt::CaseInsensitive) ||
            entree.getNumeroLot().contains(criterion, Qt::CaseInsensitive) ||
            entree.getProduitId().toString().contains(criterion, Qt::CaseInsensitive)) {
            filtered.append(entree);
        }
    }
    
    qDebug() << "[REPO-STOCK] search: found" << filtered.count() << "results";
    return filtered;
}

// ============================================================================
// EXISTS
// ============================================================================

bool RepositoryStock::exists(const QUuid& id)
{
    return !getById(id).getEntreeStockId().isNull();
}

// ============================================================================
// SPÉCIFIQUES - BY PRODUIT
// ============================================================================

QList<EntreeStock> RepositoryStock::getByProduit(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    qDebug() << "[REPO-STOCK] getByProduit: produit_id=" << produitId.toString();

    query.prepare("SELECT * FROM entrees_stock WHERE produit_id = ? ORDER BY date DESC");
    query.addBindValue(produitId.toString());

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-STOCK] ✓ getByProduit: found" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture entrees par produit: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] getByProduit ERROR:" << query.lastError().text();
    }

    return entrees;
}

// ============================================================================
// SPÉCIFIQUES - BY STATUT
// ============================================================================

QList<EntreeStock> RepositoryStock::getByStatut(const QString& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    qDebug() << "[REPO-STOCK] getByStatut: statut=" << statut;

    query.prepare("SELECT * FROM entrees_stock WHERE statut_validation = ? ORDER BY date DESC");
    query.addBindValue(statut);

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-STOCK] ✓ getByStatut: found" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture entrees par statut: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] getByStatut ERROR:" << query.lastError().text();
    }

    return entrees;
}

// ============================================================================
// SPÉCIFIQUES - EN ATTENTE
// ============================================================================

QList<EntreeStock> RepositoryStock::getEnAttente()
{
    qDebug() << "[REPO-STOCK] getEnAttente";
    return getByStatut("EN_ATTENTE");
}

// ============================================================================
// SPÉCIFIQUES - PAR SOURCE
// ============================================================================

QList<EntreeStock> RepositoryStock::getBySource(const QUuid& sourceId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    qDebug() << "[REPO-STOCK] getBySource: source_id=" << sourceId.toString();

    query.prepare("SELECT * FROM entrees_stock WHERE source_entree_id = ? ORDER BY date DESC");
    query.addBindValue(sourceId.toString());

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-STOCK] ✓ getBySource: found" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture entrees par source: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] getBySource ERROR:" << query.lastError().text();
    }

    return entrees;
}

// ============================================================================
// SPÉCIFIQUES - BY DATE RANGE
// ============================================================================

QList<EntreeStock> RepositoryStock::getByDateRange(const QDate& dateDebut, const QDate& dateFin)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    qDebug() << "[REPO-STOCK] getByDateRange: from" << dateDebut.toString("yyyy-MM-dd") 
             << "to" << dateFin.toString("yyyy-MM-dd");

    query.prepare("SELECT * FROM entrees_stock "
                  "WHERE DATE(date) BETWEEN ? AND ? "
                  "ORDER BY date DESC");
    query.addBindValue(dateDebut);
    query.addBindValue(dateFin);

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-STOCK] ✓ getByDateRange: found" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture entrees par date: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] getByDateRange ERROR:" << query.lastError().text();
    }

    return entrees;
}

// ============================================================================
// SPÉCIFIQUES - APPROVAL
// ============================================================================

bool RepositoryStock::approuver(const QUuid& entreeId, const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] approuver: id=" << entreeId << "par=" << utilisateurId;

    query.prepare("UPDATE entrees_stock SET "
                  "statut_validation = 'APPROUVE', "
                  "approuve_par = ?, "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE entree_stock_id = ?");
    query.addBindValue(utilisateurId.toString());
    query.addBindValue(entreeId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation entree stock: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] approuver ERROR:" << m_dernierErreur;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "[REPO-STOCK] ✓ approuver: rows affected=" << affected;
    return affected > 0;
}

// ============================================================================
// SPÉCIFIQUES - REJECTION
// ============================================================================

bool RepositoryStock::rejeter(const QUuid& entreeId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] rejeter: id=" << entreeId;

    query.prepare("UPDATE entrees_stock SET "
                  "statut_validation = 'REJETE', "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE entree_stock_id = ?");
    query.addBindValue(entreeId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur rejet entree stock: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] rejeter ERROR:" << m_dernierErreur;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "[REPO-STOCK] ✓ rejeter: rows affected=" << affected;
    return affected > 0;
}

// ============================================================================
// SPÉCIFIQUES - COUNT
// ============================================================================

int RepositoryStock::count()
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT COUNT(*) as total FROM entrees_stock");

    if (query.exec() && query.next()) {
        int total = query.value("total").toInt();
        qDebug() << "[REPO-STOCK] count:" << total;
        return total;
    }

    return 0;
}

// ============================================================================
// SPÉCIFIQUES - COUNT BY STATUT
// ============================================================================

int RepositoryStock::countByStatut(const QString& statut)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT COUNT(*) as total FROM entrees_stock WHERE statut_validation = ?");
    query.addBindValue(statut);

    if (query.exec() && query.next()) {
        int total = query.value("total").toInt();
        qDebug() << "[REPO-STOCK] countByStatut" << statut << ":" << total;
        return total;
    }

    return 0;
}

// ============================================================================
// SPÉCIFIQUES - SUM QUANTITÉ
// ============================================================================

int RepositoryStock::sumQuantite(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT COALESCE(SUM(quantite), 0) as total FROM entrees_stock WHERE produit_id = ?");
    query.addBindValue(produitId.toString());

    if (query.exec() && query.next()) {
        int total = query.value("total").toInt();
        qDebug() << "[REPO-STOCK] sumQuantite produit" << produitId << ":" << total;
        return total;
    }

    return 0;
}

// ============================================================================
// SPÉCIFIQUES - SUM VALEUR
// ============================================================================

double RepositoryStock::sumValeur(const QUuid& produitId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    query.prepare("SELECT COALESCE(SUM(quantite * prix_unitaire), 0.0) as total "
                  "FROM entrees_stock WHERE produit_id = ?");
    query.addBindValue(produitId.toString());

    if (query.exec() && query.next()) {
        double total = query.value("total").toDouble();
        qDebug() << "[REPO-STOCK] sumValeur produit" << produitId << ":" << total;
        return total;
    }

    return 0.0;
}

// ============================================================================
// SPÉCIFIQUES - BULK OPERATIONS
// ============================================================================

bool RepositoryStock::approuverTous(const QUuid& utilisateurId)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-STOCK] approuverTous par" << utilisateurId;

    query.prepare("UPDATE entrees_stock SET "
                  "statut_validation = 'APPROUVE', "
                  "approuve_par = ?, "
                  "date_mise_a_jour = CURRENT_TIMESTAMP "
                  "WHERE statut_validation = 'EN_ATTENTE'");
    query.addBindValue(utilisateurId.toString());

    if (!query.exec()) {
        m_dernierErreur = "Erreur approbation en masse: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] approuverTous ERROR:" << m_dernierErreur;
        return false;
    }

    int affected = query.numRowsAffected();
    qDebug() << "[REPO-STOCK] ✓ approuverTous: rows affected=" << affected;
    return true;
}

// ============================================================================
// SPÉCIFIQUES - PAGINATION
// ============================================================================

QList<EntreeStock> RepositoryStock::getPagine(int page, int pageSize)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());
    QList<EntreeStock> entrees;

    int offset = (page - 1) * pageSize;

    qDebug() << "[REPO-STOCK] getPagine: page=" << page << "size=" << pageSize << "offset=" << offset;

    query.prepare("SELECT * FROM entrees_stock ORDER BY date DESC LIMIT ? OFFSET ?");
    query.addBindValue(pageSize);
    query.addBindValue(offset);

    if (query.exec()) {
        int count = 0;
        while (query.next()) {
            EntreeStock entree = getById(QUuid(query.value("entree_stock_id").toString()));
            entrees.append(entree);
            count++;
        }
        qDebug() << "[REPO-STOCK] ✓ getPagine: returned" << count << "entries";
    } else {
        m_dernierErreur = "Erreur lecture pagine: " + query.lastError().text();
        qDebug() << "[REPO-STOCK] getPagine ERROR:" << query.lastError().text();
    }

    return entrees;
}