#include "OccultationRenderer.h"
#include <iostream>

int main() {
    std::cout << "=== Occultation Map Example ===" << std::endl;
    std::cout << "Creazione visualizzazione occultazione asteroidale...\n" << std::endl;
    
    try {
        // Crea il renderer (1600x1200 pixels)
        ioc_earth::OccultationRenderer renderer(1600, 1200);
        
        // Carica i dati dal file JSON
        std::string json_file = "../../data/chariklo_occultation.json";
        if (!renderer.loadFromJSON(json_file)) {
            std::cerr << "Errore nel caricamento del file JSON" << std::endl;
            return 1;
        }
        
        // Configura lo stile di rendering
        ioc_earth::OccultationRenderer::RenderStyle style;
        style.central_line_color = "#FF0000";        // Rosso per la linea centrale
        style.central_line_width = 3.0;
        style.sigma_lines_color = "#FF8800";         // Arancione per i limiti sigma
        style.sigma_lines_width = 2.0;
        style.time_markers_color = "#0000FF";        // Blu per i marker temporali
        style.time_marker_size = 8.0;
        style.station_marker_size = 10.0;
        style.show_time_labels = true;
        style.show_station_labels = true;
        style.label_font_size = 12;
        style.background_color = "#E8F4F8";          // Azzurro chiaro
        
        renderer.setRenderStyle(style);
        
        // Renderizza la mappa (con shapefile)
        std::string output_file = "occultation_map.png";
        std::string json_input = "../data/chariklo_occultation.json";
        bool success = renderer.renderOccultationMap(output_file, true);
        
        if (success) {
            std::cout << "\n✓ Mappa creata con successo!" << std::endl;
            std::cout << "  Output: " << output_file << std::endl;
            std::cout << "\nLegenda:" << std::endl;
            std::cout << "  - Linea ROSSA: percorso centrale dell'ombra" << std::endl;
            std::cout << "  - Linee ARANCIONI: limiti 1-sigma (incertezza)" << std::endl;
            std::cout << "  - Marker BLU: punti temporali lungo il percorso" << std::endl;
            std::cout << "  - Marker con etichette: stazioni di osservazione" << std::endl;
        } else {
            std::cerr << "\n✗ Errore nella creazione della mappa" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
