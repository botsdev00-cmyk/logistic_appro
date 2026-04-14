#include <stdio.h>
#include <string.h>
#include "bcrypt.h"

int main() {
    const char *password = "admin123";
    char salt[64];
    char hash[128];

    // Génération du salt
    if (bcrypt_gensalt(12, salt, sizeof(salt)) != 0) {
        fprintf(stderr, "Erreur lors de la génération du salt.\n");
        return 1;
    }

    // Hachage du mot de passe
    if (bcrypt_hashpw(password, salt, hash, sizeof(hash)) != 0) {
        fprintf(stderr, "Erreur lors du hachage du mot de passe.\n");
        return 1;
    }

    printf("Mot de passe   : %s\n", password);
    printf("Salt généré    : %s\n", salt);
    printf("Hash (à mettre en BDD): %s\n", hash);

    // Vérification (simule le check fait à la connexion)
    int result = bcrypt_checkpw(password, hash);
    printf("Vérification (%s, hash): %s\n", password, (result == 0) ? "OK (valide)" : "ECHEC");

    // Exemple de mauvais mot de passe
    result = bcrypt_checkpw("mauvaismotdepasse", hash);
    printf("Vérification (mauvaismotdepasse, hash): %s\n", (result == 0) ? "OK (valide)" : "ECHEC");

    return 0;
}