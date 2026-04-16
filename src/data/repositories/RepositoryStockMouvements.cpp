#include "RepositoryStockMouvements.h"
#include "../database/ConnexionBaseDonnees.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================================
// CONSTRUCTEUR & DESTRUCTEUR
// ============================================================================

RepositoryStockMouv ements::RepositoryStockMouv ements()
{
    qDebug() << "[REPO-MVT] Initialisation";
}

RepositoryStockMouv ements::~RepositoryStockMouv ements()
{
    qDebug() << "[REPO-MVT] Destruction";
}

// ============================================================================
// CRÉATION SÉCURISÉE DE MOUVEMENTS
// ============================================================================

ResultatMouvement RepositoryStockMouv ements::creerMouvementSecurise(
    const QUuid& produitId,
    const QString& typeMouvement,
    int quantiteDelta,
    const QUuid& referenceId,
    const QString& referenceType,
    const QUuid& utilisateurId,
    const QString& locationId,
    const QString& raison,
    const QString& observations)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-MVT] Création mouvement sécurisée"
             << "| Type:" << typeMouvement
             << "| Produit:" << produitId.toString().mid(0, 8) << "..."
             << "| Qté:" << quantiteDelta;

    // ✅ APPELER LA FONCTION BDD SÉCURISÉE
    query.prepare(
        "SELECT success, mouvement_id, message, current_stock, resulting_stock "
        "FROM fn_create_stock_movement(?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(produitId.toString());
    query.addBindValue(typeMouvement);
    query.addBindValue(quantiteDelta);
    query.addBindValue(referenceId.isNull() ? QVariant(QVariant::String) : referenceId.toString());
    query.addBindValue(referenceType.isEmpty() ? QVariant(QVariant::String) : referenceType);
    query.addBindValue(utilisateurId.toString());
    query.addBindValue(locationId);
    query.addBindValue(raison.isEmpty() ? QVariant(QVariant::String) : raison);
    query.addBindValue(observations.isEmpty() ? QVariant(QVariant::String) : observations);

    ResultatMouvement resultat{false, QUuid(), "Erreur inconnue", 0, 0};

    if (query.exec()) {
        if (query.next()) {
            resultat.success = query.value("success").toBool();
            resultat.mouvementId = QUuid(query.value("mouvement_id").toString());
            resultat.message = query.value("message").toString();
            resultat.stockActuel = query.value("current_stock").toInt();
            resultat.stockResultant = query.value("resulting_stock").toInt();

            if (resultat.success) {
                qDebug() << "[REPO-MVT] ✓ Mouvement créé:" << resultat.mouvementId.toString().mid(0, 8) << "..."
                         << "| Stock:" << resultat.stockActuel << "→" << resultat.stockResultant
                         << "| Message:" << resultat.message;
            } else {
                qDebug() << "[REPO-MVT] ❌ Mouvement refusé:" << resultat.message;
                m_dernierErreur = resultat.message;
            }
        } else {
            resultat.message = "Pas de réponse du serveur";
            qDebug() << "[REPO-MVT] ❌ Pas de réponse";
            m_dernierErreur = resultat.message;
        }
    } else {
        resultat.message = query.lastError().text();
        qDebug() << "[REPO-MVT] ❌ ERREUR SQL:" << resultat.message;
        m_dernierErreur = resultat.message;
    }

    return resultat;
}

// ============================================================================
// VÉRIFICATION AVANT CRÉATION
// ============================================================================

ResultatMouvement RepositoryStockMouv ements::verifierOperationStock(
    const QUuid& produitId,
    int quantiteDelta,
    const QString& typeMouvement)
{
    ConnexionBaseDonnees& bd = ConnexionBaseDonnees::getInstance();
    QSqlQuery query(bd.getDatabase());

    qDebug() << "[REPO-MVT] Vérification opération stock"
             << "| Type:" << typeMouvement
             << "| Qté:" << quantiteDelta;

    // ✅ APPELER LA FONCTION DE VÉRIFICATION BDD
    query.prepare(
        "SELECT can_proceed, current_stock, resulting_stock, message "
        "FROM fn_verify_stock_operation(?, ?, ?)"
    );

    query.addBindValue(produitId.toString());
    query.addBindValue(quantiteDelta);
    query.addBindValue(typeMouvement);

    ResultatMouvement resultat{false, QUuid(), "Erreur vérification", 0, 0};

    if (query.exec()) {
        if (query.next()) {
            resultat.success = query.value("can_proceed").toBool();
            resultat.message = query.value("message").toString();
            resultat.stockActuel = query.value("current_stock").toInt();
            resultat.stockResultant = query.value("resulting_stock").toInt();

            if (resultat.success) {
                qDebug() << "[REPO-MVT] ✓ Opération autorisée"
                         << "| Stock:" << resultat.stockActuel << "→" << resultat.stockResultant;
            } else {
                qDebug() << "[REPO-MVT] ❌ Opération refusée:" << resultat.message;
                m_dernierErreur = resultat.message;
            }
        }
    } else {
        resultat.message = query.lastError().text();
        qDebug() << "[REPO-MVT] ❌ ERREUR SQL:" << resultat.message;
        m_dernierErreur = resultat.message;
    }

    return resultat;
}