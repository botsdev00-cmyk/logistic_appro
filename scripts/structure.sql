--
-- PostgreSQL database dump
--

\restrict aE8qyv5pQgcadRRB3Gtzm7FMgAh8h3SfFvfrSAxdp2usU6SYaXjreHWE1E5sycn

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
-- Name: sync_state; Type: TYPE; Schema: public; Owner: postgres
--

CREATE TYPE public.sync_state AS ENUM (
    'PENDING',
    'SYNCED',
    'CONFLICT'
);


ALTER TYPE public.sync_state OWNER TO postgres;

--
-- Name: validation_state; Type: TYPE; Schema: public; Owner: postgres
--

CREATE TYPE public.validation_state AS ENUM (
    'EN_ATTENTE',
    'APPROUVE',
    'REJETE'
);


ALTER TYPE public.validation_state OWNER TO postgres;

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
DECLARE
    v_location_destination TEXT := 'RETURNED'; -- Par défaut pour les avaries/pertes
    v_code_raison TEXT;
BEGIN
    -- Récupérer le code de la raison pour décider de la destination
    SELECT code INTO v_code_raison FROM raisons_retour WHERE raison_retour_id = NEW.raison_retour_id;

    -- Si la raison n'est pas une avarie (ex: SURPLUS, NON_VENDU), on remet en stock global
    IF v_code_raison NOT IN ('AVARIE', 'PERTE', 'ENDOMMAGE') THEN
        v_location_destination := 'WAREHOUSE';
    END IF;

    IF NEW.statut_validation IN ('APPROUVE', 'EN_ATTENTE') THEN
        INSERT INTO public.stock_mouvements (
            produit_id, type_mouvement, quantite_delta,
            reference_id, reference_type, utilisateur_id,
            location_id, raison, observations
        ) VALUES (
            NEW.produit_id, 'RETOUR', NEW.quantite,
            NEW.retour_stock_id, 'RETOUR_STOCK',
            NEW.cree_par, v_location_destination,
            'Retour ' || v_code_raison,
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
-- Name: forbid_delete_physical(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.forbid_delete_physical() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    RAISE EXCEPTION 'Suppression physique interdite: soft-delete uniquement!';
END; $$;


ALTER FUNCTION public.forbid_delete_physical() OWNER TO postgres;

--
-- Name: trg_cuve_statut_apres_embouteillage(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.trg_cuve_statut_apres_embouteillage() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    IF NEW.statut_lot = 'EMBOUTEILLE' AND (OLD.statut_lot IS DISTINCT FROM 'EMBOUTEILLE') THEN
        UPDATE public.cuves
        SET statut = 'VIDE', date_mise_a_jour = CURRENT_TIMESTAMP
        WHERE cuve_id = NEW.cuve_id;
    END IF;
    RETURN NEW;
END;
$$;


ALTER FUNCTION public.trg_cuve_statut_apres_embouteillage() OWNER TO postgres;

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
-- Name: trg_maj_sync_version(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.trg_maj_sync_version() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    IF (TG_OP = 'UPDATE') THEN
        NEW.version = OLD.version + 1;
        NEW.sync_status = 'PENDING';
    END IF;
    RETURN NEW;
END; $$;


ALTER FUNCTION public.trg_maj_sync_version() OWNER TO postgres;

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
-- Name: analyses_labo; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.analyses_labo (
    analyse_id uuid DEFAULT gen_random_uuid() NOT NULL,
    lot_id uuid NOT NULL,
    taux_alcool numeric(5,2),
    ph numeric(4,2),
    taux_sucre numeric(5,2),
    conforme_normes boolean DEFAULT false,
    date_analyse timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    technicien_id uuid,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.analyses_labo OWNER TO postgres;

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
    quantite_totale integer GENERATED ALWAYS AS (((quantite_vente + quantite_cadeau) + quantite_degustation)) STORED,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    grille_id uuid
);


ALTER TABLE public.clients OWNER TO postgres;

--
-- Name: commandes_achats; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.commandes_achats (
    commande_id uuid DEFAULT gen_random_uuid() NOT NULL,
    fournisseur_id uuid NOT NULL,
    date_commande timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    statut character varying(20) DEFAULT 'BROUILLON'::character varying,
    cree_par uuid,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT commandes_achats_statut_check CHECK (((statut)::text = ANY ((ARRAY['BROUILLON'::character varying, 'ENVOYEE'::character varying, 'RECEPTION_PARTIELLE'::character varying, 'CLOTUREE'::character varying])::text[])))
);


ALTER TABLE public.commandes_achats OWNER TO postgres;

--
-- Name: conditions_paiement; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.conditions_paiement (
    condition_paiement_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL,
    delai_jours integer DEFAULT 0,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.credits OWNER TO postgres;

--
-- Name: cuves; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.cuves (
    cuve_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code_cuve character varying(20) NOT NULL,
    capacite_litres numeric(10,2) NOT NULL,
    statut character varying(20) DEFAULT 'VIDE'::character varying,
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT cuves_statut_check CHECK (((statut)::text = ANY ((ARRAY['VIDE'::character varying, 'EN_FERMENTATION'::character varying, 'EN_REPOS'::character varying, 'MAINTENANCE'::character varying])::text[])))
);


ALTER TABLE public.cuves OWNER TO postgres;

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
    cree_par_updated uuid,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.entrees_stock OWNER TO postgres;

--
-- Name: equipes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.equipes (
    equipe_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(100) NOT NULL,
    nom_chef character varying(100) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.equipes OWNER TO postgres;

--
-- Name: fournisseurs; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.fournisseurs (
    fournisseur_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom_entreprise character varying(100) NOT NULL,
    contact_nom character varying(100),
    telephone character varying(50),
    email character varying(100),
    adresse text,
    est_actif boolean DEFAULT true,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.fournisseurs OWNER TO postgres;

--
-- Name: grilles_tarifaires; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.grilles_tarifaires (
    grille_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(50) NOT NULL,
    est_actif boolean DEFAULT true,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.grilles_tarifaires OWNER TO postgres;

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
    date_heure timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.journaux_audit OWNER TO postgres;

--
-- Name: lots_production; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.lots_production (
    lot_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code_lot character varying(50) NOT NULL,
    produit_id uuid NOT NULL,
    cuve_id uuid,
    volume_produit numeric(10,2),
    date_debut timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    date_fin timestamp without time zone,
    statut_lot character varying(20) DEFAULT 'EN_COURS'::character varying,
    notes_vinification text,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT lots_production_statut_lot_check CHECK (((statut_lot)::text = ANY ((ARRAY['EN_COURS'::character varying, 'EMBOUTEILLE'::character varying, 'ANNULE'::character varying])::text[])))
);


ALTER TABLE public.lots_production OWNER TO postgres;

--
-- Name: matieres_premieres; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.matieres_premieres (
    matiere_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code_matiere character varying(20) NOT NULL,
    nom character varying(80) NOT NULL,
    unite character varying(20),
    seuil_alerte numeric(10,2),
    description text,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.matieres_premieres OWNER TO postgres;

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
    description text,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    is_deleted boolean DEFAULT false,
    lot_id uuid,
    source_module character varying(50),
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
    nom character varying(50) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    module character varying(50) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.permissions OWNER TO postgres;

--
-- Name: raisons_retour; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.raisons_retour (
    raison_retour_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    commentaire text,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    mouvements_generes boolean DEFAULT false,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    nom character varying(50) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.roles OWNER TO postgres;

--
-- Name: routes; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.routes (
    route_id uuid DEFAULT gen_random_uuid() NOT NULL,
    nom character varying(100) NOT NULL,
    description text,
    est_actif boolean DEFAULT true,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.routes OWNER TO postgres;

--
-- Name: sources_entree; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sources_entree (
    source_entree_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    nom character varying(50) NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    est_actif boolean DEFAULT true,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    is_deleted boolean DEFAULT false,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT check_quantite_reserve_not_negative CHECK ((quantite_reserve >= 0)),
    CONSTRAINT check_quantite_reserve_positive CHECK ((quantite_reserve >= 0)),
    CONSTRAINT check_quantite_total_not_negative CHECK ((quantite_total >= 0))
);


ALTER TABLE public.stock_soldes OWNER TO postgres;

--
-- Name: tarifs_produits; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.tarifs_produits (
    tarif_id uuid DEFAULT gen_random_uuid() NOT NULL,
    grille_id uuid,
    produit_id uuid,
    prix_personnalise numeric(12,2) NOT NULL,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.tarifs_produits OWNER TO postgres;

--
-- Name: types_vente; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.types_vente (
    type_vente_id uuid DEFAULT gen_random_uuid() NOT NULL,
    code character varying(20) NOT NULL,
    nom character varying(50) NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_mise_a_jour timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
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
    date_vente timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    sync_status public.sync_state DEFAULT 'PENDING'::public.sync_state,
    version integer DEFAULT 1,
    deleted_at timestamp without time zone,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    updated_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP
);


ALTER TABLE public.ventes OWNER TO postgres;

--
-- Data for Name: analyses_labo; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.analyses_labo (analyse_id, lot_id, taux_alcool, ph, taux_sucre, conforme_normes, date_analyse, technicien_id, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: articles_repartition; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.articles_repartition (article_repartition_id, repartition_id, produit_id, quantite_vente, quantite_cadeau, observation, quantite_degustation, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
d6df6513-d313-454b-8804-ceb8dd9c7845	ed8bf7e0-8fb6-42c9-9faa-0b50a7ef659e	857f7c59-4ff6-45af-81fc-81d534af18de	25	5	\N	2	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
ab56adec-069a-45ad-86d4-e5d0201e75d0	72a8c388-3d8d-448e-8d4e-6e7e999dce9b	857f7c59-4ff6-45af-81fc-81d534af18de	50	25	\N	7	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
e1df0a80-b697-46f8-ba17-80896d82158b	72a8c388-3d8d-448e-8d4e-6e7e999dce9b	03dc330a-6da1-41ca-86ba-60945244182a	65	7	\N	3	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
22c58511-dcdd-42df-b6cb-4b04dd283c75	b6cd0cc6-d35a-4459-94fc-b3d0f75f983e	857f7c59-4ff6-45af-81fc-81d534af18de	80	20	\N	10	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
7d5847cf-ff20-4a05-8f18-60094af1cd47	b6cd0cc6-d35a-4459-94fc-b3d0f75f983e	03dc330a-6da1-41ca-86ba-60945244182a	80	20	\N	10	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
0620fa90-8436-43dd-be88-158c7b0e282d	78a127f8-9363-489a-8c55-33efa47b6880	857f7c59-4ff6-45af-81fc-81d534af18de	10	5	\N	0	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
a0f3c34f-d43a-469c-bce6-9e8916d8fff2	040ea0ee-f7ad-4e7a-b73a-2dfdaadd472c	857f7c59-4ff6-45af-81fc-81d534af18de	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
4edb0d87-4124-4444-ab82-10e4063f12d2	9f32890c-eae9-472d-995a-17c558dae8d4	857f7c59-4ff6-45af-81fc-81d534af18de	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
68c5e4c5-e414-4361-996e-d3af17471491	f7e5acf1-0475-4ff2-8041-1aa6a9e0a08a	857f7c59-4ff6-45af-81fc-81d534af18de	12	8	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
c9913500-c247-4548-a00b-fd76ece5a1c8	e8a75968-4626-4769-b5f6-2a5200aa7663	857f7c59-4ff6-45af-81fc-81d534af18de	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
6da663a2-bf1d-4ee4-8c38-043fc826dec2	9571c0a8-b3ca-4698-b334-dbefbc474511	857f7c59-4ff6-45af-81fc-81d534af18de	35	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
a1177137-e433-4c06-83c1-86f274c058d5	b6447295-d74c-4010-80cd-885e6c40021a	857f7c59-4ff6-45af-81fc-81d534af18de	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
948bdcd0-49cd-4bee-bc03-73be12393e22	2e46ff74-a9f5-4842-894c-c4331f221242	857f7c59-4ff6-45af-81fc-81d534af18de	50	5	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
1f1dbd7f-60d1-4203-8a48-5d0be798193c	6b06cbbd-4e77-4dde-b294-ac0ed34513b5	857f7c59-4ff6-45af-81fc-81d534af18de	25	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
5fd464cf-ac73-4bb5-8071-f26d2eef614d	c5bcbf38-8256-4aba-9af5-06a8f1bc82a4	03dc330a-6da1-41ca-86ba-60945244182a	20	5	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
fd2ee6b3-0855-4d76-8ca9-403e72c739a1	1aa032bf-704f-4d1e-ac0b-5c2553129ebc	857f7c59-4ff6-45af-81fc-81d534af18de	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
39129e68-23c7-4a25-a45a-2f959085d175	85875f9b-3b0f-42a4-80ac-5cc89f36357b	857f7c59-4ff6-45af-81fc-81d534af18de	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
b3bb2b93-0694-4519-991d-f4ceab43e10a	c007eff7-e557-4369-a534-ca3b01652221	03dc330a-6da1-41ca-86ba-60945244182a	50	10	\N	5	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
64c8ab8d-3d20-46fc-be04-a57850738863	5071bf90-d610-404f-a14f-6364881c3f53	857f7c59-4ff6-45af-81fc-81d534af18de	50	2	\N	0	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
8a42b87c-9f41-4e8b-b3a9-27592818d783	5071bf90-d610-404f-a14f-6364881c3f53	03dc330a-6da1-41ca-86ba-60945244182a	30	2	\N	0	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
3a82ecdc-9be7-4775-b7ef-e5c2de881592	20d5464d-478c-4b45-b717-fee8e4be07a9	f547b468-aee0-4109-acde-7ebb93d207d8	50	2	\N	3	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
c1245d72-233f-44bd-93e5-d9af30564cda	20d5464d-478c-4b45-b717-fee8e4be07a9	857f7c59-4ff6-45af-81fc-81d534af18de	29	2	\N	4	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
e3a88637-ae66-48b9-bf94-74a09aea8b24	20d5464d-478c-4b45-b717-fee8e4be07a9	03dc330a-6da1-41ca-86ba-60945244182a	12	2	\N	1	PENDING	1	\N	2026-05-07 12:51:20.643641	2026-05-07 12:51:20.643641
\.


--
-- Data for Name: categories_produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.categories_produits (categorie_produit_id, nom, code_categorie, description, est_actif, ordre_affichage, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
53cd8cb8-c163-43a4-8864-5ea57153dbad	Soft Drinks	SOFT	Boissons non alcoolisées	t	4	2026-04-14 18:23:07.500727	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
41414f5b-0968-4d4c-a71b-26473a0abce4	Alimentaire	ALIM	Aliments - Test	f	0	2026-04-14 16:36:42.488393	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
df14e1fd-7f15-4f4e-8bce-d8ab5fda2282	Vins Test	VNS	Test category	f	1	2026-04-14 17:06:08.93193	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
357241c2-6e79-4952-b103-50e1b52b3f96	Vins	VINS	Vins alcooliques	t	1	2026-04-14 17:06:50.960208	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
a84c2258-5abd-4d9b-b103-c3b3d2f8460c	Bières	BIERES	Bières	t	2	2026-04-14 17:07:06.559007	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
98559eb1-06d7-42c0-b6f1-c31197d4e212	Spiritueux	SPIRITUEUX	Spiritueux premium et liqueurs	t	3	2026-04-14 17:07:22.832042	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
31fbd8a3-ed3e-4f76-939a-fffd58fda88f	Accessoires	ACC	Accessoires non comestibles	t	0	2026-04-21 12:31:32.155759	PENDING	1	\N	2026-05-07 12:44:02.59207	2026-05-07 12:44:02.59207
\.


--
-- Data for Name: clients; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.clients (client_id, nom, route_id, condition_paiement_id, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at, grille_id) FROM stdin;
\.


--
-- Data for Name: commandes_achats; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.commandes_achats (commande_id, fournisseur_id, date_commande, statut, cree_par, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: conditions_paiement; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.conditions_paiement (condition_paiement_id, code, nom, delai_jours, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
7a10dde0-a3cb-4004-9c7a-c8f917a28e58	CASH	Paiement cash	0	PENDING	1	\N	2026-05-07 12:45:58.474059	2026-05-07 12:45:58.474059
2eb63626-b12e-4b39-8956-97e8ef05f02d	CREDIT_7J	Crédit 7 jours	7	PENDING	1	\N	2026-05-07 12:45:58.474059	2026-05-07 12:45:58.474059
bea42eb8-e1ac-468b-8dce-14d48ad57eb0	CREDIT_15J	Crédit 15 jours	15	PENDING	1	\N	2026-05-07 12:45:58.474059	2026-05-07 12:45:58.474059
37d3f356-1b26-40c8-a67d-580c360e6108	CREDIT_30J	Crédit 30 jours	30	PENDING	1	\N	2026-05-07 12:45:58.474059	2026-05-07 12:45:58.474059
\.


--
-- Data for Name: credits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.credits (credit_id, vente_id, client_id, montant, date_echeance, statut_credit_id, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: cuves; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.cuves (cuve_id, code_cuve, capacite_litres, statut, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: entrees_stock; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.entrees_stock (entree_stock_id, produit_id, quantite, source_entree_id, date, cree_par, numero_facture, prix_unitaire, numero_lot, date_expiration, approuve_par, statut_validation, date_mise_a_jour, cree_par_updated, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: equipes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.equipes (equipe_id, nom, nom_chef, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
9a66d049-23a2-4db6-a271-10db9cc28ec0	Alpha	{78a24f11-9714-4014-ae40-f38318fef119}	PENDING	1	\N	2026-05-07 12:44:31.049984	2026-05-07 12:44:31.049984
614554c2-9d3d-4165-8092-10b1fe4cc816	Beta	78a24f11-9714-4014-ae40-f38318fef119	PENDING	1	\N	2026-05-07 12:44:31.049984	2026-05-07 12:44:31.049984
923a217f-6939-49ab-b54a-c25844713d88	Charlie	78a24f11-9714-4014-ae40-f38318fef119	PENDING	1	\N	2026-05-07 12:44:31.049984	2026-05-07 12:44:31.049984
67f0aef1-dbf3-4037-a853-52f77b15dddf	Delta	78a24f11-9714-4014-ae40-f38318fef119	PENDING	1	\N	2026-05-07 12:44:31.049984	2026-05-07 12:44:31.049984
5fa8cb53-b61e-4be1-8c8e-2a45fd2072d8	Echo	78a24f11-9714-4014-ae40-f38318fef119	PENDING	1	\N	2026-05-07 12:44:31.049984	2026-05-07 12:44:31.049984
\.


--
-- Data for Name: fournisseurs; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.fournisseurs (fournisseur_id, nom_entreprise, contact_nom, telephone, email, adresse, est_actif, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: grilles_tarifaires; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.grilles_tarifaires (grille_id, nom, est_actif, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: journaux_audit; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.journaux_audit (journal_audit_id, utilisateur_id, action, type_entite, identifiant_entite, anciennes_valeurs, nouvelles_valeurs, date_heure, created_at) FROM stdin;
7f93a79e-d6df-4df8-8754-3dc9b7244bb1	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	bde91567-bacb-4cb6-bbe1-fcfc4c9233a2	\N	{"type": "ENTREE", "raison": "Migration depuis entrees_stock", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "e9ebe02c-d9ad-4f89-b029-02897b4f651b", "quantite_delta": 100}	2026-04-16 13:38:58.619548	2026-05-07 12:48:52.572109
55174608-3dbe-441e-b709-2bbb50ca7a9d	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	04267c8b-e20f-4e93-9117-066e7039277b	\N	{"type": "ENTREE", "raison": "Migration depuis entrees_stock", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "45f2d7cc-dd91-4e0f-b65b-1a20d796c584", "quantite_delta": 100}	2026-04-16 13:38:58.619548	2026-05-07 12:48:52.572109
3a282db2-e000-477a-882b-b37c332bd0a5	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	87fdca57-c181-40fe-b396-afd9e0896dc1	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "3fce8b76-ac27-4b7c-a1cc-852e20d98a7c", "quantite_delta": 100}	2026-04-20 09:29:46.256518	2026-05-07 12:48:52.572109
147bf4ac-6061-44e0-8ec5-ae7ad6cfbff5	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	56979254-dff1-4f2e-8632-8077ab2eb400	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "c269f712-b495-4797-9380-26cb52460c6b", "quantite_delta": 120}	2026-04-20 09:39:52.294	2026-05-07 12:48:52.572109
6db5a3dc-1189-4f6a-9a23-f656c7329306	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	221660d3-4111-48f8-b959-b882740c40d0	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "0cbde3a0-2dd5-4345-a957-1dbb43bb08ea", "quantite_delta": 200}	2026-04-20 09:47:50.093985	2026-05-07 12:48:52.572109
275e2565-6525-4173-ad6a-4d826783d447	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	0dd0d2ba-4bfa-47e8-8300-cb3918178c3b	\N	{"type": "ENTREE", "raison": "Approvisionnement", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "b6684e40-3795-4d84-905a-39b0cf9b5c3b", "quantite_delta": 420}	2026-04-20 09:47:50.143678	2026-05-07 12:48:52.572109
5359d252-73e7-4f13-816e-d9c5a62fe5da	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	794ae862-7f52-44c2-9988-7d75606bfb22	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "ecd53f48-d492-4338-9f54-85625bba6f23", "quantite_delta": -48}	2026-04-21 10:47:02.561101	2026-05-07 12:48:52.572109
6f5d8415-7d3e-4532-ac7f-8ef93d6e6b36	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	9721b8ef-bed6-404c-9e66-53e75dbb14ef	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "5058c2e2-506f-42bf-aa45-9105b435f4dc", "reference_id": "38f61680-d234-483a-a5a7-b83294ab1a0d", "quantite_delta": 100}	2026-04-21 12:43:15.444513	2026-05-07 12:48:52.572109
c0bf85ec-2686-46f5-822b-ad126f3d09b6	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	8845a8d0-6879-4dc1-9cd2-25079ec25b25	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "d8af351c-4b92-4905-9507-5d1c8d9f644f", "quantite_delta": 1000}	2026-04-21 12:49:01.740701	2026-05-07 12:48:52.572109
f02c14b4-005e-402c-b181-c5f0f1c4ca20	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	b0c87cc5-1a70-4d52-b83d-03c7cdf678c8	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "8bef3ca2-bbf4-426a-a718-e387ee8710b3", "quantite_delta": 1000}	2026-04-21 13:08:41.492991	2026-05-07 12:48:52.572109
ad502dc7-86b8-42ce-8c24-9a5ef67294d8	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	a6c8c3ac-0043-4455-9399-37148b8e8752	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "87d4019b-504e-4842-8ebd-e99a833f9718", "quantite_delta": 1000}	2026-04-21 13:08:41.51868	2026-05-07 12:48:52.572109
3b33d83f-8100-4391-9d91-b35b1762190c	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	243bd881-20fc-4f27-b7ee-7d70d33b3037	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "16b1169c-4934-493e-94f6-0929d3375182", "quantite_delta": 1000}	2026-04-22 15:51:37.850362	2026-05-07 12:48:52.572109
a0ad2d8d-f74c-4989-a5bc-51d0c71267c3	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_AJUSTEMENT	STOCK_MOUVEMENT	1733e0c5-13ce-440c-b375-80424ef04fc2	\N	{"type": "AJUSTEMENT", "raison": "Approvisionnement AJUSTEMENT", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "ae214634-f241-4719-9bdb-0e624a9f2cca", "quantite_delta": 120}	2026-04-22 16:32:18.437632	2026-05-07 12:48:52.572109
1482b6ea-686d-4aa5-9a0d-27528826c250	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	e3e80593-fa5b-466d-8cf7-e02b56494e9b	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "e2e97d80-3cb3-45d4-aa4c-cae5a8cde149", "quantite_delta": 300}	2026-04-23 06:39:54.917792	2026-05-07 12:48:52.572109
d84710a1-712f-48a4-9bb6-a8f1adef909d	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	44bde859-72ab-4582-8ebf-cb42e3e3b8b7	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "d6df6513-d313-454b-8804-ceb8dd9c7845", "quantite_delta": -32}	2026-04-23 06:55:02.132014	2026-05-07 12:48:52.572109
46caf4a4-068c-44df-8370-f49802fde96e	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	e3ad00ec-bbd2-44ce-b2e2-2c5c540c010f	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "ab56adec-069a-45ad-86d4-e5d0201e75d0", "quantite_delta": -82}	2026-04-23 06:56:53.555885	2026-05-07 12:48:52.572109
b1a09e54-a77c-4bbb-b07f-e1edfc56c0ac	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	011d4f02-fbf6-4da5-8620-9598c07ec29e	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "e1df0a80-b697-46f8-ba17-80896d82158b", "quantite_delta": -75}	2026-04-23 06:56:53.567043	2026-05-07 12:48:52.572109
1116b4fb-da8e-4b67-bfa6-356907edd9ae	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	4c4cc835-bab4-43ed-b09e-e339bda66a2e	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "ffaa85e5-225c-4b56-aec6-1fbb648ce8cf", "quantite_delta": 10}	2026-04-23 13:19:33.336409	2026-05-07 12:48:52.572109
7769938c-1033-4116-9f61-da010a750f16	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	bcea86c4-6b2e-4b26-804a-f9c41c6e5feb	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "f8f4029a-09eb-44ad-a27f-a2a7c8463369", "quantite_delta": 15}	2026-04-23 13:20:01.551678	2026-05-07 12:48:52.572109
4441aab4-59f6-42e5-8988-5111cc418b70	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	17773cbc-f57e-46a0-bea8-ae3a41c90599	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "5799cc19-e2e4-49e8-8fc6-a71f689ce133", "quantite_delta": 10}	2026-04-23 13:20:27.538956	2026-05-07 12:48:52.572109
5d6e72dd-eb15-471b-8b75-29ab5e93366b	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	182712c8-c076-4b20-9fa7-0a9f578d1c02	\N	{"type": "RETOUR", "raison": "Retour ENDOMMAGE", "location": "RETURNED", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "cb6bdc9c-b780-43bd-a85d-535c21d1b237", "quantite_delta": 17}	2026-04-26 19:03:43.636668	2026-05-07 12:48:52.572109
b91eeae3-3a1b-45d6-ae8f-f0f3b8a9507f	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	c67ba45f-d6e8-4066-9237-e407293bec93	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "22c58511-dcdd-42df-b6cb-4b04dd283c75", "quantite_delta": -110}	2026-04-29 11:12:55.543143	2026-05-07 12:48:52.572109
ff47b639-5b66-43a4-b5fa-d571c8074d73	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	de0e86f4-dcde-40ef-8ff2-c1efe8d7c263	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "7d5847cf-ff20-4a05-8f18-60094af1cd47", "quantite_delta": -110}	2026-04-29 11:12:56.009474	2026-05-07 12:48:52.572109
954c7e96-6a3d-400a-b231-fca2d3832d7c	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	a43d60f2-7613-4500-bc7c-8ade0373f360	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "98c50c10-973d-4c89-abb2-749fc57f448a", "quantite_delta": 500}	2026-04-29 11:14:45.132206	2026-05-07 12:48:52.572109
4b177a45-87a7-408b-80f2-4e2ac6f3c94b	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	966e3706-4462-4d3d-982b-a885dda77312	\N	{"type": "RETOUR", "raison": "Retour ENDOMMAGE", "location": "RETURNED", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "8e638c2c-59ee-412f-a06b-fdae49fcc19b", "quantite_delta": 17}	2026-04-29 12:58:30.94754	2026-05-07 12:48:52.572109
e215dc14-e290-43c1-92dc-f3eab9bb65ee	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	89326778-3335-4c30-8c7f-2adc5798a0a0	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "ea1db095-192a-4b9f-9751-cb6119a8f81b", "quantite_delta": 10}	2026-04-29 13:02:47.709968	2026-05-07 12:48:52.572109
750920f7-690f-47aa-b678-0b88246ddd6d	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	765f97e7-3c86-4dca-84da-e0c918a68ef5	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "478838f3-3d15-47fa-9214-18660e3fb6ad", "quantite_delta": 10}	2026-04-29 13:03:16.767464	2026-05-07 12:48:52.572109
c10dcdfa-1ba9-481c-bda0-8728f8757180	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	2dd1d751-b56a-4e95-85a4-8cdd8d78bb72	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "0620fa90-8436-43dd-be88-158c7b0e282d", "quantite_delta": -15}	2026-04-30 04:26:14.142321	2026-05-07 12:48:52.572109
25a608c8-fd48-45e8-b1fc-847207c133a6	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	7cf1a0d0-aa4c-4ad4-b17f-b981ffae51b2	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "a0f3c34f-d43a-469c-bce6-9e8916d8fff2", "quantite_delta": -65}	2026-04-30 06:31:39.431878	2026-05-07 12:48:52.572109
2fed4fdc-93fb-4781-8611-97fd66473258	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	e265b72f-43c7-4f94-bc3a-b015645d3603	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "4edb0d87-4124-4444-ab82-10e4063f12d2", "quantite_delta": -65}	2026-04-30 06:44:26.727667	2026-05-07 12:48:52.572109
a495f054-0382-46bf-9fce-5d254de66eb6	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	99936def-a01e-4e57-b230-efaf7414b41c	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "68c5e4c5-e414-4361-996e-d3af17471491", "quantite_delta": -25}	2026-05-04 07:26:25.539198	2026-05-07 12:48:52.572109
934358e6-7987-4964-b26e-3226ea2e0f6b	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	b5789fb7-dfe9-4dfa-b259-8b962b6d3179	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "f7e5acf1-0475-4ff2-8041-1aa6a9e0a08a", "quantite_delta": -25}	2026-05-04 07:26:58.858062	2026-05-07 12:48:52.572109
25d28e2f-58d1-4d22-ba79-d5f4d34e3f63	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	b333106d-d617-496b-ad8d-ea45be543a80	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "c9913500-c247-4548-a00b-fd76ece5a1c8", "quantite_delta": -65}	2026-05-04 07:56:09.841095	2026-05-07 12:48:52.572109
cf082187-4810-45f8-a465-c2017a836b64	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	1b247d1e-2d44-4d1b-8e9b-325419a98769	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "e8a75968-4626-4769-b5f6-2a5200aa7663", "quantite_delta": -65}	2026-05-04 07:57:17.977983	2026-05-07 12:48:52.572109
17a70147-3c0f-47d6-bf9a-868f00cf168d	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	66c9da88-6031-406c-b846-5a350f38ffc1	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "6da663a2-bf1d-4ee4-8c38-043fc826dec2", "quantite_delta": -50}	2026-05-04 10:56:34.006049	2026-05-07 12:48:52.572109
f2f70835-656b-43d3-8a5d-8280cc3e3a39	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	326f8aa5-52be-45e3-ac7b-260b214eec6e	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "9571c0a8-b3ca-4698-b334-dbefbc474511", "quantite_delta": -50}	2026-05-04 10:57:04.006599	2026-05-07 12:48:52.572109
1757cd81-6834-46c6-89ef-8b578c1395d8	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	d7c0805c-2cec-4490-8e85-08f947bd53f8	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "a1177137-e433-4c06-83c1-86f274c058d5", "quantite_delta": -65}	2026-05-04 11:06:46.76564	2026-05-07 12:48:52.572109
8273bc8c-a1fb-471d-b9b8-f689b6ca2060	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	44ad9aba-4712-42d3-9c59-9e88fe722f1b	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "b6447295-d74c-4010-80cd-885e6c40021a", "quantite_delta": -65}	2026-05-04 11:07:21.331219	2026-05-07 12:48:52.572109
e165f4a6-246c-4c05-be30-c4fd3b728134	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	49d2f1b7-842e-40a0-80e5-4e35fe51f066	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "948bdcd0-49cd-4bee-bc03-73be12393e22", "quantite_delta": -60}	2026-05-04 11:24:30.704117	2026-05-07 12:48:52.572109
3241b267-9ce6-41cf-b00b-2a03a0ea3468	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	e0372df6-2760-416b-afab-b93f985a191f	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "2e46ff74-a9f5-4842-894c-c4331f221242", "quantite_delta": -60}	2026-05-04 11:24:53.737309	2026-05-07 12:48:52.572109
713741bf-d530-4e6c-b03b-6c0f449d1ffc	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	328d0426-e9c0-4616-8319-dc145d1b6028	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "1f1dbd7f-60d1-4203-8a48-5d0be798193c", "quantite_delta": -40}	2026-05-04 11:42:08.928093	2026-05-07 12:48:52.572109
a7dab842-bc21-4d2d-833a-868e649c6bed	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	0454c826-74a9-479b-8559-5e51f3388ea9	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "6b06cbbd-4e77-4dde-b294-ac0ed34513b5", "quantite_delta": -40}	2026-05-04 11:42:24.997358	2026-05-07 12:48:52.572109
ca06352f-e8b0-44e3-af10-7002a6baec76	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	b536cb01-f965-457f-804f-de7529041298	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "5fd464cf-ac73-4bb5-8071-f26d2eef614d", "quantite_delta": -30}	2026-05-04 17:18:38.316987	2026-05-07 12:48:52.572109
a0423f8c-9552-42ac-927b-70bf11e8bb67	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	dc45b325-4cb9-4778-842e-87f8ababb020	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "7db6c1f8-faf4-42f8-add6-807acf58f719", "quantite_delta": 12}	2026-05-04 17:18:55.924466	2026-05-07 12:48:52.572109
f3604267-737a-488d-afdb-6de80f7239dd	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	748ae9f6-d22a-4142-8b8a-d4f64b6479f7	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "c5bcbf38-8256-4aba-9af5-06a8f1bc82a4", "quantite_delta": -30}	2026-05-04 17:18:56.89589	2026-05-07 12:48:52.572109
30894807-55d7-4fda-9fc0-896992d1c390	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	3516aaf7-d597-44d4-9402-448d9baa65d7	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "fd2ee6b3-0855-4d76-8ca9-403e72c739a1", "quantite_delta": -65}	2026-05-04 17:57:43.404988	2026-05-07 12:48:52.572109
34665ce8-fb92-402a-a0c1-4050b661e31b	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	a6343555-7ebb-4bf4-8155-b2d673342740	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "7d00bf3a-77bd-42b3-a446-0cd3be2309be", "quantite_delta": 30}	2026-05-04 17:58:05.440277	2026-05-07 12:48:52.572109
ef9e9f89-5ec6-4617-881d-6d69065f05d7	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	7cdd757a-1984-4617-987c-8dd0fe1e515d	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "1aa032bf-704f-4d1e-ac0b-5c2553129ebc", "quantite_delta": -65}	2026-05-04 17:58:05.461979	2026-05-07 12:48:52.572109
b6cf6fba-7099-4587-912e-78600bfe8bf3	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	a70dab9b-2a13-4941-9529-eb5881cc4d8f	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "39129e68-23c7-4a25-a45a-2f959085d175", "quantite_delta": -65}	2026-05-04 18:06:48.659249	2026-05-07 12:48:52.572109
d7837dc9-f23f-4ee9-922a-5eb803a7de8c	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	7bf8c22f-127d-44ce-a4f9-6c31647f14ed	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "ade0519c-51f0-4edd-aab1-dc959f9bd4d9", "quantite_delta": 30}	2026-05-04 18:07:02.805026	2026-05-07 12:48:52.572109
883a0890-b2b8-43bf-a231-68355efdcc68	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	fc8257b0-3302-4007-bdde-c50adedc2bd2	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "85875f9b-3b0f-42a4-80ac-5cc89f36357b", "quantite_delta": -65}	2026-05-04 18:07:02.828167	2026-05-07 12:48:52.572109
aeb0d2b1-fce1-44b5-bece-9f21abb7966c	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	e52f9537-6d69-490d-9b71-9dbfc0d305aa	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "b3bb2b93-0694-4519-991d-f4ceab43e10a", "quantite_delta": -65}	2026-05-04 18:09:57.132392	2026-05-07 12:48:52.572109
87104d3c-6d22-4f25-a443-f24fe5f8938a	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	d13d6c2a-345f-4a90-b4d7-4d6add1850a0	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "27f52871-48ae-4e79-8eb1-a754766806ca", "quantite_delta": 30}	2026-05-04 18:10:13.91203	2026-05-07 12:48:52.572109
af9c6a7a-a1c4-4dfb-a45f-a59ffcfaebf7	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	281ed401-7a97-4957-8f7a-77390d30c735	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "c007eff7-e557-4369-a534-ca3b01652221", "quantite_delta": -65}	2026-05-04 18:10:13.940351	2026-05-07 12:48:52.572109
400c5184-2de7-465b-aa0f-62802dc566fc	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	4ac04cb2-d007-43bd-84c5-93bf1725a414	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "7a88150f-cf9b-4d0b-8906-8b3ce56fd63e", "quantite_delta": 100}	2026-05-05 11:42:23.347292	2026-05-07 12:48:52.572109
10cbb053-1dfe-44a7-9050-0bdd91269ff4	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	ee124642-e089-4650-91ae-db73dc76aeaa	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "521a2f72-b86a-4b2e-ac96-4ab6e71e575d", "quantite_delta": 100}	2026-05-05 11:42:23.956407	2026-05-07 12:48:52.572109
fa4ffdec-6c1a-4f7f-a538-c4ea875e4ced	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	80bf7b5a-1b98-42c3-9c63-e37b46d9e8c0	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "91f088ac-01cc-4741-8ddc-02a6a3e8f7ea", "quantite_delta": 200}	2026-05-05 11:43:08.556904	2026-05-07 12:48:52.572109
81c01d28-d269-427c-90ae-5edcf5ade647	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	bd83144b-2e44-474b-bcdf-38735411b00d	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "4aaa17cd-e537-4c26-8429-0199d67dfe1f", "quantite_delta": 200}	2026-05-05 11:43:08.604831	2026-05-07 12:48:52.572109
769bd533-657e-4bb0-b4db-3c10fb4b40a5	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	2e402acd-ee22-400f-aefb-366a105bbb27	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "64c8ab8d-3d20-46fc-be04-a57850738863", "quantite_delta": -52}	2026-05-05 11:46:13.47436	2026-05-07 12:48:52.572109
13af228a-5346-4330-a2b6-8c4e2148a4a0	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	6c24e114-4246-4777-b9a7-8a5b97939f80	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "8a42b87c-9f41-4e8b-b3a9-27592818d783", "quantite_delta": -32}	2026-05-05 11:46:13.541261	2026-05-07 12:48:52.572109
61807cb2-bb7a-45fe-8cb6-bba9addd27e3	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	8a095af9-1a35-4f7a-843b-194fa4bbd49b	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "8bfd5efe-e28c-4611-a2c2-e6990374b1dc", "quantite_delta": 33}	2026-05-05 11:47:56.120176	2026-05-07 12:48:52.572109
9a734148-4e4e-48e8-ac8e-565361650307	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	8b1c8639-c2e4-4e65-883b-a2d4db87324f	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "5071bf90-d610-404f-a14f-6364881c3f53", "quantite_delta": -52}	2026-05-05 11:47:56.138187	2026-05-07 12:48:52.572109
ce43f9c3-8460-411c-a0de-263567b06982	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	f7eb9ffa-53ff-4fc5-8e8c-b7e7af12969c	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "5f606c70-ae96-4242-befe-76f61eaf236e", "quantite_delta": 17}	2026-05-05 11:47:56.149446	2026-05-07 12:48:52.572109
39cf143f-2b16-4808-bdfc-2cc8f8aca044	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	1379ff79-cc9b-4ed3-be9a-119c16263bb5	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "5071bf90-d610-404f-a14f-6364881c3f53", "quantite_delta": -32}	2026-05-05 11:47:56.160464	2026-05-07 12:48:52.572109
75fd82fa-b317-413c-9376-c6bae1281963	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_ENTREE	STOCK_MOUVEMENT	446a867d-12a1-43be-9d3a-5ff929920e2f	\N	{"type": "ENTREE", "raison": "Approvisionnement - PRODUCTION", "location": "WAREHOUSE", "produit_id": "f547b468-aee0-4109-acde-7ebb93d207d8", "reference_id": "513310e7-f0f1-4a3f-b886-756a28006fe1", "quantite_delta": 1000}	2026-05-05 17:16:57.792367	2026-05-07 12:48:52.572109
97aff28a-f38d-43ce-b1a1-9d9808caa5b8	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	fbf5c922-22e5-4270-8494-ce3d2507c078	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "f547b468-aee0-4109-acde-7ebb93d207d8", "reference_id": "3a82ecdc-9be7-4775-b7ef-e5c2de881592", "quantite_delta": -55}	2026-05-05 17:18:25.357062	2026-05-07 12:48:52.572109
95d3f65c-b93b-40e5-a876-01706909ca2a	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	f1733879-00bb-4a79-a0d7-039090fdbee6	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "c1245d72-233f-44bd-93e5-d9af30564cda", "quantite_delta": -35}	2026-05-05 17:18:25.438401	2026-05-07 12:48:52.572109
c2a07aae-fe67-4be6-838e-664693baae64	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	c21487c7-b76e-4c81-bb3a-169adc6aca69	\N	{"type": "SORTIE", "raison": "Repartition équipe", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "e3a88637-ae66-48b9-bf94-74a09aea8b24", "quantite_delta": -15}	2026-05-05 17:18:25.468056	2026-05-07 12:48:52.572109
7d2875c2-c0bb-4126-aa3d-d8e64783741c	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	87110d7d-9481-4e02-bbef-2df25e7a7778	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "f547b468-aee0-4109-acde-7ebb93d207d8", "reference_id": "de582787-d402-438b-8f4d-e83f2bb58f88", "quantite_delta": 30}	2026-05-05 17:20:15.939883	2026-05-07 12:48:52.572109
7d7dfc1f-11cd-4926-acbe-d9e648c72c67	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	b7ddd05c-bb90-4308-a1bb-54e311380956	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "f547b468-aee0-4109-acde-7ebb93d207d8", "reference_id": "20d5464d-478c-4b45-b717-fee8e4be07a9", "quantite_delta": -55}	2026-05-05 17:20:15.959031	2026-05-07 12:48:52.572109
98eda331-d1e3-445d-a8a5-2d6fa67f0982	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	fe781640-1af9-4e5f-b8ef-dd13e8bd83ff	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "d455d0c8-c14f-4034-aaf6-5052206df4fa", "quantite_delta": 18}	2026-05-05 17:20:15.971295	2026-05-07 12:48:52.572109
f6e096e2-8cc7-4112-a890-b0f8e956cfda	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	e51d57c7-e221-4249-bdb2-b5b88abf3cde	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "857f7c59-4ff6-45af-81fc-81d534af18de", "reference_id": "20d5464d-478c-4b45-b717-fee8e4be07a9", "quantite_delta": -35}	2026-05-05 17:20:15.980808	2026-05-07 12:48:52.572109
d09d7c9c-5ac9-4015-a88c-d9dcd369891b	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_RETOUR	STOCK_MOUVEMENT	97a55c06-3752-4038-becd-9bbc454b7f23	\N	{"type": "RETOUR", "raison": "Retour INVENDU", "location": "WAREHOUSE", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "5ca32fce-5fe5-42a8-8521-a64058588576", "quantite_delta": 3}	2026-05-05 17:20:15.99224	2026-05-07 12:48:52.572109
57cca9cb-cf86-490c-b7a3-aef4f597a483	78a24f11-9714-4014-ae40-f38318fef119	MOUVEMENT_STOCK_SORTIE	STOCK_MOUVEMENT	cf60be5e-1bfb-4aa2-8abc-adb644c25a18	\N	{"type": "SORTIE", "raison": "Clôture répartition : purge transit", "location": "IN_TRANSIT", "produit_id": "03dc330a-6da1-41ca-86ba-60945244182a", "reference_id": "20d5464d-478c-4b45-b717-fee8e4be07a9", "quantite_delta": -15}	2026-05-05 17:20:16.003262	2026-05-07 12:48:52.572109
\.


--
-- Data for Name: lots_production; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.lots_production (lot_id, code_lot, produit_id, cuve_id, volume_produit, date_debut, date_fin, statut_lot, notes_vinification, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: matieres_premieres; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.matieres_premieres (matiere_id, code_matiere, nom, unite, seuil_alerte, description, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: permissions; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.permissions (permission_id, code, nom, module, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
4ed81207-5257-4f3e-afd3-71057a413e71	STOCK_EDIT	Modifier le stock	Stock	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
d6d71983-fdc6-41bc-9dd9-da65bf97c04f	STOCK_VIEW	Voir le stock	Stock	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
9988e1e7-97d0-4dd6-a4b9-61a24bc38606	VENTE_CREATE	Créer une vente	Ventes	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
b5714b2f-205a-4258-a276-9e98ebef57e9	VENTE_EDIT	Modifier une vente	Ventes	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
ab845788-0800-489a-9129-9be7e29d7c55	CAISSE_VALIDER	Valider la caisse	Caisse	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
7e902bc7-1361-4af6-b37e-942baee4810e	CREDIT_MANAGE	Gérer les crédits	Finance	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
03275df0-2e4e-4b05-818a-537cba69ec27	STOCK_APPROVE	Approuver entrées/retours	Stock	PENDING	1	\N	2026-05-07 12:48:42.757121	2026-05-07 12:48:42.757121
\.


--
-- Data for Name: produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.produits (produit_id, categorie_produit_id, type_produit_id, nom, code_sku, prix_unitaire, stock_minimum, est_actif, date_mise_a_jour, description, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
6f7b704d-99c4-4837-a04b-be52c27f33d9	357241c2-6e79-4952-b103-50e1b52b3f96	372480eb-993e-4ac0-862f-39466beb1863	Vin SEMULIKI	VIN-SEM	1200.00	50	f	2026-04-14 18:09:32.24038	13% Alc.\nAphrodisiaque	PENDING	1	\N	2026-05-07 12:43:15.300845	2026-05-07 12:43:15.300845
5058c2e2-506f-42bf-aa45-9105b435f4dc	31fbd8a3-ed3e-4f76-939a-fffd58fda88f	372480eb-993e-4ac0-862f-39466beb1863	Chapeau	CHP	12.00	10	f	2026-04-21 12:32:13.769856	Couleur:\nBlanc\nNoir\nRouge	PENDING	1	\N	2026-05-07 12:43:15.300845	2026-05-07 12:43:15.300845
857f7c59-4ff6-45af-81fc-81d534af18de	357241c2-6e79-4952-b103-50e1b52b3f96	187835d8-22a6-4a08-bb6e-e93d2a334147	Vin SEMULIKI-13	VIN-SEM-13	1200.00	1500	t	2026-04-15 05:44:56.894016	13% Alc.\nAphrodisiaque	PENDING	1	\N	2026-05-07 12:43:15.300845	2026-05-07 12:43:15.300845
03dc330a-6da1-41ca-86ba-60945244182a	357241c2-6e79-4952-b103-50e1b52b3f96	372480eb-993e-4ac0-862f-39466beb1863	Vin SEMULIKI-20	VIN-SEM-20	1000.00	2000	t	2026-04-15 05:44:42.449737	20% Alc.\nApperitif	PENDING	1	\N	2026-05-07 12:43:15.300845	2026-05-07 12:43:15.300845
f547b468-aee0-4109-acde-7ebb93d207d8	53cd8cb8-c163-43a4-8864-5ea57153dbad	372480eb-993e-4ac0-862f-39466beb1863	Eau Semuliki	EAU-SEM	0.50	10	t	2026-05-05 17:16:19.050505	Eau minerale Semuliki	PENDING	1	\N	2026-05-07 12:43:15.300845	2026-05-07 12:43:15.300845
\.


--
-- Data for Name: raisons_retour; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.raisons_retour (raison_retour_id, code, nom, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
9cd4e12b-4938-48c9-8e62-1c4f892410d8	INVENDU	Invendu	PENDING	1	\N	2026-05-07 12:46:23.775192	2026-05-07 12:46:23.775192
720baf31-5f69-4736-b671-2df46e81b60d	ENDOMMAGE	Endommagé	PENDING	1	\N	2026-05-07 12:46:23.775192	2026-05-07 12:46:23.775192
fa101a6e-a89c-45aa-a95f-90b60dcd6eee	EXPIRE	Expiré	PENDING	1	\N	2026-05-07 12:46:23.775192	2026-05-07 12:46:23.775192
25e01e32-5cfd-48f8-9e27-05631883a3b8	ERREUR	Erreur de commande	PENDING	1	\N	2026-05-07 12:46:23.775192	2026-05-07 12:46:23.775192
\.


--
-- Data for Name: receptions_caisse; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.receptions_caisse (reception_caisse_id, repartition_id, montant_attendu, montant_recu, statut_caisse_id, caissier_id, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: repartition_audit; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.repartition_audit (repartition_audit_id, repartition_id, utilisateur_id, date_action, ancien_statut, nouveau_statut, commentaire, created_at) FROM stdin;
7734203c-6198-4cf9-a1cd-3408e8b38177	b6cd0cc6-d35a-4459-94fc-b3d0f75f983e	\N	2026-04-30 03:28:35.835871	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
255c6b63-4747-4d60-b300-a5f6ea02ec9e	ed8bf7e0-8fb6-42c9-9faa-0b50a7ef659e	\N	2026-04-30 03:42:42.47154	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
6c30dc15-8075-462c-98c4-db14ba5fc66b	72a8c388-3d8d-448e-8d4e-6e7e999dce9b	\N	2026-04-30 03:43:17.11522	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
d728a559-beb7-4238-989f-a468b498a801	78a127f8-9363-489a-8c55-33efa47b6880	\N	2026-04-30 04:53:47.045922	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
dea837d1-db8d-4bbf-8e90-ddf30de0d022	040ea0ee-f7ad-4e7a-b73a-2dfdaadd472c	\N	2026-04-30 06:32:02.892037	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
f8b08ecd-6e30-482d-9df7-67758e091e5a	9f32890c-eae9-472d-995a-17c558dae8d4	\N	2026-04-30 06:44:44.847884	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
bd97f3f6-0315-4cfd-bda3-66c4c12fd369	f7e5acf1-0475-4ff2-8041-1aa6a9e0a08a	\N	2026-05-04 07:26:58.915032	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
79816fb6-bd6f-4583-ae4e-bb52ef21e515	e8a75968-4626-4769-b5f6-2a5200aa7663	\N	2026-05-04 07:57:18.016372	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
025c8c29-4d5f-45fa-a66c-450d00ddf454	9571c0a8-b3ca-4698-b334-dbefbc474511	\N	2026-05-04 10:57:04.033121	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
2952f365-a669-4760-9465-2d297017ff70	b6447295-d74c-4010-80cd-885e6c40021a	\N	2026-05-04 11:07:21.352181	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
a60ea290-f55a-4830-813c-ddef055e530a	2e46ff74-a9f5-4842-894c-c4331f221242	\N	2026-05-04 11:24:53.783582	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
1cc0fcba-cbb8-428e-aed6-7dfecdbe353f	6b06cbbd-4e77-4dde-b294-ac0ed34513b5	\N	2026-05-04 11:42:25.017451	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
4338ff57-e06a-4c54-bcd5-40b3d6e6b296	c5bcbf38-8256-4aba-9af5-06a8f1bc82a4	\N	2026-05-04 17:18:57.169744	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
98d61141-87d0-488b-90d7-f2015317ad29	1aa032bf-704f-4d1e-ac0b-5c2553129ebc	\N	2026-05-04 17:58:05.473894	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
32163682-ed24-4ec7-bc60-f8446511e8f0	85875f9b-3b0f-42a4-80ac-5cc89f36357b	\N	2026-05-04 18:07:02.840585	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
dc26047f-c5f6-4447-8645-bc2b1c206fdc	c007eff7-e557-4369-a534-ca3b01652221	\N	2026-05-04 18:10:13.952023	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
a06967d2-05c4-4339-a6a0-40af47736438	5071bf90-d610-404f-a14f-6364881c3f53	\N	2026-05-05 11:47:56.172402	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
8168ad3d-a153-4312-ad12-fcd68f2aa3ac	20d5464d-478c-4b45-b717-fee8e4be07a9	\N	2026-05-05 17:20:16.017529	e0059987-5a9f-44bd-b806-18434792491d	936c875a-f441-410d-9098-98531a60c073	\N	2026-05-07 12:49:13.339664
\.


--
-- Data for Name: repartitions; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.repartitions (repartition_id, equipe_id, route_id, statut_repartition_id, date_repartition, montant_cash_attendu, date_mise_a_jour, chef_id, annule, mouvements_generes, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
6b06cbbd-4e77-4dde-b294-ac0ed34513b5	67f0aef1-dbf3-4037-a853-52f77b15dddf	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-05-04	30000.00	2026-05-04 11:42:08.908	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
b6cd0cc6-d35a-4459-94fc-b3d0f75f983e	614554c2-9d3d-4165-8092-10b1fe4cc816	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-04-29	176000.00	2026-04-29 11:12:55.412	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
ed8bf7e0-8fb6-42c9-9faa-0b50a7ef659e	9a66d049-23a2-4db6-a271-10db9cc28ec0	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-04-23	30000.00	2026-04-23 06:55:02.105	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
72a8c388-3d8d-448e-8d4e-6e7e999dce9b	614554c2-9d3d-4165-8092-10b1fe4cc816	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-04-23	125000.00	2026-04-23 06:56:53.529	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
c5bcbf38-8256-4aba-9af5-06a8f1bc82a4	67f0aef1-dbf3-4037-a853-52f77b15dddf	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-05-04	20000.00	2026-05-04 17:18:38.203	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
78a127f8-9363-489a-8c55-33efa47b6880	9a66d049-23a2-4db6-a271-10db9cc28ec0	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-04-30	12000.00	2026-04-30 04:26:14.123	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
040ea0ee-f7ad-4e7a-b73a-2dfdaadd472c	9a66d049-23a2-4db6-a271-10db9cc28ec0	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-04-30	60000.00	2026-04-30 06:31:39.403	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
9f32890c-eae9-472d-995a-17c558dae8d4	614554c2-9d3d-4165-8092-10b1fe4cc816	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-04-30	60000.00	2026-04-30 06:44:26.703	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
f7e5acf1-0475-4ff2-8041-1aa6a9e0a08a	9a66d049-23a2-4db6-a271-10db9cc28ec0	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-05-04	14400.00	2026-05-04 07:26:25.432	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
e8a75968-4626-4769-b5f6-2a5200aa7663	9a66d049-23a2-4db6-a271-10db9cc28ec0	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-05-04	60000.00	2026-05-04 07:56:09.802	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
1aa032bf-704f-4d1e-ac0b-5c2553129ebc	5fa8cb53-b61e-4be1-8c8e-2a45fd2072d8	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-05-04	60000.00	2026-05-04 17:57:43.38	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
9571c0a8-b3ca-4698-b334-dbefbc474511	614554c2-9d3d-4165-8092-10b1fe4cc816	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-05-04	42000.00	2026-05-04 10:56:33.982	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
b6447295-d74c-4010-80cd-885e6c40021a	923a217f-6939-49ab-b54a-c25844713d88	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-05-04	60000.00	2026-05-04 11:06:46.739	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
85875f9b-3b0f-42a4-80ac-5cc89f36357b	5fa8cb53-b61e-4be1-8c8e-2a45fd2072d8	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-05-04	60000.00	2026-05-04 18:06:48.627	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
2e46ff74-a9f5-4842-894c-c4331f221242	923a217f-6939-49ab-b54a-c25844713d88	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-05-04	60000.00	2026-05-04 11:24:30.661	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
c007eff7-e557-4369-a534-ca3b01652221	9a66d049-23a2-4db6-a271-10db9cc28ec0	1359e5dd-e4e2-4289-8881-f8f81f229199	936c875a-f441-410d-9098-98531a60c073	2026-05-04	50000.00	2026-05-04 18:09:57.104	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
5071bf90-d610-404f-a14f-6364881c3f53	9a66d049-23a2-4db6-a271-10db9cc28ec0	99c124c9-2e02-40ba-a792-7494bc094fc3	936c875a-f441-410d-9098-98531a60c073	2026-05-05	90000.00	2026-05-05 11:46:13.401	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
20d5464d-478c-4b45-b717-fee8e4be07a9	9a66d049-23a2-4db6-a271-10db9cc28ec0	4465ed8d-443d-4ac2-a13f-468a885367d0	936c875a-f441-410d-9098-98531a60c073	2026-05-05	46825.00	2026-05-05 17:18:25.298	78a24f11-9714-4014-ae40-f38318fef119	f	t	PENDING	1	\N	2026-05-07 12:51:04.077872	2026-05-07 12:51:04.077872
\.


--
-- Data for Name: retours_stock; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.retours_stock (retour_stock_id, produit_id, quantite, raison_retour_id, date, repartition_id, observations, cree_par, approuve_par, statut_validation, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
7db6c1f8-faf4-42f8-add6-807acf58f719	03dc330a-6da1-41ca-86ba-60945244182a	12	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-04 17:18:55.924466	c5bcbf38-8256-4aba-9af5-06a8f1bc82a4	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-04 17:18:55.924466	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
7d00bf3a-77bd-42b3-a446-0cd3be2309be	857f7c59-4ff6-45af-81fc-81d534af18de	30	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-04 17:58:05.440277	1aa032bf-704f-4d1e-ac0b-5c2553129ebc	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-04 17:58:05.440277	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
ade0519c-51f0-4edd-aab1-dc959f9bd4d9	857f7c59-4ff6-45af-81fc-81d534af18de	30	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-04 18:07:02.805026	85875f9b-3b0f-42a4-80ac-5cc89f36357b	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-04 18:07:02.805026	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
27f52871-48ae-4e79-8eb1-a754766806ca	03dc330a-6da1-41ca-86ba-60945244182a	30	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-04 18:10:13.91203	c007eff7-e557-4369-a534-ca3b01652221	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-04 18:10:13.91203	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
8bfd5efe-e28c-4611-a2c2-e6990374b1dc	857f7c59-4ff6-45af-81fc-81d534af18de	33	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-05 11:47:56.120176	5071bf90-d610-404f-a14f-6364881c3f53	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-05 11:47:56.120176	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
5f606c70-ae96-4242-befe-76f61eaf236e	03dc330a-6da1-41ca-86ba-60945244182a	17	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-05 11:47:56.149446	5071bf90-d610-404f-a14f-6364881c3f53	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-05 11:47:56.149446	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
de582787-d402-438b-8f4d-e83f2bb58f88	f547b468-aee0-4109-acde-7ebb93d207d8	30	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-05 17:20:15.939883	20d5464d-478c-4b45-b717-fee8e4be07a9	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-05 17:20:15.939883	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
d455d0c8-c14f-4034-aaf6-5052206df4fa	857f7c59-4ff6-45af-81fc-81d534af18de	18	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-05 17:20:15.971295	20d5464d-478c-4b45-b717-fee8e4be07a9	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-05 17:20:15.971295	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
5ca32fce-5fe5-42a8-8521-a64058588576	03dc330a-6da1-41ca-86ba-60945244182a	3	9cd4e12b-4938-48c9-8e62-1c4f892410d8	2026-05-05 17:20:15.99224	20d5464d-478c-4b45-b717-fee8e4be07a9	Retour invendus répartition	78a24f11-9714-4014-ae40-f38318fef119	\N	EN_ATTENTE	2026-05-05 17:20:15.99224	PENDING	1	\N	2026-05-07 12:50:33.406109	2026-05-07 12:50:33.406109
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

COPY public.roles (role_id, code, nom, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
23b42043-5724-483d-89fe-0debdcc1d6e6	ADMIN	Administrator	PENDING	1	\N	2026-05-07 12:47:00.956115	2026-05-07 12:47:00.956115
\.


--
-- Data for Name: routes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.routes (route_id, nom, description, est_actif, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
99c124c9-2e02-40ba-a792-7494bc094fc3	KASAPA-MOISE		t	PENDING	1	\N	2026-05-07 12:45:06.71731	2026-05-07 12:45:06.71731
4465ed8d-443d-4ac2-a13f-468a885367d0	KASAPA-MARCHE MZEE		t	PENDING	1	\N	2026-05-07 12:45:06.71731	2026-05-07 12:45:06.71731
1359e5dd-e4e2-4289-8881-f8f81f229199	CRAA-KASANGIRI		t	PENDING	1	\N	2026-05-07 12:45:06.71731	2026-05-07 12:45:06.71731
\.


--
-- Data for Name: sources_entree; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.sources_entree (source_entree_id, code, nom, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
c656b280-7660-4ebf-b20c-28880d7cd5f7	PRODUCTION	Production	PENDING	1	\N	2026-05-07 12:46:34.095971	2026-05-07 12:46:34.095971
b458d1a4-29c7-4bc7-ab84-34f6baac904e	RETOUR	Retour	PENDING	1	\N	2026-05-07 12:46:34.095971	2026-05-07 12:46:34.095971
b2f4f9fb-3eab-4e38-a389-4b40e71cefb6	AJUSTEMENT	Ajustement	PENDING	1	\N	2026-05-07 12:46:34.095971	2026-05-07 12:46:34.095971
b0254355-788b-42ed-b52c-41096dee4f6d	ACHAT	Achat	PENDING	1	\N	2026-05-07 12:46:34.095971	2026-05-07 12:46:34.095971
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

COPY public.statuts_credit (statut_credit_id, code, nom, created_at, updated_at) FROM stdin;
e9d45298-343b-4747-ab33-e067991afb05	PAYE	Payé	2026-05-07 12:52:12.50053	2026-05-07 12:52:12.50053
7a70cbb4-ad1c-4aa8-9cae-50cc9c61b711	EN_RETARD	En retard	2026-05-07 12:52:12.50053	2026-05-07 12:52:12.50053
1182f388-881a-4690-9029-362a6601972b	EN_ATTENTE	En attente	2026-05-07 12:52:12.50053	2026-05-07 12:52:12.50053
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

COPY public.stock_locations (location_id, nom, description, est_actif, created_at, updated_at) FROM stdin;
WAREHOUSE	Entrepôt principal	Stock principal en entrepôt	t	2026-05-07 12:49:24.948962	2026-05-07 12:49:24.948962
IN_TRANSIT	En transit	Stock envoyé aux équipes	t	2026-05-07 12:49:24.948962	2026-05-07 12:49:24.948962
RETURNED	Retourné	Stock retourné des équipes	t	2026-05-07 12:49:24.948962	2026-05-07 12:49:24.948962
\.


--
-- Data for Name: stock_mouvements; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.stock_mouvements (mouvement_id, produit_id, type_mouvement, quantite_delta, reference_id, reference_type, utilisateur_id, location_id, raison, observations, created_at, updated_at, source_entree_id, sync_status, version, deleted_at, is_deleted, lot_id, source_module) FROM stdin;
9721b8ef-bed6-404c-9e66-53e75dbb14ef	5058c2e2-506f-42bf-aa45-9105b435f4dc	AJUSTEMENT	100	38f61680-d234-483a-a5a7-b83294ab1a0d	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 12:43:15.444513	2026-04-21 12:43:15.444513	\N	PENDING	1	\N	f	\N	\N
8845a8d0-6879-4dc1-9cd2-25079ec25b25	857f7c59-4ff6-45af-81fc-81d534af18de	AJUSTEMENT	1000	d8af351c-4b92-4905-9507-5d1c8d9f644f	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 12:49:01.740701	2026-04-21 12:49:01.740701	\N	PENDING	1	\N	f	\N	\N
b0c87cc5-1a70-4d52-b83d-03c7cdf678c8	03dc330a-6da1-41ca-86ba-60945244182a	AJUSTEMENT	1000	8bef3ca2-bbf4-426a-a718-e387ee8710b3	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 13:08:41.492991	2026-04-21 13:08:41.492991	\N	PENDING	1	\N	f	\N	\N
a6c8c3ac-0043-4455-9399-37148b8e8752	857f7c59-4ff6-45af-81fc-81d534af18de	AJUSTEMENT	1000	87d4019b-504e-4842-8ebd-e99a833f9718	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-21 13:08:41.51868	2026-04-21 13:08:41.51868	\N	PENDING	1	\N	f	\N	\N
243bd881-20fc-4f27-b7ee-7d70d33b3037	857f7c59-4ff6-45af-81fc-81d534af18de	AJUSTEMENT	1000	16b1169c-4934-493e-94f6-0929d3375182	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-22 15:51:37.850362	2026-04-22 15:51:37.850362	\N	PENDING	1	\N	f	\N	\N
1733e0c5-13ce-440c-b375-80424ef04fc2	03dc330a-6da1-41ca-86ba-60945244182a	AJUSTEMENT	120	ae214634-f241-4719-9bdb-0e624a9f2cca	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement AJUSTEMENT	Facture: N/A | Lot: N/A | Source: {b2f4f9fb-3eab-4e38-a389-4b40e71cefb6}	2026-04-22 16:32:18.437632	2026-04-22 16:32:18.437632	\N	PENDING	1	\N	f	\N	\N
e3e80593-fa5b-466d-8cf7-e02b56494e9b	857f7c59-4ff6-45af-81fc-81d534af18de	ENTREE	300	e2e97d80-3cb3-45d4-aa4c-cae5a8cde149	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-04-23 06:39:54.917792	2026-04-23 06:39:54.917792	\N	PENDING	1	\N	f	\N	\N
44bde859-72ab-4582-8ebf-cb42e3e3b8b7	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-32	d6df6513-d313-454b-8804-ceb8dd9c7845	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-23 06:55:02.132014	2026-04-23 06:55:02.132014	\N	PENDING	1	\N	f	\N	\N
e3ad00ec-bbd2-44ce-b2e2-2c5c540c010f	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-82	ab56adec-069a-45ad-86d4-e5d0201e75d0	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-23 06:56:53.555885	2026-04-23 06:56:53.555885	\N	PENDING	1	\N	f	\N	\N
011d4f02-fbf6-4da5-8620-9598c07ec29e	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-75	e1df0a80-b697-46f8-ba17-80896d82158b	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-23 06:56:53.567043	2026-04-23 06:56:53.567043	\N	PENDING	1	\N	f	\N	\N
4c4cc835-bab4-43ed-b09e-e339bda66a2e	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	10	ffaa85e5-225c-4b56-aec6-1fbb648ce8cf	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU		2026-04-23 13:19:33.336409	2026-04-23 13:19:33.336409	\N	PENDING	1	\N	f	\N	\N
bcea86c4-6b2e-4b26-804a-f9c41c6e5feb	857f7c59-4ff6-45af-81fc-81d534af18de	RETOUR	15	f8f4029a-09eb-44ad-a27f-a2a7c8463369	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU		2026-04-23 13:20:01.551678	2026-04-23 13:20:01.551678	\N	PENDING	1	\N	f	\N	\N
17773cbc-f57e-46a0-bea8-ae3a41c90599	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	10	5799cc19-e2e4-49e8-8fc6-a71f689ce133	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU		2026-04-23 13:20:27.538956	2026-04-23 13:20:27.538956	\N	PENDING	1	\N	f	\N	\N
182712c8-c076-4b20-9fa7-0a9f578d1c02	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	17	cb6bdc9c-b780-43bd-a85d-535c21d1b237	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	RETURNED	Retour ENDOMMAGE	n/a	2026-04-26 19:03:43.636668	2026-04-26 19:03:43.636668	\N	PENDING	1	\N	f	\N	\N
c67ba45f-d6e8-4066-9237-e407293bec93	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-110	22c58511-dcdd-42df-b6cb-4b04dd283c75	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-29 11:12:55.543143	2026-04-29 11:12:55.543143	\N	PENDING	1	\N	f	\N	\N
de0e86f4-dcde-40ef-8ff2-c1efe8d7c263	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-110	7d5847cf-ff20-4a05-8f18-60094af1cd47	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-29 11:12:56.009474	2026-04-29 11:12:56.009474	\N	PENDING	1	\N	f	\N	\N
a43d60f2-7613-4500-bc7c-8ade0373f360	857f7c59-4ff6-45af-81fc-81d534af18de	ENTREE	500	98c50c10-973d-4c89-abb2-749fc57f448a	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-04-29 11:14:45.132206	2026-04-29 11:14:45.132206	\N	PENDING	1	\N	f	\N	\N
966e3706-4462-4d3d-982b-a885dda77312	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	17	8e638c2c-59ee-412f-a06b-fdae49fcc19b	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	RETURNED	Retour ENDOMMAGE		2026-04-29 12:58:30.94754	2026-04-29 12:58:30.94754	\N	PENDING	1	\N	f	\N	\N
89326778-3335-4c30-8c7f-2adc5798a0a0	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	10	ea1db095-192a-4b9f-9751-cb6119a8f81b	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU		2026-04-29 13:02:47.709968	2026-04-29 13:02:47.709968	\N	PENDING	1	\N	f	\N	\N
765f97e7-3c86-4dca-84da-e0c918a68ef5	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	10	478838f3-3d15-47fa-9214-18660e3fb6ad	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU		2026-04-29 13:03:16.767464	2026-04-29 13:03:16.767464	\N	PENDING	1	\N	f	\N	\N
2dd1d751-b56a-4e95-85a4-8cdd8d78bb72	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-15	0620fa90-8436-43dd-be88-158c7b0e282d	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-30 04:26:14.142321	2026-04-30 04:26:14.142321	\N	PENDING	1	\N	f	\N	\N
7cf1a0d0-aa4c-4ad4-b17f-b981ffae51b2	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	a0f3c34f-d43a-469c-bce6-9e8916d8fff2	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-30 06:31:39.431878	2026-04-30 06:31:39.431878	\N	PENDING	1	\N	f	\N	\N
e265b72f-43c7-4f94-bc3a-b015645d3603	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	4edb0d87-4124-4444-ab82-10e4063f12d2	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-04-30 06:44:26.727667	2026-04-30 06:44:26.727667	\N	PENDING	1	\N	f	\N	\N
99936def-a01e-4e57-b230-efaf7414b41c	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-25	68c5e4c5-e414-4361-996e-d3af17471491	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 07:26:25.539198	2026-05-04 07:26:25.539198	\N	PENDING	1	\N	f	\N	\N
b5789fb7-dfe9-4dfa-b259-8b962b6d3179	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-25	f7e5acf1-0475-4ff2-8041-1aa6a9e0a08a	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 07:26:58.858062	2026-05-04 07:26:58.858062	\N	PENDING	1	\N	f	\N	\N
b333106d-d617-496b-ad8d-ea45be543a80	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	c9913500-c247-4548-a00b-fd76ece5a1c8	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 07:56:09.841095	2026-05-04 07:56:09.841095	\N	PENDING	1	\N	f	\N	\N
1b247d1e-2d44-4d1b-8e9b-325419a98769	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	e8a75968-4626-4769-b5f6-2a5200aa7663	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 07:57:17.977983	2026-05-04 07:57:17.977983	\N	PENDING	1	\N	f	\N	\N
66c9da88-6031-406c-b846-5a350f38ffc1	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-50	6da663a2-bf1d-4ee4-8c38-043fc826dec2	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 10:56:34.006049	2026-05-04 10:56:34.006049	\N	PENDING	1	\N	f	\N	\N
326f8aa5-52be-45e3-ac7b-260b214eec6e	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-50	9571c0a8-b3ca-4698-b334-dbefbc474511	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 10:57:04.006599	2026-05-04 10:57:04.006599	\N	PENDING	1	\N	f	\N	\N
d7c0805c-2cec-4490-8e85-08f947bd53f8	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	a1177137-e433-4c06-83c1-86f274c058d5	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 11:06:46.76564	2026-05-04 11:06:46.76564	\N	PENDING	1	\N	f	\N	\N
44ad9aba-4712-42d3-9c59-9e88fe722f1b	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	b6447295-d74c-4010-80cd-885e6c40021a	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 11:07:21.331219	2026-05-04 11:07:21.331219	\N	PENDING	1	\N	f	\N	\N
49d2f1b7-842e-40a0-80e5-4e35fe51f066	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-60	948bdcd0-49cd-4bee-bc03-73be12393e22	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 11:24:30.704117	2026-05-04 11:24:30.704117	\N	PENDING	1	\N	f	\N	\N
e0372df6-2760-416b-afab-b93f985a191f	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-60	2e46ff74-a9f5-4842-894c-c4331f221242	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 11:24:53.737309	2026-05-04 11:24:53.737309	\N	PENDING	1	\N	f	\N	\N
328d0426-e9c0-4616-8319-dc145d1b6028	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-40	1f1dbd7f-60d1-4203-8a48-5d0be798193c	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 11:42:08.928093	2026-05-04 11:42:08.928093	\N	PENDING	1	\N	f	\N	\N
0454c826-74a9-479b-8559-5e51f3388ea9	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-40	6b06cbbd-4e77-4dde-b294-ac0ed34513b5	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 11:42:24.997358	2026-05-04 11:42:24.997358	\N	PENDING	1	\N	f	\N	\N
b536cb01-f965-457f-804f-de7529041298	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-30	5fd464cf-ac73-4bb5-8071-f26d2eef614d	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 17:18:38.316987	2026-05-04 17:18:38.316987	\N	PENDING	1	\N	f	\N	\N
dc45b325-4cb9-4778-842e-87f8ababb020	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	12	7db6c1f8-faf4-42f8-add6-807acf58f719	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-04 17:18:55.924466	2026-05-04 17:18:55.924466	\N	PENDING	1	\N	f	\N	\N
748ae9f6-d22a-4142-8b8a-d4f64b6479f7	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-30	c5bcbf38-8256-4aba-9af5-06a8f1bc82a4	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 17:18:56.89589	2026-05-04 17:18:56.89589	\N	PENDING	1	\N	f	\N	\N
3516aaf7-d597-44d4-9402-448d9baa65d7	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	fd2ee6b3-0855-4d76-8ca9-403e72c739a1	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 17:57:43.404988	2026-05-04 17:57:43.404988	\N	PENDING	1	\N	f	\N	\N
a6343555-7ebb-4bf4-8155-b2d673342740	857f7c59-4ff6-45af-81fc-81d534af18de	RETOUR	30	7d00bf3a-77bd-42b3-a446-0cd3be2309be	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-04 17:58:05.440277	2026-05-04 17:58:05.440277	\N	PENDING	1	\N	f	\N	\N
7cdd757a-1984-4617-987c-8dd0fe1e515d	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	1aa032bf-704f-4d1e-ac0b-5c2553129ebc	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 17:58:05.461979	2026-05-04 17:58:05.461979	\N	PENDING	1	\N	f	\N	\N
a70dab9b-2a13-4941-9529-eb5881cc4d8f	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	39129e68-23c7-4a25-a45a-2f959085d175	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 18:06:48.659249	2026-05-04 18:06:48.659249	\N	PENDING	1	\N	f	\N	\N
7bf8c22f-127d-44ce-a4f9-6c31647f14ed	857f7c59-4ff6-45af-81fc-81d534af18de	RETOUR	30	ade0519c-51f0-4edd-aab1-dc959f9bd4d9	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-04 18:07:02.805026	2026-05-04 18:07:02.805026	\N	PENDING	1	\N	f	\N	\N
fc8257b0-3302-4007-bdde-c50adedc2bd2	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-65	85875f9b-3b0f-42a4-80ac-5cc89f36357b	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 18:07:02.828167	2026-05-04 18:07:02.828167	\N	PENDING	1	\N	f	\N	\N
e52f9537-6d69-490d-9b71-9dbfc0d305aa	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-65	b3bb2b93-0694-4519-991d-f4ceab43e10a	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-04 18:09:57.132392	2026-05-04 18:09:57.132392	\N	PENDING	1	\N	f	\N	\N
d13d6c2a-345f-4a90-b4d7-4d6add1850a0	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	30	27f52871-48ae-4e79-8eb1-a754766806ca	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-04 18:10:13.91203	2026-05-04 18:10:13.91203	\N	PENDING	1	\N	f	\N	\N
281ed401-7a97-4957-8f7a-77390d30c735	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-65	c007eff7-e557-4369-a534-ca3b01652221	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-04 18:10:13.940351	2026-05-04 18:10:13.940351	\N	PENDING	1	\N	f	\N	\N
4ac04cb2-d007-43bd-84c5-93bf1725a414	857f7c59-4ff6-45af-81fc-81d534af18de	ENTREE	100	7a88150f-cf9b-4d0b-8906-8b3ce56fd63e	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-05-05 11:42:23.347292	2026-05-05 11:42:23.347292	\N	PENDING	1	\N	f	\N	\N
ee124642-e089-4650-91ae-db73dc76aeaa	03dc330a-6da1-41ca-86ba-60945244182a	ENTREE	100	521a2f72-b86a-4b2e-ac96-4ab6e71e575d	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-05-05 11:42:23.956407	2026-05-05 11:42:23.956407	\N	PENDING	1	\N	f	\N	\N
80bf7b5a-1b98-42c3-9c63-e37b46d9e8c0	857f7c59-4ff6-45af-81fc-81d534af18de	ENTREE	200	91f088ac-01cc-4741-8ddc-02a6a3e8f7ea	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-05-05 11:43:08.556904	2026-05-05 11:43:08.556904	\N	PENDING	1	\N	f	\N	\N
bd83144b-2e44-474b-bcdf-38735411b00d	03dc330a-6da1-41ca-86ba-60945244182a	ENTREE	200	4aaa17cd-e537-4c26-8429-0199d67dfe1f	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-05-05 11:43:08.604831	2026-05-05 11:43:08.604831	\N	PENDING	1	\N	f	\N	\N
2e402acd-ee22-400f-aefb-366a105bbb27	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-52	64c8ab8d-3d20-46fc-be04-a57850738863	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-05 11:46:13.47436	2026-05-05 11:46:13.47436	\N	PENDING	1	\N	f	\N	\N
6c24e114-4246-4777-b9a7-8a5b97939f80	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-32	8a42b87c-9f41-4e8b-b3a9-27592818d783	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-05 11:46:13.541261	2026-05-05 11:46:13.541261	\N	PENDING	1	\N	f	\N	\N
8a095af9-1a35-4f7a-843b-194fa4bbd49b	857f7c59-4ff6-45af-81fc-81d534af18de	RETOUR	33	8bfd5efe-e28c-4611-a2c2-e6990374b1dc	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-05 11:47:56.120176	2026-05-05 11:47:56.120176	\N	PENDING	1	\N	f	\N	\N
8b1c8639-c2e4-4e65-883b-a2d4db87324f	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-52	5071bf90-d610-404f-a14f-6364881c3f53	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-05 11:47:56.138187	2026-05-05 11:47:56.138187	\N	PENDING	1	\N	f	\N	\N
f7eb9ffa-53ff-4fc5-8e8c-b7e7af12969c	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	17	5f606c70-ae96-4242-befe-76f61eaf236e	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-05 11:47:56.149446	2026-05-05 11:47:56.149446	\N	PENDING	1	\N	f	\N	\N
1379ff79-cc9b-4ed3-be9a-119c16263bb5	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-32	5071bf90-d610-404f-a14f-6364881c3f53	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-05 11:47:56.160464	2026-05-05 11:47:56.160464	\N	PENDING	1	\N	f	\N	\N
446a867d-12a1-43be-9d3a-5ff929920e2f	f547b468-aee0-4109-acde-7ebb93d207d8	ENTREE	1000	513310e7-f0f1-4a3f-b886-756a28006fe1	ENTREE_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Approvisionnement - PRODUCTION	Facture: N/A | Lot: N/A | Source: {c656b280-7660-4ebf-b20c-28880d7cd5f7}	2026-05-05 17:16:57.792367	2026-05-05 17:16:57.792367	\N	PENDING	1	\N	f	\N	\N
fbf5c922-22e5-4270-8494-ce3d2507c078	f547b468-aee0-4109-acde-7ebb93d207d8	SORTIE	-55	3a82ecdc-9be7-4775-b7ef-e5c2de881592	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-05 17:18:25.357062	2026-05-05 17:18:25.357062	\N	PENDING	1	\N	f	\N	\N
f1733879-00bb-4a79-a0d7-039090fdbee6	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-35	c1245d72-233f-44bd-93e5-d9af30564cda	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-05 17:18:25.438401	2026-05-05 17:18:25.438401	\N	PENDING	1	\N	f	\N	\N
c21487c7-b76e-4c81-bb3a-169adc6aca69	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-15	e3a88637-ae66-48b9-bf94-74a09aea8b24	ARTICLE_REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Repartition équipe	\N	2026-05-05 17:18:25.468056	2026-05-05 17:18:25.468056	\N	PENDING	1	\N	f	\N	\N
87110d7d-9481-4e02-bbef-2df25e7a7778	f547b468-aee0-4109-acde-7ebb93d207d8	RETOUR	30	de582787-d402-438b-8f4d-e83f2bb58f88	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-05 17:20:15.939883	2026-05-05 17:20:15.939883	\N	PENDING	1	\N	f	\N	\N
b7ddd05c-bb90-4308-a1bb-54e311380956	f547b468-aee0-4109-acde-7ebb93d207d8	SORTIE	-55	20d5464d-478c-4b45-b717-fee8e4be07a9	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-05 17:20:15.959031	2026-05-05 17:20:15.959031	\N	PENDING	1	\N	f	\N	\N
fe781640-1af9-4e5f-b8ef-dd13e8bd83ff	857f7c59-4ff6-45af-81fc-81d534af18de	RETOUR	18	d455d0c8-c14f-4034-aaf6-5052206df4fa	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-05 17:20:15.971295	2026-05-05 17:20:15.971295	\N	PENDING	1	\N	f	\N	\N
e51d57c7-e221-4249-bdb2-b5b88abf3cde	857f7c59-4ff6-45af-81fc-81d534af18de	SORTIE	-35	20d5464d-478c-4b45-b717-fee8e4be07a9	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-05 17:20:15.980808	2026-05-05 17:20:15.980808	\N	PENDING	1	\N	f	\N	\N
97a55c06-3752-4038-becd-9bbc454b7f23	03dc330a-6da1-41ca-86ba-60945244182a	RETOUR	3	5ca32fce-5fe5-42a8-8521-a64058588576	RETOUR_STOCK	78a24f11-9714-4014-ae40-f38318fef119	WAREHOUSE	Retour INVENDU	Retour invendus répartition	2026-05-05 17:20:15.99224	2026-05-05 17:20:15.99224	\N	PENDING	1	\N	f	\N	\N
cf60be5e-1bfb-4aa2-8abc-adb644c25a18	03dc330a-6da1-41ca-86ba-60945244182a	SORTIE	-15	20d5464d-478c-4b45-b717-fee8e4be07a9	REPARTITION	78a24f11-9714-4014-ae40-f38318fef119	IN_TRANSIT	Clôture répartition : purge transit	\N	2026-05-05 17:20:16.003262	2026-05-05 17:20:16.003262	\N	PENDING	1	\N	f	\N	\N
\.


--
-- Data for Name: stock_soldes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.stock_soldes (solde_id, produit_id, quantite_total, quantite_reserve, valeur_stock, prix_moyen, dernier_mouvement_date, updated_at, location_id, location_historique, derniere_location_id, sync_status, version, deleted_at, is_deleted, created_at) FROM stdin;
c15897c2-6e21-44dd-9d29-58f7298cbe81	5058c2e2-506f-42bf-aa45-9105b435f4dc	100	0	0.00	12.00	2026-04-21 12:43:15.444513	2026-05-04 12:09:38.774611	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE	PENDING	1	\N	f	2026-05-07 12:50:05.292801
50cdebdd-781b-4700-9f6b-8cdd5a526d6f	6f7b704d-99c4-4837-a04b-be52c27f33d9	0	0	0.00	1200.00	\N	2026-05-04 12:09:38.774611	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE	PENDING	1	\N	f	2026-05-07 12:50:05.292801
20a266ea-c8c5-40cf-88dd-e6fae3f8ae65	f547b468-aee0-4109-acde-7ebb93d207d8	920	0	0.00	\N	2026-05-05 17:20:15.959031	2026-05-05 17:20:15.959031	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE	PENDING	1	\N	f	2026-05-07 12:50:05.292801
a18952cf-dfd5-44a8-9bed-3bbe36c7c343	857f7c59-4ff6-45af-81fc-81d534af18de	2813	0	0.00	1200.00	2026-05-05 17:20:15.980808	2026-05-05 17:20:15.980808	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE	PENDING	1	\N	f	2026-05-07 12:50:05.292801
b78a830c-afc2-47d9-b981-9e9c78d220fb	03dc330a-6da1-41ca-86ba-60945244182a	1087	0	0.00	1000.00	2026-05-05 17:20:16.003262	2026-05-05 17:20:16.003262	WAREHOUSE	{"RETURNED": 0, "WAREHOUSE": 0, "IN_TRANSIT": 0}	WAREHOUSE	PENDING	1	\N	f	2026-05-07 12:50:05.292801
\.


--
-- Data for Name: tarifs_produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.tarifs_produits (tarif_id, grille_id, produit_id, prix_personnalise, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Data for Name: types_produits; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.types_produits (type_produit_id, code, nom, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
372480eb-993e-4ac0-862f-39466beb1863	MARCHANDISE	Marchandise	PENDING	1	\N	2026-05-07 12:44:15.4234	2026-05-07 12:44:15.4234
187835d8-22a6-4a08-bb6e-e93d2a334147	EMBALLAGE	Emballage	PENDING	1	\N	2026-05-07 12:44:15.4234	2026-05-07 12:44:15.4234
a9501a7c-4c6a-4792-820c-7b3773ac7ea6	SERVICE	Service	PENDING	1	\N	2026-05-07 12:44:15.4234	2026-05-07 12:44:15.4234
\.


--
-- Data for Name: types_vente; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.types_vente (type_vente_id, code, nom, created_at, updated_at) FROM stdin;
ea35f245-b1d4-4d57-81c3-047fa0a6f511	CASH	Vente cash	2026-05-07 12:51:43.840226	2026-05-07 12:51:43.840226
139fed84-074f-4ac1-a56c-aecc47d83c2a	CREDIT	Vente à crédit	2026-05-07 12:51:43.840226	2026-05-07 12:51:43.840226
\.


--
-- Data for Name: utilisateurs; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.utilisateurs (utilisateur_id, nom_utilisateur, email, hash_mot_passe, nom_complet, role_id, est_actif, date_creation, date_mise_a_jour, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
78a24f11-9714-4014-ae40-f38318fef119	admin	admin@semuliki.local	$2b$12$wqVrUB8BZ/OvhP9V1kru..yEO2gQeRK8Cg1sAG1Bk4VYGvo7iMUN2	Administrator	23b42043-5724-483d-89fe-0debdcc1d6e6	t	2026-04-14 18:21:35.118185	2026-04-14 18:21:35.118185	PENDING	1	\N	2026-05-07 12:46:49.939229	2026-05-07 12:46:49.939229
\.


--
-- Data for Name: ventes; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.ventes (vente_id, repartition_id, produit_id, client_id, quantite, type_vente_id, prix_unitaire, date_vente, sync_status, version, deleted_at, created_at, updated_at) FROM stdin;
\.


--
-- Name: analyses_labo analyses_labo_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.analyses_labo
    ADD CONSTRAINT analyses_labo_pkey PRIMARY KEY (analyse_id);


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
-- Name: commandes_achats commandes_achats_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.commandes_achats
    ADD CONSTRAINT commandes_achats_pkey PRIMARY KEY (commande_id);


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
-- Name: cuves cuves_code_cuve_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cuves
    ADD CONSTRAINT cuves_code_cuve_key UNIQUE (code_cuve);


--
-- Name: cuves cuves_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.cuves
    ADD CONSTRAINT cuves_pkey PRIMARY KEY (cuve_id);


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
-- Name: fournisseurs fournisseurs_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.fournisseurs
    ADD CONSTRAINT fournisseurs_pkey PRIMARY KEY (fournisseur_id);


--
-- Name: grilles_tarifaires grilles_tarifaires_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.grilles_tarifaires
    ADD CONSTRAINT grilles_tarifaires_pkey PRIMARY KEY (grille_id);


--
-- Name: journaux_audit journaux_audit_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.journaux_audit
    ADD CONSTRAINT journaux_audit_pkey PRIMARY KEY (journal_audit_id);


--
-- Name: lots_production lots_production_code_lot_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.lots_production
    ADD CONSTRAINT lots_production_code_lot_key UNIQUE (code_lot);


--
-- Name: lots_production lots_production_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.lots_production
    ADD CONSTRAINT lots_production_pkey PRIMARY KEY (lot_id);


--
-- Name: matieres_premieres matieres_premieres_code_matiere_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.matieres_premieres
    ADD CONSTRAINT matieres_premieres_code_matiere_key UNIQUE (code_matiere);


--
-- Name: matieres_premieres matieres_premieres_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.matieres_premieres
    ADD CONSTRAINT matieres_premieres_pkey PRIMARY KEY (matiere_id);


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
-- Name: tarifs_produits tarifs_produits_grille_id_produit_id_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tarifs_produits
    ADD CONSTRAINT tarifs_produits_grille_id_produit_id_key UNIQUE (grille_id, produit_id);


--
-- Name: tarifs_produits tarifs_produits_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tarifs_produits
    ADD CONSTRAINT tarifs_produits_pkey PRIMARY KEY (tarif_id);


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
-- Name: idx_analyses_labo_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_analyses_labo_sync_status ON public.analyses_labo USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_articles_repartition_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_articles_repartition_produit ON public.articles_repartition USING btree (produit_id);


--
-- Name: idx_articles_repartition_repartition; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_articles_repartition_repartition ON public.articles_repartition USING btree (repartition_id);


--
-- Name: idx_articles_repartition_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_articles_repartition_sync_status ON public.articles_repartition USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_categories_produits_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_categories_produits_sync_status ON public.categories_produits USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_clients_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_clients_sync_status ON public.clients USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_commandes_achats_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_commandes_achats_sync_status ON public.commandes_achats USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_credits_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_credits_sync_status ON public.credits USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


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
-- Name: idx_entrees_stock_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_entrees_stock_sync_status ON public.entrees_stock USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_equipes_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_equipes_sync_status ON public.equipes USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_fournisseurs_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_fournisseurs_sync_status ON public.fournisseurs USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_grilles_tarifaires_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_grilles_tarifaires_sync_status ON public.grilles_tarifaires USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


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
-- Name: idx_lots_prod_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_lots_prod_sync_status ON public.lots_production USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_mps_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_mps_sync_status ON public.matieres_premieres USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_mv_stock_cache_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_mv_stock_cache_produit ON public.mv_stock_cache USING btree (produit_id);


--
-- Name: idx_produits_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_produits_sync_status ON public.produits USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_receptions_caisse_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_receptions_caisse_sync_status ON public.receptions_caisse USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_repartitions_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_repartitions_sync_status ON public.repartitions USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


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
-- Name: idx_retours_stock_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_retours_stock_sync_status ON public.retours_stock USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_routes_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_routes_sync_status ON public.routes USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


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
-- Name: idx_stock_mouvements_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_mouvements_sync_status ON public.stock_mouvements USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_stock_soldes_location; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_soldes_location ON public.stock_soldes USING btree (location_id);


--
-- Name: idx_stock_soldes_produit; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_soldes_produit ON public.stock_soldes USING btree (produit_id);


--
-- Name: idx_stock_soldes_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_stock_soldes_sync_status ON public.stock_soldes USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_tarifs_produits_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_tarifs_produits_sync_status ON public.tarifs_produits USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


--
-- Name: idx_types_produits_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_types_produits_sync_status ON public.types_produits USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


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
-- Name: idx_ventes_sync_status; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_ventes_sync_status ON public.ventes USING btree (sync_status) WHERE (sync_status = 'PENDING'::public.sync_state);


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
-- Name: lots_production trg_no_delete_lots_production; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_no_delete_lots_production BEFORE DELETE ON public.lots_production FOR EACH ROW EXECUTE FUNCTION public.forbid_delete_physical();


--
-- Name: stock_mouvements trg_no_delete_stock_mouvements; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_no_delete_stock_mouvements BEFORE DELETE ON public.stock_mouvements FOR EACH ROW EXECUTE FUNCTION public.forbid_delete_physical();


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
-- Name: articles_repartition trg_sortie_on_articles_repartition; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sortie_on_articles_repartition AFTER INSERT ON public.articles_repartition FOR EACH ROW EXECUTE FUNCTION public.fn_create_sortie_on_repartition();


--
-- Name: lots_production trg_statut_lot_embouteille; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_statut_lot_embouteille AFTER UPDATE ON public.lots_production FOR EACH ROW WHEN (((new.statut_lot)::text = 'EMBOUTEILLE'::text)) EXECUTE FUNCTION public.trg_cuve_statut_apres_embouteillage();


--
-- Name: stock_mouvements trg_sync_stock_after_mouvement; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_stock_after_mouvement AFTER INSERT ON public.stock_mouvements FOR EACH ROW EXECUTE FUNCTION public.fn_sync_stock_after_movement();


--
-- Name: analyses_labo trg_sync_version_analyses_labo; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_analyses_labo BEFORE UPDATE ON public.analyses_labo FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: articles_repartition trg_sync_version_articles_repartition; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_articles_repartition BEFORE UPDATE ON public.articles_repartition FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: categories_produits trg_sync_version_categories_produits; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_categories_produits BEFORE UPDATE ON public.categories_produits FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: clients trg_sync_version_clients; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_clients BEFORE UPDATE ON public.clients FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: commandes_achats trg_sync_version_commandes_achats; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_commandes_achats BEFORE UPDATE ON public.commandes_achats FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: conditions_paiement trg_sync_version_conditions_paiement; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_conditions_paiement BEFORE UPDATE ON public.conditions_paiement FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: credits trg_sync_version_credits; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_credits BEFORE UPDATE ON public.credits FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: cuves trg_sync_version_cuves; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_cuves BEFORE UPDATE ON public.cuves FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: entrees_stock trg_sync_version_entrees_stock; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_entrees_stock BEFORE UPDATE ON public.entrees_stock FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: equipes trg_sync_version_equipes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_equipes BEFORE UPDATE ON public.equipes FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: fournisseurs trg_sync_version_fournisseurs; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_fournisseurs BEFORE UPDATE ON public.fournisseurs FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: grilles_tarifaires trg_sync_version_grilles_tarifaires; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_grilles_tarifaires BEFORE UPDATE ON public.grilles_tarifaires FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: lots_production trg_sync_version_lots_prod; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_lots_prod BEFORE UPDATE ON public.lots_production FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: matieres_premieres trg_sync_version_mps; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_mps BEFORE UPDATE ON public.matieres_premieres FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: permissions trg_sync_version_permissions; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_permissions BEFORE UPDATE ON public.permissions FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: produits trg_sync_version_produits; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_produits BEFORE UPDATE ON public.produits FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: raisons_retour trg_sync_version_raisons_retour; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_raisons_retour BEFORE UPDATE ON public.raisons_retour FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: receptions_caisse trg_sync_version_receptions_caisse; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_receptions_caisse BEFORE UPDATE ON public.receptions_caisse FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: repartitions trg_sync_version_repartitions; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_repartitions BEFORE UPDATE ON public.repartitions FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: retours_stock trg_sync_version_retours_stock; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_retours_stock BEFORE UPDATE ON public.retours_stock FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: roles trg_sync_version_roles; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_roles BEFORE UPDATE ON public.roles FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: routes trg_sync_version_routes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_routes BEFORE UPDATE ON public.routes FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: sources_entree trg_sync_version_sources_entree; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_sources_entree BEFORE UPDATE ON public.sources_entree FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: stock_mouvements trg_sync_version_stock_mouvements; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_stock_mouvements BEFORE UPDATE ON public.stock_mouvements FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: stock_soldes trg_sync_version_stock_soldes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_stock_soldes BEFORE UPDATE ON public.stock_soldes FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: tarifs_produits trg_sync_version_tarifs_produits; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_tarifs_produits BEFORE UPDATE ON public.tarifs_produits FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: types_produits trg_sync_version_types_produits; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_types_produits BEFORE UPDATE ON public.types_produits FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: utilisateurs trg_sync_version_utilisateurs; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_utilisateurs BEFORE UPDATE ON public.utilisateurs FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: ventes trg_sync_version_ventes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_sync_version_ventes BEFORE UPDATE ON public.ventes FOR EACH ROW EXECUTE FUNCTION public.trg_maj_sync_version();


--
-- Name: entrees_stock trg_update_date_entrees_stock; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_update_date_entrees_stock BEFORE INSERT OR UPDATE ON public.entrees_stock FOR EACH ROW EXECUTE FUNCTION public.trg_maj_date();


--
-- Name: stock_soldes trg_update_date_stock_soldes; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER trg_update_date_stock_soldes BEFORE INSERT OR UPDATE ON public.stock_soldes FOR EACH ROW EXECUTE FUNCTION public.trg_maj_date();


--
-- Name: analyses_labo analyses_labo_lot_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.analyses_labo
    ADD CONSTRAINT analyses_labo_lot_id_fkey FOREIGN KEY (lot_id) REFERENCES public.lots_production(lot_id);


--
-- Name: analyses_labo analyses_labo_technicien_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.analyses_labo
    ADD CONSTRAINT analyses_labo_technicien_id_fkey FOREIGN KEY (technicien_id) REFERENCES public.utilisateurs(utilisateur_id);


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
-- Name: clients clients_grille_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_grille_id_fkey FOREIGN KEY (grille_id) REFERENCES public.grilles_tarifaires(grille_id);


--
-- Name: clients clients_route_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.clients
    ADD CONSTRAINT clients_route_id_fkey FOREIGN KEY (route_id) REFERENCES public.routes(route_id);


--
-- Name: commandes_achats commandes_achats_cree_par_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.commandes_achats
    ADD CONSTRAINT commandes_achats_cree_par_fkey FOREIGN KEY (cree_par) REFERENCES public.utilisateurs(utilisateur_id);


--
-- Name: commandes_achats commandes_achats_fournisseur_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.commandes_achats
    ADD CONSTRAINT commandes_achats_fournisseur_id_fkey FOREIGN KEY (fournisseur_id) REFERENCES public.fournisseurs(fournisseur_id);


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
-- Name: stock_mouvements fk_stock_mvt_lot; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.stock_mouvements
    ADD CONSTRAINT fk_stock_mvt_lot FOREIGN KEY (lot_id) REFERENCES public.lots_production(lot_id);


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
-- Name: lots_production lots_production_cuve_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.lots_production
    ADD CONSTRAINT lots_production_cuve_id_fkey FOREIGN KEY (cuve_id) REFERENCES public.cuves(cuve_id);


--
-- Name: lots_production lots_production_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.lots_production
    ADD CONSTRAINT lots_production_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id);


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
-- Name: tarifs_produits tarifs_produits_grille_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tarifs_produits
    ADD CONSTRAINT tarifs_produits_grille_id_fkey FOREIGN KEY (grille_id) REFERENCES public.grilles_tarifaires(grille_id) ON DELETE CASCADE;


--
-- Name: tarifs_produits tarifs_produits_produit_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.tarifs_produits
    ADD CONSTRAINT tarifs_produits_produit_id_fkey FOREIGN KEY (produit_id) REFERENCES public.produits(produit_id) ON DELETE CASCADE;


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

\unrestrict aE8qyv5pQgcadRRB3Gtzm7FMgAh8h3SfFvfrSAxdp2usU6SYaXjreHWE1E5sycn

