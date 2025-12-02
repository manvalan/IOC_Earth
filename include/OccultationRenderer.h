#ifndef OCCULTATIONRENDERER_H
#define OCCULTATIONRENDERER_H

#include "MapPathRenderer.h"
#include <string>
#include <vector>
#include <memory>

namespace ioc_earth {

/**
 * @brief Struttura per rappresentare un punto sulla linea di un'occultazione
 */
struct OccultationPathPoint {
    double longitude;      // Longitudine in gradi
    double latitude;       // Latitudine in gradi
    std::string timestamp; // Timestamp UTC (formato ISO 8601)
    
    OccultationPathPoint(double lon, double lat, const std::string& ts = "")
        : longitude(lon), latitude(lat), timestamp(ts) {}
};

/**
 * @brief Struttura per rappresentare i dati di un'occultazione asteroidale
 */
struct OccultationData {
    // Informazioni generali
    std::string event_id;           // ID univoco dell'evento
    std::string asteroid_name;      // Nome dell'asteroide
    std::string star_name;          // Nome della stella occultata
    std::string date_time_utc;      // Data e ora UTC dell'evento
    double magnitude_drop;          // Calo di magnitudine atteso
    double duration_seconds;        // Durata prevista in secondi
    
    // Linea centrale dell'occultazione
    std::vector<OccultationPathPoint> central_line;
    
    // Limiti dell'ombra (sigma 1)
    std::vector<OccultationPathPoint> northern_limit;   // Limite nord (1-sigma)
    std::vector<OccultationPathPoint> southern_limit;   // Limite sud (1-sigma)
    
    // Marker temporali sulla linea centrale
    struct TimeMarker {
        double longitude;
        double latitude;
        std::string time_utc;      // Tempo UTC
        int seconds_from_start;    // Secondi dall'inizio
    };
    std::vector<TimeMarker> time_markers;
    
    // Stazioni di osservazione (opzionale)
    struct ObservationStation {
        std::string name;
        double longitude;
        double latitude;
        std::string status;  // "positive", "negative", "clouded", etc.
    };
    std::vector<ObservationStation> stations;
};

/**
 * @brief Classe specializzata per renderizzare mappe di occultazioni asteroidali
 * 
 * Estende MapPathRenderer con funzionalità specifiche per:
 * - Visualizzare la linea centrale dell'occultazione
 * - Disegnare i limiti sigma-1 dell'ombra
 * - Aggiungere marker temporali lungo il percorso
 * - Mostrare le stazioni di osservazione
 */
class OccultationRenderer {
public:
    /**
     * @brief Costruttore
     * @param width Larghezza dell'immagine in pixel
     * @param height Altezza dell'immagine in pixel
     */
    OccultationRenderer(unsigned int width, unsigned int height);
    
    /**
     * @brief Distruttore
     */
    ~OccultationRenderer();
    
    /**
     * @brief Carica i dati di occultazione da un file JSON
     * @param json_path Percorso al file JSON
     * @return true se il caricamento è avvenuto con successo
     */
    bool loadFromJSON(const std::string& json_path);
    
    /**
     * @brief Imposta i dati di occultazione manualmente
     * @param data Struttura con i dati dell'occultazione
     */
    void setOccultationData(const OccultationData& data);
    
    /**
     * @brief Renderizza la mappa dell'occultazione
     * @param output_path Percorso del file PNG di output
     * @param include_shapefile Se true, include i confini geografici
     * @return true se il rendering è avvenuto con successo
     */
    bool renderOccultationMap(const std::string& output_path, 
                              bool include_shapefile = true);
    
    /**
     * @brief Configura i colori e gli stili della visualizzazione
     */
    struct RenderStyle {
        std::string central_line_color = "#FF0000";     // Rosso per linea centrale
        double central_line_width = 3.0;
        
        std::string sigma_lines_color = "#FFA500";      // Arancione per limiti sigma
        double sigma_lines_width = 2.0;
        
        std::string time_markers_color = "#0000FF";     // Blu per marker temporali
        double time_marker_size = 10.0;
        
        std::string station_positive_color = "#00FF00"; // Verde per osservazioni positive
        std::string station_negative_color = "#FF0000"; // Rosso per osservazioni negative
        std::string station_clouded_color = "#CCCCCC";  // Grigio per cloud-out
        double station_marker_size = 8.0;
        
        std::string background_color = "#E0E0E0";       // Grigio chiaro per sfondo
        
        bool show_time_labels = true;
        bool show_station_labels = true;
        int label_font_size = 10;
    };
    
    void setRenderStyle(const RenderStyle& style);
    RenderStyle getRenderStyle() const { return style_; }
    
    /**
     * @brief Calcola automaticamente l'estensione della mappa
     * @param margin_percent Margine percentuale (default 15%)
     */
    void autoCalculateExtent(double margin_percent = 15.0);

private:
    std::unique_ptr<MapPathRenderer> renderer_;
    OccultationData data_;
    RenderStyle style_;
    
    unsigned int width_;
    unsigned int height_;
    
    // Metodi helper privati
    void renderCentralLine();
    void renderSigmaLimits();
    void renderTimeMarkers();
    void renderObservationStations();
    void addMapLegend();
};

} // namespace ioc_earth

#endif // OCCULTATIONRENDERER_H
