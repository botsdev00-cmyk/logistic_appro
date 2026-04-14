#include "ServiceAuthentification.h"
#include "../exceptions/ValidationException.h"
#include "../../data/repositories/RepositoryUtilisateur.h"
#include "../../core/entities/Utilisateur.h"
#include <bcrypt.h>
#include <QDebug>
#include <memory>
#include <cstring>

ServiceAuthentification::ServiceAuthentification()
    : m_currentUserId(QUuid())
{
    // bcrypt n'a pas besoin d'initialisation globale
    qDebug() << "[OK] ServiceAuthentification initialisé avec bcrypt";
}

bool ServiceAuthentification::authenticate(const QString& username, const QString& password)
{
    try {
        if (username.isEmpty() || password.isEmpty()) {
            m_lastError = "Nom d'utilisateur et mot de passe requis";
            qDebug() << "[AUTH] Erreur: champs vides";
            return false;
        }
        
        RepositoryUtilisateur userRepo;
        Utilisateur user = userRepo.getByUsername(username);
        
        if (user.getUtilisateurId().isNull()) {
            m_lastError = "Utilisateur non trouvé";
            qDebug() << "[AUTH] Erreur: utilisateur" << username << "non trouvé";
            return false;
        }
        
        qDebug() << "[AUTH] Utilisateur trouvé:" << username;
        
        if (!user.estActif()) {
            m_lastError = "Utilisateur inactif";
            qDebug() << "[AUTH] Erreur: utilisateur inactif";
            return false;
        }
        
        qDebug() << "[AUTH] Hash BDD récupéré:" << user.getHashMotPasse().left(50) << "...";
        
        if (!verifyPassword(password, user.getHashMotPasse())) {
            m_lastError = "Mot de passe incorrect";
            qDebug() << "[AUTH] Erreur: mot de passe incorrect";
            return false;
        }
        
        m_currentUserId = user.getUtilisateurId();
        m_currentUser = std::make_unique<Utilisateur>(user);
        m_lastError = "";
        
        qDebug() << "[AUTH] ✓ Authentification réussie pour" << username;
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = QString::fromStdString(e.what());
        qWarning() << "[AUTH] Exception:" << m_lastError;
        return false;
    }
}

bool ServiceAuthentification::logout()
{
    m_currentUserId = QUuid();
    m_currentUser.reset();
    m_lastError = "";
    qDebug() << "[AUTH] Déconnexion réussie";
    return true;
}

QString ServiceAuthentification::hashPassword(const QString& plainPassword)
{
    // Convertir QString en std::string pour bcrypt
    std::string password = plainPassword.toStdString();
    
    // Buffer pour le salt et le hash
    char salt[128];
    char hashed[128];
    
    memset(salt, 0, sizeof(salt));
    memset(hashed, 0, sizeof(hashed));
    
    qDebug() << "[HASH] Génération du salt avec workfactor 12...";
    
    // Générer un salt avec workfactor de 12 (sécurisé et rapide)
    if (bcrypt_gensalt(12, salt, sizeof(salt)) != 0) {
        qWarning() << "[HASH] Erreur: bcrypt_gensalt a échoué";
        return QString();
    }
    
    qDebug() << "[HASH] Salt généré:" << salt;
    qDebug() << "[HASH] Hachage du mot de passe...";
    
    // Hacher le mot de passe avec le salt généré
    if (bcrypt_hashpw(password.c_str(), salt, hashed, sizeof(hashed)) != 0) {
        qWarning() << "[HASH] Erreur: bcrypt_hashpw a échoué";
        return QString();
    }
    
    QString hashResult = QString::fromLatin1(hashed);
    qDebug() << "[HASH] ✓ Hash généré:" << hashResult.left(50) << "...";
    
    return hashResult;
}

bool ServiceAuthentification::verifyPassword(const QString& plainPassword, const QString& hash)
{
    if (plainPassword.isEmpty()) {
        qWarning() << "[VERIFY] Erreur: mot de passe vide";
        return false;
    }
    
    if (hash.isEmpty()) {
        qWarning() << "[VERIFY] Erreur: hash vide en base de données";
        return false;
    }
    
    std::string password = plainPassword.toStdString();
    std::string hashedPassword = hash.toStdString();
    
    qDebug() << "[VERIFY] Vérification du mot de passe...";
    qDebug() << "[VERIFY] Hash en BDD:" << hash.left(50) << "...";
    
    // Vérifier le mot de passe avec bcrypt
    // Retourne 0 si correspondance, -1 sinon
    int result = bcrypt_checkpw(password.c_str(), hashedPassword.c_str());
    
    if (result == 0) {
        qDebug() << "[VERIFY] ✓ Mot de passe valide";
        return true;
    } else {
        qWarning() << "[VERIFY] ✗ Mot de passe invalide (result=" << result << ")";
        return false;
    }
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