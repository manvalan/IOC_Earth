#include "MapPathRenderer.h"
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/value.hpp>
#include <sstream>
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>

namespace ioc_earth {

MapPathRenderer::MapPathRenderer(unsigned int width, unsigned int height)
    : width_(width), height_(height) {
    initializeMap();
}

MapPathRenderer::~MapPathRenderer() = default;

void MapPathRenderer::initializeMap() {
    // Inizializza Mapnik - percorsi per macOS con Homebrew
    try {
        mapnik::datasource_cache::instance().register_datasources("/opt/homebrew/lib/mapnik/input");
    } catch (...) {
        std::cerr << "Warning: Could not register datasources" << std::endl;
    }
    
    try {
        mapnik::freetype_engine::register_fonts("/opt/homebrew/share/fonts", true);
        mapnik::freetype_engine::register_fonts("/System/Library/Fonts", true);
    } catch (...) {
        std::cerr << "Warning: Could not register fonts" << std::endl;
    }
    
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
    try {
        // Configura i parametri del datasource
        mapnik::parameters params;
        params["type"] = "shape";
        params["file"] = shapefile_path;
        
        // Crea il layer
        mapnik::layer lyr(layer_name);
        lyr.set_datasource(mapnik::datasource_cache::instance().create(params));
        lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");
        
        // Crea uno stile semplice per il layer
        mapnik::feature_type_style style;
        mapnik::rule r;
        
        // Aggiungi un line symbolizer di base
        mapnik::line_symbolizer line_sym;
        mapnik::put(line_sym, mapnik::keys::stroke, mapnik::color(128, 128, 128));
        mapnik::put(line_sym, mapnik::keys::stroke_width, 1.0);
        r.append(std::move(line_sym));
        
        style.add_rule(std::move(r));
        
        // Aggiungi lo stile e il layer alla mappa
        map_->insert_style(layer_name + "_style", style);
        lyr.add_style(layer_name + "_style");
        map_->add_layer(lyr);
    } catch (const std::exception& e) {
        std::cerr << "Error adding shapefile layer: " << e.what() << std::endl;
    }
}

void MapPathRenderer::addGPSPath(const std::vector<GPSPoint>& points,
                                 const std::string& line_color,
                                 double line_width) {
    if (points.empty()) {
        return;
    }
    
    try {
        // Crea un memory datasource
        mapnik::parameters params;
        params["type"] = "memory";
        auto ds = std::make_shared<mapnik::memory_datasource>(params);
        
        // Crea una geometria linestring usando l'API corretta di Mapnik 4
        mapnik::geometry::line_string<double> line;
        for (const auto& point : points) {
            line.emplace_back(point.longitude, point.latitude);
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
        mapnik::color color_obj(line_color);
        mapnik::put(line_sym, mapnik::keys::stroke, color_obj);
        mapnik::put(line_sym, mapnik::keys::stroke_width, line_width);
        r.append(std::move(line_sym));
        
        style.add_rule(std::move(r));
        
        // Aggiungi alla mappa
        map_->insert_style("gps_path_style", style);
        lyr.add_style("gps_path_style");
        map_->add_layer(lyr);
    } catch (const std::exception& e) {
        std::cerr << "Error adding GPS path: " << e.what() << std::endl;
    }
}

void MapPathRenderer::addPointLabels(const std::vector<GPSPoint>& points,
                                     const std::string& label_field,
                                     int font_size) {
    if (points.empty()) {
        return;
    }
    
    try {
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
            
            // Imposta l'etichetta usando UnicodeString
            if (!point.timestamp.empty()) {
                feature->put("label", mapnik::value_unicode_string(point.timestamp.c_str()));
            }
            
            ds->push(feature);
        }
        
        // Crea il layer
        mapnik::layer lyr("gps_points");
        lyr.set_datasource(ds);
        lyr.set_srs("+proj=longlat +datum=WGS84 +no_defs");
        
        // Crea lo stile con solo markers (semplificato)
        mapnik::feature_type_style style;
        mapnik::rule r;
        
        // Aggiungi marker per i punti
        mapnik::markers_symbolizer marker_sym;
        mapnik::put(marker_sym, mapnik::keys::fill, mapnik::color(255, 0, 0));
        mapnik::put(marker_sym, mapnik::keys::width, mapnik::value_double(8.0));
        mapnik::put(marker_sym, mapnik::keys::height, mapnik::value_double(8.0));
        r.append(std::move(marker_sym));
        
        // Nota: Text symbolizer in Mapnik 4 ha un'API complessa
        // Per ora usiamo solo i markers. Il text symbolizer richiede
        // una configurazione più avanzata che dipende dalla versione specifica
        
        style.add_rule(std::move(r));
        
        // Aggiungi alla mappa
        map_->insert_style("gps_points_style", style);
        lyr.add_style("gps_points_style");
        map_->add_layer(lyr);
    } catch (const std::exception& e) {
        std::cerr << "Error adding point labels: " << e.what() << std::endl;
    }
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
        std::cerr << "Error rendering to file: " << e.what() << std::endl;
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
    
    // Assicurati che ci sia almeno un margine minimo
    if (lon_margin < 0.1) lon_margin = 0.1;
    if (lat_margin < 0.1) lat_margin = 0.1;
    
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
