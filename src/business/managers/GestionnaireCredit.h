#ifndef GESTIONNAIRECREDIT_H
#define GESTIONNAIRECREDIT_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDate>
#include "../../core/entities/Credit.h"

class GestionnaireCredit
{
public:
    GestionnaireCredit();

    // Credit operations
    QUuid creerCredit(const QUuid& venteId, const QUuid& clientId, double montant, const QDate& dateEcheance, const QString& notes = "");
    bool payerCredit(const QUuid& creditId, const QDate& datePaiement);
    bool annulerCredit(const QUuid& creditId);  // logical delete

    // Queries
    Credit obtenirCredit(const QUuid& creditId);
    QList<Credit> obtenirCreditsClient(const QUuid& clientId);
    QList<Credit> obtenirCreditsEnRetard();
    QList<Credit> obtenirCreditEnAttente();

    // Offline queries/statistics
    QList<Credit> obtenirPendingSync();
    QList<Credit> obtenirDepuisVersion(int version);

    // Statistics
    double obtenirTotalCreditsEnAttente();
    double obtenirTotalCreditsEnRetard();
    double obtenirTotalCreditsClient(const QUuid& clientId);

    // Validation and alerts
    bool validerCredit(const QUuid& creditId);
    QList<Credit> obtenirCreditsAlerte();

    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    void mettreAJourStatutCredit(const QUuid& creditId);
};

#endif // GESTIONNAIRECREDIT_H
