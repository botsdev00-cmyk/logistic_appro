/*
 * SEMULIKI ERP - Bcrypt Hash Generator
 * Utilitaire pour générer des hashes bcrypt sécurisés
 * Compile avec: gcc -o hash_generator hash_generator.c vendor/bcrypt/bcrypt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bcrypt.h"

#define MAX_PASSWORD_LEN 128
#define MAX_HASH_LEN 128
#define BCRYPT_WORKFACTOR 12

void print_usage(const char *program_name) {
    printf("Usage: %s <password> [workfactor]\n", program_name);
    printf("\nArguments:\n");
    printf("  <password>    : Le mot de passe à hacher\n");
    printf("  [workfactor]  : Coût de calcul (4-31, défaut: 12)\n");
    printf("\nExemples:\n");
    printf("  %s \"Admin123\"\n", program_name);
    printf("  %s \"MonMotDePasse\" 14\n", program_name);
    printf("\nNote: Workfactor de 12-13 est recommandé pour la sécurité\n");
}

int main(int argc, char *argv[]) {
    char password[MAX_PASSWORD_LEN];
    char salt[MAX_HASH_LEN];
    char hash[MAX_HASH_LEN];
    int workfactor = BCRYPT_WORKFACTOR;
    
    // Vérifier les arguments
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Récupérer le mot de passe
    strncpy(password, argv[1], MAX_PASSWORD_LEN - 1);
    password[MAX_PASSWORD_LEN - 1] = '\0';
    
    // Récupérer le workfactor optionnel
    if (argc >= 3) {
        workfactor = atoi(argv[2]);
        if (workfactor < 4 || workfactor > 31) {
            fprintf(stderr, "Erreur: workfactor doit être entre 4 et 31\n");
            return 1;
        }
    }
    
    // Afficher les paramètres
    printf("════════════════════════════════════════════════════════════\n");
    printf("  SEMULIKI ERP - Bcrypt Hash Generator\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    printf("Paramètres:\n");
    printf("  Mot de passe  : %s\n", password);
    printf("  Workfactor    : %d\n", workfactor);
    printf("  Longueur      : %lu caractères\n\n", strlen(password));
    
    // Vérifier la longueur du mot de passe
    if (strlen(password) < 8) {
        fprintf(stderr, "Erreur: Le mot de passe doit contenir au moins 8 caractères\n");
        return 1;
    }
    
    if (strlen(password) > 72) {
        fprintf(stderr, "Avertissement: Bcrypt tronque les mots de passe > 72 caractères\n");
    }
    
    printf("Génération du salt...\n");
    
    // Étape 1: Générer le salt
    if (bcrypt_gensalt(workfactor, salt, sizeof(salt)) != 0) {
        fprintf(stderr, "Erreur: Impossible de générer le salt\n");
        return 1;
    }
    
    printf("  ✓ Salt généré: %s\n\n", salt);
    
    printf("Hachage du mot de passe (cela peut prendre quelques secondes)...\n");
    
    // Étape 2: Hacher le mot de passe
    if (bcrypt_hashpw(password, salt, hash, sizeof(hash)) != 0) {
        fprintf(stderr, "Erreur: Impossible de hacher le mot de passe\n");
        return 1;
    }
    
    printf("  ✓ Hachage complété\n\n");
    
    // Étape 3: Vérifier que le hachage fonctionne
    printf("Vérification du hash...\n");
    
    if (bcrypt_checkpw(password, hash) == 0) {
        printf("  ✓ Vérification réussie\n\n");
    } else {
        fprintf(stderr, "  ✗ Erreur de vérification\n\n");
        return 1;
    }
    
    // Afficher le résultat final
    printf("════════════════════════════════════════════════════════════\n");
    printf("RÉSULTAT - Hash Bcrypt généré:\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    printf("%s\n\n", hash);
    
    // SQL pour insérer l'utilisateur
    printf("════════════════════════════════════════════════════════════\n");
    printf("Requête SQL pour insérer dans la base de données:\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    printf("-- Insérer un nouvel utilisateur admin\n");
    printf("INSERT INTO utilisateurs (\n");
    printf("    utilisateur_id,\n");
    printf("    nom_utilisateur,\n");
    printf("    email,\n");
    printf("    hash_mot_passe,\n");
    printf("    nom_complet,\n");
    printf("    role_id,\n");
    printf("    est_actif,\n");
    printf("    date_creation,\n");
    printf("    date_mise_a_jour\n");
    printf(") VALUES (\n");
    printf("    gen_random_uuid(),\n");
    printf("    'admin',\n");
    printf("    'admin@semuliki.local',\n");
    printf("    '%s',\n", hash);
    printf("    'Administrator',\n");
    printf("    (SELECT role_id FROM roles WHERE code = 'ADMIN'),\n");
    printf("    true,\n");
    printf("    CURRENT_TIMESTAMP,\n");
    printf("    CURRENT_TIMESTAMP\n");
    printf(") ON CONFLICT (nom_utilisateur) DO UPDATE\n");
    printf("SET hash_mot_passe = EXCLUDED.hash_mot_passe;\n\n");
    
    // Copier facile dans le presse-papiers
    printf("════════════════════════════════════════════════════════════\n");
    printf("Hash seul (à copier-coller):\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("%s\n\n", hash);
    
    printf("════════════════════════════════════════════════════════════\n");
    printf("✓ Génération complétée avec succès\n");
    printf("════════════════════════════════════════════════════════════\n");
    
    return 0;
}