#include "MapPathRenderer.h"
#include <iostream>
#include <vector>

/**
 * Esempio di visualizzazione di un tracciato GPS con etichette
 * Simula un percorso da Roma a Firenze
 */
int main() {
    try {
        std::cout << "IOC_Earth - Esempio tracciato GPS" << std::endl;
        std::cout << "===================================" << std::endl;
        
        // Crea il renderer con dimensioni 1200x800
        ioc_earth::MapPathRenderer renderer(1200, 800);
        
        // Crea un tracciato GPS di esempio (Roma -> Firenze)
        std::vector<ioc_earth::GPSPoint> gps_points = {
            {12.4964, 41.9028, "08:00 Roma"},          // Roma
            {12.5736, 42.0883, "08:30 Monterotondo"},  // Monterotondo
            {12.3895, 42.4151, "09:15 Orte"},          // Orte
            {12.1084, 42.5673, "09:45 Orvieto"},       // Orvieto
            {11.8796, 42.8380, "10:30 Chiusi"},        // Chiusi
            {11.7196, 43.0956, "11:15 Arezzo"},        // Arezzo
            {11.5794, 43.3188, "12:00 Valdarno"},      // Valdarno
            {11.2558, 43.7696, "13:00 Firenze"}        // Firenze
        };
        
        std::cout << "\nPunti GPS nel tracciato: " << gps_points.size() << std::endl;
        
        // Calcola automaticamente l'estensione basata sui punti
        renderer.autoSetExtentFromPoints(gps_points, 15.0);
        
        // Imposta lo sfondo
        renderer.setBackgroundColor("#F0F8FF");
        
        // Opzionale: Aggiungi uno shapefile di base
        // renderer.addShapefileLayer("data/italy_roads.shp", "roads");
        
        // Aggiungi il tracciato GPS (linea blu, spessore 3)
        std::cout << "Aggiunta tracciato GPS..." << std::endl;
        renderer.addGPSPath(gps_points, "#0066CC", 3.0);
        
        // Aggiungi le etichette ai punti
        std::cout << "Aggiunta etichette..." << std::endl;
        renderer.addPointLabels(gps_points, "timestamp", 12);
        
        // Renderizza la mappa
        std::string output_file = "gps_track.png";
        std::cout << "Rendering della mappa..." << std::endl;
        
        if (renderer.renderToFile(output_file)) {
            std::cout << "\n✓ Tracciato GPS generato con successo: " << output_file << std::endl;
            std::cout << "\nDettagli del percorso:" << std::endl;
            std::cout << "  Partenza: Roma (08:00)" << std::endl;
            std::cout << "  Arrivo: Firenze (13:00)" << std::endl;
            std::cout << "  Punti intermedi: " << (gps_points.size() - 2) << std::endl;
        } else {
            std::cerr << "\n✗ Errore nella generazione del tracciato" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
