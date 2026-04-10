#ifndef GESTIONNAIREDALES_H
#define GESTIONNAIREDALES_H

#include <QString>
#include <QList>
#include <QUuid>

class Vente;

class GestionnaireSales
{
public:
    GestionnaireSales();

    // Sale operations
    QUuid enregistrerVente(const QUuid& repartitionId, const QUuid& produitId, const QUuid& clientId,
                          int quantite, const QString& typeVente, double prixUnitaire);
    bool modifierVente(const QUuid& venteId, int nouvelleQuantite, double nouveauPrix);
    bool supprimerVente(const QUuid& venteId);
    
    // Sale queries
    Vente obtenirVente(const QUuid& venteId);
    QList<Vente> obtenirVentesRepartition(const QUuid& repartitionId);
    QList<Vente> obtenirVentesClient(const QUuid& clientId);
    
    // Statistics
    double obtenirTotalVentes(const QUuid& repartitionId);
    double obtenirTotalVentesCash(const QUuid& repartitionId);
    double obtenirTotalVentesCredit(const QUuid& repartitionId);
    int obtenirNombreVentes(const QUuid& repartitionId);
    
    // Validation
    bool validerVente(const QUuid& repartitionId, const QUuid& produitId, int quantite);
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREDALES_H