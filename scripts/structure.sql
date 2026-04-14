-- ============================================================================
-- SEMULIKI ERP - Schéma de Base de Données (Version Normalisée)
-- Objectif: Gestion logistique, répartition, ventes, crédits et caisse
-- Base de données: PostgreSQL 12+
-- Langue: Français
-- IDs: UUID
-- ============================================================================

-- Activer l'extension UUID
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ============================================================================
-- 0. PARAMÉTRAGES (TABLES DE RÉFÉRENCE)
-- ============================================================================

CREATE TABLE roles (
    role_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE permissions (
    permission_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(50) UNIQUE NOT NULL,
    nom VARCHAR(100) NOT NULL,
    module VARCHAR(50) NOT NULL
);

CREATE TABLE types_produits (
    type_produit_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE sources_entree (
    source_entree_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE raisons_retour (
    raison_retour_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE conditions_paiement (
    condition_paiement_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL,
    delai_jours INTEGER DEFAULT 0
);

CREATE TABLE statuts_repartition (
    statut_repartition_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE types_vente (
    type_vente_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE statuts_credit (
    statut_credit_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

CREATE TABLE statuts_caisse (
    statut_caisse_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    code VARCHAR(20) UNIQUE NOT NULL,
    nom VARCHAR(50) NOT NULL
);

-- ============================================================================
-- 1. UTILISATEURS & PERMISSIONS
-- ============================================================================

CREATE TABLE utilisateurs (
    utilisateur_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    nom_utilisateur VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    hash_mot_passe VARCHAR(255) NOT NULL,
    nom_complet VARCHAR(100) NOT NULL,
    role_id UUID NOT NULL REFERENCES roles(role_id),
    est_actif BOOLEAN DEFAULT true,
    date_creation TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE role_permissions (
    role_id UUID NOT NULL REFERENCES roles(role_id) ON DELETE CASCADE,
    permission_id UUID NOT NULL REFERENCES permissions(permission_id) ON DELETE CASCADE,
    PRIMARY KEY (role_id, permission_id)
);

-- ============================================================================
-- 2. PRODUITS & STOCK
-- ============================================================================

-- ✅ CORRECTED: Added missing columns
CREATE TABLE categories_produits (
    categorie_produit_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    nom VARCHAR(100) NOT NULL UNIQUE,
    code_categorie VARCHAR(20) UNIQUE,
    description TEXT DEFAULT '',
    est_actif BOOLEAN DEFAULT true,
    ordre_affichage INTEGER DEFAULT 0,
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE produits (
    produit_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    categorie_produit_id UUID NOT NULL REFERENCES categories_produits(categorie_produit_id),
    type_produit_id UUID NOT NULL REFERENCES types_produits(type_produit_id),
    nom VARCHAR(100) NOT NULL,
    code_sku VARCHAR(50) UNIQUE NOT NULL,
    prix_unitaire DECIMAL(12, 2) NOT NULL,
    stock_minimum INTEGER DEFAULT 10,
    est_actif BOOLEAN DEFAULT true,
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE entrees_stock (
    entree_stock_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    produit_id UUID NOT NULL REFERENCES produits(produit_id),
    quantite INTEGER NOT NULL,
    source_entree_id UUID NOT NULL REFERENCES sources_entree(source_entree_id),
    date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    cree_par UUID REFERENCES utilisateurs(utilisateur_id)
);

CREATE TABLE retours_stock (
    retour_stock_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    produit_id UUID NOT NULL REFERENCES produits(produit_id),
    quantite INTEGER NOT NULL,
    raison_retour_id UUID NOT NULL REFERENCES raisons_retour(raison_retour_id),
    date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 3. RÉPARTITION ET VENTES
-- ============================================================================

CREATE TABLE equipes (
    equipe_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    nom VARCHAR(100) NOT NULL,
    nom_chef VARCHAR(100) NOT NULL
);

CREATE TABLE routes (
    route_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    nom VARCHAR(100) NOT NULL
);

CREATE TABLE clients (
    client_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    nom VARCHAR(100) NOT NULL,
    route_id UUID REFERENCES routes(route_id),
    condition_paiement_id UUID REFERENCES conditions_paiement(condition_paiement_id),
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE repartitions (
    repartition_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    equipe_id UUID NOT NULL REFERENCES equipes(equipe_id),
    route_id UUID NOT NULL REFERENCES routes(route_id),
    statut_repartition_id UUID NOT NULL REFERENCES statuts_repartition(statut_repartition_id),
    date_repartition DATE NOT NULL,
    montant_cash_attendu DECIMAL(12, 2) DEFAULT 0,
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE articles_repartition (
    article_repartition_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    repartition_id UUID NOT NULL REFERENCES repartitions(repartition_id) ON DELETE CASCADE,
    produit_id UUID NOT NULL REFERENCES produits(produit_id),
    quantite_vente INTEGER DEFAULT 0,
    quantite_cadeau INTEGER DEFAULT 0,
    quantite_totale INTEGER GENERATED ALWAYS AS (quantite_vente + quantite_cadeau) STORED
);

CREATE TABLE ventes (
    vente_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    repartition_id UUID NOT NULL REFERENCES repartitions(repartition_id),
    produit_id UUID NOT NULL REFERENCES produits(produit_id),
    client_id UUID NOT NULL REFERENCES clients(client_id),
    quantite INTEGER NOT NULL,
    type_vente_id UUID NOT NULL REFERENCES types_vente(type_vente_id),
    prix_unitaire DECIMAL(12, 2) NOT NULL,
    montant_total DECIMAL(12, 2) GENERATED ALWAYS AS (quantite * prix_unitaire) STORED,
    date_vente TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- 4. FINANCE & AUDIT
-- ============================================================================

CREATE TABLE credits (
    credit_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    vente_id UUID NOT NULL REFERENCES ventes(vente_id),
    client_id UUID NOT NULL REFERENCES clients(client_id),
    montant DECIMAL(12, 2) NOT NULL,
    date_echeance DATE NOT NULL,
    statut_credit_id UUID NOT NULL REFERENCES statuts_credit(statut_credit_id),
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE receptions_caisse (
    reception_caisse_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    repartition_id UUID NOT NULL REFERENCES repartitions(repartition_id),
    montant_attendu DECIMAL(12, 2) NOT NULL,
    montant_recu DECIMAL(12, 2),
    ecart DECIMAL(12, 2) GENERATED ALWAYS AS (montant_attendu - COALESCE(montant_recu, 0)) STORED,
    statut_caisse_id UUID NOT NULL REFERENCES statuts_caisse(statut_caisse_id),
    caissier_id UUID REFERENCES utilisateurs(utilisateur_id),
    date_mise_a_jour TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE journaux_audit (
    journal_audit_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    utilisateur_id UUID REFERENCES utilisateurs(utilisateur_id),
    action VARCHAR(50) NOT NULL,
    type_entite VARCHAR(50) NOT NULL,
    identifiant_entite UUID,
    anciennes_valeurs JSONB,
    nouvelles_valeurs JSONB,
    date_heure TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- ============================================================================
-- VIEWS & TRIGGERS
-- ============================================================================

CREATE VIEW v_statut_stock AS
SELECT 
    p.nom, p.code_sku, cp.nom as categorie, tp.nom as type,
    COALESCE(SUM(es.quantite), 0) as stock_total
FROM produits p
JOIN categories_produits cp ON p.categorie_produit_id = cp.categorie_produit_id
JOIN types_produits tp ON p.type_produit_id = tp.type_produit_id
LEFT JOIN entrees_stock es ON p.produit_id = es.produit_id
GROUP BY p.nom, p.code_sku, cp.nom, tp.nom;

CREATE OR REPLACE FUNCTION trg_maj_date() RETURNS TRIGGER AS $$
BEGIN NEW.date_mise_a_jour = CURRENT_TIMESTAMP; RETURN NEW; END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_prod_maj BEFORE UPDATE ON produits FOR EACH ROW EXECUTE FUNCTION trg_maj_date();
CREATE TRIGGER trg_cat_maj BEFORE UPDATE ON categories_produits FOR EACH ROW EXECUTE FUNCTION trg_maj_date();
CREATE TRIGGER trg_cli_maj BEFORE UPDATE ON clients FOR EACH ROW EXECUTE FUNCTION trg_maj_date();