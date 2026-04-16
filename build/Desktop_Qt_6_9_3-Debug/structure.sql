--
-- PostgreSQL database dump
--

\restrict wUd7ofDSYYIUVhbfd94UguzZzilehFFiVcSJkSj9CRHxlJc5YIxIfelSfYVaWWf

-- Dumped from database version 17.9 (Debian 17.9-0+deb13u1)
-- Dumped by pg_dump version 17.9 (Debian 17.9-0+deb13u1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: uuid-ossp; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS "uuid-ossp" WITH SCHEMA public;


--
-- Name: EXTENSION "uuid-ossp"; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION "uuid-ossp" IS 'generate universally unique identifiers (UUIDs)';


--
-- Name: trg_maj_date(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.trg_maj_date() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN 
    IF TG_TABLE_NAME = 'stock_soldes' THEN
        NEW.updated_at = CURRENT_TIMESTAMP;
    ELSE
        NEW.date_mise_a_jour = CURRENT_TIMESTAMP;
    END IF;
    RETURN NEW; 
END;
$$;


ALTER FUNCTION public.trg_maj_date() OWNER TO postgres;

--
-- Name: trg_sync_stock_soldes(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.trg_sync_stock_soldes() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    total_entrees INT;
    total_sorties INT;
    total_retours INT;
BEGIN
    -- Calculer les totaux par produit
    SELECT COALESCE(SUM(quantite), 0) INTO total_entrees
    FROM public.entrees_stock 
    WHERE produit_id = NEW.produit_id 
    AND statut_validation IN ('APPROUVE', 'EN_ATTENTE');
    
    SELECT COALESCE(SUM(quantite_vente + quantite_cadeau), 0) INTO total_sorties
    FROM public.articles_repartition 
    WHERE produit_id = NEW.produit_id;
    
    SELECT COALESCE(SUM(quantite), 0) INTO total_retours
    FROM public.retours_stock 
    WHERE produit_id = NEW.produit_id
    AND statut_validation IN ('APPROUVE', 'EN_ATTENTE');
    
    -- Upsert le solde
    INSERT INTO public.stock_soldes (produit_id, quantite_total, quantite_reserve, prix_moyen, dernier_mouvement_date)
    VALUES (
        NEW.produit_id,
        total_entrees - total_sorties + total_retours,
        total_sorties,
        COALESCE(NEW.prix_unitaire, (SELECT prix_unitaire FROM public.produits WHERE produit_id = NEW.produit_id)),
        CURRENT_TIMESTAMP
    )
    ON CONFLICT (produit_id) DO UPDATE SET
        quantite_total = total_entrees - total_sorties + total_retours,
        quantite_reserve = total_sorties,
        prix_moyen = COALESCE(EXCLUDED.prix_moyen, NEW.prix_unitaire),
        dernier_mouvement_date = CURRENT_TIMESTAMP,
        updated_at = CURRENT_TIMESTAMP;
    
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.trg_sync_stock_soldes() OWNER TO postgres;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: articles_repartition; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.articles_repartition (
    article_repartition_id uuid DEFAULT gen_random_uuid() NOT NULL,
    repartition_id uuid NOT NULL,
    produit_id uuid NOT NULL,
    quantite_vente integer DEFAULT 0,
    quantite_cadeau integer DEFAULT 0,
    quantite_totale integer GENERATED ALWAYS AS ((quantite_vente + quantite_cadeau)) STORED
);


ALTER TABLE public.articles_repartition OWNER TO postgres;

--
-- Name: categories_produits; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.categories_produits (
    categorie_produit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(100) NOT NULL,
    code_categorie character varying(20),
    description text DEFAULT ''::text,
    est_actif boolean DEFAULT true,
    ordre_affichage integer DEFAULT 0,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.categories_produits OWNER TO postgres;

--
-- Name: clients; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.clients (
    client_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(100) NOT NULL,
    route_id uuid,
    condition_paiement_id uuid,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.clients OWNER TO postgres;

--
-- Name: conditions_paiement; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.conditions_paiement (
    condition_paiement_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL,
    delai_jours integer DEFAULT 0
);


ALTER TABLE public.conditions_paiement OWNER TO postgres;

--
-- Name: credits; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.credits (
    credit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    vente_id uuid NOT NULL,
    client_id uuid NOT NULL,
    montant numeric(12,2) NOT NULL,
    date_echeance date NOT NULL,
    statut_credit_id uuid NOT NULL,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.credits OWNER TO postgres;

--
-- Name: entrees_stock; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.entrees_stock (
    entree_stock_id uuid DEFAULT gen_random_uuid() NOT NULL,
    produit_id uuid NOT NULL,
    quantite integer NOT NULL,
    source_entree_id uuid NOT NULL,
    date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    cree_par uuid,
    numero_facture character varying(50),
    prix_unitaire numeric(12,2),
    numero_lot character varying(50),
    date_expiration date,
    approuve_par uuid,
    statut_validation character varying(20) DEFAULT 'EN_ATTENTE'::character varying,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    cree_par_updated uuid
);


ALTER TABLE public.entrees_stock OWNER TO postgres;

--
-- Name: equipes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.equipes (
    equipe_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(100) NOT NULL,
    nom_chef character varying(100) NOT NULL
);


ALTER TABLE public.equipes OWNER TO postgres;

--
-- Name: journaux_audit; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.journaux_audit (
    journal_audit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    utilisateur_id uuid,
    action character varying(50) NOT NULL,
    type_entite character varying(50) NOT NULL,
    identifiant_entite uuid,
    anciennes_valeurs jsonb,
    nouvelles_valeurs jsonb,
    date_heure timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.journaux_audit OWNER TO postgres;

--
-- Name: permissions; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.permissions (
    permission_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(50) NOT NULL,
    nom character varying(100) NOT NULL,
    module character varying(50) NOT NULL
);


ALTER TABLE public.permissions OWNER TO postgres;

--
-- Name: produits; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.produits (
    produit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    categorie_produit_id uuid NOT NULL,
    type_produit_id uuid NOT NULL,
    nom character varying(100) NOT NULL,
    code_sku character varying(50) NOT NULL,
    prix_unitaire numeric(12,2) NOT NULL,
    stock_minimum integer DEFAULT 10,
    est_actif boolean DEFAULT true,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    description text
);


ALTER TABLE public.produits OWNER TO postgres;

--
-- Name: raisons_retour; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.raisons_retour (
    raison_retour_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.raisons_retour OWNER TO postgres;

--
-- Name: receptions_caisse; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.receptions_caisse (
    reception_caisse_id uuid DEFAULT gen_random_uuid() NOT NULL,
    repartition_id uuid NOT NULL,
    montant_attendu numeric(12,2) NOT NULL,
    montant_recu numeric(12,2),
    ecart numeric(12,2) GENERATED ALWAYS AS ((montant_attendu - COALESCE(montant_recu, (0)::numeric))) STORED,
    statut_caisse_id uuid NOT NULL,
    caissier_id uuid,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.receptions_caisse OWNER TO postgres;

--
-- Name: repartitions; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.repartitions (
    repartition_id uuid DEFAULT gen_random_uuid() NOT NULL,
    equipe_id uuid NOT NULL,
    route_id uuid NOT NULL,
    statut_repartition_id uuid NOT NULL,
    date_repartition date NOT NULL,
    montant_cash_attendu numeric(12,2) DEFAULT 0,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.repartitions OWNER TO postgres;

--
-- Name: retours_stock; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.retours_stock (
    retour_stock_id uuid DEFAULT gen_random_uuid() NOT NULL,
    produit_id uuid NOT NULL,
    quantite integer NOT NULL,
    raison_retour_id uuid NOT NULL,
    date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    repartition_id uuid,
    observations text,
    cree_par uuid NOT NULL,
    approuve_par uuid,
    statut_validation character varying(20) DEFAULT 'EN_ATTENTE'::character varying,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.retours_stock OWNER TO postgres;

--
-- Name: role_permissions; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.role_permissions (
    role_id uuid NOT NULL,
    permission_id uuid NOT NULL
);


ALTER TABLE public.role_permissions OWNER TO postgres;

--
-- Name: roles; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.roles (
    role_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.roles OWNER TO postgres;

--
-- Name: routes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.routes (
    route_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(100) NOT NULL
);


ALTER TABLE public.routes OWNER TO postgres;

--
-- Name: sources_entree; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sources_entree (
    source_entree_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.sources_entree OWNER TO postgres;

--
-- Name: statuts_caisse; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.statuts_caisse (
    statut_caisse_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.statuts_caisse OWNER TO postgres;

--
-- Name: statuts_credit; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.statuts_credit (
    statut_credit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.statuts_credit OWNER TO postgres;

--
-- Name: statuts_repartition; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.statuts_repartition (
    statut_repartition_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.statuts_repartition OWNER TO postgres;

--
-- Name: stock_soldes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.stock_soldes (
    solde_id uuid DEFAULT gen_random_uuid() NOT NULL,
    produit_id uuid NOT NULL,
    quantite_total integer DEFAULT 0,
    quantite_reserve integer DEFAULT 0,
    quantite_disponible integer GENERATED ALWAYS AS ((quantite_total - quantite_reserve)) STORED,
    valeur_stock numeric(15,2) DEFAULT 0.00,
    prix_moyen numeric(12,2),
    dernier_mouvement_date timestamp without time zone,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.stock_soldes OWNER TO postgres;

--
-- Name: types_produits; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.types_produits (
    type_produit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.types_produits OWNER TO postgres;

--
-- Name: types_vente; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.types_vente (
    type_vente_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL
);


ALTER TABLE public.types_vente OWNER TO postgres;

--
-- Name: utilisateurs; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.utilisateurs (
    utilisateur_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom_utilisateur character varying(50) NOT NULL,
    email character varying(100) NOT NULL,
    hash_mot_passe character varying(255) NOT NULL,
    nom_complet character varying(100) NOT NULL,
    role_id uuid NOT NULL,
    est_actif boolean DEFAULT true,
    date_creation timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.utilisateurs OWNER TO postgres;

--
-- Name: v_statut_stock; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_statut_stock AS
 SELECT p.nom,
    p.code_sku,
    cp.nom AS categorie,
    tp.nom AS type,
    COALESCE(sum(es.quantite), (0)::bigint) AS stock_total
   FROM (((public.produits p
     LEFT JOIN public.categories_produits cp ON ((p.categorie_produit_id = cp.categorie_produit_id)))
     LEFT JOIN public.types_produits tp ON ((p.type_produit_id = tp.type_produit_id)))
     LEFT JOIN public.entrees_stock es ON ((p.produit_id = es.produit_id)))
  GROUP BY p.produit_id, p.nom, p.code_sku, cp.nom, tp.nom;


ALTER VIEW public.v_statut_stock OWNER TO postgres;

--
-- Name: v_stock_detail; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_detail AS
 SELECT p.produit_id,
    p.nom AS produit_nom,
    p.code_sku,
    cp.nom AS categorie,
    tp.nom AS type,
    COALESCE(ss.quantite_total, 0) AS quantite_total,
    COALESCE(ss.quantite_reserve, 0) AS quantite_reserve,
    COALESCE(ss.quantite_disponible, 0) AS quantite_disponible,
    COALESCE(ss.valeur_stock, (0)::numeric) AS valeur_stock,
    COALESCE(ss.prix_moyen, p.prix_unitaire) AS prix_moyen,
    p.stock_minimum,
        CASE
            WHEN (COALESCE(ss.quantite_disponible, 0) <= 0) THEN 'RUPTURE'::text
            WHEN (COALESCE(ss.quantite_disponible, 0) < p.stock_minimum) THEN 'CRITIQUE'::text
            WHEN ((COALESCE(ss.quantite_disponible, 0))::numeric < ((p.stock_minimum)::numeric * 1.5)) THEN 'FAIBLE'::text
            ELSE 'OK'::text
        END AS statut_stock,
    ss.dernier_mouvement_date
   FROM (((public.produits p
     LEFT JOIN public.categories_produits cp ON ((p.categorie_produit_id = cp.categorie_produit_id)))
     LEFT JOIN public.types_produits tp ON ((p.type_produit_id = tp.type_produit_id)))
     LEFT JOIN public.stock_soldes ss ON ((p.produit_id = ss.produit_id)))
  WHERE (p.est_actif = true);


ALTER VIEW public.v_stock_detail OWNER TO postgres;

--
-- Name: v_stock_mouvements; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_mouvements AS
 SELECT 'ENTREE'::character varying AS type_mouvement,
    (es.entree_stock_id)::character varying AS mouvement_id,
    es.produit_id,
    p.nom AS produit_nom,
    p.code_sku,
    es.quantite AS quantite_delta,
    'PRODUCTION'::character varying AS raison,
    es.cree_par AS utilisateur_id,
    es.date AS created_at,
    NULL::uuid AS repartition_id,
    se.nom AS source
   FROM ((public.entrees_stock es
     JOIN public.produits p ON ((es.produit_id = p.produit_id)))
     JOIN public.sources_entree se ON ((es.source_entree_id = se.source_entree_id)))
  WHERE ((es.statut_validation)::text = ANY (ARRAY[('APPROUVE'::character varying)::text, ('EN_ATTENTE'::character varying)::text]))
UNION ALL
 SELECT 'SORTIE'::character varying AS type_mouvement,
    (ar.article_repartition_id)::character varying AS mouvement_id,
    ar.produit_id,
    p.nom AS produit_nom,
    p.code_sku,
    (- (ar.quantite_vente + ar.quantite_cadeau)) AS quantite_delta,
    'REPARTITION'::character varying AS raison,
    NULL::uuid AS utilisateur_id,
    r.date_repartition AS created_at,
    r.repartition_id,
    'Repartition'::character varying AS source
   FROM ((public.articles_repartition ar
     JOIN public.repartitions r ON ((ar.repartition_id = r.repartition_id)))
     JOIN public.produits p ON ((ar.produit_id = p.produit_id)))
UNION ALL
 SELECT 'RETOUR'::character varying AS type_mouvement,
    (rs.retour_stock_id)::character varying AS mouvement_id,
    rs.produit_id,
    p.nom AS produit_nom,
    p.code_sku,
    rs.quantite AS quantite_delta,
    COALESCE(rr.code, 'INCONNU'::character varying) AS raison,
    rs.cree_par AS utilisateur_id,
    rs.date AS created_at,
    rs.repartition_id,
    'Retour'::character varying AS source
   FROM ((public.retours_stock rs
     JOIN public.produits p ON ((rs.produit_id = p.produit_id)))
     LEFT JOIN public.raisons_retour rr ON ((rs.raison_retour_id = rr.raison_retour_id)))
  WHERE ((rs.statut_validation)::text = ANY (ARRAY[('APPROUVE'::character varying)::text, ('EN_ATTENTE'::character varying)::text]))
  ORDER BY 9 DESC;


ALTER VIEW public.v_stock_mouvements OWNER TO postgres;

--
-- Name: ventes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.ventes (
    vente_id uuid DEFAULT gen_random_uuid() NOT NULL,
    repartition_id uuid NOT NULL,
    produit_id uuid NOT NULL,
    client_id uuid NOT NULL,
    quantite integer NOT NULL,
    type_vente_id uuid NOT NULL,
    prix_unitaire numeric(12,2) NOT NULL,
    montant_total numeric(12,2) GENERATED ALWAYS AS (((quantite)::numeric * prix_unitaire)) STORED,
    date_vente timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.ventes OWNER TO postgres;

--
-- Data for Name: articles_repartition; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.articles_repartition (article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau) FROM stdin;
\.


--
-- Data for Name: categories_produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.categories_produits (categorie_produit_id, nom, code_categorie, description, est_actif, ordre_affichage, date_mise_a_jour) FROM stdin;
53cd8cb8-c163-43a4-8864-5ea57153dbad	Soft Drinks	SOFT	Boissons non alcoolisées	t	4	2026-04-14 18:23:07.500727
41414f5b-0968-4d4c-a71b-26473a0abce4	Alimentaire	ALIM	Aliments - Test	f	0	2026-04-14 16:36:42.488393
df14e1fd-7f15-4f4e-8bce-d8ab5fda2282	Vins Test	VNS	Test category	f	1	2026-04-14 17:06:08.93193
357241c2-6e79-4952-b103-50e1b52b3f96	Vins	VINS	Vins alcooliques	t	1	2026-04-14 17:06:50.960208
a84c2258-5abd-4d9b-b103-c3b3d2f8460c	Bières	BIERES	Bières	t	2	2026-04-14 17:07:06.559007
98559eb1-06d7-42c0-b6f1-c31197d4e212	Spiritueux	SPIRITUEUX	Spiritueux premium et liqueurs	t	3	2026-04-14 17:07:22.832042
\.


--
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.clients (client_id, nom, route_id, condition_paiement_id, date_mise_a_jour) FROM stdin;
\.


--
-- Data for Name: conditions_paiement; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.conditions_paiement (condition_paiement_id, code, nom, delai_jours) FROM stdin;
7a10dde0-a3cb-4004-9c7a-c8f917a28e58	CASH	Paiement cash	0
2eb63626-b12e-4b39-8956-97e8ef05f02d	CREDIT_7J	Crédit 7 jours	7
bea42eb8-e1ac-468b-8dce-14d48ad57eb0	CREDIT_15J	Crédit 15 jours	15
37d3f356-1b26-40c8-a67d-580c360e6108	CREDIT_30J	Crédit 30 jours	30
\.


--
-- Data for Name: credits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.credits (credit_id, vente_id, client_id, montant, date_echeance, statut_credit_id, date_mise_a_jour) FROM stdin;
\.


--
-- Data for Name: entrees_stock; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.entrees_stock (entree_stock_id, produit_id, quantite, source_entree_id, date, cree_par, numero_facture, prix_unitaire, numero_lot, date_expiration, approuve_par, statut_validation, date_mise_a_jour, cree_par_updated) FROM stdin;
e9ebe02c-d9ad-4f89-b029-02897b4f651b	857f7c59-4ff6-45af-81fc-81d534af18de	100	c656b280-7660-4ebf-b20c-28880d7cd5f7	2026-04-16 07:49:59.537	78a24f11-9714-4014-ae40-f38318fef119	\N	1200.00	\N	2027-04-16	\N	EN_ATTENTE	2026-04-16 07:49:59.537509	\N
45f2d7cc-dd91-4e0f-b65b-1a20d796c584	03dc330a-6da1-41ca-86ba-60945244182a	100	c656b280-7660-4ebf-b20c-28880d7cd5f7	2026-04-16 07:51:39.589	78a24f11-9714-4014-ae40-f38318fef119	\N	1000.00	\N	2027-04-16	\N	EN_ATTENTE	2026-04-16 07:51:39.589764	\N
\.


--
-- Data for Name: equipes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.equipes (equipe_id, nom, nom_chef) FROM stdin;
\.


--
-- Data for Name: journaux_audit; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.journaux_audit (journal_audit_id, utilisateur_id, action, type_entite, identifiant_entite, anciennes_valeurs, nouvelles_valeurs, date_heure) FROM stdin;
\.


--
-- Data for Name: permissions; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.permissions (permission_id, code, nom, module) FROM stdin;
4ed81207-5257-4f3e-afd3-71057a413e71	STOCK_EDIT	Modifier le stock	Stock
d6d71983-fdc6-41bc-9dd9-da65bf97c04f	STOCK_VIEW	Voir le stock	Stock
9988e1e7-97d0-4dd6-a4b9-61a24bc38606	VENTE_CREATE	Créer une vente	Ventes
b5714b2f-205a-4258-a276-9e98ebef57e9	VENTE_EDIT	Modifier une vente	Ventes
ab845788-0800-489a-9129-9be7e29d7c55	CAISSE_VALIDER	Valider la caisse	Caisse
7e902bc7-1361-4af6-b37e-942baee4810e	CREDIT_MANAGE	Gérer les crédits	Finance
03275df0-2e4e-4b05-818a-537cba69ec27	STOCK_APPROVE	Approuver entrées/retours	Stock
\.


--
-- Data for Name: produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.produits (produit_id, categorie_produit_id, type_produit_id, nom, code_sku, prix_unitaire, stock_minimum, est_actif, date_mise_a_jour, description) FROM stdin;
6f7b704d-99c4-4837-a04b-be52c27f33d9	357241c2-6e79-4952-b103-50e1b52b3f96	372480eb-993e-4ac0-862f-39466beb1863	Vin SEMULIKI	VIN-SEM	1200.00	50	f	2026-04-14 18:09:32.24038	13% Alc.\nAphrodisiaque
03dc330a-6da1-41ca-86ba-60945244182a	357241c2-6e79-4952-b103-50e1b52b3f96	372480eb-993e-4ac0-862f-39466beb1863	Vin SEMULIKI-20	VIN-SEM-20	1000.00	50	t	2026-04-15 05:44:42.449737	20% Alc.\nApperitif
857f7c59-4ff6-45af-81fc-81d534af18de	357241c2-6e79-4952-b103-50e1b52b3f96	187835d8-22a6-4a08-bb6e-e93d2a334147	Vin SEMULIKI-13	VIN-SEM-13	1200.00	50	t	2026-04-15 05:44:56.894016	13% Alc.\nAphrodisiaque
\.


--
-- Data for Name: raisons_retour; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.raisons_retour (raison_retour_id, code, nom) FROM stdin;
9cd4e12b-4938-48c9-8e62-1c4f892410d8	INVENDU	Invendu
720baf31-5f69-4736-b671-2df46e81b60d	ENDOMMAGE	Endommagé
fa101a6e-a89c-45aa-a95f-90b60dcd6eee	EXPIRE	Expiré
25e01e32-5cfd-48f8-9e27-05631883a3b8	ERREUR	Erreur de commande
\.


--
-- Data for Name: receptions_caisse; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.receptions_caisse (reception_caisse_id, repartition_id, montant_attendu, montant_recu, statut_caisse_id, caissier_id, date_mise_a_jour) FROM stdin;
\.


--
-- Data for Name: repartitions; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.repartitions (repartition_id, equipe_id, route_id, statut_repartition_id, date_repartition, montant_cash_attendu, date_mise_a_jour) FROM stdin;
\.


--
-- Data for Name: retours_stock; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.retours_stock (retour_stock_id, produit_id, quantite, raison_retour_id, date, repartition_id, observations, cree_par, approuve_par, statut_validation, date_mise_a_jour) FROM stdin;
\.


--
-- Data for Name: role_permissions; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.role_permissions (role_id, permission_id) FROM stdin;
23b42043-5724-483d-89fe-0debdcc1d6e6	4ed81207-5257-4f3e-afd3-71057a413e71
23b42043-5724-483d-89fe-0debdcc1d6e6	d6d71983-fdc6-41bc-9dd9-da65bf97c04f
23b42043-5724-483d-89fe-0debdcc1d6e6	9988e1e7-97d0-4dd6-a4b9-61a24bc38606
23b42043-5724-483d-89fe-0debdcc1d6e6	b5714b2f-205a-4258-a276-9e98ebef57e9
23b42043-5724-483d-89fe-0debdcc1d6e6	ab845788-0800-489a-9129-9be7e29d7c55
23b42043-5724-483d-89fe-0debdcc1d6e6	7e902bc7-1361-4af6-b37e-942baee4810e
\.


--
-- Data for Name: roles; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.roles (role_id, code, nom) FROM stdin;
23b42043-5724-483d-89fe-0debdcc1d6e6	ADMIN	Administrator
\.


--
-- Data for Name: routes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.routes (route_id, nom) FROM stdin;
\.


--
-- Data for Name: sources_entree; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.sources_entree (source_entree_id, code, nom) FROM stdin;
c656b280-7660-4ebf-b20c-28880d7cd5f7	PRODUCTION	Production
b458d1a4-29c7-4bc7-ab84-34f6baac904e	RETOUR	Retour
b2f4f9fb-3eab-4e38-a389-4b40e71cefb6	AJUSTEMENT	Ajustement
b0254355-788b-42ed-b52c-41096dee4f6d	ACHAT	Achat
\.


--
-- Data for Name: statuts_caisse; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.statuts_caisse (statut_caisse_id, code, nom) FROM stdin;
6a7d91ad-9011-4d56-8fe4-34b4e40b0e53	VALIDE	Validée
1c667132-9461-4622-8950-fc2f9171276d	DISCREPANCE	Discrepance
960957e3-a06d-4b43-9c9c-ea791e4ba008	EN_ATTENTE	En attente
\.


--
-- Data for Name: statuts_credit; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.statuts_credit (statut_credit_id, code, nom) FROM stdin;
e9d45298-343b-4747-ab33-e067991afb05	PAYE	Payé
7a70cbb4-ad1c-4aa8-9cae-50cc9c61b711	EN_RETARD	En retard
1182f388-881a-4690-9029-362a6601972b	EN_ATTENTE	En attente
\.


--
-- Data for Name: statuts_repartition; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.statuts_repartition (statut_repartition_id, code, nom) FROM stdin;
e0059987-5a9f-44bd-b806-18434792491d	EN_COURS	En cours
936c875a-f441-410d-9098-98531a60c073	COMPLETEE	Complétée
d8a91671-fd79-4af7-a75e-ea7e39b8be24	ANNULEE	Annulée
\.


--
-- Data for Name: stock_soldes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.stock_soldes (solde_id, produit_id, quantite_total, quantite_reserve, valeur_stock, prix_moyen, dernier_mouvement_date, updated_at) FROM stdin;
f083b1c6-96f7-4476-9c3c-798f199c58c9	857f7c59-4ff6-45af-81fc-81d534af18de	100	0	0.00	1200.00	2026-04-16 07:55:48.947538	2026-04-16 07:55:48.947538
7721677f-e9a2-4a5c-94fa-870488acaf04	03dc330a-6da1-41ca-86ba-60945244182a	100	0	0.00	1000.00	2026-04-16 07:55:48.947538	2026-04-16 07:55:48.947538
\.


--
-- Data for Name: types_produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.types_produits (type_produit_id, code, nom) FROM stdin;
372480eb-993e-4ac0-862f-39466beb1863	MARCHANDISE	Marchandise
187835d8-22a6-4a08-bb6e-e93d2a334147	EMBALLAGE	Emballage
a9501a7c-4c6a-4792-820c-7b3773ac7ea6	SERVICE	Service
\.


--
-- Data for Name: types_vente; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.types_vente (type_vente_id, code, nom) FROM stdin;
ea35f245-b1d4-4d57-81c3-047fa0a6f511	CASH	Vente cash
139fed84-074f-4ac1-a56c-aecc47d83c2a	CREDIT	Vente à crédit
\.


--
-- Data for Name: utilisateurs; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.utilisateurs (utilisateur_id, nom_utilisateur, email, hash_mot_passe, nom_complet, role_id, est_actif, date_creation, date_mise_a_jour) FROM stdin;
78a24f11-9714-4014-ae40-f38318fef119	admin	admin@semuliki.local	$2b$12$wqVrUB8BZ/OvhP9V1kru..yEO2gQeRK8Cg1sAG1Bk4VYGvo7iMUN2	Administrator	23b42043-5724-483d-89fe-0debdcc1d6e6	t	2026-04-14 18:21:35.118185	2026-04-14 18:21:35.118185
\.


--
-- Data for Name: ventes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.ventes (vente_id, repartition_id, produit_id, client_id, quantite, type_vente_id, prix_unitaire, date_vente) FROM stdin;
\.


--
-- Name: articles_repartition articles_repartition_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.articles_repartition
    ADD CONSTRAINT articles_repartition_pkey PRIMARY KEY (article_repartition_id);


--
-- Name: categories_produits categories_produits_code_categorie_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.categories_produits
    ADD CONSTRAINT categories_produits_code_categorie_key UNIQUE (code_categorie);


--
-- Name: categories_produits categories_produits_nom_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.categories_produits
    ADD CONSTRAINT categories_produits_nom_key UNIQUE (nom);


--
-- Name: categories_produits categories_produits_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.categories_produits
    ADD CONSTRAINT categories_produits_pkey PRIMARY KEY (categorie_produit_id);


--
-- Name: clients clients_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_pkey PRIMARY KEY (client_id);


--
-- Name: conditions_paiement conditions_paiement_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.conditions_paiement
    ADD CONSTRAINT conditions_paiement_code_key UNIQUE (code);


--
-- Name: conditions_paiement conditions_paiement_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.conditions_paiement
    ADD CONSTRAINT conditions_paiement_pkey PRIMARY KEY (condition_paiement_id);


--
-- Name: credits credits_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.credits
    ADD CONSTRAINT credits_pkey PRIMARY KEY (credit_id);


--
-- Name: entrees_stock entrees_stock_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.entrees_stock
    ADD CONSTRAINT entrees_stock_pkey PRIMARY KEY (entree_stock_id);


--
-- Name: equipes equipes_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.equipes
    ADD CONSTRAINT equipes_pkey PRIMARY KEY (equipe_id);


--
-- Name: journaux_audit journaux_audit_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.journaux_audit
    ADD CONSTRAINT journaux_audit_pkey PRIMARY KEY (journal_audit_id);


--
-- Name: permissions permissions_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.permissions
    ADD CONSTRAINT permissions_code_key UNIQUE (code);


--
-- Name: permissions permissions_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.permissions
    ADD CONSTRAINT permissions_pkey PRIMARY KEY (permission_id);


--
-- Name: produits produits_code_sku_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.produits
    ADD CONSTRAINT produits_code_sku_key UNIQUE (code_sku);


--
-- Name: produits produits_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.produits
    ADD CONSTRAINT produits_pkey PRIMARY KEY (produit_id);


--
-- Name: raisons_retour raisons_retour_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.raisons_retour
    ADD CONSTRAINT raisons_retour_code_key UNIQUE (code);


--
-- Name: raisons_retour raisons_retour_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.raisons_retour
    ADD CONSTRAINT raisons_retour_pkey PRIMARY KEY (raison_retour_id);


--
-- Name: receptions_caisse receptions_caisse_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.receptions_caisse
    ADD CONSTRAINT receptions_caisse_pkey PRIMARY KEY (reception_caisse_id);


--
-- Name: repartitions repartitions_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartitions
    ADD CONSTRAINT repartitions_pkey PRIMARY KEY (repartition_id);


--
-- Name: retours_stock retours_stock_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.retours_stock
    ADD CONSTRAINT retours_stock_pkey PRIMARY KEY (retour_stock_id);


--
-- Name: role_permissions role_permissions_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.role_permissions
    ADD CONSTRAINT role_permissions_pkey PRIMARY KEY (role_id, permission_id);


--
-- Name: roles roles_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles
    ADD CONSTRAINT roles_code_key UNIQUE (code);


--
-- Name: roles roles_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.roles
    ADD CONSTRAINT roles_pkey PRIMARY KEY (role_id);


--
-- Name: routes routes_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.routes
    ADD CONSTRAINT routes_pkey PRIMARY KEY (route_id);


--
-- Name: sources_entree sources_entree_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sources_entree
    ADD CONSTRAINT sources_entree_code_key UNIQUE (code);


--
-- Name: sources_entree sources_entree_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sources_entree
    ADD CONSTRAINT sources_entree_pkey PRIMARY KEY (source_entree_id);


--
-- Name: statuts_caisse statuts_caisse_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.statuts_caisse
    ADD CONSTRAINT statuts_caisse_code_key UNIQUE (code);


--
-- Name: statuts_caisse statuts_caisse_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.statuts_caisse
    ADD CONSTRAINT statuts_caisse_pkey PRIMARY KEY (statut_caisse_id);


--
-- Name: statuts_credit statuts_credit_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.statuts_credit
    ADD CONSTRAINT statuts_credit_code_key UNIQUE (code);


--
-- Name: statuts_credit statuts_credit_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.statuts_credit
    ADD CONSTRAINT statuts_credit_pkey PRIMARY KEY (statut_credit_id);


--
-- Name: statuts_repartition statuts_repartition_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.statuts_repartition
    ADD CONSTRAINT statuts_repartition_code_key UNIQUE (code);


--
-- Name: statuts_repartition statuts_repartition_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.statuts_repartition
    ADD CONSTRAINT statuts_repartition_pkey PRIMARY KEY (statut_repartition_id);


--
-- Name: stock_soldes stock_soldes_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_soldes
    ADD CONSTRAINT stock_soldes_pkey PRIMARY KEY (solde_id);


--
-- Name: stock_soldes stock_soldes_produit_id_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_soldes
    ADD CONSTRAINT stock_soldes_produit_id_key UNIQUE (produit_id);


--
-- Name: types_produits types_produits_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.types_produits
    ADD CONSTRAINT types_produits_code_key UNIQUE (code);


--
-- Name: types_produits types_produits_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.types_produits
    ADD CONSTRAINT types_produits_pkey PRIMARY KEY (type_produit_id);


--
-- Name: types_vente types_vente_code_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.types_vente
    ADD CONSTRAINT types_vente_code_key UNIQUE (code);


--
-- Name: types_vente types_vente_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.types_vente
    ADD CONSTRAINT types_vente_pkey PRIMARY KEY (type_vente_id);


--
-- Name: utilisateurs utilisateurs_email_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateurs
    ADD CONSTRAINT utilisateurs_email_key UNIQUE (email);


--
-- Name: utilisateurs utilisateurs_nom_utilisateur_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateurs
    ADD CONSTRAINT utilisateurs_nom_utilisateur_key UNIQUE (nom_utilisateur);


--
-- Name: utilisateurs utilisateurs_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateurs
    ADD CONSTRAINT utilisateurs_pkey PRIMARY KEY (utilisateur_id);


--
-- Name: ventes ventes_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ventes
    ADD CONSTRAINT ventes_pkey PRIMARY KEY (vente_id);


--
-- Name: idx_articles_repartition_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_articles_repartition_produit ON public.articles_repartition USING btree (produit_id);


--
-- Name: idx_articles_repartition_repartition; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_articles_repartition_repartition ON public.articles_repartition USING btree (repartition_id);


--
-- Name: idx_entrees_stock_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_date ON public.entrees_stock USING btree (date);


--
-- Name: idx_entrees_stock_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_produit ON public.entrees_stock USING btree (produit_id);


--
-- Name: idx_entrees_stock_statut; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_statut ON public.entrees_stock USING btree (statut_validation);


--
-- Name: idx_retours_stock_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_date ON public.retours_stock USING btree (date);


--
-- Name: idx_retours_stock_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_produit ON public.retours_stock USING btree (produit_id);


--
-- Name: idx_retours_stock_statut; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_statut ON public.retours_stock USING btree (statut_validation);


--
-- Name: idx_stock_soldes_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_soldes_produit ON public.stock_soldes USING btree (produit_id);


--
-- Name: idx_ventes_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_ventes_date ON public.ventes USING btree (date_vente);


--
-- Name: idx_ventes_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_ventes_produit ON public.ventes USING btree (produit_id);


--
-- Name: idx_ventes_repartition; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_ventes_repartition ON public.ventes USING btree (repartition_id);


--
-- Name: articles_repartition trg_articles_repartition_sync; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_articles_repartition_sync AFTER INSERT OR DELETE OR UPDATE ON public.articles_repartition FOR EACH ROW EXECUTE FUNCTION public.trg_sync_stock_soldes();


--
-- Name: entrees_stock trg_entrees_stock_sync; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_entrees_stock_sync AFTER INSERT OR UPDATE ON public.entrees_stock FOR EACH ROW EXECUTE FUNCTION public.trg_sync_stock_soldes();


--
-- Name: retours_stock trg_retours_stock_sync; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_retours_stock_sync AFTER INSERT OR UPDATE ON public.retours_stock FOR EACH ROW EXECUTE FUNCTION public.trg_sync_stock_soldes();


--
-- Name: entrees_stock trg_sync_stock; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_stock AFTER INSERT OR UPDATE ON public.entrees_stock FOR EACH ROW EXECUTE FUNCTION public.trg_sync_stock_soldes();


--
-- Name: entrees_stock trg_update_date_entrees_stock; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_update_date_entrees_stock BEFORE INSERT OR UPDATE ON public.entrees_stock FOR EACH ROW EXECUTE FUNCTION public.trg_maj_date();


--
-- Name: stock_soldes trg_update_date_stock_soldes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_update_date_stock_soldes BEFORE INSERT OR UPDATE ON public.stock_soldes FOR EACH ROW EXECUTE FUNCTION public.trg_maj_date();


--
-- Name: articles_repartition articles_repartition_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.articles_repartition
    ADD CONSTRAINT articles_repartition_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


--
-- Name: articles_repartition articles_repartition_repartition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.articles_repartition
    ADD CONSTRAINT articles_repartition_repartition_id_fkey FOREIGN KEY (repartition_id) REFERENCES public.repartitions(repartition_id) ON DELETE CASCADE;


--
-- Name: clients clients_condition_paiement_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_condition_paiement_id_fkey FOREIGN KEY (condition_paiement_id) REFERENCES public.conditions_paiement(condition_paiement_id);


--
-- Name: clients clients_route_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_route_id_fkey FOREIGN KEY (route_id) REFERENCES public.routes(route_id);


--
-- Name: credits credits_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.credits
    ADD CONSTRAINT credits_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(client_id);


--
-- Name: credits credits_statut_credit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.credits
    ADD CONSTRAINT credits_statut_credit_id_fkey FOREIGN KEY (statut_credit_id) REFERENCES public.statuts_credit(statut_credit_id);


--
-- Name: credits credits_vente_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.credits
    ADD CONSTRAINT credits_vente_id_fkey FOREIGN KEY (vente_id) REFERENCES public.ventes(vente_id);


--
-- Name: entrees_stock entrees_stock_approuve_par_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.entrees_stock
    ADD CONSTRAINT entrees_stock_approuve_par_fkey FOREIGN KEY (approuve_par) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: entrees_stock entrees_stock_cree_par_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.entrees_stock
    ADD CONSTRAINT entrees_stock_cree_par_fkey FOREIGN KEY (cree_par) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: entrees_stock entrees_stock_cree_par_updated_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.entrees_stock
    ADD CONSTRAINT entrees_stock_cree_par_updated_fkey FOREIGN KEY (cree_par_updated) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: entrees_stock entrees_stock_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.entrees_stock
    ADD CONSTRAINT entrees_stock_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


--
-- Name: entrees_stock entrees_stock_source_entree_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.entrees_stock
    ADD CONSTRAINT entrees_stock_source_entree_id_fkey FOREIGN KEY (source_entree_id) REFERENCES public.sources_entree(source_entree_id);


--
-- Name: journaux_audit journaux_audit_utilisateur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.journaux_audit
    ADD CONSTRAINT journaux_audit_utilisateur_id_fkey FOREIGN KEY (utilisateur_id) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: produits produits_categorie_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.produits
    ADD CONSTRAINT produits_categorie_produit_id_fkey FOREIGN KEY (categorie_produit_id) REFERENCES public.categories_produits(categorie_produit_id);


--
-- Name: produits produits_type_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.produits
    ADD CONSTRAINT produits_type_produit_id_fkey FOREIGN KEY (type_produit_id) REFERENCES public.types_produits(type_produit_id);


--
-- Name: receptions_caisse receptions_caisse_caissier_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.receptions_caisse
    ADD CONSTRAINT receptions_caisse_caissier_id_fkey FOREIGN KEY (caissier_id) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: receptions_caisse receptions_caisse_repartition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.receptions_caisse
    ADD CONSTRAINT receptions_caisse_repartition_id_fkey FOREIGN KEY (repartition_id) REFERENCES public.repartitions(repartition_id);


--
-- Name: receptions_caisse receptions_caisse_statut_caisse_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.receptions_caisse
    ADD CONSTRAINT receptions_caisse_statut_caisse_id_fkey FOREIGN KEY (statut_caisse_id) REFERENCES public.statuts_caisse(statut_caisse_id);


--
-- Name: repartitions repartitions_equipe_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartitions
    ADD CONSTRAINT repartitions_equipe_id_fkey FOREIGN KEY (equipe_id) REFERENCES public.equipes(equipe_id);


--
-- Name: repartitions repartitions_route_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartitions
    ADD CONSTRAINT repartitions_route_id_fkey FOREIGN KEY (route_id) REFERENCES public.routes(route_id);


--
-- Name: repartitions repartitions_statut_repartition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartitions
    ADD CONSTRAINT repartitions_statut_repartition_id_fkey FOREIGN KEY (statut_repartition_id) REFERENCES public.statuts_repartition(statut_repartition_id);


--
-- Name: retours_stock retours_stock_approuve_par_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.retours_stock
    ADD CONSTRAINT retours_stock_approuve_par_fkey FOREIGN KEY (approuve_par) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: retours_stock retours_stock_cree_par_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.retours_stock
    ADD CONSTRAINT retours_stock_cree_par_fkey FOREIGN KEY (cree_par) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: retours_stock retours_stock_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.retours_stock
    ADD CONSTRAINT retours_stock_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


--
-- Name: retours_stock retours_stock_raison_retour_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.retours_stock
    ADD CONSTRAINT retours_stock_raison_retour_id_fkey FOREIGN KEY (raison_retour_id) REFERENCES public.raisons_retour(raison_retour_id);


--
-- Name: retours_stock retours_stock_repartition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.retours_stock
    ADD CONSTRAINT retours_stock_repartition_id_fkey FOREIGN KEY (repartition_id) REFERENCES public.repartitions(repartition_id);


--
-- Name: role_permissions role_permissions_permission_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.role_permissions
    ADD CONSTRAINT role_permissions_permission_id_fkey FOREIGN KEY (permission_id) REFERENCES public.permissions(permission_id) ON DELETE CASCADE;


--
-- Name: role_permissions role_permissions_role_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.role_permissions
    ADD CONSTRAINT role_permissions_role_id_fkey FOREIGN KEY (role_id) REFERENCES public.roles(role_id) ON DELETE CASCADE;


--
-- Name: stock_soldes stock_soldes_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_soldes
    ADD CONSTRAINT stock_soldes_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


--
-- Name: utilisateurs utilisateurs_role_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.utilisateurs
    ADD CONSTRAINT utilisateurs_role_id_fkey FOREIGN KEY (role_id) REFERENCES public.roles(role_id);


--
-- Name: ventes ventes_client_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ventes
    ADD CONSTRAINT ventes_client_id_fkey FOREIGN KEY (client_id) REFERENCES public.clients(client_id);


--
-- Name: ventes ventes_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ventes
    ADD CONSTRAINT ventes_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


--
-- Name: ventes ventes_repartition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ventes
    ADD CONSTRAINT ventes_repartition_id_fkey FOREIGN KEY (repartition_id) REFERENCES public.repartitions(repartition_id);


--
-- Name: ventes ventes_type_vente_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.ventes
    ADD CONSTRAINT ventes_type_vente_id_fkey FOREIGN KEY (type_vente_id) REFERENCES public.types_vente(type_vente_id);


--
-- PostgreSQL database dump complete
--

\unrestrict wUd7ofDSYYIUVhbfd94UguzZzilehFFiVcSJkSj9CRHxlJc5YIxIfelSfYVaWWf

