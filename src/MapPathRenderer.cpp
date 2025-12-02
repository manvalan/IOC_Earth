#include "MapPathRenderer.hpp"

// Mapnik headers
#include <mapnik/map.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/color.hpp>
#include <mapnik/image.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/expression.hpp>

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace ioc_earth {

MapPathRenderer::MapPathRenderer(int width, int height, const std::string& projection)
    : map_(std::make_unique<mapnik::Map>(width, height))
    , projection_(projection)
    , layer_count_(0)
    , font_name_("DejaVu Sans Book")
{
    // Set the map spatial reference system
    map_->set_srs("+init=" + projection_);
    
    // Set a default background color (light blue for ocean)
    map_->set_background(mapnik::color(200, 220, 240));
}

MapPathRenderer::~MapPathRenderer() = default;

bool MapPathRenderer::loadBaseLayer(const std::string& shp_path, const std::string& style_name)
{
    try {
        // Create datasource parameters for Shapefile
        mapnik::parameters params;
        params["type"] = "shape";
        params["file"] = shp_path;
        
        // Create the datasource
        auto ds = mapnik::datasource_cache::instance().create(params);
        if (!ds) {
            return false;
        }
        
        // Create a layer
        mapnik::layer lyr(style_name);
        lyr.set_datasource(ds);
        lyr.set_srs("+init=epsg:4326"); // Shapefiles are typically in WGS84
        
        // Create style with fill and stroke
        mapnik::feature_type_style style;
        mapnik::rule rule;
        
        // Add polygon symbolizer (fill)
        mapnik::polygon_symbolizer poly_sym;
        put(poly_sym, mapnik::keys::fill, mapnik::color(240, 240, 230)); // Light beige fill
        put(poly_sym, mapnik::keys::fill_opacity, 1.0);
        rule.append(std::move(poly_sym));
        
        // Add line symbolizer (stroke/border)
        mapnik::line_symbolizer line_sym;
        put(line_sym, mapnik::keys::stroke, mapnik::color(128, 128, 128)); // Gray stroke
        put(line_sym, mapnik::keys::stroke_width, 0.5);
        rule.append(std::move(line_sym));
        
        style.add_rule(std::move(rule));
        
        // Add style to map and assign to layer
        map_->insert_style(style_name, std::move(style));
        lyr.add_style(style_name);
        
        // Add layer to map
        map_->add_layer(std::move(lyr));
        
        ++layer_count_;
        return true;
    }
    catch (const std::exception& e) {
        last_error_ = e.what();
        return false;
    }
}

void MapPathRenderer::addTrackingPath(const std::vector<TrackingPoint>& points,
                                      const std::string& line_color,
                                      double line_width)
{
    if (points.size() < 2) {
        return; // Need at least 2 points for a line
    }
    
    // Store points for later extent calculation
    all_tracking_points_.insert(all_tracking_points_.end(), points.begin(), points.end());
    
    // Create a unique style name
    std::string style_name = "tracking_path_" + std::to_string(layer_count_);
    
    // Create the line style
    createLineStyle(style_name, line_color, line_width);
    
    // Create memory datasource for the path
    mapnik::parameters params;
    params["type"] = "memory";
    
    auto ctx = std::make_shared<mapnik::context_type>();
    auto ds = std::make_shared<mapnik::memory_datasource>(params);
    
    // Create a linestring geometry
    mapnik::geometry::line_string<double> line;
    
    for (const auto& pt : points) {
        double x, y;
        transformCoordinates(pt.longitude, pt.latitude, x, y);
        line.emplace_back(x, y);
    }
    
    // Create feature with the linestring
    mapnik::feature_ptr feature = mapnik::feature_factory::create(ctx, 1);
    feature->set_geometry(mapnik::geometry::geometry<double>(std::move(line)));
    ds->push(feature);
    
    // Create and add layer
    mapnik::layer lyr(style_name);
    lyr.set_datasource(ds);
    lyr.set_srs(map_->srs());
    lyr.add_style(style_name);
    
    map_->add_layer(std::move(lyr));
    ++layer_count_;
}

void MapPathRenderer::addPointMarkers(const std::vector<TrackingPoint>& points,
                                      const std::string& marker_color,
                                      double marker_size,
                                      bool show_labels)
{
    if (points.empty()) {
        return;
    }
    
    // Store points for later extent calculation
    all_tracking_points_.insert(all_tracking_points_.end(), points.begin(), points.end());
    
    // Create a unique style name
    std::string style_name = "point_markers_" + std::to_string(layer_count_);
    
    // Create the point style
    createPointStyle(style_name, marker_color, marker_size, show_labels);
    
    // Create memory datasource for the points
    mapnik::parameters params;
    params["type"] = "memory";
    
    auto ctx = std::make_shared<mapnik::context_type>();
    ctx->push("timestamp");
    
    auto ds = std::make_shared<mapnik::memory_datasource>(params);
    
    mapnik::value_integer id = 1;
    for (const auto& pt : points) {
        double x, y;
        transformCoordinates(pt.longitude, pt.latitude, x, y);
        
        mapnik::feature_ptr feature = mapnik::feature_factory::create(ctx, id++);
        feature->set_geometry(mapnik::geometry::geometry<double>(
            mapnik::geometry::point<double>(x, y)));
        
        // Add timestamp attribute for labeling
        if (!pt.timestamp.empty()) {
            feature->put("timestamp", mapnik::value_unicode_string::fromUTF8(pt.timestamp));
        }
        
        ds->push(feature);
    }
    
    // Create and add layer
    mapnik::layer lyr(style_name);
    lyr.set_datasource(ds);
    lyr.set_srs(map_->srs());
    lyr.add_style(style_name);
    
    map_->add_layer(std::move(lyr));
    ++layer_count_;
}

void MapPathRenderer::setExtent(double min_lon, double min_lat, double max_lon, double max_lat)
{
    double min_x, min_y, max_x, max_y;
    transformCoordinates(min_lon, min_lat, min_x, min_y);
    transformCoordinates(max_lon, max_lat, max_x, max_y);
    
    map_->zoom_to_box(mapnik::box2d<double>(min_x, min_y, max_x, max_y));
}

void MapPathRenderer::zoomToTrackingPoints(double padding)
{
    if (all_tracking_points_.empty()) {
        return;
    }
    
    // Find bounding box of all tracking points
    double min_lon = all_tracking_points_[0].longitude;
    double max_lon = all_tracking_points_[0].longitude;
    double min_lat = all_tracking_points_[0].latitude;
    double max_lat = all_tracking_points_[0].latitude;
    
    for (const auto& pt : all_tracking_points_) {
        min_lon = std::min(min_lon, pt.longitude);
        max_lon = std::max(max_lon, pt.longitude);
        min_lat = std::min(min_lat, pt.latitude);
        max_lat = std::max(max_lat, pt.latitude);
    }
    
    // Add padding
    double lon_range = max_lon - min_lon;
    double lat_range = max_lat - min_lat;
    
    // Ensure minimum extent
    if (lon_range < 0.01) lon_range = 0.01;
    if (lat_range < 0.01) lat_range = 0.01;
    
    min_lon -= lon_range * padding;
    max_lon += lon_range * padding;
    min_lat -= lat_range * padding;
    max_lat += lat_range * padding;
    
    setExtent(min_lon, min_lat, max_lon, max_lat);
}

bool MapPathRenderer::render(const std::string& output_path)
{
    try {
        // Create an image buffer
        mapnik::image_rgba8 image(map_->width(), map_->height());
        
        // Create and run the renderer
        mapnik::agg_renderer<mapnik::image_rgba8> renderer(*map_, image);
        renderer.apply();
        
        // Save to PNG file
        mapnik::save_to_file(image, output_path, "png");
        
        return true;
    }
    catch (const std::exception& e) {
        last_error_ = e.what();
        return false;
    }
}

int MapPathRenderer::getWidth() const
{
    return static_cast<int>(map_->width());
}

int MapPathRenderer::getHeight() const
{
    return static_cast<int>(map_->height());
}

std::string MapPathRenderer::getProjection() const
{
    return projection_;
}

std::string MapPathRenderer::getLastError() const
{
    return last_error_;
}

void MapPathRenderer::setFontName(const std::string& font_name)
{
    font_name_ = font_name;
}

std::string MapPathRenderer::getFontName() const
{
    return font_name_;
}

void MapPathRenderer::transformCoordinates(double lon, double lat, double& x, double& y) const
{
    // Transform from WGS84 (epsg:4326) to map projection
    mapnik::projection source("+init=epsg:4326");
    mapnik::projection dest(map_->srs());
    mapnik::proj_transform transform(source, dest);
    
    x = lon;
    y = lat;
    double z = 0;
    
    transform.forward(x, y, z);
}

void MapPathRenderer::createLineStyle(const std::string& style_name, 
                                      const std::string& color, 
                                      double width)
{
    mapnik::feature_type_style style;
    mapnik::rule rule;
    
    mapnik::line_symbolizer line_sym;
    put(line_sym, mapnik::keys::stroke, mapnik::color(color));
    put(line_sym, mapnik::keys::stroke_width, width);
    put(line_sym, mapnik::keys::stroke_linecap, mapnik::line_cap_enum::ROUND_CAP);
    put(line_sym, mapnik::keys::stroke_linejoin, mapnik::line_join_enum::ROUND_JOIN);
    
    rule.append(std::move(line_sym));
    style.add_rule(std::move(rule));
    
    map_->insert_style(style_name, std::move(style));
}

void MapPathRenderer::createPointStyle(const std::string& style_name,
                                       const std::string& color,
                                       double size,
                                       bool show_labels)
{
    mapnik::feature_type_style style;
    mapnik::rule rule;
    
    // Add marker symbolizer for points
    mapnik::markers_symbolizer marker_sym;
    put(marker_sym, mapnik::keys::fill, mapnik::color(color));
    put(marker_sym, mapnik::keys::width, size);
    put(marker_sym, mapnik::keys::height, size);
    put(marker_sym, mapnik::keys::stroke, mapnik::color("white"));
    put(marker_sym, mapnik::keys::stroke_width, 1.0);
    put(marker_sym, mapnik::keys::allow_overlap, true);
    
    rule.append(std::move(marker_sym));
    
    // Add text symbolizer for labels if requested
    if (show_labels) {
        mapnik::text_symbolizer text_sym;
        
        // Create a simple expression for the timestamp field
        mapnik::text_placements_ptr placements = 
            std::make_shared<mapnik::text_placements_dummy>();
        
        placements->defaults.format_defaults.face_name = font_name_;
        placements->defaults.format_defaults.text_size = 10.0;
        placements->defaults.format_defaults.fill = mapnik::color("black");
        placements->defaults.format_defaults.halo_fill = mapnik::color("white");
        placements->defaults.format_defaults.halo_radius = 1.0;
        
        // Set the text expression
        placements->defaults.set_format_tree(
            std::make_shared<mapnik::formatting::text_node>(
                mapnik::parse_expression("[timestamp]")));
        
        put(text_sym, mapnik::keys::text_placements_, placements);
        put(text_sym, mapnik::keys::allow_overlap, false);
        
        // Set label placement
        mapnik::enumeration_wrapper placement_type(
            static_cast<int>(mapnik::label_placement_enum::POINT_PLACEMENT));
        put(text_sym, mapnik::keys::label_placement, placement_type);
        
        rule.append(std::move(text_sym));
    }
    
    style.add_rule(std::move(rule));
    map_->insert_style(style_name, std::move(style));
}

} // namespace ioc_earth
