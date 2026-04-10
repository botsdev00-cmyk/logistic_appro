#include "Utilisateur.h"

Utilisateur::Utilisateur()
    : m_utilisateurId(QUuid::createUuid()),
      m_role(Role::Logistique),
      m_estActif(true),
      m_dateCreation(QDateTime::currentDateTime()),
      m_dateMiseAJour(QDateTime::currentDateTime())
{
}

Utilisateur::~Utilisateur()
{
}

QString Utilisateur::roleToString(Role role)
{
    switch (role) {
        case Role::Admin:
            return "admin";
        case Role::Logistique:
            return "logistique";
        case Role::Caissier:
            return "caissier";
        case Role::Gestionnaire:
            return "gestionnaire";
        default:
            return "logistique";
    }
}

Utilisateur::Role Utilisateur::stringToRole(const QString& roleStr)
{
    QString lower = roleStr.toLower();
    if (lower == "admin") return Role::Admin;
    if (lower == "caissier") return Role::Caissier;
    if (lower == "gestionnaire") return Role::Gestionnaire;
    return Role::Logistique;
}

QString Utilisateur::getRoleLabel() const
{
    switch (m_role) {
        case Role::Admin:
            return "Administrateur";
        case Role::Logistique:
            return "Logistique";
        case Role::Caissier:
            return "Caissier";
        case Role::Gestionnaire:
            return "Gestionnaire";
        default:
            return "Inconnu";
    }
}