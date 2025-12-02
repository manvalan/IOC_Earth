#include "MapPathRenderer.h"
#include <iostream>

/**
 * Esempio semplice di utilizzo della libreria IOC_Earth
 * Crea una mappa di base con uno sfondo colorato
 */
int main() {
    try {
        std::cout << "IOC_Earth - Esempio mappa semplice" << std::endl;
        std::cout << "===================================" << std::endl;
        
        // Crea il renderer con dimensioni 800x600
        ioc_earth::MapPathRenderer renderer(800, 600);
        
        // Imposta l'estensione geografica (esempio: Italia centrale)
        // Longitudine: 11.0 - 13.0, Latitudine: 41.5 - 43.5
        renderer.setExtent(11.0, 41.5, 13.0, 43.5);
        
        // Imposta un colore di sfondo
        renderer.setBackgroundColor("#E8F4F8");
        
        // Se hai uno shapefile, puoi aggiungerlo qui:
        // renderer.addShapefileLayer("data/italy.shp", "italy_base");
        
        // Aggiungi alcuni punti di esempio per visualizzare la mappa
        std::vector<ioc_earth::GPSPoint> demo_points = {
            {11.5, 42.0, "Nord-Ovest"},
            {12.5, 42.0, "Nord-Est"},
            {11.5, 43.0, "Sud-Ovest"},
            {12.5, 43.0, "Sud-Est"},
            {12.0, 42.5, "Centro"}
        };
        
        std::cout << "Aggiunta punti di riferimento..." << std::endl;
        renderer.addPointLabels(demo_points, "timestamp", 14);
        
        // Renderizza la mappa
        std::string output_file = "simple_map.png";
        if (renderer.renderToFile(output_file)) {
            std::cout << "\n✓ Mappa generata con successo: " << output_file << std::endl;
        } else {
            std::cerr << "\n✗ Errore nella generazione della mappa" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
