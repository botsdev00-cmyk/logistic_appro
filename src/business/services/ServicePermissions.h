#ifndef SERVICEPERMISSIONS_H
#define SERVICEPERMISSIONS_H

#include <QUuid>
#include <QString>
#include <QList>

class RepositoryUtilisateur;

struct Permission {
    QString code;
    QString nom;
    QString module;
};

class ServicePermissions
{
public:
    ServicePermissions();
    ~ServicePermissions();

    void setRepositoryUtilisateur(RepositoryUtilisateur* repo);

    // Vérifier une permission
    bool verifierPermission(const QUuid& utilisateurId, const QString& codePermission);
    bool verifierPermissions(const QUuid& utilisateurId, const QList<QString>& permissions);

    // Lister les permissions d'un utilisateur
    QList<Permission> obtenirPermissionsUtilisateur(const QUuid& utilisateurId);

    // Vérifier des actions spécifiques
    bool peutVisualiserStock(const QUuid& utilisateurId);
    bool peutModifierStock(const QUuid& utilisateurId);
    bool peutApprouverStock(const QUuid& utilisateurId);
    bool peutCreerVente(const QUuid& utilisateurId);
    bool peutModifierVente(const QUuid& utilisateurId);
    bool peutValiderCaisse(const QUuid& utilisateurId);
    bool peutGererCredits(const QUuid& utilisateurId);
    bool peutAccederRapports(const QUuid& utilisateurId);
    bool estAdministrateur(const QUuid& utilisateurId);

    QString obtenirDernierErreur() const { return m_dernierErreur; }

private:
    RepositoryUtilisateur* m_repo;
    QString m_dernierErreur;
};

#endif // SERVICEPERMISSIONS_H