#ifndef GESTIONNAIREREPRTITION_H
#define GESTIONNAIREREPRTITION_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDate>

class Repartition;
class ArticleRepartition;

class GestionnaireRepartition
{
public:
    GestionnaireRepartition();

    // Dispatch operations
    QUuid creerRepartition(const QUuid& equipeId, const QUuid& routeId, const QDate& dateRepartition);
    bool ajouterArticle(const QUuid& repartitionId, const QUuid& produitId, int quantiteVente, 
                       int quantiteCadeau, int quantiteDegustation);
    bool marquerEnCours(const QUuid& repartitionId);
    bool marquerCompletee(const QUuid& repartitionId);
    bool annulerRepartition(const QUuid& repartitionId);
    
    // Dispatch queries
    Repartition obtenirRepartition(const QUuid& repartitionId);
    QList<Repartition> obtenirRepartitionsEnCours();
    QList<Repartition> obtenirRepartitionsParEquipe(const QUuid& equipeId);
    
    // Performance tracking
    int obtenirNombreClientsVisites(const QUuid& repartitionId);
    double obtenirMontantVentes(const QUuid& repartitionId);
    double obtenirMontantCash(const QUuid& repartitionId);
    
    // Validation
    bool validerRepartition(const QUuid& repartitionId);
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    
    bool verifierQuantitesDisponibles(const QUuid& repartitionId);
};

#endif // GESTIONNAIREREPRTITION_H