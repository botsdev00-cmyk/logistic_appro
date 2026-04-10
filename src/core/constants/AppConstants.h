#ifndef APP_CONSTANTS_H
#define APP_CONSTANTS_H

#include <QString>

namespace AppConstants {

// ============================================================================
// Configuration Base de Données
// ============================================================================

constexpr const char* DB_HOST = "localhost";
constexpr const char* DB_PORT = "5432";
constexpr const char* DB_NAME = "semuliki_erp";
constexpr const char* DB_USER = "semuliki_user";

// ============================================================================
// Configuration Application
// ============================================================================

constexpr const char* APP_NAME = "SEMULIKI ERP";
constexpr const char* APP_VERSION = "1.0.0";
constexpr const char* APP_ORGANIZATION = "SEMULIKI";

constexpr int MIN_PASSWORD_LENGTH = 8;
constexpr int SESSION_TIMEOUT_MINUTES = 30;

// ============================================================================
// Constantes Textuelles - Rôles
// ============================================================================

constexpr const char* LABEL_ROLE_ADMIN = "Administrateur";
constexpr const char* LABEL_ROLE_LOGISTIQUE = "Logistique";
constexpr const char* LABEL_ROLE_CAISSIER = "Caissier";
constexpr const char* LABEL_ROLE_GESTIONNAIRE = "Gestionnaire";

// ============================================================================
// Constantes Textuelles - Statuts
// ============================================================================

constexpr const char* LABEL_STATUT_EN_ATTENTE = "En attente";
constexpr const char* LABEL_STATUT_EN_COURS = "En cours";
constexpr const char* LABEL_STATUT_COMPLETEE = "Complétée";
constexpr const char* LABEL_STATUT_ANNULEE = "Annulée";
constexpr const char* LABEL_STATUT_RECU = "Reçu";
constexpr const char* LABEL_STATUT_VALIDE = "Validé";
constexpr const char* LABEL_STATUT_DISCREPANCE = "Discordance";

// ============================================================================
// Constantes Textuelles - Types de Vente
// ============================================================================

constexpr const char* LABEL_TYPE_CASH = "Espèces";
constexpr const char* LABEL_TYPE_CREDIT = "Crédit";

// ============================================================================
// Constantes Textuelles - Crédits
// ============================================================================

constexpr const char* LABEL_STATUT_CREDIT_EN_ATTENTE = "En attente";
constexpr const char* LABEL_STATUT_CREDIT_PAYE = "Payé";
constexpr const char* LABEL_STATUT_CREDIT_EN_RETARD = "En retard";
constexpr const char* LABEL_STATUT_CREDIT_ANNULE = "Annulé";

// ============================================================================
// Constantes Textuelles - Conditions de Paiement
// ============================================================================

constexpr const char* LABEL_CONDITIONS_CASH = "Espèces";
constexpr const char* LABEL_CONDITIONS_CREDIT_5JOURS = "Crédit 5 jours";
constexpr const char* LABEL_CONDITIONS_CREDIT_7JOURS = "Crédit 7 jours";

// ============================================================================
// Constantes de Format
// ============================================================================

constexpr const char* DATE_FORMAT = "dd/MM/yyyy";
constexpr const char* DATETIME_FORMAT = "dd/MM/yyyy hh:mm:ss";
constexpr const char* TIME_FORMAT = "hh:mm:ss";
constexpr const char* CURRENCY_FORMAT = "%1 FCFA";  // Francs CFA

// ============================================================================
// Constantes de Validation
// ============================================================================

constexpr int MAX_USERNAME_LENGTH = 50;
constexpr int MAX_EMAIL_LENGTH = 100;
constexpr int MAX_NAME_LENGTH = 100;

} // namespace AppConstants

#endif // APP_CONSTANTS_H