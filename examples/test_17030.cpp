/**
 * Test per Asteroid 17030 Finder Chart
 * Periodo: 26-29 Novembre 2025
 * 
 * Genera una carta di avvicinamento per l'asteroide 17030
 * mostrando la sua traiettoria dal 26 al 29 novembre 2025
 */

#include "FinderChartRenderer.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Funzione per leggere il file JSON
std::string readJSONFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Parser JSON semplice per il test
struct TrajectoryPoint {
    std::string timestamp;
    double ra_deg;
    double dec_deg;
};

struct JSONData {
    ioc_earth::TargetInfo target;
    std::vector<ioc_earth::SAOStar> stars;
    std::vector<ioc_earth::ConstellationLine> lines;
    std::vector<ioc_earth::ConstellationBoundary> boundaries;
    std::vector<TrajectoryPoint> trajectory_points;
    double center_ra = 0.0;
    double center_dec = 0.0;
    double fov = 8.0;
    double mag_limit = 14.0;
};

JSONData parseJSON(const std::string& json) {
    JSONData data;
    
    // Parse target
    size_t pos = json.find("\"name\":");
    if (pos != std::string::npos) {
        size_t start = json.find("\"", pos + 7) + 1;
        size_t end = json.find("\"", start);
        data.target.name = json.substr(start, end - start);
    }
    
    // Parse field of view
    pos = json.find("\"center_ra_deg\":");
    if (pos != std::string::npos) {
        data.center_ra = std::stod(json.substr(pos + 16, 20));
    }
    
    pos = json.find("\"center_dec_deg\":");
    if (pos != std::string::npos) {
        data.center_dec = std::stod(json.substr(pos + 17, 20));
    }
    
    pos = json.find("\"fov_deg\":");
    if (pos != std::string::npos) {
        data.fov = std::stod(json.substr(pos + 10, 20));
    }
    
    pos = json.find("\"magnitude_limit\":");
    if (pos != std::string::npos) {
        data.mag_limit = std::stod(json.substr(pos + 18, 20));
    }
    
    // Parse SAO stars (simplified)
    size_t stars_start = json.find("\"sao_stars\":");
    size_t stars_end = json.find("]", stars_start);
    std::string stars_section = json.substr(stars_start, stars_end - stars_start);
    
    size_t star_pos = 0;
    while ((star_pos = stars_section.find("{", star_pos)) != std::string::npos) {
        ioc_earth::SAOStar star;
        
        size_t sao_pos = stars_section.find("\"sao_number\":", star_pos);
        if (sao_pos != std::string::npos && sao_pos < stars_section.find("}", star_pos)) {
            star.sao_number = std::stoi(stars_section.substr(sao_pos + 13, 10));
        }
        
        size_t ra_pos = stars_section.find("\"ra_deg\":", star_pos);
        if (ra_pos != std::string::npos && ra_pos < stars_section.find("}", star_pos)) {
            star.ra_deg = std::stod(stars_section.substr(ra_pos + 9, 20));
        }
        
        size_t dec_pos = stars_section.find("\"dec_deg\":", star_pos);
        if (dec_pos != std::string::npos && dec_pos < stars_section.find("}", star_pos)) {
            star.dec_deg = std::stod(stars_section.substr(dec_pos + 10, 20));
        }
        
        size_t mag_pos = stars_section.find("\"magnitude\":", star_pos);
        if (mag_pos != std::string::npos && mag_pos < stars_section.find("}", star_pos)) {
            star.magnitude = std::stod(stars_section.substr(mag_pos + 12, 20));
        }
        
        data.stars.push_back(star);
        star_pos = stars_section.find("}", star_pos) + 1;
    }
    
    // Parse constellation lines
    size_t lines_start = json.find("\"constellation_lines\":");
    size_t lines_end = json.find("]", lines_start);
    std::string lines_section = json.substr(lines_start, lines_end - lines_start);
    
    size_t line_pos = 0;
    while ((line_pos = lines_section.find("{", line_pos)) != std::string::npos) {
        ioc_earth::ConstellationLine line;
        
        size_t ra1_pos = lines_section.find("\"ra1_deg\":", line_pos);
        if (ra1_pos != std::string::npos && ra1_pos < lines_section.find("}", line_pos)) {
            line.ra1_deg = std::stod(lines_section.substr(ra1_pos + 10, 20));
        }
        
        size_t dec1_pos = lines_section.find("\"dec1_deg\":", line_pos);
        if (dec1_pos != std::string::npos && dec1_pos < lines_section.find("}", line_pos)) {
            line.dec1_deg = std::stod(lines_section.substr(dec1_pos + 11, 20));
        }
        
        size_t ra2_pos = lines_section.find("\"ra2_deg\":", line_pos);
        if (ra2_pos != std::string::npos && ra2_pos < lines_section.find("}", line_pos)) {
            line.ra2_deg = std::stod(lines_section.substr(ra2_pos + 10, 20));
        }
        
        size_t dec2_pos = lines_section.find("\"dec2_deg\":", line_pos);
        if (dec2_pos != std::string::npos && dec2_pos < lines_section.find("}", line_pos)) {
            line.dec2_deg = std::stod(lines_section.substr(dec2_pos + 11, 20));
        }
        
        data.lines.push_back(line);
        line_pos = lines_section.find("}", line_pos) + 1;
    }
    
    // Parse trajectory
    size_t traj_start = json.find("\"asteroid_trajectory\":");
    size_t traj_end = json.find("]", traj_start);
    std::string traj_section = json.substr(traj_start, traj_end - traj_start);
    
    size_t point_pos = 0;
    while ((point_pos = traj_section.find("{", point_pos)) != std::string::npos) {
        TrajectoryPoint point;
        
        size_t time_pos = traj_section.find("\"timestamp\":", point_pos);
        if (time_pos != std::string::npos && time_pos < traj_section.find("}", point_pos)) {
            size_t start = traj_section.find("\"", time_pos + 12) + 1;
            size_t end = traj_section.find("\"", start);
            point.timestamp = traj_section.substr(start, end - start);
        }
        
        size_t ra_pos = traj_section.find("\"ra_deg\":", point_pos);
        if (ra_pos != std::string::npos && ra_pos < traj_section.find("}", point_pos)) {
            point.ra_deg = std::stod(traj_section.substr(ra_pos + 9, 20));
        }
        
        size_t dec_pos = traj_section.find("\"dec_deg\":", point_pos);
        if (dec_pos != std::string::npos && dec_pos < traj_section.find("}", point_pos)) {
            point.dec_deg = std::stod(traj_section.substr(dec_pos + 10, 20));
        }
        
        data.trajectory_points.push_back(point);
        
        // Aggiungi anche alle strutture del target
        data.target.trajectory.push_back({point.ra_deg, point.dec_deg});
        data.target.trajectory_times.push_back(point.timestamp);
        
        point_pos = traj_section.find("}", point_pos) + 1;
    }
    
    // Parse constellation boundary
    ioc_earth::ConstellationBoundary boundary;
    boundary.constellation = "Aries";
    
    size_t bound_start = json.find("\"constellation_boundaries\":");
    size_t bound_end = json.find("]", bound_start);
    std::string bound_section = json.substr(bound_start, bound_end - bound_start);
    
    size_t coord_pos = 0;
    while ((coord_pos = bound_section.find("{\"ra_deg\":", coord_pos)) != std::string::npos) {
        double ra = std::stod(bound_section.substr(coord_pos + 10, 20));
        
        size_t dec_pos = bound_section.find("\"dec_deg\":", coord_pos);
        double dec = 0.0;
        if (dec_pos != std::string::npos) {
            dec = std::stod(bound_section.substr(dec_pos + 10, 20));
        }
        
        boundary.points.push_back({ra, dec});
        coord_pos = bound_section.find("}", coord_pos) + 1;
    }
    
    data.boundaries.push_back(boundary);
    
    return data;
}

int main() {
    try {
        std::cout << "\n=== Test Finder Chart Asteroid 17030 ===" << std::endl;
        std::cout << "Periodo: 26-29 Novembre 2025\n" << std::endl;
        
        // Leggi il file JSON
        std::string json = readJSONFile("../../data/17030_finder_chart.json");
        JSONData data = parseJSON(json);
        
        std::cout << "📊 Dati caricati:" << std::endl;
        std::cout << "   Target: " << data.target.name << std::endl;
        std::cout << "   Stelle SAO: " << data.stars.size() << std::endl;
        std::cout << "   Linee costellazioni: " << data.lines.size() << std::endl;
        std::cout << "   Confini costellazioni: " << data.boundaries.size() << std::endl;
        std::cout << "   Punti traiettoria: " << data.trajectory_points.size() << std::endl;
        std::cout << std::endl;
        
        // Crea il renderer
        ioc_earth::FinderChartRenderer renderer(1000, 1000);
        
        // Configura il campo visivo
        std::cout << "🔭 Configurazione campo visivo:" << std::endl;
        renderer.setFieldOfView(data.center_ra, data.center_dec, data.fov);
        std::cout << "   Centro: RA " << data.center_ra << "° Dec " << data.center_dec << "°" << std::endl;
        std::cout << "   Campo: " << data.fov << "°" << std::endl;
        
        // Imposta limite di magnitudine
        renderer.setMagnitudeLimit(data.mag_limit);
        std::cout << "   Magnitudine limite: " << data.mag_limit << std::endl;
        std::cout << std::endl;
        
        // Aggiungi stelle SAO
        std::cout << "⭐ Aggiunta stelle SAO..." << std::endl;
        renderer.addSAOStars(data.stars);
        
        // Aggiungi linee costellazioni
        std::cout << "🔷 Aggiunta linee costellazioni..." << std::endl;
        renderer.addConstellationLines(data.lines);
        
        // Aggiungi confini costellazioni
        std::cout << "🔶 Aggiunta confini costellazioni..." << std::endl;
        renderer.addConstellationBoundaries(data.boundaries);
        
        // Imposta il target
        std::cout << "🎯 Impostazione target con traiettoria..." << std::endl;
        renderer.setTarget(data.target);
        std::cout << std::endl;
        
        // Genera la carta (lo stile è già configurato con sfondo bianco di default)
        std::string output = "17030_finder_chart.png";
        std::cout << "🖼️  Generazione carta..." << std::endl;
        renderer.renderFinderChart(output);
        
        std::cout << "✅ Carta generata: " << output << std::endl;
        std::cout << "\n📋 Dettagli traiettoria:" << std::endl;
        for (const auto& point : data.trajectory_points) {
            std::cout << "   " << point.timestamp << " - RA: " << point.ra_deg 
                      << "° Dec: " << point.dec_deg << "°" << std::endl;
        }
        
        std::cout << "\n✨ Test completato con successo!" << std::endl;
        std::cout << "\n📝 Caratteristiche carta:" << std::endl;
        std::cout << "   ✓ Sfondo bianco per stampa" << std::endl;
        std::cout << "   ✓ " << data.stars.size() << " stelle SAO con numeri identificativi" << std::endl;
        std::cout << "   ✓ Linee costellazioni (blu)" << std::endl;
        std::cout << "   ✓ Confini costellazioni (verdi)" << std::endl;
        std::cout << "   ✓ Asteroide 17030 evidenziato (rosso)" << std::endl;
        std::cout << "   ✓ Traiettoria 26-29 Nov 2025 (arancione)" << std::endl;
        std::cout << "   ✓ Griglia coordinate RA/Dec" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Errore: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
