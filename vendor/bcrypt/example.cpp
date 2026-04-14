#include <iostream>
#include <bcrypt.h>
#include <cstring>

int main() {
    const char* password = "Admin123";
    char salt[64];
    char hash[64];
    
    // Générer salt
    if (bcrypt_gensalt(12, salt, sizeof(salt)) != 0) {
        std::cerr << "Erreur: bcrypt_gensalt" << std::endl;
        return 1;
    }
    
    // Générer hash
    if (bcrypt_hashpw(password, salt, hash, sizeof(hash)) != 0) {
        std::cerr << "Erreur: bcrypt_hashpw" << std::endl;
        return 1;
    }
    
    std::cout << "Hash pour '" << password << "': " << std::endl;
    std::cout << hash << std::endl;
    
    // Tester la vérification
    if (bcrypt_checkpw(password, hash) == 0) {
        std::cout << "✓ Vérification réussie!" << std::endl;
    } else {
        std::cout << "✗ Vérification échouée!" << std::endl;
    }
    
    return 0;
}