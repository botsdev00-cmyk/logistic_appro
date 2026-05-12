#ifndef GESTIONNAIREEQUIPE_H
#define GESTIONNAIREEQUIPE_H

#include <QString>
#include <QUuid>
#include <QList>
#include "../../core/entities/Equipe.h"

class GestionnaireEquipe
{
public:
    GestionnaireEquipe();
    ~GestionnaireEquipe();

    // CRUD métier
    QUuid creerEquipe(const QString& nom, 
                      const QString& nomChef,
                      // const QString& description,
                      const QUuid& createdBy);
    
    bool modifierEquipe(const Equipe& equipe, const QUuid& updatedBy);
    bool supprimerEquipe(const QUuid& equipeId, const QUuid& deletedBy);
    Equipe obtenirEquipe(const QUuid& equipeId) const;
    QList<Equipe> listerEquipes() const;
    QList<Equipe> rechercherEquipes(const QString& criterion) const;

    // Offline-first
    QList<Equipe> obtenirEquipesPendantes() const;
    QList<Equipe> obtenirEquipesEnConflit() const;
    int compterEquipesPendantes() const;
    
    // Synchronisation - API C++ pur
    struct SyncReport {
        int totalTreated;
        int syncedCount;
        int conflictCount;
        QString message;
    };
    SyncReport synchroniserEquipes(const QList<Equipe>& equipes, const QUuid& utilisateurId);

    // Error handling
    QString getDernierErreur() const { return m_dernierErreur; }

private:
    QString m_dernierErreur;
};

#endif // GESTIONNAIREEQUIPE_H
