#include "ServiceAuthentification.h"
#include "../exceptions/ValidationException.h"
#include "../../data/repositories/RepositoryUtilisateur.h"
#include "../../core/entities/Utilisateur.h"
#include <QCryptographicHash>
#include <QDebug>
#include <memory>

ServiceAuthentification::ServiceAuthentification()
    : m_currentUserId(QUuid())
{
}

bool ServiceAuthentification::authenticate(const QString& username, const QString& password)
{
    try {
        if (username.isEmpty() || password.isEmpty()) {
            m_lastError = "Nom d'utilisateur et mot de passe requis";
            return false;
        }
        
        RepositoryUtilisateur userRepo;
        Utilisateur user = userRepo.getByUsername(username);
        
        if (user.getUtilisateurId().isNull()) {
            m_lastError = "Utilisateur non trouvé";
            return false;
        }
        
        if (!user.estActif()) {
            m_lastError = "Utilisateur inactif";
            return false;
        }
        
        if (!verifyPassword(password, user.getHashMotPasse())) {
            m_lastError = "Mot de passe incorrect";
            return false;
        }
        
        m_currentUserId = user.getUtilisateurId();
        m_currentUser = std::make_unique<Utilisateur>(user);
        m_lastError = "";
        
        qDebug() << "Authentification réussie pour" << username;
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = QString::fromStdString(e.what());
        return false;
    }
}

bool ServiceAuthentification::logout()
{
    m_currentUserId = QUuid();
    m_currentUser.reset();
    m_lastError = "";
    return true;
}

QString ServiceAuthentification::hashPassword(const QString& plainPassword)
{
    QByteArray hash = QCryptographicHash::hash(
        plainPassword.toUtf8(),
        QCryptographicHash::Sha256
    );
    return QString(hash.toHex());
}

bool ServiceAuthentification::verifyPassword(const QString& plainPassword, const QString& hash)
{
    QString computedHash = hashPassword(plainPassword);
    return computedHash == hash;
}

bool ServiceAuthentification::hasRole(const QString& requiredRole) const
{
    if (!m_currentUser) {
        return false;
    }
    
    return Utilisateur::roleToString(m_currentUser->getRole()) == requiredRole.toLower();
}

bool ServiceAuthentification::hasPermission(const QString& permission) const
{
    if (!isAuthenticated()) {
        return false;
    }
    
    // Mapping des permissions par rôle
    QString role = Utilisateur::roleToString(m_currentUser->getRole());
    
    if (role == "admin") {
        return true; // Les admins ont tous les droits
    }
    
    if (role == "logistique") {
        return permission.contains("dispatch") || 
               permission.contains("stock") || 
               permission.contains("sales");
    }
    
    if (role == "caissier") {
        return permission.contains("cash") || 
               permission.contains("credit");
    }
    
    if (role == "gestionnaire") {
        return permission.contains("report") || 
               permission.contains("analytics");
    }
    
    return false;
}