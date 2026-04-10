#include "ServiceAuthentification.h"
#include "../exceptions/ValidationException.h"
#include "../../data/repositories/RepositoryUtilisateur.h"
#include "../../core/entities/Utilisateur.h"
#include <sodium.h>
#include <QDebug>
#include <memory>
#include <cstring>

ServiceAuthentification::ServiceAuthentification()
    : m_currentUserId(QUuid())
{
    // Initialiser libsodium (thread-safe)
    static bool sodium_initialized = false;
    if (!sodium_initialized) {
        if (sodium_init() < 0) {
            qWarning() << "[ERREUR] Impossible d'initialiser libsodium";
        } else {
            qDebug() << "[OK] libsodium initialisé avec succès";
            sodium_initialized = true;
        }
    }
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
    // Convertir QString en std::string pour libsodium
    std::string password = plainPassword.toStdString();
    
    // Buffer pour le hash (crypto_pwhash_STRBYTES = 128 bytes)
    char hashed[crypto_pwhash_STRBYTES];
    memset(hashed, 0, crypto_pwhash_STRBYTES);
    
    // Hachage Argon2i avec libsodium
    // Paramètres : MODERATE pour équilibrer sécurité et performance
    // crypto_pwhash utilise crypto_pwhash_str_verify en interne
    if (crypto_pwhash_str(
        hashed,
        password.c_str(),
        password.length(),
        crypto_pwhash_OPSLIMIT_MODERATE,
        crypto_pwhash_MEMLIMIT_MODERATE) != 0) {
        
        qWarning() << "[ERREUR] Échec du hachage du mot de passe";
        return QString();
    }
    
    // Convertir le hash en QString
    return QString::fromLatin1(hashed);
}

bool ServiceAuthentification::verifyPassword(const QString& plainPassword, const QString& hash)
{
    if (plainPassword.isEmpty() || hash.isEmpty()) {
        return false;
    }
    
    std::string password = plainPassword.toStdString();
    std::string hashedPassword = hash.toStdString();
    
    // Vérification du mot de passe avec Argon2
    // crypto_pwhash_str_verify est thread-safe et protégé contre les timing attacks
    int result = crypto_pwhash_str_verify(
        hashedPassword.c_str(),
        password.c_str(),
        password.length());
    
    return result == 0;  // 0 = correspondance, -1 = ne correspond pas
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