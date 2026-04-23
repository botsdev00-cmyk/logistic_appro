--
-- PostgreSQL database dump
--

\restrict BACf1dA57qQRa0F7dAVkD1ExVUYyvJV7ZDc9ZPYCZ02coNFcsjQgWwZVU7NWFcq

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
-- Name: fn_audit_repartition_statut(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_audit_repartition_statut() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
  IF (NEW.statut_repartition_id IS DISTINCT FROM OLD.statut_repartition_id) THEN
    INSERT INTO repartition_audit(repartition_id, utilisateur_id, ancien_statut, nouveau_statut)
    VALUES (NEW.repartition_id, current_setting('my.app.user', true)::uuid, OLD.statut_repartition_id, NEW.statut_repartition_id);
  END IF;
  RETURN NEW;
END; $$;


ALTER FUNCTION public.fn_audit_repartition_statut() OWNER TO postgres;

--
-- Name: fn_audit_stock_movements(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_audit_stock_movements() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    INSERT INTO public.journaux_audit (
        utilisateur_id,
        action,
        type_entite,
        identifiant_entite,
        nouvelles_valeurs,
        date_heure
    ) VALUES (
        NEW.utilisateur_id,
        'MOUVEMENT_STOCK_' || NEW.type_mouvement,
        'STOCK_MOUVEMENT',
        NEW.mouvement_id,
        jsonb_build_object(
            'produit_id', NEW.produit_id,
            'type', NEW.type_mouvement,
            'quantite_delta', NEW.quantite_delta,
            'reference_id', NEW.reference_id,
            'location', NEW.location_id,
            'raison', NEW.raison
        ),
        CURRENT_TIMESTAMP
    );
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.fn_audit_stock_movements() OWNER TO postgres;

--
-- Name: fn_audit_stock_movements_update(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_audit_stock_movements_update() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    INSERT INTO public.journaux_audit (
        utilisateur_id,
        action,
        type_entite,
        identifiant_entite,
        anciennes_valeurs,
        nouvelles_valeurs,
        date_heure
    ) VALUES (
        CURRENT_USER::UUID,
        'MOUVEMENT_STOCK_UPDATE',
        'STOCK_MOUVEMENT',
        NEW.mouvement_id,
        jsonb_build_object(
            'quantite_delta', OLD.quantite_delta,
            'location_id', OLD.location_id,
            'raison', OLD.raison
        ),
        jsonb_build_object(
            'quantite_delta', NEW.quantite_delta,
            'location_id', NEW.location_id,
            'raison', NEW.raison
        ),
        CURRENT_TIMESTAMP
    );
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.fn_audit_stock_movements_update() OWNER TO postgres;

--
-- Name: fn_check_stock_availability(uuid, integer); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_check_stock_availability(p_produit_id uuid, p_quantite_requise integer) RETURNS TABLE(disponible boolean, quantite_actuelle integer, message text)
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_quantite_disponible INTEGER;
BEGIN
    SELECT COALESCE(SUM(sm.quantite_delta), 0)::INTEGER
    INTO v_quantite_disponible
    FROM public.stock_mouvements sm
    WHERE sm.produit_id = p_produit_id
        AND sm.location_id IN ('WAREHOUSE', 'IN_TRANSIT');
    
    RETURN QUERY SELECT
        (v_quantite_disponible >= p_quantite_requise),
        v_quantite_disponible,
        CASE 
            WHEN v_quantite_disponible >= p_quantite_requise 
                THEN 'OK - Stock suffisant'::TEXT
            ELSE ('ERREUR - Stock insuffisant: ' || v_quantite_disponible || '/' || p_quantite_requise)::TEXT
        END;
END;
$$;


ALTER FUNCTION public.fn_check_stock_availability(p_produit_id uuid, p_quantite_requise integer) OWNER TO postgres;

--
-- Name: fn_create_retour_on_retour_stock(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_create_retour_on_retour_stock() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF NEW.statut_validation IN ('APPROUVE', 'EN_ATTENTE') THEN
        INSERT INTO public.stock_mouvements (
            produit_id, type_mouvement, quantite_delta,
            reference_id, reference_type, utilisateur_id,
            location_id, raison, observations
        ) VALUES (
            NEW.produit_id, 'RETOUR', NEW.quantite,
            NEW.retour_stock_id, 'RETOUR_STOCK',
            NEW.cree_par, 'RETURNED',
            'Retour',
            NEW.observations
        );
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.fn_create_retour_on_retour_stock() OWNER TO postgres;

--
-- Name: fn_create_sortie_on_repartition(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_create_sortie_on_repartition() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_utilisateur_id UUID;
BEGIN
    -- Récupérer l'utilisateur (chef) depuis la repartition
    SELECT chef_id INTO v_utilisateur_id
    FROM public.repartitions
    WHERE repartition_id = NEW.repartition_id
    LIMIT 1;

    -- Créer mouvement SORTIE
    IF NEW.quantite_totale > 0 THEN
        INSERT INTO public.stock_mouvements (
            produit_id, type_mouvement, quantite_delta,
            reference_id, reference_type, utilisateur_id,
            location_id, raison
        ) VALUES (
            NEW.produit_id, 'SORTIE', -(NEW.quantite_totale),
            NEW.article_repartition_id, 'ARTICLE_REPARTITION',
            COALESCE(v_utilisateur_id, gen_random_uuid()),
            'IN_TRANSIT', 'Repartition équipe'
        );
    END IF;

    RETURN NEW;
END;
$$;


ALTER FUNCTION public.fn_create_sortie_on_repartition() OWNER TO postgres;

--
-- Name: fn_create_stock_movement(uuid, text, integer, uuid, text, uuid, text, text, text); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_create_stock_movement(p_produit_id uuid, p_type_mouvement text, p_quantite integer, p_reference_id uuid, p_reference_type text, p_utilisateur_id uuid, p_location_id text, p_raison text, p_observations text) RETURNS TABLE(success boolean, mouvement_id uuid, message text, current_stock integer, resulting_stock integer)
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_mvt_id uuid;
    v_current int;
BEGIN
    -- 1. Récupérer le stock actuel (solde avant mouvement)
    SELECT COALESCE(quantite_total, 0) INTO v_current 
    FROM public.stock_soldes 
    WHERE produit_id = p_produit_id;
    
    -- Si le produit n'existe pas encore dans les soldes, on part de 0
    IF NOT FOUND THEN 
        v_current := 0; 
    END IF;

    -- 2. Insertion du mouvement dans la table de traçabilité
    INSERT INTO public.stock_mouvements (
        produit_id, 
        type_mouvement, 
        quantite_delta, 
        reference_id, 
        reference_type, 
        utilisateur_id, 
        location_id, 
        raison, 
        observations
    ) 
    VALUES (
        p_produit_id, 
        p_type_mouvement, 
        p_quantite, 
        p_reference_id, 
        p_reference_type, 
        p_utilisateur_id, 
        p_location_id, 
        p_raison, 
        p_observations
    ) 
    RETURNING public.stock_mouvements.mouvement_id INTO v_mvt_id;

    -- Note : Le trigger 'trg_sync_stock_after_movement' sur la table stock_mouvements
    -- va automatiquement mettre à jour la table stock_soldes.

    -- 3. Retourner les résultats attendus par RepositoryStockMouvements.cpp
    RETURN QUERY 
    SELECT 
        true, 
        v_mvt_id, 
        'Mouvement enregistré avec succès'::text, 
        v_current, 
        v_current + p_quantite;
EXCEPTION
    WHEN OTHERS THEN
        -- En cas d'erreur (ex: violation de contrainte), on retourne un échec proprement
        RETURN QUERY SELECT false, NULL::uuid, SQLERRM::text, 0, 0;
END;
$$;


ALTER FUNCTION public.fn_create_stock_movement(p_produit_id uuid, p_type_mouvement text, p_quantite integer, p_reference_id uuid, p_reference_type text, p_utilisateur_id uuid, p_location_id text, p_raison text, p_observations text) OWNER TO postgres;

--
-- Name: fn_create_stock_movement(uuid, character varying, integer, uuid, character varying, uuid, character varying, character varying, text); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_create_stock_movement(p_produit_id uuid, p_type_mouvement character varying, p_quantite_delta integer, p_reference_id uuid, p_reference_type character varying, p_utilisateur_id uuid, p_location_id character varying DEFAULT 'WAREHOUSE'::character varying, p_raison character varying DEFAULT NULL::character varying, p_observations text DEFAULT NULL::text) RETURNS TABLE(success boolean, mouvement_id uuid, message text)
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_mouvement_id UUID;
    v_verification RECORD;
    v_product_exists BOOLEAN;
BEGIN
    -- Vérifier que le produit existe
    SELECT EXISTS(SELECT 1 FROM public.produits WHERE produit_id = p_produit_id)
    INTO v_product_exists;
    
    IF NOT v_product_exists THEN
        RETURN QUERY SELECT 
            false, 
            NULL::UUID,
            'ERREUR: Produit introuvable';
        RETURN;
    END IF;
    
    -- Vérifier l'opération
    SELECT * INTO v_verification
    FROM public.fn_verify_stock_operation(p_produit_id, p_quantite_delta, p_type_mouvement)
    LIMIT 1;
    
    IF NOT v_verification.can_proceed THEN
        RETURN QUERY SELECT 
            false,
            NULL::UUID,
            v_verification.message;
        RETURN;
    END IF;
    
    -- Créer le mouvement
    INSERT INTO public.stock_mouvements (
        produit_id,
        type_mouvement,
        quantite_delta,
        reference_id,
        reference_type,
        utilisateur_id,
        location_id,
        raison,
        observations
    ) VALUES (
        p_produit_id,
        p_type_mouvement,
        p_quantite_delta,
        p_reference_id,
        p_reference_type,
        p_utilisateur_id,
        p_location_id,
        p_raison,
        p_observations
    ) RETURNING mouvement_id INTO v_mouvement_id;
    
    RETURN QUERY SELECT 
        true,
        v_mouvement_id,
        'Mouvement créé avec succès';
END;
$$;


ALTER FUNCTION public.fn_create_stock_movement(p_produit_id uuid, p_type_mouvement character varying, p_quantite_delta integer, p_reference_id uuid, p_reference_type character varying, p_utilisateur_id uuid, p_location_id character varying, p_raison character varying, p_observations text) OWNER TO postgres;

--
-- Name: fn_get_current_stock(uuid); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_get_current_stock(p_produit_id uuid) RETURNS TABLE(produit_id uuid, quantite_total integer, quantite_warehouse integer, quantite_in_transit integer, quantite_returned integer)
    LANGUAGE plpgsql
    AS $_$
BEGIN
    RETURN QUERY
    SELECT 
        $1,
        COALESCE(SUM(sm.quantite_delta), 0)::INTEGER,
        COALESCE(SUM(CASE WHEN sm.location_id = 'WAREHOUSE' THEN sm.quantite_delta ELSE 0 END), 0)::INTEGER,
        COALESCE(SUM(CASE WHEN sm.location_id = 'IN_TRANSIT' THEN sm.quantite_delta ELSE 0 END), 0)::INTEGER,
        COALESCE(SUM(CASE WHEN sm.location_id = 'RETURNED' THEN sm.quantite_delta ELSE 0 END), 0)::INTEGER
    FROM public.stock_mouvements sm
    WHERE sm.produit_id = $1;
END;
$_$;


ALTER FUNCTION public.fn_get_current_stock(p_produit_id uuid) OWNER TO postgres;

--
-- Name: fn_get_stock_by_location(uuid); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_get_stock_by_location(p_produit_id uuid) RETURNS TABLE(warehouse integer, in_transit integer, returned integer)
    LANGUAGE plpgsql
    AS $$
BEGIN
    RETURN QUERY
    SELECT
        COALESCE(SUM(CASE WHEN sm.location_id = 'WAREHOUSE' THEN sm.quantite_delta ELSE 0 END), 0)::INTEGER,
        COALESCE(SUM(CASE WHEN sm.location_id = 'IN_TRANSIT' THEN sm.quantite_delta ELSE 0 END), 0)::INTEGER,
        COALESCE(SUM(CASE WHEN sm.location_id = 'RETURNED' THEN sm.quantite_delta ELSE 0 END), 0)::INTEGER
    FROM public.stock_mouvements sm
    WHERE sm.produit_id = p_produit_id;
END;
$$;


ALTER FUNCTION public.fn_get_stock_by_location(p_produit_id uuid) OWNER TO postgres;

--
-- Name: fn_log_entree_stock_movement(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_log_entree_stock_movement() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_source_nom text;
    v_mvt_type text;
BEGIN
    SELECT nom INTO v_source_nom 
    FROM public.sources_entree 
    WHERE source_entree_id = NEW.source_entree_id;

    CASE 
        WHEN v_source_nom ILIKE '%Achat%' THEN v_mvt_type := 'ENTREE';
        WHEN v_source_nom ILIKE '%Production%' THEN v_mvt_type := 'ENTREE';
        WHEN v_source_nom ILIKE '%Retour%' THEN v_mvt_type := 'RETOUR';
        ELSE v_mvt_type := 'AJUSTEMENT';
    END CASE;

    INSERT INTO public.stock_mouvements (
        produit_id, 
        type_mouvement, 
        quantite_delta, 
        reference_id, 
        reference_type, 
        utilisateur_id, 
        raison,
        observations,
        source_entree_id
    ) VALUES (
        NEW.produit_id, 
        v_mvt_type, 
        NEW.quantite, 
        NEW.entree_stock_id, 
        'ENTREE_STOCK', 
        NEW.cree_par, 
        'Approvisionnement via ' || v_source_nom,
        'Facture: ' || COALESCE(NEW.numero_facture, 'N/A'),
        NEW.source_entree_id
    );

    RETURN NEW;
END;
$$;


ALTER FUNCTION public.fn_log_entree_stock_movement() OWNER TO postgres;

--
-- Name: fn_prevent_direct_stock_modification(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_prevent_direct_stock_modification() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF pg_trigger_depth() > 0 THEN
        RETURN NEW;
    END IF;
    RAISE EXCEPTION 'Modification directe de stock_soldes interdite. Utiliser les mouvements.';
END;
$$;


ALTER FUNCTION public.fn_prevent_direct_stock_modification() OWNER TO postgres;

--
-- Name: fn_prevent_stock_movements_deletion(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_prevent_stock_movements_deletion() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    RAISE EXCEPTION 'Suppression de mouvements de stock interdite. Les mouvements doivent rester pour l''audit.';
END;
$$;


ALTER FUNCTION public.fn_prevent_stock_movements_deletion() OWNER TO postgres;

--
-- Name: fn_refresh_stock_cache(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_refresh_stock_cache() RETURNS void
    LANGUAGE plpgsql
    AS $$
BEGIN
    REFRESH MATERIALIZED VIEW CONCURRENTLY public.mv_stock_cache;
    RAISE NOTICE 'Cache stock rafraîchi à %', CURRENT_TIMESTAMP;
END;
$$;


ALTER FUNCTION public.fn_refresh_stock_cache() OWNER TO postgres;

--
-- Name: fn_repair_stock_integrity(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_repair_stock_integrity() RETURNS TABLE(produit_id uuid, ancien_total integer, nouveau_total integer)
    LANGUAGE plpgsql
    AS $$
BEGIN
    RETURN QUERY
    UPDATE public.stock_soldes ss
    SET quantite_total = movements_sum.total
    FROM (
        SELECT 
            sm.produit_id,
            COALESCE(SUM(sm.quantite_delta), 0)::INTEGER as total
        FROM public.stock_mouvements sm
        GROUP BY sm.produit_id
    ) movements_sum
    WHERE ss.produit_id = movements_sum.produit_id
        AND ss.quantite_total != movements_sum.total
    RETURNING ss.produit_id, ss.quantite_total, movements_sum.total;
END;
$$;


ALTER FUNCTION public.fn_repair_stock_integrity() OWNER TO postgres;

--
-- Name: fn_repartition_generate_stock_movements(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_repartition_generate_stock_movements() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_chef_id uuid;
    v_count int;
BEGIN
    IF COALESCE(NEW.mouvements_generes, false) THEN
        -- Déjà traité, ne rien faire
        RETURN NEW;
    END IF;

    -- Exécution normale comme plus haut ...

    -- À la fin, noter que c’est fait
    UPDATE repartitions
    SET mouvements_generes = true
    WHERE repartition_id = NEW.repartition_id;

    RETURN NEW;
END; $$;


ALTER FUNCTION public.fn_repartition_generate_stock_movements() OWNER TO postgres;

--
-- Name: fn_sync_stock_after_movement(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_sync_stock_after_movement() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Recalculer le solde
    UPDATE public.stock_soldes ss
    SET 
        quantite_total = (
            SELECT COALESCE(SUM(quantite_delta), 0)
            FROM public.stock_mouvements 
            WHERE produit_id = NEW.produit_id
        ),
        dernier_mouvement_date = CURRENT_TIMESTAMP,
        updated_at = CURRENT_TIMESTAMP
    WHERE ss.produit_id = NEW.produit_id;

    -- Créer solde s'il n'existe pas
    INSERT INTO public.stock_soldes (produit_id, quantite_total, dernier_mouvement_date)
    SELECT NEW.produit_id, COALESCE(SUM(quantite_delta), 0), CURRENT_TIMESTAMP
    FROM public.stock_mouvements
    WHERE produit_id = NEW.produit_id
    ON CONFLICT (produit_id) DO NOTHING;

    RETURN NEW;
END;
$$;


ALTER FUNCTION public.fn_sync_stock_after_movement() OWNER TO postgres;

--
-- Name: fn_sync_stock_soldes_from_movements(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_sync_stock_soldes_from_movements() RETURNS void
    LANGUAGE plpgsql
    AS $$
BEGIN
    -- Vider les soldes existants
    TRUNCATE TABLE public.stock_soldes;
    
    -- Insérer depuis les mouvements
    INSERT INTO public.stock_soldes (
        solde_id,
        produit_id,
        quantite_total,
        quantite_reserve,
        prix_moyen,
        location_id,
        dernier_mouvement_date,
        updated_at
    )
    SELECT
        gen_random_uuid(),
        p.produit_id,
        COALESCE(SUM(sm.quantite_delta), 0),
        0, -- À remplir par logique métier
        p.prix_unitaire,
        'WAREHOUSE',
        MAX(sm.created_at),
        CURRENT_TIMESTAMP
    FROM public.produits p
    LEFT JOIN public.stock_mouvements sm ON p.produit_id = sm.produit_id
    GROUP BY p.produit_id, p.prix_unitaire;
    
    RAISE NOTICE 'Synchronisation stock_soldes terminée';
END;
$$;


ALTER FUNCTION public.fn_sync_stock_soldes_from_movements() OWNER TO postgres;

--
-- Name: fn_verify_stock_operation(uuid, integer, character varying); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.fn_verify_stock_operation(p_produit_id uuid, p_quantite_delta integer, p_type_mouvement character varying) RETURNS TABLE(can_proceed boolean, current_stock integer, resulting_stock integer, message text)
    LANGUAGE plpgsql
    AS $$
DECLARE
    v_current_stock INTEGER;
    v_resulting_stock INTEGER;
BEGIN
    -- Obtenir le stock actuel
    SELECT COALESCE(quantite_total, 0)
    INTO v_current_stock
    FROM public.stock_soldes
    WHERE produit_id = p_produit_id;
    
    -- Calculer le stock résultant
    v_resulting_stock := v_current_stock + p_quantite_delta;
    
    -- Vérifier les règles selon le type de mouvement
    IF p_type_mouvement = 'SORTIE' OR p_type_mouvement = 'RETOUR' THEN
        -- Les sorties/retours ne doivent pas créer de stock négatif
        IF v_resulting_stock < 0 THEN
            RETURN QUERY SELECT 
                false,
                v_current_stock,
                v_resulting_stock,
                'ERREUR: Stock insuffisant. Disponible: ' || v_current_stock || 
                ', Demandé: ' || ABS(p_quantite_delta);
        ELSE
            RETURN QUERY SELECT 
                true,
                v_current_stock,
                v_resulting_stock,
                'OK: Opération autorisée';
        END IF;
    ELSE
        -- Les entrées/ajustements sont autorisés
        RETURN QUERY SELECT 
            true,
            v_current_stock,
            v_resulting_stock,
            'OK: Entrée autorisée';
    END IF;
END;
$$;


ALTER FUNCTION public.fn_verify_stock_operation(p_produit_id uuid, p_quantite_delta integer, p_type_mouvement character varying) OWNER TO postgres;

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
    v_prix_unitaire NUMERIC(12,2);
BEGIN
    -- Total entrées
    SELECT COALESCE(SUM(e.quantite), 0)
    INTO total_entrees
    FROM public.entrees_stock e
    WHERE e.produit_id = NEW.produit_id
      AND e.statut_validation IN ('APPROUVE', 'EN_ATTENTE');

    -- Total sorties
    SELECT COALESCE(SUM(ar.quantite_vente + ar.quantite_cadeau), 0)
    INTO total_sorties
    FROM public.articles_repartition ar
    WHERE ar.produit_id = NEW.produit_id;

    -- Total retours
    SELECT COALESCE(SUM(r.quantite), 0)
    INTO total_retours
    FROM public.retours_stock r
    WHERE r.produit_id = NEW.produit_id
      AND r.statut_validation IN ('APPROUVE', 'EN_ATTENTE');

    -- Prix unitaire (corrigé avec alias + variable différente)
    SELECT p.prix_unitaire
    INTO v_prix_unitaire
    FROM public.produits p
    WHERE p.produit_id = NEW.produit_id;

    -- Upsert
    INSERT INTO public.stock_soldes (
        produit_id,
        quantite_total,
        quantite_reserve,
        prix_moyen,
        dernier_mouvement_date
    )
    VALUES (
        NEW.produit_id,
        total_entrees - total_sorties + total_retours,
        total_sorties,
        v_prix_unitaire,
        CURRENT_TIMESTAMP
    )
    ON CONFLICT (produit_id)
    DO UPDATE SET
        quantite_total = EXCLUDED.quantite_total,
        quantite_reserve = EXCLUDED.quantite_reserve,
        prix_moyen = EXCLUDED.prix_moyen,
        dernier_mouvement_date = CURRENT_TIMESTAMP;

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
    observation text,
    quantite_degustation integer DEFAULT 0,
    quantite_totale integer GENERATED ALWAYS AS (((quantite_vente + quantite_cadeau) + quantite_degustation)) STORED
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
-- Name: stock_mouvements; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.stock_mouvements (
    mouvement_id uuid DEFAULT gen_random_uuid() NOT NULL,
    produit_id uuid NOT NULL,
    type_mouvement character varying(20) NOT NULL,
    quantite_delta integer NOT NULL,
    reference_id uuid,
    reference_type character varying(50),
    utilisateur_id uuid NOT NULL,
    location_id character varying(50) DEFAULT 'WAREHOUSE'::character varying,
    raison character varying(255),
    observations text,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP NOT NULL,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP NOT NULL,
    source_entree_id uuid,
    CONSTRAINT check_location CHECK (((location_id)::text = ANY ((ARRAY['WAREHOUSE'::character varying, 'IN_TRANSIT'::character varying, 'RETURNED'::character varying])::text[]))),
    CONSTRAINT check_quantite_not_zero CHECK ((quantite_delta <> 0)),
    CONSTRAINT check_type_mouvement CHECK (((type_mouvement)::text = ANY ((ARRAY['ENTREE'::character varying, 'SORTIE'::character varying, 'RETOUR'::character varying, 'AJUSTEMENT'::character varying])::text[])))
);


ALTER TABLE public.stock_mouvements OWNER TO postgres;

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
-- Name: mv_stock_cache; Type: MATERIALIZED VIEW; Schema: public; Owner: postgres
--

CREATE MATERIALIZED VIEW public.mv_stock_cache AS
 SELECT p.produit_id,
    p.nom,
    p.code_sku,
    cp.nom AS categorie,
    tp.nom AS type,
    p.stock_minimum,
    COALESCE(sum(sm.quantite_delta), (0)::bigint) AS quantite_total,
    COALESCE(sum(
        CASE
            WHEN ((sm.location_id)::text = 'WAREHOUSE'::text) THEN sm.quantite_delta
            ELSE 0
        END), (0)::bigint) AS quantite_warehouse,
    COALESCE(sum(
        CASE
            WHEN ((sm.location_id)::text = 'IN_TRANSIT'::text) THEN sm.quantite_delta
            ELSE 0
        END), (0)::bigint) AS quantite_in_transit,
    COALESCE(sum(
        CASE
            WHEN ((sm.location_id)::text = 'RETURNED'::text) THEN sm.quantite_delta
            ELSE 0
        END), (0)::bigint) AS quantite_returned,
    max(sm.created_at) AS derniere_modification,
    CURRENT_TIMESTAMP AS cache_updated_at
   FROM (((public.produits p
     LEFT JOIN public.categories_produits cp ON ((p.categorie_produit_id = cp.categorie_produit_id)))
     LEFT JOIN public.types_produits tp ON ((p.type_produit_id = tp.type_produit_id)))
     LEFT JOIN public.stock_mouvements sm ON ((p.produit_id = sm.produit_id)))
  WHERE (p.est_actif = true)
  GROUP BY p.produit_id, p.nom, p.code_sku, cp.nom, tp.nom, p.stock_minimum
  WITH NO DATA;


ALTER MATERIALIZED VIEW public.mv_stock_cache OWNER TO postgres;

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
-- Name: repartition_audit; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.repartition_audit (
    repartition_audit_id uuid DEFAULT gen_random_uuid() NOT NULL,
    repartition_id uuid NOT NULL,
    utilisateur_id uuid,
    date_action timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    ancien_statut uuid,
    nouveau_statut uuid,
    commentaire text
);


ALTER TABLE public.repartition_audit OWNER TO postgres;

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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    chef_id uuid,
    annule boolean DEFAULT false,
    mouvements_generes boolean DEFAULT false
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
    nom character varying(100) NOT NULL,
    description text,
    est_actif boolean DEFAULT true
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
-- Name: stock_locations; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.stock_locations (
    location_id character varying(50) NOT NULL,
    nom character varying(100) NOT NULL,
    description text,
    est_actif boolean DEFAULT true
);


ALTER TABLE public.stock_locations OWNER TO postgres;

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
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    location_id character varying(50) DEFAULT 'WAREHOUSE'::character varying,
    location_historique jsonb DEFAULT '{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}'::jsonb,
    derniere_location_id character varying(50) DEFAULT 'WAREHOUSE'::character varying,
    CONSTRAINT check_quantite_reserve_not_negative CHECK ((quantite_reserve >= 0)),
    CONSTRAINT check_quantite_reserve_positive CHECK ((quantite_reserve >= 0)),
    CONSTRAINT check_quantite_total_not_negative CHECK ((quantite_total >= 0))
);


ALTER TABLE public.stock_soldes OWNER TO postgres;

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
-- Name: v_audit_stock_trail; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_audit_stock_trail AS
 SELECT ja.journal_audit_id,
    ja.date_heure,
    u.nom_complet AS utilisateur,
    ja.action,
    ja.type_entite,
    ja.identifiant_entite,
    ja.anciennes_valeurs,
    ja.nouvelles_valeurs
   FROM (public.journaux_audit ja
     LEFT JOIN public.utilisateurs u ON ((ja.utilisateur_id = u.utilisateur_id)))
  WHERE ((ja.type_entite)::text = ANY ((ARRAY['STOCK_MOUVEMENT'::character varying, 'ENTREE_STOCK'::character varying, 'RETOUR_STOCK'::character varying])::text[]))
  ORDER BY ja.date_heure DESC;


ALTER VIEW public.v_audit_stock_trail OWNER TO postgres;

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
-- Name: v_stock_by_location; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_by_location AS
 SELECT p.produit_id,
    p.nom,
    p.code_sku,
    cp.nom AS categorie,
    (COALESCE(sum(
        CASE
            WHEN ((sm.location_id)::text = 'WAREHOUSE'::text) THEN sm.quantite_delta
            ELSE 0
        END), (0)::bigint))::integer AS warehouse,
    (COALESCE(sum(
        CASE
            WHEN ((sm.location_id)::text = 'IN_TRANSIT'::text) THEN sm.quantite_delta
            ELSE 0
        END), (0)::bigint))::integer AS in_transit,
    (COALESCE(sum(
        CASE
            WHEN ((sm.location_id)::text = 'RETURNED'::text) THEN sm.quantite_delta
            ELSE 0
        END), (0)::bigint))::integer AS returned,
    (COALESCE(sum(sm.quantite_delta), (0)::bigint))::integer AS total
   FROM ((public.produits p
     LEFT JOIN public.categories_produits cp ON ((p.categorie_produit_id = cp.categorie_produit_id)))
     LEFT JOIN public.stock_mouvements sm ON ((p.produit_id = sm.produit_id)))
  WHERE (p.est_actif = true)
  GROUP BY p.produit_id, p.nom, p.code_sku, cp.nom;


ALTER VIEW public.v_stock_by_location OWNER TO postgres;

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
-- Name: v_stock_integrity_check; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_integrity_check AS
 SELECT p.produit_id,
    p.nom,
    p.code_sku,
    ( SELECT COALESCE(sum(stock_mouvements.quantite_delta), (0)::bigint) AS "coalesce"
           FROM public.stock_mouvements
          WHERE (stock_mouvements.produit_id = p.produit_id)) AS stock_from_movements,
    COALESCE(ss.quantite_total, 0) AS stock_from_soldes,
    (( SELECT COALESCE(sum(stock_mouvements.quantite_delta), (0)::bigint) AS "coalesce"
           FROM public.stock_mouvements
          WHERE (stock_mouvements.produit_id = p.produit_id)) - COALESCE(ss.quantite_total, 0)) AS difference,
        CASE
            WHEN (( SELECT COALESCE(sum(stock_mouvements.quantite_delta), (0)::bigint) AS "coalesce"
               FROM public.stock_mouvements
              WHERE (stock_mouvements.produit_id = p.produit_id)) = COALESCE(ss.quantite_total, 0)) THEN '✓ OK'::text
            ELSE '❌ ERREUR'::text
        END AS status
   FROM (public.produits p
     LEFT JOIN public.stock_soldes ss ON ((p.produit_id = ss.produit_id)))
  WHERE (p.est_actif = true)
  ORDER BY p.nom;


ALTER VIEW public.v_stock_integrity_check OWNER TO postgres;

--
-- Name: v_stock_mouvements; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_mouvements AS
 SELECT sm.mouvement_id,
    sm.type_mouvement,
    sm.produit_id,
    p.nom AS produit_nom,
    p.code_sku,
    sm.quantite_delta,
    sm.reference_id,
    sm.reference_type,
    sm.utilisateur_id,
    u.nom_complet AS utilisateur_nom,
    sm.location_id,
    sm.raison,
    sm.observations,
    sm.created_at,
    se.nom AS source
   FROM (((public.stock_mouvements sm
     JOIN public.produits p ON ((sm.produit_id = p.produit_id)))
     JOIN public.utilisateurs u ON ((sm.utilisateur_id = u.utilisateur_id)))
     LEFT JOIN public.sources_entree se ON (((sm.reference_type)::text = 'ENTREE_STOCK'::text)))
  ORDER BY sm.created_at DESC;


ALTER VIEW public.v_stock_mouvements OWNER TO postgres;

--
-- Name: v_stock_movements_audit; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_movements_audit AS
 SELECT sm.mouvement_id,
    sm.created_at,
    p.nom AS produit,
    p.code_sku,
    sm.type_mouvement,
    sm.quantite_delta,
    sm.location_id,
    u.nom_complet AS utilisateur,
    sm.raison,
    sm.reference_type,
    sm.observations,
    ja.action AS audit_action
   FROM (((public.stock_mouvements sm
     JOIN public.produits p ON ((sm.produit_id = p.produit_id)))
     JOIN public.utilisateurs u ON ((sm.utilisateur_id = u.utilisateur_id)))
     LEFT JOIN public.journaux_audit ja ON ((ja.identifiant_entite = sm.mouvement_id)))
  ORDER BY sm.created_at DESC;


ALTER VIEW public.v_stock_movements_audit OWNER TO postgres;

--
-- Name: v_stock_reconciliation; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.v_stock_reconciliation AS
 SELECT p.produit_id,
    p.nom,
    p.code_sku,
    (COALESCE(sum(sm.quantite_delta), (0)::bigint))::integer AS stock_from_movements,
    COALESCE(ss.quantite_total, 0) AS stock_in_soldes,
    ((COALESCE(sum(sm.quantite_delta), (0)::bigint) - COALESCE(ss.quantite_total, 0)))::integer AS difference,
        CASE
            WHEN (COALESCE(sum(sm.quantite_delta), (0)::bigint) = COALESCE(ss.quantite_total, 0)) THEN '✓ OK'::text
            ELSE '❌ DISCREPANCY'::text
        END AS status
   FROM ((public.produits p
     LEFT JOIN public.stock_mouvements sm ON ((p.produit_id = sm.produit_id)))
     LEFT JOIN public.stock_soldes ss ON ((p.produit_id = ss.produit_id)))
  WHERE (p.est_actif = true)
  GROUP BY p.produit_id, p.nom, p.code_sku, ss.quantite_total;


ALTER VIEW public.v_stock_reconciliation OWNER TO postgres;

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

COPY public.articles_repartition (article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau, observation, quantite_degustation) FROM stdin;
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
31fbd8a3-ed3e-4f76-939a-fffd58fda88f	Accessoires	ACC	Accessoires non comestibles	t	0	2026-04-21 12:31:32.155759
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
\.


--
-- Data for Name: equipes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.equipes (equipe_id, nom, nom_chef) FROM stdin;
9a66d049-23a2-4db6-a271-10db9cc28ec0	Alpha	{78a24f11-9714-4014-ae40-f38318fef119}
614554c2-9d3d-4165-8092-10b1fe4cc816	Beta	78a24f11-9714-4014-ae40-f38318fef119
\.


--
-- Data for Name: journaux_audit; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.journaux_audit (journal_audit_id, utilisateur_id, action, type_entite, identifiant_entite, anciennes_valeurs, nouvelles_valeurs, date_heure) FROM stdin;
7f93a79e-d6df-4df8-8754-3dc9b7244bb1	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	bde91567-bacb-4cb6-bbe1-fcfc4c9233a2	\N	{"type": "ENTREE", "raison": "Migration depuis entrees_stock", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "e9ebe02c-d9ad-4f89-b029-02897b4f651b", "quantite_delta": 100}	2026-04-16 13:38:58.619548
55174608-3dbe-441e-b709-2bbb50ca7a9d	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	04267c8b-e20f-4e93-9117-066e7039277b	\N	{"type": "ENTREE", "raison": "Migration depuis entrees_stock", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "45f2d7cc-dd91-4e0f-b65b-1a20d796c584", "quantite_delta": 100}	2026-04-16 13:38:58.619548
3a282db2-e000-477a-882b-b37c332bd0a5	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	87fdca57-c181-40fe-b396-afd9e0896dc1	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "3fce8b76-ac27-4b7c-a1cc-852e20d98a7c", "quantite_delta": 100}	2026-04-20 09:29:46.256518
147bf4ac-6061-44e0-8ec5-ae7ad6cfbff5	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	56979254-dff1-4f2e-8632-8077ab2eb400	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "c269f712-b495-4797-9380-26cb52460c6b", "quantite_delta": 120}	2026-04-20 09:39:52.294
6db5a3dc-1189-4f6a-9a23-f656c7329306	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	221660d3-4111-48f8-b959-b882740c40d0	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "0cbde3a0-2dd5-4345-a957-1dbb43bb08ea", "quantite_delta": 200}	2026-04-20 09:47:50.093985
275e2565-6525-4173-ad6a-4d826783d447	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	0dd0d2ba-4bfa-47e8-8300-cb3918178c3b	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "b6684e40-3795-4d84-905a-39b0cf9b5c3b", "quantite_delta": 420}	2026-04-20 09:47:50.143678
5359d252-73e7-4f13-816e-d9c5a62fe5da	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	794ae862-7f52-44c2-9988-7d75606bfb22	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "ecd53f48-d492-4338-9f54-85625bba6f23", "quantite_delta": -48}	2026-04-21 10:47:02.561101
6f5d8415-7d3e-4532-ac7f-8ef93d6e6b36	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	9721b8ef-bed6-404c-9e66-53e75dbb14ef	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "5058c2e2-506f-42bf-aa45-9105b435f4dc", "reference_id": "38f61680-d234-483a-a5a7-b83294ab1a0d", "quantite_delta": 100}	2026-04-21 12:43:15.444513
c0bf85ec-2686-46f5-822b-ad126f3d09b6	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	8845a8d0-6879-4dc1-9cd2-25079ec25b25	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "d8af351c-4b92-4905-9507-5d1c8d9f644f", "quantite_delta": 1000}	2026-04-21 12:49:01.740701
f02c14b4-005e-402c-b181-c5f0f1c4ca20	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	b0c87cc5-1a70-4d52-b83d-03c7cdf678c8	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "8bef3ca2-bbf4-426a-a718-e387ee8710b3", "quantite_delta": 1000}	2026-04-21 13:08:41.492991
ad502dc7-86b8-42ce-8c24-9a5ef67294d8	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	a6c8c3ac-0043-4455-9399-37148b8e8752	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "87d4019b-504e-4842-8ebd-e99a833f9718", "quantite_delta": 1000}	2026-04-21 13:08:41.51868
3b33d83f-8100-4391-9d91-b35b1762190c	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	243bd881-20fc-4f27-b7ee-7d70d33b3037	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "16b1169c-4934-493e-94f6-0929d3375182", "quantite_delta": 1000}	2026-04-22 15:51:37.850362
a0ad2d8d-f74c-4989-a5bc-51d0c71267c3	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	1733e0c5-13ce-440c-b375-80424ef04fc2	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "ae214634-f241-4719-9bdb-0e624a9f2cca", "quantite_delta": 120}	2026-04-22 16:32:18.437632
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
5058c2e2-506f-42bf-aa45-9105b435f4dc	31fbd8a3-ed3e-4f76-939a-fffd58fda88f	372480eb-993e-4ac0-862f-39466beb1863	Chapeau	CHP	12.00	10	f	2026-04-21 12:32:13.769856	Couleur:\nBlanc\nNoir\nRouge
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
-- Data for Name: repartition_audit; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.repartition_audit (repartition_audit_id, repartition_id, utilisateur_id, date_action, ancien_statut, nouveau_statut, commentaire) FROM stdin;
\.


--
-- Data for Name: repartitions; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.repartitions (repartition_id, equipe_id, route_id, statut_repartition_id, date_repartition, montant_cash_attendu, date_mise_a_jour, chef_id, annule, mouvements_generes) FROM stdin;
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

COPY public.routes (route_id, nom, description, est_actif) FROM stdin;
99c124c9-2e02-40ba-a792-7494bc094fc3	KASAPA-MOISE		t
4465ed8d-443d-4ac2-a13f-468a885367d0	KASAPA-MARCHE MZEE		t
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
d6f4c36d-9951-44ba-b5e1-1e0609968275	BROUILLON	Brouillon
\.


--
-- Data for Name: stock_locations; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.stock_locations (location_id, nom, description, est_actif) FROM stdin;
WAREHOUSE	Entrepôt principal	Stock principal en entrepôt	t
IN_TRANSIT	En transit	Stock envoyé aux équipes	t
RETURNED	Retourné	Stock retourné des équipes	t
\.


--
-- Data for Name: stock_mouvements; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.stock_mouvements (mouvement_id, produit_id, type_mouvement, quantite_delta, reference_id, reference_type, utilisateur_id, location_id, raison, observations, created_at, updated_at, source_entree_id) FROM stdin;
9721b8ef-bed6-404c-9e66-53e75dbb14ef	5058c2e2-506f-42bf-aa45-9105b435f4dc	AJUSTEMENT	100	38f61680-d234-483a-a5a7-b83294ab1a0d	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 12:43:15.444513	2026-04-21 12:43:15.444513	\N
8845a8d0-6879-4dc1-9cd2-25079ec25b25	857f7c59-4ff6-45af-81fc-81d534af18de	AJUSTEMENT	1000	d8af351c-4b92-4905-9507-5d1c8d9f644f	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 12:49:01.740701	2026-04-21 12:49:01.740701	\N
b0c87cc5-1a70-4d52-b83d-03c7cdf678c8	03dc330a-6da1-41ca-86ba-60945244182a	AJUSTEMENT	1000	8bef3ca2-bbf4-426a-a718-e387ee8710b3	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 13:08:41.492991	2026-04-21 13:08:41.492991	\N
a6c8c3ac-0043-4455-9399-37148b8e8752	857f7c59-4ff6-45af-81fc-81d534af18de	AJUSTEMENT	1000	87d4019b-504e-4842-8ebd-e99a833f9718	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 13:08:41.51868	2026-04-21 13:08:41.51868	\N
243bd881-20fc-4f27-b7ee-7d70d33b3037	857f7c59-4ff6-45af-81fc-81d534af18de	AJUSTEMENT	1000	16b1169c-4934-493e-94f6-0929d3375182	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-22 15:51:37.850362	2026-04-22 15:51:37.850362	\N
1733e0c5-13ce-440c-b375-80424ef04fc2	03dc330a-6da1-41ca-86ba-60945244182a	AJUSTEMENT	120	ae214634-f241-4719-9bdb-0e624a9f2cca	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-22 16:32:18.437632	2026-04-22 16:32:18.437632	\N
\.


--
-- Data for Name: stock_soldes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.stock_soldes (solde_id, produit_id, quantite_total, quantite_reserve, valeur_stock, prix_moyen, dernier_mouvement_date, updated_at, location_id, location_historique, derniere_location_id) FROM stdin;
9673da95-9ee8-4266-9380-33a5d6823375	5058c2e2-506f-42bf-aa45-9105b435f4dc	100	0	0.00	\N	2026-04-21 12:43:15.444513	2026-04-21 12:43:15.444513	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE
368115a0-0644-44a4-95c2-8b5168d68cc4	857f7c59-4ff6-45af-81fc-81d534af18de	3000	0	0.00	\N	2026-04-22 15:51:37.850362	2026-04-22 15:51:37.850362	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE
f93fabda-4860-4e2b-a644-ce4dc98b7688	03dc330a-6da1-41ca-86ba-60945244182a	1120	0	0.00	\N	2026-04-22 16:32:18.437632	2026-04-22 16:32:18.437632	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE
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
-- Name: repartition_audit repartition_audit_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartition_audit
    ADD CONSTRAINT repartition_audit_pkey PRIMARY KEY (repartition_audit_id);


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
-- Name: stock_locations stock_locations_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_locations
    ADD CONSTRAINT stock_locations_pkey PRIMARY KEY (location_id);


--
-- Name: stock_mouvements stock_mouvements_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_mouvements
    ADD CONSTRAINT stock_mouvements_pkey PRIMARY KEY (mouvement_id);


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
-- Name: repartitions unq_repartition_unique; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartitions
    ADD CONSTRAINT unq_repartition_unique UNIQUE (date_repartition, equipe_id, route_id);


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
-- Name: idx_entrees_stock_produit_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_produit_date ON public.entrees_stock USING btree (produit_id, date);


--
-- Name: idx_entrees_stock_statut; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_statut ON public.entrees_stock USING btree (statut_validation);


--
-- Name: idx_entrees_stock_statut_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_statut_date ON public.entrees_stock USING btree (statut_validation, date DESC);


--
-- Name: idx_journaux_audit_entite; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_journaux_audit_entite ON public.journaux_audit USING btree (type_entite, identifiant_entite, date_heure);


--
-- Name: idx_journaux_audit_type_entite_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_journaux_audit_type_entite_date ON public.journaux_audit USING btree (type_entite, identifiant_entite DESC, date_heure DESC);


--
-- Name: idx_journaux_audit_utilisateur_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_journaux_audit_utilisateur_date ON public.journaux_audit USING btree (utilisateur_id, date_heure DESC);


--
-- Name: idx_mv_stock_cache_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_mv_stock_cache_produit ON public.mv_stock_cache USING btree (produit_id);


--
-- Name: idx_retours_stock_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_date ON public.retours_stock USING btree (date);


--
-- Name: idx_retours_stock_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_produit ON public.retours_stock USING btree (produit_id);


--
-- Name: idx_retours_stock_produit_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_produit_date ON public.retours_stock USING btree (produit_id, date);


--
-- Name: idx_retours_stock_statut; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_statut ON public.retours_stock USING btree (statut_validation);


--
-- Name: idx_retours_stock_statut_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_statut_date ON public.retours_stock USING btree (statut_validation, date DESC);


--
-- Name: idx_stock_mouvements_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_date ON public.stock_mouvements USING btree (created_at);


--
-- Name: idx_stock_mouvements_location; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_location ON public.stock_mouvements USING btree (location_id);


--
-- Name: idx_stock_mouvements_location_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_location_date ON public.stock_mouvements USING btree (location_id, created_at DESC);


--
-- Name: idx_stock_mouvements_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_produit ON public.stock_mouvements USING btree (produit_id);


--
-- Name: idx_stock_mouvements_produit_date; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_produit_date ON public.stock_mouvements USING btree (produit_id, created_at DESC);


--
-- Name: idx_stock_mouvements_produit_date_desc; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_produit_date_desc ON public.stock_mouvements USING btree (produit_id, created_at DESC) WHERE ((type_mouvement)::text = ANY ((ARRAY['ENTREE'::character varying, 'SORTIE'::character varying, 'RETOUR'::character varying])::text[]));


--
-- Name: idx_stock_mouvements_reference; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_reference ON public.stock_mouvements USING btree (reference_id);


--
-- Name: idx_stock_soldes_location; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_soldes_location ON public.stock_soldes USING btree (location_id);


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
-- Name: repartitions trg_audit_repartition_statut; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_audit_repartition_statut AFTER UPDATE ON public.repartitions FOR EACH ROW EXECUTE FUNCTION public.fn_audit_repartition_statut();


--
-- Name: stock_mouvements trg_audit_stock_movements_insert; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_audit_stock_movements_insert AFTER INSERT ON public.stock_mouvements FOR EACH ROW EXECUTE FUNCTION public.fn_audit_stock_movements();


--
-- Name: repartitions trg_generate_stock_on_edit_repartition; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_generate_stock_on_edit_repartition AFTER UPDATE ON public.repartitions FOR EACH ROW WHEN ((old.statut_repartition_id IS DISTINCT FROM new.statut_repartition_id)) EXECUTE FUNCTION public.fn_repartition_generate_stock_movements();


--
-- Name: entrees_stock trg_log_entree_stock_movement; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_log_entree_stock_movement AFTER INSERT OR UPDATE ON public.entrees_stock FOR EACH ROW EXECUTE FUNCTION public.fn_log_entree_stock_movement();


--
-- Name: stock_soldes trg_prevent_direct_stock_soldes_modification; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_prevent_direct_stock_soldes_modification BEFORE UPDATE ON public.stock_soldes FOR EACH ROW EXECUTE FUNCTION public.fn_prevent_direct_stock_modification();


--
-- Name: stock_mouvements trg_prevent_stock_movements_delete; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_prevent_stock_movements_delete BEFORE DELETE ON public.stock_mouvements FOR EACH ROW EXECUTE FUNCTION public.fn_prevent_stock_movements_deletion();


--
-- Name: retours_stock trg_retour_on_retour_stock; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_retour_on_retour_stock AFTER INSERT OR UPDATE ON public.retours_stock FOR EACH ROW EXECUTE FUNCTION public.fn_create_retour_on_retour_stock();


--
-- Name: retours_stock trg_retours_stock_sync; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_retours_stock_sync AFTER INSERT OR UPDATE ON public.retours_stock FOR EACH ROW EXECUTE FUNCTION public.trg_sync_stock_soldes();


--
-- Name: articles_repartition trg_sortie_on_articles_repartition; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sortie_on_articles_repartition AFTER INSERT ON public.articles_repartition FOR EACH ROW EXECUTE FUNCTION public.fn_create_sortie_on_repartition();


--
-- Name: stock_mouvements trg_sync_stock_after_mouvement; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_stock_after_mouvement AFTER INSERT ON public.stock_mouvements FOR EACH ROW EXECUTE FUNCTION public.fn_sync_stock_after_movement();


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
-- Name: stock_mouvements fk_stock_mouvements_produit; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_mouvements
    ADD CONSTRAINT fk_stock_mouvements_produit FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


--
-- Name: stock_mouvements fk_stock_mouvements_utilisateur; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_mouvements
    ADD CONSTRAINT fk_stock_mouvements_utilisateur FOREIGN KEY (utilisateur_id) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: stock_mouvements fk_stock_mvt_source_entree; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_mouvements
    ADD CONSTRAINT fk_stock_mvt_source_entree FOREIGN KEY (source_entree_id) REFERENCES public.sources_entree(source_entree_id);


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
-- Name: repartition_audit repartition_audit_ancien_statut_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartition_audit
    ADD CONSTRAINT repartition_audit_ancien_statut_fkey FOREIGN KEY (ancien_statut) REFERENCES public.statuts_repartition(statut_repartition_id);


--
-- Name: repartition_audit repartition_audit_nouveau_statut_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartition_audit
    ADD CONSTRAINT repartition_audit_nouveau_statut_fkey FOREIGN KEY (nouveau_statut) REFERENCES public.statuts_repartition(statut_repartition_id);


--
-- Name: repartition_audit repartition_audit_repartition_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartition_audit
    ADD CONSTRAINT repartition_audit_repartition_id_fkey FOREIGN KEY (repartition_id) REFERENCES public.repartitions(repartition_id);


--
-- Name: repartitions repartitions_chef_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.repartitions
    ADD CONSTRAINT repartitions_chef_id_fkey FOREIGN KEY (chef_id) REFERENCES public.utilisateurs(utilisateur_id);


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
-- Name: FUNCTION fn_check_stock_availability(p_produit_id uuid, p_quantite_requise integer); Type: ACL; Schema: public; Owner: postgres
--

GRANT ALL ON FUNCTION public.fn_check_stock_availability(p_produit_id uuid, p_quantite_requise integer) TO stock_editor;


--
-- Name: FUNCTION fn_create_stock_movement(p_produit_id uuid, p_type_mouvement character varying, p_quantite_delta integer, p_reference_id uuid, p_reference_type character varying, p_utilisateur_id uuid, p_location_id character varying, p_raison character varying, p_observations text); Type: ACL; Schema: public; Owner: postgres
--

GRANT ALL ON FUNCTION public.fn_create_stock_movement(p_produit_id uuid, p_type_mouvement character varying, p_quantite_delta integer, p_reference_id uuid, p_reference_type character varying, p_utilisateur_id uuid, p_location_id character varying, p_raison character varying, p_observations text) TO stock_editor;


--
-- Name: FUNCTION fn_refresh_stock_cache(); Type: ACL; Schema: public; Owner: postgres
--

GRANT ALL ON FUNCTION public.fn_refresh_stock_cache() TO stock_approver;


--
-- Name: FUNCTION fn_verify_stock_operation(p_produit_id uuid, p_quantite_delta integer, p_type_mouvement character varying); Type: ACL; Schema: public; Owner: postgres
--

GRANT ALL ON FUNCTION public.fn_verify_stock_operation(p_produit_id uuid, p_quantite_delta integer, p_type_mouvement character varying) TO stock_editor;


--
-- Name: TABLE articles_repartition; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.articles_repartition TO stock_viewer;


--
-- Name: TABLE categories_produits; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.categories_produits TO stock_viewer;


--
-- Name: TABLE clients; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.clients TO stock_viewer;


--
-- Name: TABLE conditions_paiement; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.conditions_paiement TO stock_viewer;


--
-- Name: TABLE credits; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.credits TO stock_viewer;


--
-- Name: TABLE entrees_stock; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.entrees_stock TO stock_viewer;
GRANT SELECT,INSERT ON TABLE public.entrees_stock TO stock_editor;
GRANT SELECT,UPDATE ON TABLE public.entrees_stock TO stock_approver;


--
-- Name: TABLE equipes; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.equipes TO stock_viewer;


--
-- Name: TABLE journaux_audit; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.journaux_audit TO stock_viewer;


--
-- Name: TABLE produits; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.produits TO stock_viewer;


--
-- Name: TABLE stock_mouvements; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.stock_mouvements TO stock_viewer;
GRANT SELECT,INSERT ON TABLE public.stock_mouvements TO stock_editor;


--
-- Name: TABLE types_produits; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.types_produits TO stock_viewer;


--
-- Name: TABLE mv_stock_cache; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.mv_stock_cache TO stock_viewer;


--
-- Name: TABLE permissions; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.permissions TO stock_viewer;


--
-- Name: TABLE raisons_retour; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.raisons_retour TO stock_viewer;


--
-- Name: TABLE receptions_caisse; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.receptions_caisse TO stock_viewer;


--
-- Name: TABLE repartitions; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.repartitions TO stock_viewer;


--
-- Name: TABLE retours_stock; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.retours_stock TO stock_viewer;
GRANT SELECT,INSERT ON TABLE public.retours_stock TO stock_editor;
GRANT SELECT,UPDATE ON TABLE public.retours_stock TO stock_approver;


--
-- Name: TABLE role_permissions; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.role_permissions TO stock_viewer;


--
-- Name: TABLE roles; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.roles TO stock_viewer;


--
-- Name: TABLE routes; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.routes TO stock_viewer;


--
-- Name: TABLE sources_entree; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.sources_entree TO stock_viewer;


--
-- Name: TABLE statuts_caisse; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.statuts_caisse TO stock_viewer;


--
-- Name: TABLE statuts_credit; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.statuts_credit TO stock_viewer;


--
-- Name: TABLE statuts_repartition; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.statuts_repartition TO stock_viewer;


--
-- Name: TABLE stock_locations; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.stock_locations TO stock_viewer;


--
-- Name: TABLE stock_soldes; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.stock_soldes TO stock_viewer;


--
-- Name: TABLE types_vente; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.types_vente TO stock_viewer;


--
-- Name: TABLE utilisateurs; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.utilisateurs TO stock_viewer;


--
-- Name: TABLE v_audit_stock_trail; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.v_audit_stock_trail TO stock_viewer;


--
-- Name: TABLE v_statut_stock; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.v_statut_stock TO stock_viewer;


--
-- Name: TABLE v_stock_detail; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.v_stock_detail TO stock_viewer;


--
-- Name: TABLE v_stock_integrity_check; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.v_stock_integrity_check TO stock_viewer;


--
-- Name: TABLE v_stock_mouvements; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.v_stock_mouvements TO stock_viewer;


--
-- Name: TABLE ventes; Type: ACL; Schema: public; Owner: postgres
--

GRANT SELECT ON TABLE public.ventes TO stock_viewer;


--
-- Name: mv_stock_cache; Type: MATERIALIZED VIEW DATA; Schema: public; Owner: postgres
--

REFRESH MATERIALIZED VIEW public.mv_stock_cache;


--
-- PostgreSQL database dump complete
--

\unrestrict BACf1dA57qQRa0F7dAVkD1ExVUYyvJV7ZDc9ZPYCZ02coNFcsjQgWwZVU7NWFcq

