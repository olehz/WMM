DROP TABLE IF EXISTS wpt_deltas;CREATE TABLE wpt_deltas AS
WITH points AS (
    SELECT
        identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_terminal_waypoint
    UNION
    SELECT
        identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_enroute_waypoint
    UNION
    SELECT
        ndb_identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_enroute_ndb_navaid
    UNION
    SELECT
        vor_identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_vhf_navaid
    UNION
    SELECT
        ndb_identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_terminal_ndb_navaid
)

SELECT
    ident,
    magnetic_variation AS jeppesen_mag_var,
    ROUND(noaa * 10) / 10 AS wmm_mag_var,
    delta
    --,ST_X(geom) AS lon, ST_Y(geom) AS lat
    --,geom
FROM
    points, import.mag_var,
    ST_Value(declination, geom) noaa,
    ROUND(ABS(magnetic_variation - noaa)) delta
WHERE
    ABS(ST_Y(geom)) < 85 AND delta > 1 AND
    ST_Intersects(declination, geom) AND delta > 2
ORDER BY delta DESC;



WITH points AS (
    SELECT
        identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_terminal_waypoint
    UNION
    SELECT
        identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_enroute_waypoint
    UNION
    SELECT
        ndb_identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_enroute_ndb_navaid
    UNION
    SELECT
        vor_identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_vhf_navaid
    UNION
    SELECT
        ndb_identifier AS ident, magnetic_variation, geom
    FROM arinc424_gis.tbl_terminal_ndb_navaid
), dif AS (
    SELECT
        ARRAY_TO_STRING(ARRAY_AGG(DISTINCT ident), '/') AS ident,
        ARRAY_AGG(magnetic_variation ORDER BY magnetic_variation) AS magnetic_variation,
        ST_X(geom) AS lon, ST_Y(geom) AS lat
    FROM points
    GROUP BY geom
    HAVING COUNT(*) > 1 AND ARRAY_LENGTH(ARRAY_AGG(DISTINCT magnetic_variation), 1) > 1
)

SELECT
    ident,
    ARRAY_TO_STRING(magnetic_variation, '/'),
    lon, lat
FROM dif ORDER BY LENGTH(ident) DESC, ABS(magnetic_variation[1] - magnetic_variation[2]) DESC;