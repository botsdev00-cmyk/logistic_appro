#ifndef REPOSITORYSTOCKMOUVEMENTS_H
#define REPOSITORYSTOCKMOUVEMENTS_H

#include <QUuid>
#include <QString>

// ============================================================================
// STRUCTURE: Résultat de création de mouvement
// ============================================================================

struct ResultatMouvement {
    bool success;
    QUuid mouvementId;
    QString message;
    int stockActuel;
    int stockResultant;
};

// ============================================================================
// CLASSE: Repository pour Mouvements de Stock
// ============================================================================

class RepositoryStockMouvements
{
public:
    RepositoryStockMouvements();
    ~RepositoryStockMouvements();

    // ====== CRÉATION SÉCURISÉE DE MOUVEMENTS ======
    ResultatMouvement creerMouvementSecurise(
        const QUuid& produitId,
        const QString& typeMouvement,
        int quantiteDelta,
        const QUuid& referenceId,
        const QString& referenceType,
        const QUuid& utilisateurId,
        const QString& locationId = "WAREHOUSE",
        const QString& raison = "",
        const QString& observations = ""
    );

    // ====== CONSULTATIONS ======
    ResultatMouvement verifierOperationStock(
        const QUuid& produitId,
        int quantiteDelta,
        const QString& typeMouvement
    );

    // ====== GESTION DES ERREURS ======
    QString getLastError() const { return m_dernierErreur; }
    void clearError() { m_dernierErreur.clear(); }

private:
    QString m_dernierErreur;
};

#endif // REPOSITORYSTOCKMOUVEMENTS_H