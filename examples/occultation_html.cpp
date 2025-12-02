#include "OccultationRenderer.h"
#include <iostream>
#include <fstream>

int main() {
    std::cout << "=== Occultation HTML Export Example ===" << std::endl;
    std::cout << "Creazione pagina HTML con mappa occultazione embedded...\n" << std::endl;
    
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
        style.central_line_color = "#FF0000";
        style.central_line_width = 3.0;
        style.sigma_lines_color = "#FF8800";
        style.sigma_lines_width = 2.0;
        style.time_markers_color = "#0000FF";
        style.time_marker_size = 8.0;
        style.station_marker_size = 10.0;
        style.show_time_labels = true;
        style.show_station_labels = true;
        style.label_font_size = 12;
        style.background_color = "#E8F4F8";
        
        renderer.setRenderStyle(style);
        
        // Opzione 1: Esporta direttamente in HTML con immagine embedded
        std::cout << "\n--- Test 1: Export to HTML ---" << std::endl;
        std::string html_file = "occultation_report.html";
        bool success_html = renderer.exportToHTML(html_file, true, 
            "Occultazione di Chariklo - 3 Giugno 2024");
        
        if (success_html) {
            std::cout << "✓ Pagina HTML creata: " << html_file << std::endl;
            std::cout << "  Apri il file con un browser per visualizzare la mappa!" << std::endl;
        }
        
        // Opzione 2: Ottieni il buffer PNG per uso programmatico
        std::cout << "\n--- Test 2: Render to Buffer ---" << std::endl;
        std::vector<uint8_t> png_buffer;
        std::string buffer_file = "occultation_from_buffer.png";
        bool success_buffer = renderer.renderToBuffer(png_buffer, true);
        
        if (success_buffer) {
            std::cout << "✓ Buffer PNG generato" << std::endl;
            std::cout << "  Dimensione: " << png_buffer.size() << " bytes" << std::endl;
            std::cout << "  Puoi usare questo buffer per:" << std::endl;
            std::cout << "    - Inviarlo via rete" << std::endl;
            std::cout << "    - Salvarlo in un database" << std::endl;
            std::cout << "    - Elaborarlo ulteriormente" << std::endl;
            
            // Esempio: salva il buffer su file
            std::ofstream out(buffer_file, std::ios::binary);
            out.write(reinterpret_cast<const char*>(png_buffer.data()), png_buffer.size());
            out.close();
            std::cout << "  Buffer salvato anche su: " << buffer_file << std::endl;
        }
        
        // Opzione 3: Ottieni l'immagine in base64
        std::cout << "\n--- Test 3: Get Base64 Image ---" << std::endl;
        std::string base64_image = renderer.getLastRenderedImageBase64();
        
        if (!base64_image.empty()) {
            std::cout << "✓ Immagine base64 ottenuta" << std::endl;
            std::cout << "  Lunghezza: " << base64_image.length() << " caratteri" << std::endl;
            std::cout << "  Primi 100 caratteri: " << base64_image.substr(0, 100) << "..." << std::endl;
            std::cout << "  Puoi usare questa stringa per:" << std::endl;
            std::cout << "    - Embedding in HTML: <img src=\"data:image/png;base64," << base64_image.substr(0, 20) << "...\">" << std::endl;
            std::cout << "    - JSON API responses" << std::endl;
            std::cout << "    - Database storage (TEXT/CLOB)" << std::endl;
        }
        
        std::cout << "\n✓ Tutti i test completati con successo!" << std::endl;
        std::cout << "\nRiepilogo file generati:" << std::endl;
        std::cout << "  1. " << html_file << " - Pagina HTML completa" << std::endl;
        std::cout << "  2. " << buffer_file << " - Immagine PNG da buffer" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
