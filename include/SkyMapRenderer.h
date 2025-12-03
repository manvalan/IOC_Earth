#ifndef IOC_EARTH_SKY_MAP_RENDERER_H
#define IOC_EARTH_SKY_MAP_RENDERER_H

#include <string>
#include <vector>
#include <memory>

namespace ioc_earth {

/**
 * @brief Stella nel catalogo SAO
 */
struct StarData {
    int sao_number;              // Numero SAO
    double ra_deg;               // Ascensione retta (gradi)
    double dec_deg;              // Declinazione (gradi)
    double magnitude;            // Magnitudine visuale (min=più brillante)
    std::string spectral_type;   // Tipo spettrale (A, B, F, G, K, M, etc.)
};

/**
 * @brief Linea di costellazione (asterismo)
 */
struct ConstellationLineData {
    double ra1_deg, dec1_deg;    // Punto iniziale
    double ra2_deg, dec2_deg;    // Punto finale
    std::string constellation;
};

/**
 * @brief Confine di costellazione (poligono)
 */
struct ConstellationBoundaryData {
    std::vector<std::pair<double, double>> points; // RA, Dec in gradi
    std::string constellation;
};

/**
 * @brief Oggetto target (stella, asteroide, etc.)
 */
struct TargetData {
    std::string name;
    double ra_deg;
    double dec_deg;
    double magnitude;
    std::vector<std::pair<double, double>> trajectory;      // Punti della traiettoria
    std::vector<std::string> trajectory_timestamps;         // Timestamp per ogni punto
};

/**
 * @brief Configurazione stile per le mappe celesti
 */
struct SkyMapStyle {
    // Colori
    std::string background_color = "#FFFFFF";              // Bianco
    std::string grid_color = "#CCCCCC";                    // Grigio chiaro
    std::string star_color = "#000000";                    // Nero
    std::string star_label_color = "#003366";              // Blu scuro
    std::string constellation_line_color = "#0066CC";      // Blu
    std::string constellation_boundary_color = "#00AA00";  // Verde
    std::string target_color = "#FF0000";                  // Rosso
    std::string trajectory_color = "#FF6600";              // Arancione
    std::string fov_rect_color = "#990000";                // Rosso scuro per il rettangolo tratteggiato
    
    // Dimensioni e stile
    double grid_line_width = 0.5;
    double constellation_line_width = 1.0;
    double constellation_boundary_width = 1.5;
    double star_base_size = 2.0;                           // Dimensione base stella
    double target_size = 6.0;
    double trajectory_line_width = 2.0;
    double fov_rect_line_width = 2.0;
    
    // Font
    int label_font_size = 8;
    std::string label_font = "DejaVu Sans";
    
    // Visualizzazione
    bool show_grid = true;
    bool show_star_labels = true;
    bool show_constellation_lines = true;
    bool show_constellation_boundaries = true;
    bool show_constellation_names = false;
    bool show_magnitude_scale = true;                       // Stelle più grandi per magnitudini basse
};

/**
 * @brief Renderer per mappe celesti astronomiche
 * 
 * Genera mappe del cielo di alta qualità con:
 * - Sfondo bianco per stampa
 * - Stelle SAO con dimensioni proporzionali alla magnitudine
 * - Linee asterismi delle costellazioni
 * - Confini ufficiali IAU delle costellazioni
 * - Target evidenziato con traiettoria
 * - Griglia RA/Dec
 * - Rettangolo tratteggiato per FOV di osservazione
 */
class SkyMapRenderer {
public:
    /**
     * @brief Costruttore
     * @param width Larghezza in pixel
     * @param height Altezza in pixel
     */
    SkyMapRenderer(unsigned int width, unsigned int height);
    
    /**
     * @brief Distruttore
     */
    ~SkyMapRenderer();
    
    /**
     * @brief Imposta l'area del cielo da visualizzare
     * @param center_ra_deg Ascensione retta del centro (gradi)
     * @param center_dec_deg Declinazione del centro (gradi)
     * @param field_of_view_deg Campo visivo (gradi)
     */
    void setFieldOfView(double center_ra_deg, double center_dec_deg, 
                        double field_of_view_deg);
    
    /**
     * @brief Imposta la magnitudine limite per le stelle
     * @param mag_limit Magnitudine limite (es: 12.0, più alto = più deboli)
     */
    void setMagnitudeLimit(double mag_limit);
    
    /**
     * @brief Aggiunge stelle dal catalogo SAO
     * @param stars Vector di stelle
     */
    void addStars(const std::vector<StarData>& stars);
    
    /**
     * @brief Aggiunge linee delle costellazioni
     * @param lines Vector di linee asterismo
     */
    void addConstellationLines(const std::vector<ConstellationLineData>& lines);
    
    /**
     * @brief Aggiunge confini delle costellazioni
     * @param boundaries Vector di confini poligonali
     */
    void addConstellationBoundaries(const std::vector<ConstellationBoundaryData>& boundaries);
    
    /**
     * @brief Imposta il target e la sua traiettoria
     * @param target Informazioni del target
     */
    void setTarget(const TargetData& target);
    
    /**
     * @brief Imposta uno stile secondario per la mappa di ricerca (finder chart)
     * Se questo viene impostato, verrà visualizzato un rettangolo tratteggiato
     * che indica il campo della mappa di osservazione
     * @param center_ra Ascensione retta del centro del finder chart
     * @param center_dec Declinazione del centro del finder chart
     * @param fov_deg Campo visivo del finder chart
     */
    void setFinderChartBounds(double center_ra, double center_dec, double fov_deg);
    
    /**
     * @brief Configura lo stile della mappa
     * @param style Configurazione stile
     */
    void setStyle(const SkyMapStyle& style);
    
    /**
     * @brief Renderizza la mappa celeste in PNG
     * @param output_path Path del file PNG di output
     * @return true se il rendering è riuscito, false altrimenti
     */
    bool renderSkyMap(const std::string& output_path);
    
    /**
     * @brief Ottiene l'ultima immagine renderizzata come buffer
     * @return Vector di byte della mappa PNG, oppure vuoto se non disponibile
     */
    std::vector<uint8_t> getLastRenderedBuffer() const;
    
private:
    // Implementazione interna
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    // Dati della mappa
    unsigned int width_;
    unsigned int height_;
    double center_ra_;
    double center_dec_;
    double field_of_view_;
    double mag_limit_;
    
    // Dati dei componenti
    std::vector<StarData> stars_;
    std::vector<ConstellationLineData> constellation_lines_;
    std::vector<ConstellationBoundaryData> constellation_boundaries_;
    TargetData target_;
    SkyMapStyle style_;
    
    // Bounding box del finder chart (se impostato)
    bool has_finder_chart_bounds_ = false;
    double finder_chart_ra_;
    double finder_chart_dec_;
    double finder_chart_fov_;
    
    // Buffer dell'ultima immagine renderizzata
    mutable std::vector<uint8_t> last_rendered_buffer_;
};

} // namespace ioc_earth

#endif // IOC_EARTH_SKY_MAP_RENDERER_H
