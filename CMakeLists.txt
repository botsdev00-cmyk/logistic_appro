cmake_minimum_required(VERSION 3.21)
project(logistic_appro
    VERSION 1.0.0
    DESCRIPTION "SEMULIKI ERP - Système de Gestion de Logistique et Répartition"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /W4 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /EHsc")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -Wextra -fPIC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -fPIC")
endif()

find_package(Qt6 COMPONENTS
    Core Gui Widgets Sql PrintSupport Concurrent Pdf Network
    REQUIRED
)
find_package(PostgreSQL REQUIRED)

set(SRC_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DATABASE_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/database)
set(RESOURCES_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(VENDOR_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# ==================== BCRYPT ====================
add_library(bcrypt_vendor STATIC
    ${VENDOR_DIR}/bcrypt/bcrypt.c
)
target_include_directories(bcrypt_vendor PUBLIC
    ${VENDOR_DIR}/bcrypt
)

# ==================== SOURCES ====================
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

set(DATA_DATABASE
    ${SRC_DIR}/data/database/ConnexionBaseDonnees.cpp
    ${SRC_DIR}/data/database/MigrationBaseDonnees.cpp
    ${SRC_DIR}/data/database/PoolConnexions.cpp
)
set(DATA_REPOSITORIES
    ${SRC_DIR}/data/repositories/RepositoryUtilisateur.cpp
    ${SRC_DIR}/data/repositories/RepositoryProduit.cpp
    ${SRC_DIR}/data/repositories/RepositoryTypeProduit.cpp
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
    ${SRC_DIR}/business/managers/GestionnaireCatalogue.cpp
    ${SRC_DIR}/business/managers/GestionnaireStock.cpp
    ${SRC_DIR}/business/managers/GestionnaireRepartition.cpp
    ${SRC_DIR}/business/managers/GestionnaireSales.cpp
    ${SRC_DIR}/business/managers/GestionnaireCredit.cpp
    ${SRC_DIR}/business/managers/GestionnaireCaisse.cpp
    ${SRC_DIR}/business/managers/GestionnaireClient.cpp
    ${SRC_DIR}/business/managers/GestionnaireRapport.cpp
)
set(BUSINESS_SOURCES ${BUSINESS_EXCEPTIONS} ${BUSINESS_SERVICES} ${BUSINESS_MANAGERS})

set(UI_MAIN_WINDOW
    ${SRC_DIR}/ui/main_window/FenetreMain.cpp
)
set(UI_DIALOGS
    ${SRC_DIR}/ui/dialogs/BoiteDialogConnexion.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogRepartition.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogVente.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogReceptionCaisse.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogAlerte.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogCategorie.cpp
    ${SRC_DIR}/ui/dialogs/BoiteDialogProduit.cpp
)
set(UI_WIDGETS
    ${SRC_DIR}/ui/widgets/TableauStock.cpp
    ${SRC_DIR}/ui/widgets/TableauClient.cpp
    ${SRC_DIR}/ui/widgets/TableauRepartition.cpp
    ${SRC_DIR}/ui/widgets/TableauVentes.cpp
    ${SRC_DIR}/ui/widgets/TableauCredit.cpp
    ${SRC_DIR}/ui/widgets/TableauCatalogue.cpp
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

set(RESOURCES
    ${RESOURCES_DIR}/ressources.qrc
)

set(MAIN_SOURCE
    ${SRC_DIR}/main.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${BUSINESS_SOURCES}
    ${UI_SOURCES}
    ${UTIL_SOURCES}
    ${RESOURCES}
    ${MAIN_SOURCE}
)

# ==================== EXECUTABLE ====================
add_executable(${PROJECT_NAME}
    ${ALL_SOURCES}
)

# ==================== INCLUDE DIRS ====================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    ${SRC_DIR}
    ${PostgreSQL_INCLUDE_DIRS}
)

# ==================== LINK LIBRARIES ====================
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
    bcrypt_vendor
)

# ----- Platform specific: pthread, crypt, etc -----
if(UNIX AND NOT APPLE)
    # Linux, BSD: crypt et pthread souvent requis
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread crypt)
elseif(APPLE)
    # macOS : pthread oui, crypt non (crypt intégrée dans libc)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)
elseif(WIN32)
    # Windows – pas de libcrypt, mais librairies Win32 pour réseau/crypto
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32)
endif()

# ==================== CUSTOM COMMANDS + INSTALL ====================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/structure.sql    
    ${CMAKE_BINARY_DIR}/structure.sql
    COMMENT "Copying database schema"
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(FILES ${DATABASE_DIR}/structure.sql
    DESTINATION share/logistic_appro
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /O2)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
endif()

message(STATUS "")
message(STATUS "╔══════════════════════════════════════════════════════════╗")
message(STATUS "║ SEMULIKI ERP Configuration Summary                      ║")
message(STATUS "╠══════════════════════════════════════════════════════════╣")
message(STATUS "║ Project Name: ${PROJECT_NAME}")
message(STATUS "║ Version: ${PROJECT_VERSION}")
message(STATUS "║ C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "║ Qt Version: ${Qt6_VERSION}")
message(STATUS "║ PostgreSQL: FOUND")
message(STATUS "║ bcrypt: custom vendor implementation")
message(STATUS "║ Build Type: ${CMAKE_BUILD_TYPE}")
if(WIN32)
    message(STATUS "║ Platform: Windows")
elseif(APPLE)
    message(STATUS "║ Platform: macOS")
else()
    message(STATUS "║ Platform: Linux/BSD")
endif()
message(STATUS "╚══════════════════════════════════════════════════════════╝")
message(STATUS "")