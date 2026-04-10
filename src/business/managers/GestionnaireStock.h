#ifndef GESTIONNAIRESTOCK_H
#define GESTIONNAIRESTOCK_H

#include <QString>
#include <QList>
#include <QUuid>

class EntreeStock;
class Produit;

class GestionnaireStock
{
public:
    GestionnaireStock();

    // Stock operations
    bool ajouterStock(const QUuid& produitId, int quantite, const QString& source);
    bool retirerStock(const QUuid& produitId, int quantite);
    bool retournerStock(const QUuid& produitId, int quantite, const QString& raison);
    
    // Stock queries
    int obtenirStockActuel(const QUuid& produitId);
    QList<Produit> obtenirProduitsEnRupture();
    QList<Produit> obtenirProduitsBasStock();
    
    // Stock adjustments
    bool ajusterStock(const QUuid& produitId, int nouvelleQuantite, const QString& raison);
    
    // Validation
    bool validerQuantiteDisponible(const QUuid& produitId, int quantiteRequise);
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    
    bool verifierDisponibilite(const QUuid& produitId, int quantite);
    void enregistrerMouvement(const QUuid& produitId, int quantite, const QString& type);
};

#endif // GESTIONNAIRESTOCK_H