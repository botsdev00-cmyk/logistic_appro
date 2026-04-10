#ifndef VALIDATEURENTREES_H
#define VALIDATEURENTREES_H

#include <QString>
#include <QStringList>
#include <climits>

/**
 * @class ValidateurEntrees
 * @brief Classe utilitaire pour la validation des entrées utilisateur
 * 
 * Fournit des méthodes statiques pour valider les emails, mots de passe,
 * noms d'utilisateur, numéros de téléphone et autres formats courants.
 */
class ValidateurEntrees
{
public:
    /**
     * @brief Valide une adresse email
     * @param email L'adresse email à valider
     * @return true si valide, false sinon
     */
    static bool validerEmail(const QString& email);

    /**
     * @brief Valide un mot de passe
     * 
     * Critères:
     * - Minimum 8 caractères
     * - Au moins une majuscule
     * - Au moins une minuscule
     * - Au moins un chiffre
     * - Au moins un caractère spécial
     * 
     * @param password Le mot de passe à valider
     * @param erreurs Liste pour stocker les messages d'erreur
     * @return true si valide, false sinon
     */
    static bool validerMotDePasse(const QString& password, QStringList& erreurs);

    /**
     * @brief Valide un nom d'utilisateur
     * 
     * Critères:
     * - Minimum 3 caractères
     * - Maximum 50 caractères
     * - Caractères alphanumériques et underscores uniquement
     * 
     * @param username Le nom d'utilisateur à valider
     * @return true si valide, false sinon
     */
    static bool validerNomUtilisateur(const QString& username);

    /**
     * @brief Valide un numéro de téléphone
     * 
     * Accepte les formats:
     * - +243 123 456 789
     * - +243123456789
     * - 0123456789
     * 
     * @param phone Le numéro de téléphone à valider
     * @return true si valide, false sinon
     */
    static bool validerTelephone(const QString& phone);

    /**
     * @brief Valide une adresse URL
     * @param url L'URL à valider
     * @return true si valide, false sinon
     */
    static bool validerURL(const QString& url);

    /**
     * @brief Valide un montant numérique
     * 
     * @param montant Le montant à valider
     * @param min Valeur minimale acceptable
     * @param max Valeur maximale acceptable
     * @return true si valide, false sinon
     */
    static bool validerMontant(double montant, double min = 0.0, double max = 999999.99);

    /**
     * @brief Valide une quantité (nombre entier positif)
     * @param quantite La quantité à valider
     * @param min Valeur minimale (par défaut 1)
     * @param max Valeur maximale (par défaut INT_MAX)
     * @return true si valide, false sinon
     */
    static bool validerQuantite(int quantite, int min = 1, int max = INT_MAX);

    /**
     * @brief Valide une chaîne non vide
     * @param str La chaîne à valider
     * @return true si non vide, false sinon
     */
    static bool validerNonVide(const QString& str);

    /**
     * @brief Valide la longueur d'une chaîne
     * @param str La chaîne à valider
     * @param min Longueur minimale
     * @param max Longueur maximale
     * @return true si la longueur est valide, false sinon
     */
    static bool validerLongueur(const QString& str, int min, int max);

    /**
     * @brief Valide une date au format YYYY-MM-DD
     * @param dateStr La date en chaîne de caractères
     * @return true si valide, false sinon
     */
    static bool validerDate(const QString& dateStr);

    /**
     * @brief Valide un code postal
     * @param codePostal Le code postal à valider
     * @return true si valide, false sinon
     */
    static bool validerCodePostal(const QString& codePostal);

    /**
     * @brief Obtient le dernier message d'erreur
     * @return Le dernier message d'erreur enregistré
     */
    static QString getLastError();

    /**
     * @brief Réinitialise le message d'erreur
     */
    static void clearError();

private:
    /**
     * @brief Constructeur privé (classe utilitaire statique)
     */
    ValidateurEntrees() = default;

    /**
     * @brief Enregistre un message d'erreur
     * @param message Le message à enregistrer
     */
    static void setError(const QString& message);

    // Membre statique pour stocker le dernier message d'erreur
    static QString m_lastError;

    // Constantes de validation
    static constexpr int MIN_USERNAME_LENGTH = 3;
    static constexpr int MAX_USERNAME_LENGTH = 50;
    static constexpr int MIN_PASSWORD_LENGTH = 8;
    static constexpr int MAX_PASSWORD_LENGTH = 255;
    static constexpr int MIN_PHONE_LENGTH = 10;
    static constexpr int MAX_PHONE_LENGTH = 20;
};

#endif // VALIDATEURENTREES_H