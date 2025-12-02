#ifndef MAP_PATH_RENDERER_HPP
#define MAP_PATH_RENDERER_HPP

#include <string>
#include <vector>
#include <memory>
#include <mapnik/map.hpp>
#include <mapnik/geometry.hpp>

namespace ioc_earth {

/**
 * @brief Structure to hold GPS tracking point data
 */
struct TrackingPoint {
    double longitude;
    double latitude;
    std::string timestamp;
    
    TrackingPoint(double lon, double lat, const std::string& ts = "")
        : longitude(lon), latitude(lat), timestamp(ts) {}
};

/**
 * @brief Modern C++ class for rendering maps to PNG files using Mapnik
 * 
 * This class provides functionality to:
 * - Initialize a map canvas with specified dimensions
 * - Load Shapefile base layers (political boundaries/countries)
 * - Add GPS tracking paths with timestamps
 * - Render the final map to PNG format
 */
class MapPathRenderer {
public:
    /**
     * @brief Construct a new MapPathRenderer object
     * 
     * @param width Canvas width in pixels (default: 800)
     * @param height Canvas height in pixels (default: 600)
     * @param projection Map projection (default: "epsg:3857" - Web Mercator)
     */
    explicit MapPathRenderer(int width = 800, 
                            int height = 600, 
                            const std::string& projection = "epsg:3857");
    
    /**
     * @brief Destroy the MapPathRenderer object
     */
    ~MapPathRenderer();
    
    // Prevent copying
    MapPathRenderer(const MapPathRenderer&) = delete;
    MapPathRenderer& operator=(const MapPathRenderer&) = delete;
    
    // Allow moving
    MapPathRenderer(MapPathRenderer&&) noexcept = default;
    MapPathRenderer& operator=(MapPathRenderer&&) noexcept = default;
    
    /**
     * @brief Load a base layer from a Shapefile
     * 
     * @param shp_path Path to the Shapefile (.shp)
     * @param style_name Name for the style to apply
     * @return true if loading succeeded
     * @return false if loading failed
     */
    bool loadBaseLayer(const std::string& shp_path, const std::string& style_name);
    
    /**
     * @brief Add a GPS tracking path to the map
     * 
     * @param points Vector of TrackingPoint objects containing coordinates and timestamps
     * @param line_color Color for the path line (default: "blue")
     * @param line_width Width of the path line in pixels (default: 2.0)
     */
    void addTrackingPath(const std::vector<TrackingPoint>& points,
                        const std::string& line_color = "blue",
                        double line_width = 2.0);
    
    /**
     * @brief Add point markers with labels at specific tracking points
     * 
     * @param points Vector of TrackingPoint objects to mark
     * @param marker_color Color for the markers (default: "red")
     * @param marker_size Size of the markers (default: 8.0)
     * @param show_labels Whether to show timestamp labels (default: true)
     */
    void addPointMarkers(const std::vector<TrackingPoint>& points,
                        const std::string& marker_color = "red",
                        double marker_size = 8.0,
                        bool show_labels = true);
    
    /**
     * @brief Set the geographic extent of the map
     * 
     * @param min_lon Minimum longitude
     * @param min_lat Minimum latitude
     * @param max_lon Maximum longitude
     * @param max_lat Maximum latitude
     */
    void setExtent(double min_lon, double min_lat, double max_lon, double max_lat);
    
    /**
     * @brief Automatically zoom to fit all tracking points
     * 
     * @param padding Padding around the extent in percentage (default: 0.1 = 10%)
     */
    void zoomToTrackingPoints(double padding = 0.1);
    
    /**
     * @brief Render the map to a PNG file
     * 
     * @param output_path Path for the output PNG file
     * @return true if rendering succeeded
     * @return false if rendering failed
     */
    bool render(const std::string& output_path);
    
    /**
     * @brief Get the width of the map canvas
     * @return int Width in pixels
     */
    int getWidth() const;
    
    /**
     * @brief Get the height of the map canvas
     * @return int Height in pixels
     */
    int getHeight() const;
    
    /**
     * @brief Get the current projection string
     * @return std::string Projection identifier
     */
    std::string getProjection() const;
    
    /**
     * @brief Get the last error message
     * @return std::string Last error message, empty if no error
     */
    std::string getLastError() const;
    
    /**
     * @brief Set the font name used for text labels
     * @param font_name Font name (e.g., "DejaVu Sans Book")
     */
    void setFontName(const std::string& font_name);
    
    /**
     * @brief Get the current font name used for text labels
     * @return std::string Current font name
     */
    std::string getFontName() const;

private:
    std::unique_ptr<mapnik::Map> map_;
    std::string projection_;
    std::vector<TrackingPoint> all_tracking_points_;
    int layer_count_;
    mutable std::string last_error_;
    std::string font_name_;
    
    /**
     * @brief Convert coordinates from WGS84 (epsg:4326) to map projection
     * 
     * @param lon Longitude in WGS84
     * @param lat Latitude in WGS84
     * @param x Output X coordinate in map projection
     * @param y Output Y coordinate in map projection
     */
    void transformCoordinates(double lon, double lat, double& x, double& y) const;
    
    /**
     * @brief Create a line style for tracking paths
     * 
     * @param style_name Name for the style
     * @param color Line color
     * @param width Line width
     */
    void createLineStyle(const std::string& style_name, 
                        const std::string& color, 
                        double width);
    
    /**
     * @brief Create a point style for markers
     * 
     * @param style_name Name for the style
     * @param color Marker color
     * @param size Marker size
     * @param show_labels Whether to include text labels
     */
    void createPointStyle(const std::string& style_name,
                         const std::string& color,
                         double size,
                         bool show_labels);
};

} // namespace ioc_earth

#endif // MAP_PATH_RENDERER_HPP
