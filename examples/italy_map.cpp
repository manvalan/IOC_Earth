#include "MapPathRenderer.h"
#include <iostream>
#include <vector>

/**
 * Esempio di mappa dell'Italia con tracciato GPS
 * Mostra le principali città italiane e un percorso turistico
 */
int main() {
    try {
        std::cout << "IOC_Earth - Mappa dell'Italia" << std::endl;
        std::cout << "=============================" << std::endl;
        
        // Crea il renderer con dimensioni 1600x1200 per una mappa dettagliata
        ioc_earth::MapPathRenderer renderer(1600, 1200);
        
        // Imposta l'estensione geografica per l'Italia
        // Longitudine: 6.5° - 18.5° E
        // Latitudine: 36.5° - 47.5° N
        std::cout << "\nImpostazione estensione geografica Italia..." << std::endl;
        renderer.setExtent(6.5, 36.5, 18.5, 47.5);
        
        // Imposta un colore di sfondo azzurro chiaro (come il mare)
        renderer.setBackgroundColor("#B3D9FF");
        
        // Aggiungi shapefile geografici
        std::cout << "Caricamento dati geografici..." << std::endl;
        renderer.addShapefileLayer("../data/ne_50m_admin_0_countries.shp", "countries");
        renderer.addShapefileLayer("../data/ne_50m_coastline.shp", "coastline");
        renderer.addShapefileLayer("../data/ne_50m_lakes.shp", "lakes");
        renderer.addShapefileLayer("../data/ne_50m_rivers_lake_centerlines.shp", "rivers");
        
        // Principali città italiane come punti GPS
        std::vector<ioc_earth::GPSPoint> italian_cities = {
            {9.1900, 45.4642, "Milano"},
            {7.6869, 45.0703, "Torino"},
            {8.9463, 44.4056, "Genova"},
            {11.8719, 45.4064, "Venezia"},
            {11.3426, 44.4949, "Bologna"},
            {12.4964, 41.9028, "Roma"},
            {14.2681, 40.8518, "Napoli"},
            {15.5500, 38.1157, "Messina"},
            {13.3615, 38.1157, "Palermo"},
            {9.0500, 39.2238, "Cagliari"},
            {11.2558, 43.7696, "Firenze"},
            {12.9388, 43.6158, "Ancona"},
            {16.8719, 41.1171, "Bari"}
        };
        
        std::cout << "Aggiunta " << italian_cities.size() << " città italiane..." << std::endl;
        renderer.addPointLabels(italian_cities, "timestamp", 14);
        
        // Crea un percorso turistico: Roma -> Firenze -> Venezia -> Milano
        std::vector<ioc_earth::GPSPoint> tour_path = {
            {12.4964, 41.9028, "Roma (Partenza)"},
            {12.1084, 42.5673, "Orvieto"},
            {11.8796, 42.8380, "Chiusi"},
            {11.2558, 43.7696, "Firenze"},
            {11.0794, 43.7731, "Prato"},
            {11.0188, 44.1056, "Pistoia"},
            {10.8750, 44.4056, "Lucca"},
            {11.3426, 44.4949, "Bologna"},
            {11.6158, 44.8378, "Ferrara"},
            {12.2388, 45.1333, "Rovigo"},
            {11.8719, 45.4064, "Venezia"},
            {11.9927, 45.6650, "Treviso"},
            {11.3561, 46.0664, "Trento"},
            {10.9916, 46.4336, "Bolzano"},
            {11.1211, 46.0664, "Passo Brennero"},
            {11.3426, 45.4383, "Verona"},
            {10.5270, 45.5678, "Brescia"},
            {10.2175, 45.5497, "Bergamo"},
            {9.1900, 45.4642, "Milano (Arrivo)"}
        };
        
        std::cout << "Aggiunta percorso turistico (Roma -> Milano)..." << std::endl;
        renderer.addGPSPath(tour_path, "#FF6600", 3.0);
        
        // Renderizza la mappa
        std::string output_file = "italy_map.png";
        std::cout << "\nRendering della mappa dell'Italia..." << std::endl;
        std::cout << "Questo potrebbe richiedere alcuni secondi..." << std::endl;
        
        if (renderer.renderToFile(output_file)) {
            std::cout << "\n✓ Mappa dell'Italia generata con successo: " << output_file << std::endl;
            std::cout << "\nDettagli:" << std::endl;
            std::cout << "  - Dimensioni: 1600x1200 pixel" << std::endl;
            std::cout << "  - Città visualizzate: " << italian_cities.size() << std::endl;
            std::cout << "  - Punti del percorso: " << tour_path.size() << std::endl;
            std::cout << "  - Percorso: Roma → Firenze → Venezia → Milano" << std::endl;
            std::cout << "\nNota: Per una visualizzazione migliore, aggiungi uno shapefile" << std::endl;
            std::cout << "      dell'Italia nella directory 'data/' e decommentare la linea" << std::endl;
            std::cout << "      addShapefileLayer() nel codice." << std::endl;
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
