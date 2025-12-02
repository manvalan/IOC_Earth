#include "MapPathRenderer.h"
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cmath>

namespace ioc_earth {

MapPathRenderer::MapPathRenderer(unsigned int width, unsigned int height)
    : width_(width), height_(height) {
    initializeMap();
}

MapPathRenderer::~MapPathRenderer() = default;

void MapPathRenderer::initializeMap() {
    // Inizializza Mapnik
    mapnik::datasource_cache::instance().register_datasources("/usr/local/lib/mapnik/input");
    mapnik::freetype_engine::register_fonts("/usr/share/fonts", true);
    
    // Crea la mappa
    map_ = std::make_unique<mapnik::Map>(width_, height_);
    map_->set_background(mapnik::color("white"));
    
    // Imposta il sistema di proiezione (WGS84)
    map_->set_srs("+proj=longlat +datum=WGS84 +no_defs");
}

void MapPathRenderer::setExtent(double min_lon, double min_lat, double max_lon, double max_lat) {
    mapnik::box2d<double> bbox(min_lon, min_lat, max_lon, max_lat);
    map_->zoom_to_box(bbox);
}

void MapPathRenderer::addShapefileLayer(const std::string& shapefile_path, const std::string& layer_name) {
    // Configura i parametri del datasource
    mapnik::parameters params;
    params["type"] = "shape";
    params["file"] = shapefile_path;
    
    // Crea il layer
    mapnik::layer lyr(layer_name);
    lyr.set_datasource(mapnik::datasource_cache::instance().create(params));
    lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");
    
    // Crea uno stile per il layer
    mapnik::feature_type_style style;
    mapnik::rule r;
    
    // Aggiungi simbolizzatori per poligoni e linee
    mapnik::line_symbolizer line_sym;
    mapnik::color line_color(128, 128, 128); // grigio
    line_sym.set_stroke(mapnik::stroke(line_color, 1.0));
    r.append(std::move(line_sym));
    
    style.add_rule(std::move(r));
    
    // Aggiungi lo stile e il layer alla mappa
    map_->insert_style(layer_name + "_style", style);
    lyr.add_style(layer_name + "_style");
    map_->add_layer(lyr);
}

void MapPathRenderer::addGPSPath(const std::vector<GPSPoint>& points,
                                 const std::string& line_color,
                                 double line_width) {
    if (points.empty()) {
        return;
    }
    
    // Crea un memory datasource
    mapnik::parameters params;
    params["type"] = "memory";
    auto ds = std::make_shared<mapnik::memory_datasource>(params);
    
    // Crea una geometria linestring
    mapnik::geometry::line_string<double> line;
    for (const auto& point : points) {
        line.add_coord(point.longitude, point.latitude);
    }
    
    // Crea una feature e aggiungi la geometria
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    mapnik::feature_ptr feature = std::make_shared<mapnik::feature_impl>(ctx, 1);
    feature->set_geometry(mapnik::geometry::geometry<double>(line));
    ds->push(feature);
    
    // Crea il layer
    mapnik::layer lyr("gps_path");
    lyr.set_datasource(ds);
    lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");
    
    // Crea lo stile
    mapnik::feature_type_style style;
    mapnik::rule r;
    
    mapnik::line_symbolizer line_sym;
    mapnik::color color_obj;
    if (line_color[0] == '#') {
        color_obj = mapnik::color(line_color);
    } else {
        color_obj = mapnik::color(line_color);
    }
    line_sym.set_stroke(mapnik::stroke(color_obj, line_width));
    r.append(std::move(line_sym));
    
    style.add_rule(std::move(r));
    
    // Aggiungi alla mappa
    map_->insert_style("gps_path_style", style);
    lyr.add_style("gps_path_style");
    map_->add_layer(lyr);
}

void MapPathRenderer::addPointLabels(const std::vector<GPSPoint>& points,
                                     const std::string& label_field,
                                     int font_size) {
    if (points.empty()) {
        return;
    }
    
    // Crea un memory datasource per i punti
    mapnik::parameters params;
    params["type"] = "memory";
    auto ds = std::make_shared<mapnik::memory_datasource>(params);
    
    // Crea un context con il campo per le etichette
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    ctx->push("label");
    
    // Aggiungi i punti
    int feature_id = 1;
    for (const auto& point : points) {
        mapnik::feature_ptr feature = std::make_shared<mapnik::feature_impl>(ctx, feature_id++);
        
        // Crea la geometria del punto
        mapnik::geometry::point<double> pt(point.longitude, point.latitude);
        feature->set_geometry(mapnik::geometry::geometry<double>(pt));
        
        // Imposta l'etichetta
        if (label_field == "timestamp") {
            feature->put("label", point.timestamp);
        } else {
            feature->put("label", point.timestamp); // Default al timestamp
        }
        
        ds->push(feature);
    }
    
    // Crea il layer
    mapnik::layer lyr("gps_points");
    lyr.set_datasource(ds);
    lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");
    
    // Crea lo stile con markers e testo
    mapnik::feature_type_style style;
    mapnik::rule r;
    
    // Aggiungi marker per i punti
    mapnik::markers_symbolizer marker_sym;
    r.append(std::move(marker_sym));
    
    // Aggiungi text symbolizer
    mapnik::text_symbolizer text_sym;
    mapnik::text_placements_ptr placements = std::make_shared<mapnik::text_placements_dummy>();
    text_sym.set_placements(placements);
    
    // Configura le proprietà del testo
    auto text_props = std::make_shared<mapnik::formatting::text_node>(
        mapnik::parse_expression("[label]"));
    text_sym.properties.from_xml = text_props;
    text_sym.properties.format_defaults.face_name = "DejaVu Sans Book";
    text_sym.properties.format_defaults.text_size = font_size;
    text_sym.properties.format_defaults.fill = mapnik::color("black");
    
    r.append(std::move(text_sym));
    
    style.add_rule(std::move(r));
    
    // Aggiungi alla mappa
    map_->insert_style("gps_points_style", style);
    lyr.add_style("gps_points_style");
    map_->add_layer(lyr);
}

void MapPathRenderer::setBackgroundColor(const std::string& color) {
    map_->set_background(mapnik::color(color));
}

bool MapPathRenderer::renderToFile(const std::string& output_path) {
    try {
        // Crea l'immagine
        mapnik::image_rgba8 img(width_, height_);
        
        // Renderizza
        mapnik::agg_renderer<mapnik::image_rgba8> renderer(*map_, img);
        renderer.apply();
        
        // Salva su file
        mapnik::save_to_file(img, output_path, "png");
        
        return true;
    } catch (const std::exception& e) {
        // Log error (in una implementazione reale, usare un sistema di logging)
        return false;
    }
}

void MapPathRenderer::autoSetExtentFromPoints(const std::vector<GPSPoint>& points,
                                               double margin_percent) {
    if (points.empty()) {
        return;
    }
    
    // Trova i limiti del bounding box
    double min_lon = std::numeric_limits<double>::max();
    double max_lon = std::numeric_limits<double>::lowest();
    double min_lat = std::numeric_limits<double>::max();
    double max_lat = std::numeric_limits<double>::lowest();
    
    for (const auto& point : points) {
        min_lon = std::min(min_lon, point.longitude);
        max_lon = std::max(max_lon, point.longitude);
        min_lat = std::min(min_lat, point.latitude);
        max_lat = std::max(max_lat, point.latitude);
    }
    
    // Aggiungi margine
    double lon_margin = (max_lon - min_lon) * (margin_percent / 100.0);
    double lat_margin = (max_lat - min_lat) * (margin_percent / 100.0);
    
    min_lon -= lon_margin;
    max_lon += lon_margin;
    min_lat -= lat_margin;
    max_lat += lat_margin;
    
    setExtent(min_lon, min_lat, max_lon, max_lat);
}

std::string MapPathRenderer::createGeoJSONFromPoints(const std::vector<GPSPoint>& points) {
    std::ostringstream oss;
    oss << "{\"type\":\"FeatureCollection\",\"features\":[";
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":["
            << points[i].longitude << "," << points[i].latitude << "]},"
            << "\"properties\":{\"timestamp\":\"" << points[i].timestamp << "\"}}";
    }
    
    oss << "]}";
    return oss.str();
}

} // namespace ioc_earth
