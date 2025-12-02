#ifndef MAPPATHRENDERER_H
#define MAPPATHRENDERER_H

#include <string>
#include <vector>
#include <memory>
#include <mapnik/map.hpp>

namespace ioc_earth {

/**
 * @brief Struttura per rappresentare un punto GPS con timestamp
 */
struct GPSPoint {
    double longitude;
    double latitude;
    std::string timestamp;
    
    GPSPoint(double lon, double lat, const std::string& ts = "")
        : longitude(lon), latitude(lat), timestamp(ts) {}
};

/**
 * @brief Classe per il rendering di mappe e tracciati GPS
 * 
 * Questa classe fornisce funzionalità per:
 * - Renderizzare mappe di base da shapefile
 * - Visualizzare tracciati GPS
 * - Aggiungere etichette ai punti
 */
class MapPathRenderer {
public:
    /**
     * @brief Costruttore
     * @param width Larghezza dell'immagine in pixel
     * @param height Altezza dell'immagine in pixel
     */
    MapPathRenderer(unsigned int width, unsigned int height);
    
    /**
     * @brief Distruttore
     */
    ~MapPathRenderer();
    
    /**
     * @brief Imposta l'estensione geografica della mappa
     * @param min_lon Longitudine minima
     * @param min_lat Latitudine minima
     * @param max_lon Longitudine massima
     * @param max_lat Latitudine massima
     */
    void setExtent(double min_lon, double min_lat, double max_lon, double max_lat);
    
    /**
     * @brief Aggiunge uno shapefile come layer di base
     * @param shapefile_path Percorso al file .shp
     * @param layer_name Nome del layer
     */
    void addShapefileLayer(const std::string& shapefile_path, const std::string& layer_name);
    
    /**
     * @brief Aggiunge un tracciato GPS alla mappa
     * @param points Vector di punti GPS
     * @param line_color Colore della linea (formato: "red", "#FF0000", ecc.)
     * @param line_width Spessore della linea
     */
    void addGPSPath(const std::vector<GPSPoint>& points, 
                    const std::string& line_color = "blue", 
                    double line_width = 2.0);
    
    /**
     * @brief Aggiunge etichette ai punti GPS
     * @param points Vector di punti GPS da etichettare
     * @param label_field Campo da usare per l'etichetta ("timestamp" o custom)
     * @param font_size Dimensione del font
     */
    void addPointLabels(const std::vector<GPSPoint>& points,
                       const std::string& label_field = "timestamp",
                       int font_size = 10);
    
    /**
     * @brief Imposta lo stile del background della mappa
     * @param color Colore del background
     */
    void setBackgroundColor(const std::string& color);
    
    /**
     * @brief Renderizza la mappa in un file PNG
     * @param output_path Percorso del file PNG di output
     * @return true se il rendering è avvenuto con successo
     */
    bool renderToFile(const std::string& output_path);
    
    /**
     * @brief Calcola automaticamente l'estensione basata sui punti GPS
     * @param points Vector di punti GPS
     * @param margin_percent Margine percentuale da aggiungere (default 10%)
     */
    void autoSetExtentFromPoints(const std::vector<GPSPoint>& points, 
                                  double margin_percent = 10.0);

private:
    std::unique_ptr<mapnik::Map> map_;
    unsigned int width_;
    unsigned int height_;
    
    // Metodi helper privati
    void initializeMap();
    std::string createGeoJSONFromPoints(const std::vector<GPSPoint>& points);
};

} // namespace ioc_earth

#endif // MAPPATHRENDERER_H
