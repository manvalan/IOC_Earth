/**
 * @file test_occultation_17030.cpp
 * @brief Test del renderer per l'occultazione di 17030 del 28/11/2025
 * 
 * Questo test genera una mappa professionale in bianco e nero del path
 * dell'occultazione dell'asteroide 17030 che occultavà la stella SAO 76740
 * il 28 novembre 2025 alle 18:45 UTC.
 * 
 * La mappa mostra:
 * - Linea centrale dell'ombra (nero)
 * - Limiti di incertezza 1-sigma (grigio scuro)
 * - Griglia di coordinate geografiche
 * - Confini politici e città (OpenStreetMap)
 * - Stazioni di osservazione con risultati
 */

#include "OccultationRenderer.h"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace ioc_earth;

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test OccultationRenderer - Asteroide 17030              ║" << std::endl;
    std::cout << "║  Occultazione del 28 Novembre 2025 - 18:45 UTC           ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;
    
    try {
        // Crea il renderer (1800x1200 pixels per migliore risoluzione)
        std::cout << "🎬 Inizializzazione renderer (1800x1200 px)..." << std::endl;
        OccultationRenderer renderer(1800, 1200);
        
        // Carica i dati dal file JSON
        std::cout << "📂 Caricamento dati occultazione..." << std::endl;
        std::string json_file = "../../data/17030_20251128.json";
        
        // Prova anche il percorso assoluto se relativo non funziona
        std::ifstream test_file(json_file);
        if (!test_file.good()) {
            // Prova il percorso dal build directory
            json_file = "../data/17030_20251128.json";
            test_file.open(json_file);
            if (!test_file.good()) {
                std::cerr << "\n❌ File non trovato: 17030_20251128.json" << std::endl;
                std::cerr << "   Percorsi provati:" << std::endl;
                std::cerr << "   - ../../data/17030_20251128.json" << std::endl;
                std::cerr << "   - ../data/17030_20251128.json" << std::endl;
                return 1;
            }
        }
        test_file.close();
        
        if (!renderer.loadFromJSON(json_file)) {
            std::cerr << "\n❌ Errore nel caricamento del file JSON: " << json_file << std::endl;
            return 1;
        }
        
        // Configura lo stile BIANCO E NERO PROFESSIONALE
        std::cout << "\n🎨 Configurazione stile bianco e nero professionale..." << std::endl;
        OccultationRenderer::RenderStyle style;
        
        // Colori bianco e nero
        style.background_color = "#FFFFFF";          // Sfondo bianco
        style.central_line_color = "#000000";        // Nero per linea centrale
        style.central_line_width = 2.5;
        
        style.sigma_lines_color = "#333333";         // Grigio scuro per limiti sigma
        style.sigma_lines_width = 1.5;
        
        style.grid_color = "#CCCCCC";                // Grigio chiaro per griglia
        style.grid_step_degrees = 5.0;               // Griglia ogni 5 gradi
        
        style.time_markers_color = "#000000";        // Nero per marker temporali
        style.time_marker_size = 6.0;
        
        style.station_positive_color = "#000000";    // Nero per osservazioni positive
        style.station_negative_color = "#666666";    // Grigio per osservazioni negative
        style.station_marker_size = 8.0;
        
        style.show_time_labels = true;
        style.show_station_labels = true;
        style.show_grid = true;
        style.label_font_size = 9;
        
        renderer.setRenderStyle(style);
        
        // Renderizza la mappa dell'occultazione
        std::cout << "\n🗺️  Rendering mappa occultazione..." << std::endl;
        std::string output_file = "occultation_17030_20251128.png";
        bool success = renderer.renderOccultationMap(output_file, true);
        
        if (success) {
            std::cout << "\n✅ === SUCCESSO ===" << std::endl;
            std::cout << "\n📊 Mappa dell'occultazione generata con successo!" << std::endl;
            std::cout << "\n📈 Dettagli dell'evento:" << std::endl;
            std::cout << "   Asteroide: 17030" << std::endl;
            std::cout << "   Stella occultata: SAO 76740" << std::endl;
            std::cout << "   Data: 28 Novembre 2025" << std::endl;
            std::cout << "   Ora: 18:45:00 UTC" << std::endl;
            std::cout << "   Durata: 8.7 secondi" << std::endl;
            std::cout << "   Calo magnitudine: 3.5 mag" << std::endl;
            std::cout << "   Regione: Europa centrale (Italia, Francia, Germania)" << std::endl;
            
            std::cout << "\n📝 Caratteristiche della mappa:" << std::endl;
            std::cout << "   ✓ Sfondo bianco per stampa professionale" << std::endl;
            std::cout << "   ✓ Linea centrale dell'ombra in nero" << std::endl;
            std::cout << "   ✓ Limiti di incertezza 1-sigma (grigio scuro)" << std::endl;
            std::cout << "   ✓ Griglia di coordinate (lat/lon) ogni 5°" << std::endl;
            std::cout << "   ✓ Confini politici e città (OpenStreetMap)" << std::endl;
            std::cout << "   ✓ Marker temporali lungo il percorso" << std::endl;
            std::cout << "   ✓ 6 stazioni di osservazione con risultati:" << std::endl;
            std::cout << "     • Roma (IT) - Osservazione positiva" << std::endl;
            std::cout << "     • Milano (IT) - Osservazione positiva" << std::endl;
            std::cout << "     • Firenze (IT) - Osservazione negativa" << std::endl;
            std::cout << "     • Torino (IT) - Osservazione positiva" << std::endl;
            std::cout << "     • Parigi (FR) - Osservazione positiva" << std::endl;
            std::cout << "     • Berlino (DE) - Osservazione negativa" << std::endl;
            
            std::cout << "\n💾 File generato: " << output_file << " (1800x1200 px)" << std::endl;
            
            std::cout << "\n🎓 Uso della mappa:" << std::endl;
            std::cout << "   1. Stampa la mappa su carta (formato A3 consigliato)" << std::endl;
            std::cout << "   2. La linea nera indica dove l'ombra passerà" << std::endl;
            std::cout << "   3. Le due linee grigie mostrano l'incertezza (±1σ)" << std::endl;
            std::cout << "   4. Localizza le città e stazioni di osservazione" << std::endl;
            std::cout << "   5. Pianifica la tua stazione di osservazione sulla mappa" << std::endl;
            
            std::cout << "\n✨ Esporta in HTML per visualizzazione web:" << std::endl;
            
            // Esporta anche in HTML
            std::string html_file = "occultation_17030_20251128.html";
            if (renderer.exportToHTML(html_file, true, 
                "Occultazione 17030 - 28 Novembre 2025")) {
                std::cout << "   ✓ HTML generato: " << html_file << std::endl;
            }
            
        } else {
            std::cerr << "\n❌ Errore nella creazione della mappa" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Errore: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n" << std::endl;
    return 0;
}
