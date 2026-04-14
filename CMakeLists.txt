# ============================================================================
# SEMULIKI ERP - Configuration CMake (MODE OFFLINE)
# ============================================================================

cmake_minimum_required(VERSION 3.21)
project(logistic_appro
    VERSION 1.0.0
    DESCRIPTION "SEMULIKI ERP - Système de Gestion de Logistique et Répartition"
    LANGUAGES CXX C
)

# ============================================================================
# CONFIGURATION CMAKE
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================================
# BUILD FLAGS
# ============================================================================

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /W4 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /EHsc")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -Wextra -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -fPIC")
endif()

# ============================================================================
# FIND PACKAGES
# ============================================================================

find_package(Qt6 COMPONENTS
    Core Gui Widgets Sql PrintSupport Concurrent Pdf Network
    REQUIRED
)

find_package(PostgreSQL REQUIRED)

# ============================================================================
# PROJECT DIRECTORIES
# ============================================================================

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DATABASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/database)
set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

# ============================================================================
# LIBSODIUM - LOCAL BUILD (OFFLINE)
# ============================================================================

include(ExternalProject)

# Chemin vers les sources déjà dézippées (à ajuster si le nom du dossier diffère)
set(LIBSODIUM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libsodium-1.0.18")

set(LIBSODIUM_BUILD_DIR ${CMAKE_BINARY_DIR}/libsodium-build)
set(LIBSODIUM_INSTALL_DIR ${CMAKE_BINARY_DIR}/libsodium-install)

if(WIN32)
    set(LIBSODIUM_LIB_NAME libsodium.lib)
    set(LIBSODIUM_LIB_PATH ${LIBSODIUM_INSTALL_DIR}/lib/${LIBSODIUM_LIB_NAME})
else()
    set(LIBSODIUM_LIB_NAME libsodium.a)
    set(LIBSODIUM_LIB_PATH ${LIBSODIUM_INSTALL_DIR}/lib/${LIBSODIUM_LIB_NAME})
endif()

ExternalProject_Add(libsodium_external
    SOURCE_DIR ${LIBSODIUM_SRC_DIR}
    BINARY_DIR ${LIBSODIUM_BUILD_DIR}/build
    INSTALL_DIR ${LIBSODIUM_INSTALL_DIR}
    # Désactivation du téléchargement puisque les sources sont locales
    DOWNLOAD_COMMAND "" 
    CONFIGURE_COMMAND ${LIBSODIUM_SRC_DIR}/configure 
        --prefix=${LIBSODIUM_INSTALL_DIR}
        --enable-static
        --disable-shared
        --with-pic
    BUILD_COMMAND make -j4
    INSTALL_COMMAND make install
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    # Correction pour Ninja
    BUILD_BYPRODUCTS ${LIBSODIUM_LIB_PATH}
)

set(LIBSODIUM_INCLUDE_DIR ${LIBSODIUM_INSTALL_DIR}/include)
set(LIBSODIUM_LIBRARY ${LIBSODIUM_LIB_PATH})

# Création préventive du dossier d'inclusion
file(MAKE_DIRECTORY ${LIBSODIUM_INCLUDE_DIR})

# ============================================================================
# SOURCE FILES
# ============================================================================

# Core Entities
set(CORE_SOURCES
    ${SRC_DIR}/core/entities/Utilisateur.cpp
    ${SRC_DIR}/core/entities/Produit.cpp
    ${SRC_DIR}/core/entities/CategorieProduit.cpp
    ${SRC_DIR}/core/entities/Equipe.cpp
    ${SRC_DIR}/core/entities/Route.cpp
    ${SRC_DIR}/core/entities/Client.cpp
    ${SRC_DIR}/core/entities/Repartition.cpp
    ${SRC_DIR}/core/entities/ArticleRepartition.cpp
    ${SRC_DIR}/core/entities/Vente.cpp
    ${SRC_DIR}/core/entities/Credit.cpp
    ${SRC_DIR}/core/entities/ReceptionCaisse.cpp
    ${SRC_DIR}/core/entities/EntreeStock.cpp
    ${SRC_DIR}/core/constants/AppConstants.cpp
)

# Data Layer
set(DATA_SOURCES
    ${SRC_DIR}/data/database/ConnexionBaseDonnees.cpp
    ${SRC_DIR}/data/database/MigrationBaseDonnees.cpp
    ${SRC_DIR}/data/database/PoolConnexions.cpp
    ${SRC_DIR}/data/repositories/RepositoryUtilisateur.cpp
    ${SRC_DIR}/data/repositories/RepositoryProduit.cpp
    ${SRC_DIR}/data/repositories/RepositoryCategorieProduit.cpp
    ${SRC_DIR}/data/repositories/RepositoryEquipe.cpp
    ${SRC_DIR}/data/repositories/RepositoryRoute.cpp
    ${SRC_DIR}/data/repositories/RepositoryClient.cpp
    ${SRC_DIR}/data/repositories/RepositoryRepartition.cpp
    ${SRC_DIR}/data/repositories/RepositorySales.cpp
    ${SRC_DIR}/data/repositories/RepositoryCredit.cpp
    ${SRC_DIR}/data/repositories/RepositoryReceptionCaisse.cpp
    ${SRC_DIR}/data/repositories/RepositoryStock.cpp
)

# Business Layer
set(BUSINESS_SOURCES
    ${SRC_DIR}/business/exceptions/AppException.cpp
    ${SRC_DIR}/business/exceptions/DatabaseException.cpp
    ${SRC_DIR}/business/exceptions/ValidationException.cpp
    ${SRC_DIR}/business/exceptions/BusinessException.cpp
    ${SRC_DIR}/business/services/ServiceValidation.cpp
    ${SRC_DIR}/business/services/ServiceAuthentification.cpp
    ${SRC_DIR}/business/managers/GestionnaireStock.cpp
    ${SRC_DIR}/business/managers/GestionnaireRepartition.cpp
    ${SRC_DIR}/business/managers/GestionnaireSales.cpp
    ${SRC_DIR}/business/managers/GestionnaireCredit.cpp
    ${SRC_DIR}/business/managers/GestionnaireCaisse.cpp
    ${SRC_DIR}/business/managers/GestionnaireClient.cpp
    ${SRC_DIR}/business/managers/GestionnaireRapport.cpp
)

# UI Layer
set(UI_SOURCES
    ${SRC_DIR}/ui/main_window/FenetreMain.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogConnexion.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogRepartition.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogVente.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogReceptionCaisse.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogAlerte.cpp
    ${SRC_DIR}/ui/widgets/TableauStock.cpp
    ${SRC_DIR}/ui/widgets/TableauClient.cpp
    ${SRC_DIR}/ui/widgets/TableauRepartition.cpp
    ${SRC_DIR}/ui/widgets/TableauVentes.cpp
    ${SRC_DIR}/ui/widgets/TableauCredit.cpp
    ${SRC_DIR}/ui/views/VueTableau.cpp
    ${SRC_DIR}/ui/views/VueStock.cpp
    ${SRC_DIR}/ui/views/VueRepartition.cpp
    ${SRC_DIR}/ui/views/VueVentes.cpp
    ${SRC_DIR}/ui/views/VueCredit.cpp
    ${SRC_DIR}/ui/views/VueCaisse.cpp
    ${SRC_DIR}/ui/views/VueRapport.cpp
    ${SRC_DIR}/ui/views/VueClient.cpp
)

# Utils & Resources
set(UTIL_SOURCES
    ${SRC_DIR}/utils/logger/Enregistreur.cpp
    ${SRC_DIR}/utils/validators/ValidateurEntrees.cpp
    ${SRC_DIR}/utils/helpers/AideDateTime.cpp
    ${SRC_DIR}/utils/helpers/GenerateurRapport.cpp
    ${SRC_DIR}/utils/helpers/GenerateurPDF.cpp
)

set(RESOURCES ${RESOURCES_DIR}/ressources.qrc)
set(MAIN_SOURCE ${SRC_DIR}/main.cpp)

set(ALL_SOURCES ${CORE_SOURCES} ${DATA_SOURCES} ${BUSINESS_SOURCES} ${UI_SOURCES} ${UTIL_SOURCES} ${RESOURCES} ${MAIN_SOURCE})

# ============================================================================
# TARGET CONFIGURATION
# ============================================================================

add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# Dépendance explicite vers libsodium
add_dependencies(${PROJECT_NAME} libsodium_external)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${SRC_DIR}
    ${PostgreSQL_INCLUDE_DIRS}
    ${LIBSODIUM_INCLUDE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE SODIUM_STATIC=1)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::PrintSupport Qt6::Concurrent Qt6::Pdf Qt6::Network
    ${PostgreSQL_LIBRARIES}
    ${LIBSODIUM_LIBRARY}
    pthread
)

# ============================================================================
# POST-BUILD & INSTALL
# ============================================================================

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/structure.sql    
    ${CMAKE_BINARY_DIR}/structure.sql
    COMMENT "Copying database schema"
)

message(STATUS "✓ Configuration complete.")
message(STATUS "✓ Offline libsodium source: ${LIBSODIUM_SRC_DIR}")