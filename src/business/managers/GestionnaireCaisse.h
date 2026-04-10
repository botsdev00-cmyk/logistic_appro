#ifndef GESTIONNAIREKAISSE_H
#define GESTIONNAIREKAISSE_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDateTime>

class ReceptionCaisse;

class GestionnaireCaisse
{
public:
    GestionnaireCaisse();

    // Cash reception operations
    QUuid creerReceptionCaisse(const QUuid& repartitionId, double montantAttendu);
    bool enregistrerMontantRecu(const QUuid& receptionId, double montantRecu);
    bool validerReception(const QUuid& receptionId);
    bool genererRecu(const QUuid& receptionId);
    
    // Cash queries
    ReceptionCaisse obtenirReception(const QUuid& receptionId);
    ReceptionCaisse obtenirReceptionRepartition(const QUuid& repartitionId);
    QList<ReceptionCaisse> obtenirReceptionsEnAttente();
    QList<ReceptionCaisse> obtenirReceptionsAvecDiscrepance();
    
    // Statistics
    double obtenirTotalCashRecu();
    double obtenirTotalEcarts();
    int obtenirNombreDiscrepances();
    
    // Validation and alerts
    bool validerDiscrepance(const QUuid& receptionId);
    QList<ReceptionCaisse> obtenirAlertesDiscrepance();
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
    
    QString genererNumeroRecu();
    bool detecterDiscrepance(const QUuid& receptionId);
};

#endif // GESTIONNAIREKAISSE_H