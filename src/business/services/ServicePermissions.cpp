#include "ServicePermissions.h"
#include "../../data/repositories/RepositoryUtilisateur.h"
#include <QDebug>

ServicePermissions::ServicePermissions()
    : m_repo(nullptr)
{
}

ServicePermissions::~ServicePermissions()
{
}

void ServicePermissions::setRepositoryUtilisateur(RepositoryUtilisateur* repo)
{
    m_repo = repo;
}

bool ServicePermissions::verifierPermission(const QUuid& utilisateurId, const QString& codePermission)
{
    if (!m_repo) {
        m_dernierErreur = "Repository utilisateur non initialisé";
        return false;
    }

    // TODO: Implémenter vérification via base de données
    // Requête: SELECT * FROM role_permissions rp
    //          JOIN permissions p ON rp.permission_id = p.permission_id
    //          WHERE rp.role_id = (SELECT role_id FROM utilisateurs WHERE utilisateur_id = ?)
    //          AND p.code = ?

    qDebug() << "[SERVICE PERMISSIONS] Vérification:" << codePermission << "pour utilisateur" << utilisateurId;
    return true; // Placeholder
}

bool ServicePermissions::verifierPermissions(const QUuid& utilisateurId, const QList<QString>& permissions)
{
    for (const auto& permission : permissions) {
        if (!verifierPermission(utilisateurId, permission)) {
            return false;
        }
    }
    return true;
}

QList<Permission> ServicePermissions::obtenirPermissionsUtilisateur(const QUuid& utilisateurId)
{
    QList<Permission> permissions;

    // TODO: Implémenter récupération depuis base de données
    // Requête: SELECT p.* FROM permissions p
    //          JOIN role_permissions rp ON p.permission_id = rp.permission_id
    //          WHERE rp.role_id = (SELECT role_id FROM utilisateurs WHERE utilisateur_id = ?)

    qDebug() << "[SERVICE PERMISSIONS] Permissions de l'utilisateur" << utilisateurId << "=" << permissions.count();
    return permissions;
}

bool ServicePermissions::peutVisualiserStock(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "STOCK_VIEW");
}

bool ServicePermissions::peutModifierStock(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "STOCK_EDIT");
}

bool ServicePermissions::peutApprouverStock(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "STOCK_APPROVE");
}

bool ServicePermissions::peutCreerVente(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "VENTE_CREATE");
}

bool ServicePermissions::peutModifierVente(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "VENTE_EDIT");
}

bool ServicePermissions::peutValiderCaisse(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "CAISSE_VALIDER");
}

bool ServicePermissions::peutGererCredits(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "CREDIT_MANAGE");
}

bool ServicePermissions::peutAccederRapports(const QUuid& utilisateurId)
{
    return verifierPermission(utilisateurId, "RAPPORTS_VIEW");
}

bool ServicePermissions::estAdministrateur(const QUuid& utilisateurId)
{
    // TODO: Vérifier si l'utilisateur a le rôle ADMIN
    return false;
}