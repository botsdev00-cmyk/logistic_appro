#ifndef GESTIONNAIRECAISSE_H
#define GESTIONNAIRECAISSE_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDateTime>
#include "../../core/entities/ReceptionCaisse.h"

class GestionnaireCaisse
{
public:
    GestionnaireCaisse();

    // Opérations de caisse (offline-first)
    QUuid creerReceptionCaisse(const QUuid& repartitionId, double montantAttendu, const QUuid& caissierId = QUuid(), const QString& notes = "");
    bool enregistrerMontantRecu(const QUuid& receptionId, double montantRecu);
    bool validerReception(const QUuid& receptionId);
    bool softDeleteReception(const QUuid& receptionId);

    // Requêtes
    ReceptionCaisse obtenirReception(const QUuid& receptionId);
    ReceptionCaisse obtenirReceptionParRepartition(const QUuid& repartitionId);
    QList<ReceptionCaisse> obtenirReceptionsEnAttente();
    QList<ReceptionCaisse> obtenirReceptionsAvecDiscrepance();

    // Statistiques / alertes
    double obtenirTotalCashValide();
    double obtenirTotalEcarts();
    int obtenirNombreDiscrepances();

    // Gestion du statut de synchronisation
    QList<ReceptionCaisse> obtenirReceptionsASynchroniser();
    QList<ReceptionCaisse> obtenirReceptionsEnConflit();
    bool marquerSynced(const QUuid& receptionId, int nouvelleVersion);
    bool marquerConflit(const QUuid& receptionId);

    // Génération utilitaire
    QString genererNumeroRecu() const;

    // Gestion des erreurs
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIRECAISSE_H
