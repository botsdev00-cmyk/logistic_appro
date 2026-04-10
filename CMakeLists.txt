# ============================================================================
# SEMULIKI ERP - Configuration CMake avec libsodium auto-téléchargé
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
# LIBSODIUM - AUTO-DOWNLOAD & BUILD
# ============================================================================

include(ExternalProject)

set(LIBSODIUM_VERSION "1.0.18")
set(LIBSODIUM_BUILD_DIR ${CMAKE_BINARY_DIR}/libsodium-build)
set(LIBSODIUM_INSTALL_DIR ${CMAKE_BINARY_DIR}/libsodium-install)
set(LIBSODIUM_SRC_DIR ${LIBSODIUM_BUILD_DIR}/src/libsodium_external)

if(WIN32)
    set(LIBSODIUM_LIB_NAME libsodium.lib)
    set(LIBSODIUM_LIB_PATH ${LIBSODIUM_INSTALL_DIR}/lib/${LIBSODIUM_LIB_NAME})
else()
    set(LIBSODIUM_LIB_NAME libsodium.a)
    set(LIBSODIUM_LIB_PATH ${LIBSODIUM_INSTALL_DIR}/lib/${LIBSODIUM_LIB_NAME})
endif()

ExternalProject_Add(libsodium_external
    URL "https://github.com/jedisct1/libsodium/releases/download/${LIBSODIUM_VERSION}-RELEASE/libsodium-${LIBSODIUM_VERSION}.tar.gz"
    URL_HASH SHA256=06b3b52b8b714f074d44c6647be3814a230ee208b02327145257e2f258e080a6
    SOURCE_DIR ${LIBSODIUM_SRC_DIR}
    BINARY_DIR ${LIBSODIUM_BUILD_DIR}/build
    INSTALL_DIR ${LIBSODIUM_INSTALL_DIR}
    CONFIGURE_COMMAND ${LIBSODIUM_SRC_DIR}/configure 
        --prefix=${LIBSODIUM_INSTALL_DIR}
        --enable-static
        --disable-shared
        --with-pic
    BUILD_COMMAND make -j4
    INSTALL_COMMAND make install
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
)

set(LIBSODIUM_INCLUDE_DIR ${LIBSODIUM_INSTALL_DIR}/include)
set(LIBSODIUM_LIBRARY ${LIBSODIUM_LIB_PATH})

message(STATUS "✓ libsodium will be downloaded and built to: ${LIBSODIUM_INSTALL_DIR}")
message(STATUS "✓ libsodium library: ${LIBSODIUM_LIBRARY}")

# ============================================================================
# SOURCE FILES - CORE ENTITIES
# ============================================================================

set(CORE_ENTITIES
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
)

set(CORE_CONSTANTS
    ${SRC_DIR}/core/constants/AppConstants.cpp
)

set(CORE_SOURCES ${CORE_ENTITIES} ${CORE_CONSTANTS})

# ============================================================================
# SOURCE FILES - DATA LAYER
# ============================================================================

set(DATA_DATABASE
    ${SRC_DIR}/data/database/ConnexionBaseDonnees.cpp
    ${SRC_DIR}/data/database/MigrationBaseDonnees.cpp
    ${SRC_DIR}/data/database/PoolConnexions.cpp
)

set(DATA_REPOSITORIES
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

set(DATA_SOURCES ${DATA_DATABASE} ${DATA_REPOSITORIES})

# ============================================================================
# SOURCE FILES - BUSINESS LAYER
# ============================================================================

set(BUSINESS_EXCEPTIONS
    ${SRC_DIR}/business/exceptions/AppException.cpp
    ${SRC_DIR}/business/exceptions/DatabaseException.cpp
    ${SRC_DIR}/business/exceptions/ValidationException.cpp
    ${SRC_DIR}/business/exceptions/BusinessException.cpp
)

set(BUSINESS_SERVICES
    ${SRC_DIR}/business/services/ServiceValidation.cpp
    ${SRC_DIR}/business/services/ServiceAuthentification.cpp
)

set(BUSINESS_MANAGERS
    ${SRC_DIR}/business/managers/GestionnaireStock.cpp
    ${SRC_DIR}/business/managers/GestionnaireRepartition.cpp
    ${SRC_DIR}/business/managers/GestionnaireSales.cpp
    ${SRC_DIR}/business/managers/GestionnaireCredit.cpp
    ${SRC_DIR}/business/managers/GestionnaireCaisse.cpp
    ${SRC_DIR}/business/managers/GestionnaireClient.cpp
    ${SRC_DIR}/business/managers/GestionnaireRapport.cpp
)

set(BUSINESS_SOURCES ${BUSINESS_EXCEPTIONS} ${BUSINESS_SERVICES} ${BUSINESS_MANAGERS})

# ============================================================================
# SOURCE FILES - UI LAYER
# ============================================================================

set(UI_MAIN_WINDOW
    ${SRC_DIR}/ui/main_window/FenetreMain.cpp
)

set(UI_DIALOGS
    ${SRC_DIR}/ui/dialogs/BoiteDialogConnexion.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogRepartition.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogVente.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogReceptionCaisse.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogAlerte.cpp
)

set(UI_WIDGETS
    ${SRC_DIR}/ui/widgets/TableauStock.cpp
    ${SRC_DIR}/ui/widgets/TableauClient.cpp
    ${SRC_DIR}/ui/widgets/TableauRepartition.cpp
    ${SRC_DIR}/ui/widgets/TableauVentes.cpp
    ${SRC_DIR}/ui/widgets/TableauCredit.cpp
)

set(UI_VIEWS
    ${SRC_DIR}/ui/views/VueTableau.cpp
    ${SRC_DIR}/ui/views/VueStock.cpp
    ${SRC_DIR}/ui/views/VueRepartition.cpp
    ${SRC_DIR}/ui/views/VueVentes.cpp
    ${SRC_DIR}/ui/views/VueCredit.cpp
    ${SRC_DIR}/ui/views/VueCaisse.cpp
    ${SRC_DIR}/ui/views/VueRapport.cpp
    ${SRC_DIR}/ui/views/VueClient.cpp
)

set(UI_SOURCES ${UI_MAIN_WINDOW} ${UI_DIALOGS} ${UI_WIDGETS} ${UI_VIEWS})

# ============================================================================
# SOURCE FILES - UTILS
# ============================================================================

set(UTIL_LOGGER
    ${SRC_DIR}/utils/logger/Enregistreur.cpp
)

set(UTIL_VALIDATORS
    ${SRC_DIR}/utils/validators/ValidateurEntrees.cpp
)

set(UTIL_HELPERS
    ${SRC_DIR}/utils/helpers/AideDateTime.cpp
    ${SRC_DIR}/utils/helpers/GenerateurRapport.cpp
    ${SRC_DIR}/utils/helpers/GenerateurPDF.cpp
)

set(UTIL_SOURCES ${UTIL_LOGGER} ${UTIL_VALIDATORS} ${UTIL_HELPERS})

# ============================================================================
# RESOURCES
# ============================================================================

set(RESOURCES
    ${RESOURCES_DIR}/ressources.qrc
)

# ============================================================================
# MAIN SOURCE FILE
# ============================================================================

set(MAIN_SOURCE
    ${SRC_DIR}/main.cpp
)

# ============================================================================
# COMBINE ALL SOURCES
# ============================================================================

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${BUSINESS_SOURCES}
    ${UI_SOURCES}
    ${UTIL_SOURCES}
    ${RESOURCES}
    ${MAIN_SOURCE}
)

# ============================================================================
# CREATE EXECUTABLE
# ============================================================================

add_executable(${PROJECT_NAME}
    ${ALL_SOURCES}
)

# Dépendance: attendre que libsodium soit compilé avant de compiler l'app
add_dependencies(${PROJECT_NAME} libsodium_external)

# ============================================================================
# INCLUDE DIRECTORIES
# ============================================================================

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${SRC_DIR}
    ${PostgreSQL_INCLUDE_DIRS}
    ${LIBSODIUM_INCLUDE_DIR}
)

# ============================================================================
# COMPILE DEFINITIONS
# ============================================================================

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SODIUM_STATIC=1
)

# ============================================================================
# COMPILER FLAGS
# ============================================================================

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-unused-function
        -Wno-conversion
        -Wno-sign-compare
        -Wno-unused-parameter
    )
endif()

# ============================================================================
# LINK LIBRARIES
# ============================================================================

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Concurrent
    Qt6::Pdf
    Qt6::Network
    ${PostgreSQL_LIBRARIES}
    ${LIBSODIUM_LIBRARY}
)

# Windows specific
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32
        crypt32
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pthread
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pthread
    )
endif()

# ============================================================================
# COPY DATABASE SCHEMA & CONFIG FILES
# ============================================================================

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/structure.sql    
    ${CMAKE_BINARY_DIR}/structure.sql
    COMMENT "Copying database schema"
)

# ============================================================================
# INSTALLATION
# ============================================================================

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES ${DATABASE_DIR}/structure.sql
    DESTINATION share/logistic_appro
)

# ============================================================================
# COMPILER OPTIONS (PRODUCTION)
# ============================================================================

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

# ============================================================================
# SUMMARY MESSAGE
# ============================================================================

message(STATUS "")
message(STATUS "╔════════════════════════════════════════════════════════════╗")
message(STATUS "║ SEMULIKI ERP Configuration Summary                          ║")
message(STATUS "╠════════════════════════════════════════════════════════════╣")
message(STATUS "║ Project Name: ${PROJECT_NAME}")
message(STATUS "║ Version: ${PROJECT_VERSION}")
message(STATUS "║ C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "║ Qt Version: ${Qt6_VERSION}")
message(STATUS "║ PostgreSQL: FOUND")
message(STATUS "║ libsodium: Auto-Downloaded v${LIBSODIUM_VERSION}")
message(STATUS "║ libsodium Build Dir: ${LIBSODIUM_BUILD_DIR}")
message(STATUS "║ libsodium Install: ${LIBSODIUM_INSTALL_DIR}")
message(STATUS "║ Password Hashing: Argon2")
message(STATUS "║ Build Type: ${CMAKE_BUILD_TYPE}")
if(WIN32)
    message(STATUS "║ Platform: Windows")
elseif(APPLE)
    message(STATUS "║ Platform: macOS")
else()
    message(STATUS "║ Platform: Linux")
endif()
message(STATUS "╚════════════════════════════════════════════════════════════╝")
message(STATUS "")