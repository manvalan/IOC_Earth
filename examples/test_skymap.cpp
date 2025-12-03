/**
 * @file test_skymap.cpp
 * @brief Test della nuova classe SkyMapRenderer
 * 
 * Genera una bella mappa celeste con:
 * - Sfondo bianco per stampa
 * - Stelle SAO con numeri
 * - Linee e confini delle costellazioni
 * - Target evidenziato
 * - Rettangolo tratteggiato per indicare il campo del finder chart
 */

#include "SkyMapRenderer.h"
#include <iostream>
#include <vector>

using namespace ioc_earth;

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  Test SkyMapRenderer - Mappa Celeste Astronomica        ║" << std::endl;
    std::cout << "║  Asteromide 17030 - 26-29 Novembre 2025                 ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝\n" << std::endl;
    
    try {
        // Crea il renderer
        SkyMapRenderer skymap(1200, 1200);
        
        // Configura il campo visivo
        std::cout << "🔧 Configurazione della mappa celeste..." << std::endl;
        skymap.setFieldOfView(45.678, 12.345, 10.0);
        skymap.setMagnitudeLimit(13.0);
        
        // Aggiungi stelle SAO
        std::cout << "\n⭐ Caricamento stelle del catalogo SAO..." << std::endl;
        std::vector<StarData> stars = {
            {75234, 45.2, 12.8, 8.5, "G2"},
            {75245, 45.9, 12.1, 9.1, "K0"},
            {75256, 46.3, 12.7, 9.8, "A5"},
            {75267, 45.5, 11.9, 10.2, "F8"},
            {75278, 44.8, 12.5, 10.6, "G5"},
            {75289, 46.1, 11.8, 11.1, "K2"},
            {75300, 44.9, 13.0, 11.5, "M0"},
            {75311, 46.5, 12.3, 12.0, "F5"},
            {75322, 45.1, 11.5, 12.4, "K5"},
            {75333, 46.8, 12.9, 13.0, "G8"},
            {75344, 44.5, 12.2, 13.5, "A0"},
            {75355, 46.0, 13.2, 13.8, "K8"}
        };
        skymap.addStars(stars);
        
        // Aggiungi linee costellazioni
        std::cout << "\n📐 Aggiunta linee asterismi..." << std::endl;
        std::vector<ConstellationLineData> lines = {
            {45.2, 12.8, 45.9, 12.1, "Ari"},
            {45.9, 12.1, 46.3, 12.7, "Ari"},
            {46.3, 12.7, 45.5, 11.9, "Ari"},
            {45.5, 11.9, 44.8, 12.5, "Ari"},
            {44.8, 12.5, 45.2, 12.8, "Ari"}
        };
        skymap.addConstellationLines(lines);
        
        // Aggiungi confini costellazioni
        std::cout << "\n🔶 Aggiunta confini IAU delle costellazioni..." << std::endl;
        std::vector<ConstellationBoundaryData> boundaries = {
            {{
                {44.0, 11.0}, {47.5, 11.0}, {47.5, 13.5}, 
                {44.0, 13.5}, {44.0, 11.0}
            }, "Aries"}
        };
        skymap.addConstellationBoundaries(boundaries);
        
        // Configura il target
        std::cout << "\n🎯 Configurazione target Asteroid 17030..." << std::endl;
        TargetData target;
        target.name = "Asteroid 17030";
        target.ra_deg = 45.678;
        target.dec_deg = 12.345;
        target.magnitude = 13.2;
        target.trajectory = {
            {45.2, 12.1}, {45.35, 12.18}, {45.5, 12.25},
            {45.65, 12.32}, {45.8, 12.38}, {45.95, 12.44},
            {46.1, 12.5}
        };
        target.trajectory_timestamps = {
            "26 Nov 00:00", "26 Nov 12:00", "27 Nov 00:00",
            "27 Nov 12:00", "28 Nov 00:00", "28 Nov 12:00",
            "29 Nov 00:00"
        };
        skymap.setTarget(target);
        
        // Configura lo stile
        std::cout << "\n🎨 Configurazione stile della mappa..." << std::endl;
        SkyMapStyle style;
        style.background_color = "#FFFFFF";          // Bianco
        style.star_color = "#000000";                // Nero
        style.constellation_line_color = "#0066CC";  // Blu
        style.constellation_boundary_color = "#00AA00"; // Verde
        style.target_color = "#FF0000";              // Rosso
        style.trajectory_color = "#FF6600";          // Arancione
        style.fov_rect_color = "#990000";            // Rosso scuro
        style.show_star_labels = true;
        style.show_constellation_lines = true;
        style.show_constellation_boundaries = true;
        style.show_grid = true;
        skymap.setStyle(style);
        
        // Imposta il rettangolo FOV del finder chart (per mostrare un ingrandimento)
        std::cout << "\n📦 Impostazione rettangolo FOV per il finder chart..." << std::endl;
        skymap.setFinderChartBounds(45.678, 12.345, 5.0);
        
        // Renderizza la mappa celeste
        std::cout << "\n🖼️  Generazione della mappa celeste..." << std::endl;
        std::string output = "skymap_17030.png";
        bool success = skymap.renderSkyMap(output);
        
        if (success) {
            std::cout << "\n✅ === SUCCESSO ===" << std::endl;
            std::cout << "\n📊 Mappa celeste generata con successo!" << std::endl;
            std::cout << "\n📝 Caratteristiche della mappa:" << std::endl;
            std::cout << "   ✓ Sfondo bianco per stampa su carta" << std::endl;
            std::cout << "   ✓ 12 stelle del catalogo SAO con numeri identificativi" << std::endl;
            std::cout << "   ✓ Linee asterismi della costellazione di Ariete (blu)" << std::endl;
            std::cout << "   ✓ Confini ufficiali IAU della costellazione (verde)" << std::endl;
            std::cout << "   ✓ Asteroid 17030 evidenziato in rosso" << std::endl;
            std::cout << "   ✓ Traiettoria dell'asteroide dal 26 al 29 novembre 2025 (arancione)" << std::endl;
            std::cout << "   ✓ Rettangolo rosso scuro tratteggiato che indica il FOV del finder chart" << std::endl;
            std::cout << "   ✓ Griglia di coordinate RA/Dec per orientamento" << std::endl;
            std::cout << "\n💾 File generato: " << output << " (1200x1200 px)" << std::endl;
            std::cout << "\n🎓 Uso per osservazioni:" << std::endl;
            std::cout << "   1. Stampa la mappa su carta" << std::endl;
            std::cout << "   2. Usa il rettangolo per identificare il campo di osservazione" << std::endl;
            std::cout << "   3. Localizza le stelle SAO per orientarti al telescopio" << std::endl;
            std::cout << "   4. Segui la traiettoria per predire il movimento dell'asteroide" << std::endl;
        } else {
            std::cerr << "\n❌ Errore nella generazione della mappa!" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Errore: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n" << std::endl;
    return 0;
}
