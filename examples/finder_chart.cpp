#include "FinderChartRenderer.h"
#include <iostream>

int main() {
    std::cout << "=== Finder Chart Example ===" << std::endl;
    std::cout << "Creazione carta di avvicinamento astronomica...\n" << std::endl;
    
    try {
        // Crea il renderer (800x800 pixels per carta quadrata)
        ioc_earth::FinderChartRenderer renderer(800, 800);
        
        // Imposta il campo visivo centrato sul target
        renderer.setFieldOfView(
            280.45,  // RA centro (gradi)
            -23.12,  // Dec centro (gradi)
            5.0      // Campo visivo (gradi)
        );
        
        // Imposta magnitudine limite
        renderer.setMagnitudeLimit(13.0);
        
        // Aggiungi stelle SAO (normalmente caricate da catalogo)
        std::vector<ioc_earth::SAOStar> sao_stars = {
            {187600, 280.20, -23.50, 8.2, "K0", "Cap"},
            {187610, 280.35, -23.30, 9.1, "G5", "Cap"},
            {187620, 280.50, -23.00, 8.8, "K2", "Cap"},
            {187630, 280.60, -23.40, 9.5, "A0", "Cap"},
            {187640, 280.25, -22.90, 10.1, "F8", "Cap"},
            {187650, 280.70, -23.20, 10.5, "G0", "Cap"},
            {187660, 280.15, -23.15, 11.0, "K5", "Cap"},
            {187670, 280.55, -22.80, 11.5, "M0", "Cap"},
            {187680, 280.40, -23.45, 12.0, "K0", "Cap"},
            {187690, 280.30, -22.95, 12.5, "G8", "Cap"}
        };
        renderer.addSAOStars(sao_stars);
        
        // Aggiungi linee delle costellazioni
        std::vector<ioc_earth::ConstellationLine> const_lines = {
            {280.0, -24.0, 280.5, -23.5, "Capricornus"},
            {280.5, -23.5, 281.0, -23.0, "Capricornus"},
            {280.5, -23.5, 280.3, -22.8, "Capricornus"},
            {280.0, -24.0, 279.8, -23.2, "Capricornus"}
        };
        renderer.addConstellationLines(const_lines);
        
        // Aggiungi confini delle costellazioni
        std::vector<ioc_earth::ConstellationBoundary> boundaries;
        ioc_earth::ConstellationBoundary cap_boundary;
        cap_boundary.constellation = "Capricornus";
        cap_boundary.points = {
            {279.5, -22.0},
            {281.5, -22.0},
            {281.5, -24.5},
            {279.5, -24.5},
            {279.5, -22.0}
        };
        boundaries.push_back(cap_boundary);
        renderer.addConstellationBoundaries(boundaries);
        
        // Imposta il target (stella da occultare)
        ioc_earth::TargetInfo target;
        target.name = "TYC 6009-01262-1";
        target.ra_deg = 280.45;
        target.dec_deg = -23.12;
        target.magnitude = 11.3;
        
        // Aggiungi traiettoria dell'asteroide
        target.trajectory = {
            {280.40, -23.15},
            {280.42, -23.13},
            {280.44, -23.11},
            {280.45, -23.12},
            {280.46, -23.10},
            {280.48, -23.08},
            {280.50, -23.06}
        };
        target.trajectory_times = {
            "22:10:00", "22:12:00", "22:14:00", 
            "22:15:30", "22:16:00", "22:18:00", "22:20:00"
        };
        
        renderer.setTarget(target);
        
        // Configura lo stile (sfondo bianco!)
        ioc_earth::FinderChartRenderer::ChartStyle style;
        style.background_color = "#FFFFFF";           // Sfondo bianco
        style.grid_color = "#DDDDDD";                 // Griglia grigio chiaro
        style.constellation_line_color = "#0066CC";   // Linee costellazioni blu
        style.constellation_boundary_color = "#00AA00"; // Confini verdi
        style.star_color = "#000000";                 // Stelle nere
        style.target_color = "#FF0000";               // Target rosso
        style.trajectory_color = "#FF6600";           // Traiettoria arancione
        style.show_star_labels = true;                // Mostra numeri SAO
        style.show_constellation_names = true;
        style.constellation_line_width = 1.5;
        style.constellation_boundary_width = 1.0;
        
        renderer.setChartStyle(style);
        
        // Renderizza la carta
        std::string output_file = "finder_chart.png";
        bool success = renderer.renderFinderChart(output_file);
        
        if (success) {
            std::cout << "\n✓ Carta di avvicinamento creata!" << std::endl;
            std::cout << "  Output: " << output_file << std::endl;
            std::cout << "\nCaratteristiche:" << std::endl;
            std::cout << "  ✓ Sfondo bianco" << std::endl;
            std::cout << "  ✓ Stelle del catalogo SAO con numeri" << std::endl;
            std::cout << "  ✓ Linee delle costellazioni (blu)" << std::endl;
            std::cout << "  ✓ Confini delle costellazioni (verdi)" << std::endl;
            std::cout << "  ✓ Target evidenziato (rosso)" << std::endl;
            std::cout << "  ✓ Traiettoria asteroide (arancione)" << std::endl;
        } else {
            std::cerr << "\n✗ Errore nella creazione della carta" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
