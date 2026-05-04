// globals.h
#pragma once

#include <QUuid>
#include "../../business/managers/GestionnaireRepartition.h"
#include "../../business/managers/GestionnaireSales.h"
#include "../../business/managers/GestionnaireCredit.h"
#include "../../business/managers/GestionnaireStock.h"
#include "../../core/entities/ArticleRepartition.h"

// Déclarations externes
extern GestionnaireRepartition* g_repartitionMgr;
extern GestionnaireSales* g_venteMgr;
extern GestionnaireCredit* g_creditMgr;
extern GestionnaireStock* g_stockMgr;
extern QUuid g_utilisateurId;
extern ArticleRepartition* art;