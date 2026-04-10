#ifndef GESTIONNAIREKREDIT_H
#define GESTIONNAIREKREDIT_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDate>

class Credit;

class GestionnaireCredit
{
public:
    GestionnaireCredit();

    // Credit operations
    QUuid creerCredit(const QUuid& venteId, const QUuid& clientId, double montant, const QDate& dateEcheance);
    bool payerCredit(const QUuid& creditId, const QDate& datePaiement);
    bool annulerCredit(const QUuid& creditId);
    
    // Credit queries
    Credit obtenirCredit(const QUuid& creditId);
    QList<Credit> obtenirCreditsClient(const QUuid& clientId);
    QList<Credit> obtenirCreditsEnRetard();
    QList<Credit> obtenirCreditEnAttente();
    
    // Statistics
    double obtenirTotalCreditsEnAttente();
    double obtenirTotalCreditsEnRetard();
    double obtenirTotalCreditsClient(const QUuid& clientId);
    
    // Validation and alerts
    bool validerCredit(const QUuid& creditId);
    QList<Credit> obtenirCreditsAlerte();
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    
    void mettreAJourStatutCredit(const QUuid& creditId);
};

#endif // GESTIONNAIREKREDIT_H