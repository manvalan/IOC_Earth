#ifndef IOC_EARTH_FINDER_CHART_RENDERER_H
#define IOC_EARTH_FINDER_CHART_RENDERER_H

#include <string>
#include <vector>
#include <memory>

namespace ioc_earth {

/**
 * @brief Dati per una stella nel catalogo SAO
 */
struct SAOStar {
    int sao_number;              // Numero catalogo SAO
    double ra_deg;               // Ascensione retta (gradi)
    double dec_deg;              // Declinazione (gradi)
    double magnitude;            // Magnitudine visuale
    std::string spectral_type;   // Tipo spettrale
    std::string constellation;   // Costellazione
};

/**
 * @brief Linea di costellazione
 */
struct ConstellationLine {
    double ra1_deg, dec1_deg;    // Punto iniziale
    double ra2_deg, dec2_deg;    // Punto finale
    std::string constellation;   // Nome costellazione
};

/**
 * @brief Confine di costellazione
 */
struct ConstellationBoundary {
    std::vector<std::pair<double, double>> points; // RA, Dec in gradi
    std::string constellation;
};

/**
 * @brief Dati del target per la finder chart
 */
struct TargetInfo {
    std::string name;            // Nome del target (stella/asteroide)
    double ra_deg;               // Ascensione retta (gradi)
    double dec_deg;              // Declinazione (gradi)
    double magnitude;            // Magnitudine
    
    // Percorso del target (per occultazioni)
    std::vector<std::pair<double, double>> trajectory; // RA, Dec
    std::vector<std::string> trajectory_times;         // Timestamp
};

/**
 * @brief Renderer per carte di avvicinamento astronomiche
 * 
 * Genera finder charts con:
 * - Sfondo bianco
 * - Stelle del catalogo SAO
 * - Linee delle costellazioni
 * - Confini delle costellazioni
 * - Target evidenziato
 * - Griglia di coordinate (RA/Dec)
 */
class FinderChartRenderer {
public:
    /**
     * @brief Costruttore
     * @param width Larghezza in pixel
     * @param height Altezza in pixel
     */
    FinderChartRenderer(unsigned int width, unsigned int height);
    
    /**
     * @brief Distruttore
     */
    ~FinderChartRenderer();
    
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
     * @param mag_limit Magnitudine limite (es: 12.0)
     */
    void setMagnitudeLimit(double mag_limit);
    
    /**
     * @brief Aggiunge stelle dal catalogo SAO
     * @param stars Vector di stelle SAO
     */
    void addSAOStars(const std::vector<SAOStar>& stars);
    
    /**
     * @brief Aggiunge linee delle costellazioni
     * @param lines Vector di linee
     */
    void addConstellationLines(const std::vector<ConstellationLine>& lines);
    
    /**
     * @brief Aggiunge confini delle costellazioni
     * @param boundaries Vector di confini
     */
    void addConstellationBoundaries(const std::vector<ConstellationBoundary>& boundaries);
    
    /**
     * @brief Imposta il target da evidenziare
     * @param target Informazioni sul target
     */
    void setTarget(const TargetInfo& target);
    
    /**
     * @brief Stile di rendering della carta
     */
    struct ChartStyle {
        std::string background_color = "#FFFFFF";     // Bianco
        std::string grid_color = "#CCCCCC";           // Grigio chiaro per griglia
        std::string constellation_line_color = "#0000FF"; // Blu per linee
        std::string constellation_boundary_color = "#00AA00"; // Verde per confini
        std::string star_color = "#000000";           // Nero per stelle
        std::string target_color = "#FF0000";         // Rosso per target
        std::string trajectory_color = "#FF8800";     // Arancione per traiettoria
        
        double constellation_line_width = 1.0;
        double constellation_boundary_width = 0.5;
        double grid_line_width = 0.5;
        
        bool show_star_labels = true;                 // Mostra numeri SAO
        bool show_constellation_names = true;
        bool show_grid = true;
        bool show_magnitude_scale = true;             // Dimensione stella = f(mag)
        
        int label_font_size = 8;
    };
    
    void setChartStyle(const ChartStyle& style);
    ChartStyle getChartStyle() const { return style_; }
    
    /**
     * @brief Renderizza la finder chart
     * @param output_path Percorso file PNG output
     * @return true se successo
     */
    bool renderFinderChart(const std::string& output_path);
    
    /**
     * @brief Renderizza in buffer
     * @param png_data Buffer per i dati PNG
     * @return true se successo
     */
    bool renderToBuffer(std::vector<uint8_t>& png_data);
    
    /**
     * @brief Esporta in HTML
     * @param output_html_path Percorso file HTML
     * @param page_title Titolo pagina
     * @return true se successo
     */
    bool exportToHTML(const std::string& output_html_path,
                     const std::string& page_title = "Finder Chart");
    
    /**
     * @brief Ottiene l'ultima immagine in base64
     * @return Stringa base64 o vuota se nessuna immagine
     */
    std::string getLastRenderedImageBase64() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    unsigned int width_;
    unsigned int height_;
    
    double center_ra_;
    double center_dec_;
    double field_of_view_;
    double mag_limit_;
    
    std::vector<SAOStar> stars_;
    std::vector<ConstellationLine> constellation_lines_;
    std::vector<ConstellationBoundary> constellation_boundaries_;
    TargetInfo target_;
    
    ChartStyle style_;
    
    mutable std::vector<uint8_t> last_rendered_buffer_;
    
    // Conversione coordinate celesti -> pixel
    void celestialToPixel(double ra, double dec, int& x, int& y) const;
    
    // Rendering componenti
    void renderGrid();
    void renderConstellationBoundaries();
    void renderConstellationLines();
    void renderStars();
    void renderTarget();
    void renderLabels();
};

} // namespace ioc_earth

#endif // IOC_EARTH_FINDER_CHART_RENDERER_H
