// globals.cpp
#include "globals.h"

// Définitions (initialisation à nullptr)
GestionnaireRepartition* g_repartitionMgr = nullptr;
GestionnaireSales* g_venteMgr = nullptr;
GestionnaireCredit* g_creditMgr = nullptr;
GestionnaireStock* g_stockMgr = nullptr;
QUuid g_utilisateurId;
ArticleRepartition* art = nullptr;