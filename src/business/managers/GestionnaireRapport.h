#ifndef GESTIONNAIREAPPORT_H
#define GESTIONNAIREAPPORT_H

#include <QString>
#include <QList>
#include <QUuid>
#include <QDate>
#include <QMap>

class GestionnaireRapport
{
public:
    GestionnaireRapport();

    // Daily reports
    QMap<QString, double> obtenirRapportJournalier(const QDate& date);
    
    // Team reports
    QMap<QString, double> obtenirRapportEquipe(const QUuid& equipeId, const QDate& dateDebut, const QDate& dateFin);
    
    // Sales reports
    QMap<QString, double> obtenirRapportVentes(const QDate& dateDebut, const QDate& dateFin);
    QMap<QString, double> obtenirRapportVentesParCategorie(const QDate& dateDebut, const QDate& dateFin);
    
    // Credit reports
    QMap<QString, double> obtenirRapportCredits();
    
    // Cash reports
    QMap<QString, double> obtenirRapportCaisse(const QDate& dateDebut, const QDate& dateFin);
    
    // Stock reports
    QMap<QString, int> obtenirRapportStock();
    
    // Client reports
    QList<QMap<QString, QVariant>> obtenirRapportClients();
    
    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREAPPORT_H