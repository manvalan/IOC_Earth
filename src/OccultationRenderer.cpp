#include "OccultationRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <limits>

// Parser JSON semplice (per ora manuale, poi si può integrare nlohmann/json)
#include <regex>

// Per encoding base64
namespace {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string base64_encode(const std::vector<uint8_t>& data) {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        size_t in_len = data.size();
        const unsigned char* bytes_to_encode = data.data();
        
        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; (i < 4); i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];
            
            while((i++ < 3))
                ret += '=';
        }
        
        return ret;
    }
}

namespace ioc_earth {

OccultationRenderer::OccultationRenderer(unsigned int width, unsigned int height)
    : width_(width), height_(height) {
    renderer_ = std::make_unique<MapPathRenderer>(width, height);
}

OccultationRenderer::~OccultationRenderer() = default;

bool OccultationRenderer::loadFromJSON(const std::string& json_path) {
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << json_path << std::endl;
            return false;
        }
        
        // Leggi tutto il file
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string json_content = buffer.str();
        
        // Parser semplice per il JSON (usa regex per estrarre i dati)
        // Nota: In produzione, usare una libreria come nlohmann/json
        
        // Estrai event_id
        std::regex id_regex(R"raw("id"\s*:\s*"([^"]+)")raw");
        std::smatch match;
        if (std::regex_search(json_content, match, id_regex)) {
            data_.event_id = match[1];
        }
        
        // Estrai nome asteroide (formato semplice)
        std::regex asteroid_regex(R"raw("name"\s*:\s*"([^"]+)")raw");
        if (std::regex_search(json_content, match, asteroid_regex)) {
            data_.asteroid_name = match[1];
        }
        
        // Estrai nome stella
        std::regex star_regex(R"raw("catalog_id"\s*:\s*"([^"]+)")raw");
        if (std::regex_search(json_content, match, star_regex)) {
            data_.star_name = match[1];
        }
        
        // Estrai tempo evento
        std::regex time_regex(R"raw("gregorian"\s*:\s*"([^"]+)")raw");
        if (std::regex_search(json_content, match, time_regex)) {
            data_.date_time_utc = match[1];
        }
        
        // Estrai magnitude drop
        std::regex mag_regex(R"raw("magnitude_drop"\s*:\s*([\d.]+))raw");
        if (std::regex_search(json_content, match, mag_regex)) {
            data_.magnitude_drop = std::stod(match[1]);
        }
        
        // Estrai durata
        std::regex dur_regex(R"raw("duration_seconds"\s*:\s*([\d.]+))raw");
        if (std::regex_search(json_content, match, dur_regex)) {
            data_.duration_seconds = std::stod(match[1]);
        }
        
        // Parse central_line
        std::regex central_line_regex(R"raw("central_line"\s*:\s*\[)raw");
        if (std::regex_search(json_content, match, central_line_regex)) {
            std::regex point_regex(R"raw(\{"lon"\s*:\s*([-\d.]+),\s*"lat"\s*:\s*([-\d.]+),\s*"time"\s*:\s*"([^"]+)"\})raw");
            
            auto points_begin = std::sregex_iterator(json_content.begin(), json_content.end(), point_regex);
            auto points_end = std::sregex_iterator();
            
            bool in_central = false;
            for (std::sregex_iterator i = points_begin; i != points_end; ++i) {
                std::smatch point_match = *i;
                // Controllo semplificato: assume che i primi punti siano della central_line
                if (data_.central_line.size() < 11) {  // Limite per central_line
                    data_.central_line.emplace_back(
                        std::stod(point_match[1]),
                        std::stod(point_match[2]),
                        point_match[3]
                    );
                }
            }
        }
        
        // Parse northern_limit - parsing semplificato
        std::regex northern_start_regex(R"raw("northern_limit_1sigma")raw");
        if (std::regex_search(json_content, match, northern_start_regex)) {
            std::regex point_regex(R"raw(\{"lon"\s*:\s*([-\d.]+),\s*"lat"\s*:\s*([-\d.]+),\s*"time"\s*:\s*"([^"]+)"\})raw");
            
            auto points_begin = std::sregex_iterator(json_content.begin(), json_content.end(), point_regex);
            auto points_end = std::sregex_iterator();
            
            int count = 0;
            for (std::sregex_iterator i = points_begin; i != points_end; ++i) {
                count++;
                if (count > 11 && count <= 22) {  // Northern limit è il secondo gruppo
                    std::smatch point_match = *i;
                    data_.northern_limit.emplace_back(
                        std::stod(point_match[1]),
                        std::stod(point_match[2]),
                        point_match[3]
                    );
                }
            }
        }
        
        // Parse southern_limit - parsing semplificato
        std::regex southern_start_regex(R"raw("southern_limit_1sigma")raw");
        if (std::regex_search(json_content, match, southern_start_regex)) {
            std::regex point_regex(R"raw(\{"lon"\s*:\s*([-\d.]+),\s*"lat"\s*:\s*([-\d.]+),\s*"time"\s*:\s*"([^"]+)"\})raw");
            
            auto points_begin = std::sregex_iterator(json_content.begin(), json_content.end(), point_regex);
            auto points_end = std::sregex_iterator();
            
            int count = 0;
            for (std::sregex_iterator i = points_begin; i != points_end; ++i) {
                count++;
                if (count > 22) {  // Southern limit è il terzo gruppo
                    std::smatch point_match = *i;
                    data_.southern_limit.emplace_back(
                        std::stod(point_match[1]),
                        std::stod(point_match[2]),
                        point_match[3]
                    );
                }
            }
        }
        
        // Parse time_markers
        std::regex markers_regex(R"raw("time_markers")raw");
        if (std::regex_search(json_content, match, markers_regex)) {
            std::regex marker_regex(R"raw(\{"lon"\s*:\s*([-\d.]+),\s*"lat"\s*:\s*([-\d.]+),\s*"time"\s*:\s*"([^"]+)",\s*"seconds_from_mid"\s*:\s*([-\d]+)\})raw");
            
            auto markers_begin = std::sregex_iterator(json_content.begin(), json_content.end(), marker_regex);
            auto markers_end = std::sregex_iterator();
            
            for (std::sregex_iterator i = markers_begin; i != markers_end; ++i) {
                std::smatch marker_match = *i;
                OccultationData::TimeMarker tm;
                tm.longitude = std::stod(marker_match[1]);
                tm.latitude = std::stod(marker_match[2]);
                tm.time_utc = marker_match[3];
                tm.seconds_from_start = std::stoi(marker_match[4]);
                data_.time_markers.push_back(tm);
            }
        }
        
        // Parse observation_stations
        std::regex stations_regex(R"raw("observation_stations")raw");
        if (std::regex_search(json_content, match, stations_regex)) {
            std::regex station_regex(R"raw(\{"name"\s*:\s*"([^"]+)",\s*"lon"\s*:\s*([-\d.]+),\s*"lat"\s*:\s*([-\d.]+),\s*"status"\s*:\s*"([^"]+)"\})raw");
            
            auto stations_begin = std::sregex_iterator(json_content.begin(), json_content.end(), station_regex);
            auto stations_end = std::sregex_iterator();
            
            for (std::sregex_iterator i = stations_begin; i != stations_end; ++i) {
                std::smatch station_match = *i;
                OccultationData::ObservationStation station;
                station.name = station_match[1];
                station.longitude = std::stod(station_match[2]);
                station.latitude = std::stod(station_match[3]);
                station.status = station_match[4];
                data_.stations.push_back(station);
            }
        }
        
        std::cout << "✓ Dati occultazione caricati con successo" << std::endl;
        std::cout << "  Evento: " << data_.event_id << std::endl;
        std::cout << "  Asteroide: " << data_.asteroid_name << std::endl;
        std::cout << "  Stella: " << data_.star_name << std::endl;
        std::cout << "  Punti linea centrale: " << data_.central_line.size() << std::endl;
        std::cout << "  Time markers: " << data_.time_markers.size() << std::endl;
        std::cout << "  Stazioni: " << data_.stations.size() << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }
}

void OccultationRenderer::setOccultationData(const OccultationData& data) {
    data_ = data;
}

void OccultationRenderer::setRenderStyle(const RenderStyle& style) {
    style_ = style;
}

void OccultationRenderer::autoCalculateExtent(double margin_percent) {
    if (data_.central_line.empty()) {
        std::cerr << "Warning: No data to calculate extent" << std::endl;
        return;
    }
    
    // Trova i limiti considerando tutti i punti
    double min_lon = std::numeric_limits<double>::max();
    double max_lon = std::numeric_limits<double>::lowest();
    double min_lat = std::numeric_limits<double>::max();
    double max_lat = std::numeric_limits<double>::lowest();
    
    // Considera central_line
    for (const auto& p : data_.central_line) {
        min_lon = std::min(min_lon, p.longitude);
        max_lon = std::max(max_lon, p.longitude);
        min_lat = std::min(min_lat, p.latitude);
        max_lat = std::max(max_lat, p.latitude);
    }
    
    // Considera northern_limit
    for (const auto& p : data_.northern_limit) {
        min_lon = std::min(min_lon, p.longitude);
        max_lon = std::max(max_lon, p.longitude);
        min_lat = std::min(min_lat, p.latitude);
        max_lat = std::max(max_lat, p.latitude);
    }
    
    // Considera southern_limit
    for (const auto& p : data_.southern_limit) {
        min_lon = std::min(min_lon, p.longitude);
        max_lon = std::max(max_lon, p.longitude);
        min_lat = std::min(min_lat, p.latitude);
        max_lat = std::max(max_lat, p.latitude);
    }
    
    // Aggiungi margine
    double lon_margin = (max_lon - min_lon) * (margin_percent / 100.0);
    double lat_margin = (max_lat - min_lat) * (margin_percent / 100.0);
    
    // Margini minimi
    if (lon_margin < 0.5) lon_margin = 0.5;
    if (lat_margin < 0.5) lat_margin = 0.5;
    
    min_lon -= lon_margin;
    max_lon += lon_margin;
    min_lat -= lat_margin;
    max_lat += lat_margin;
    
    renderer_->setExtent(min_lon, min_lat, max_lon, max_lat);
}

void OccultationRenderer::renderCentralLine() {
    if (data_.central_line.empty()) return;
    
    // Converti in GPSPoint
    std::vector<GPSPoint> points;
    for (const auto& p : data_.central_line) {
        points.emplace_back(p.longitude, p.latitude, p.timestamp);
    }
    
    renderer_->addGPSPath(points, style_.central_line_color, style_.central_line_width);
}

void OccultationRenderer::renderSigmaLimits() {
    // Northern limit
    if (!data_.northern_limit.empty()) {
        std::vector<GPSPoint> points;
        for (const auto& p : data_.northern_limit) {
            points.emplace_back(p.longitude, p.latitude, "");
        }
        renderer_->addGPSPath(points, style_.sigma_lines_color, style_.sigma_lines_width);
    }
    
    // Southern limit
    if (!data_.southern_limit.empty()) {
        std::vector<GPSPoint> points;
        for (const auto& p : data_.southern_limit) {
            points.emplace_back(p.longitude, p.latitude, "");
        }
        renderer_->addGPSPath(points, style_.sigma_lines_color, style_.sigma_lines_width);
    }
}

void OccultationRenderer::renderTimeMarkers() {
    if (data_.time_markers.empty()) return;
    
    std::vector<GPSPoint> marker_points;
    for (const auto& tm : data_.time_markers) {
        std::string label = style_.show_time_labels ? tm.time_utc : "";
        marker_points.emplace_back(tm.longitude, tm.latitude, label);
    }
    
    renderer_->addPointLabels(marker_points, "timestamp", style_.label_font_size);
}

void OccultationRenderer::renderObservationStations() {
    if (data_.stations.empty()) return;
    
    // Raggruppa le stazioni per stato
    std::vector<GPSPoint> positive_stations, negative_stations, other_stations;
    
    for (const auto& station : data_.stations) {
        std::string label = style_.show_station_labels ? station.name : "";
        
        if (station.status == "positive") {
            positive_stations.emplace_back(station.longitude, station.latitude, label);
        } else if (station.status == "negative") {
            negative_stations.emplace_back(station.longitude, station.latitude, label);
        } else {
            other_stations.emplace_back(station.longitude, station.latitude, label);
        }
    }
    
    // Aggiungi le stazioni (con colori diversi se possibile)
    if (!positive_stations.empty()) {
        renderer_->addPointLabels(positive_stations, "timestamp", style_.label_font_size);
    }
    if (!negative_stations.empty()) {
        renderer_->addPointLabels(negative_stations, "timestamp", style_.label_font_size);
    }
    if (!other_stations.empty()) {
        renderer_->addPointLabels(other_stations, "timestamp", style_.label_font_size);
    }
}

bool OccultationRenderer::renderOccultationMap(const std::string& output_path, 
                                                bool include_shapefile) {
    try {
        std::cout << "\n=== Rendering Occultation Map ===" << std::endl;
        
        // Imposta lo sfondo
        renderer_->setBackgroundColor(style_.background_color);
        
        // Calcola l'estensione automaticamente
        std::cout << "Calcolo estensione mappa..." << std::endl;
        autoCalculateExtent(15.0);
        
        // Aggiungi griglia di coordinate (lat/lon)
        if (style_.show_grid) {
            std::cout << "Aggiunta griglia di coordinate..." << std::endl;
            double step = style_.grid_step_degrees;
            
            // Ottieni i limiti dalla mappa
            double min_lon, min_lat, max_lon, max_lat;
            // Nota: MapPathRenderer non espone questi valori direttamente
            // Possiamo calcolarli dai dati dell'occultazione
            min_lon = std::numeric_limits<double>::max();
            max_lon = std::numeric_limits<double>::lowest();
            min_lat = std::numeric_limits<double>::max();
            max_lat = std::numeric_limits<double>::lowest();
            
            for (const auto& p : data_.central_line) {
                min_lon = std::min(min_lon, p.longitude);
                max_lon = std::max(max_lon, p.longitude);
                min_lat = std::min(min_lat, p.latitude);
                max_lat = std::max(max_lat, p.latitude);
            }
            for (const auto& p : data_.northern_limit) {
                min_lon = std::min(min_lon, p.longitude);
                max_lon = std::max(max_lon, p.longitude);
                min_lat = std::min(min_lat, p.latitude);
                max_lat = std::max(max_lat, p.latitude);
            }
            for (const auto& p : data_.southern_limit) {
                min_lon = std::min(min_lon, p.longitude);
                max_lon = std::max(max_lon, p.longitude);
                min_lat = std::min(min_lat, p.latitude);
                max_lat = std::max(max_lat, p.latitude);
            }
            
            // Aggiungi margine
            double lon_margin = (max_lon - min_lon) * 0.15;
            double lat_margin = (max_lat - min_lat) * 0.15;
            min_lon -= lon_margin;
            max_lon += lon_margin;
            min_lat -= lat_margin;
            max_lat += lat_margin;
            
            // Linee verticali di longitudine
            double lon_start = std::floor(min_lon / step) * step;
            for (double lon = lon_start; lon <= max_lon; lon += step) {
                if (lon >= min_lon && lon <= max_lon) {
                    std::vector<GPSPoint> lon_line;
                    lon_line.emplace_back(lon, min_lat, "");
                    lon_line.emplace_back(lon, max_lat, "");
                    renderer_->addGPSPath(lon_line, style_.grid_color, 0.3);
                }
            }
            
            // Linee orizzontali di latitudine
            double lat_start = std::floor(min_lat / step) * step;
            for (double lat = lat_start; lat <= max_lat; lat += step) {
                if (lat >= min_lat && lat <= max_lat) {
                    std::vector<GPSPoint> lat_line;
                    lat_line.emplace_back(min_lon, lat, "");
                    lat_line.emplace_back(max_lon, lat, "");
                    renderer_->addGPSPath(lat_line, style_.grid_color, 0.3);
                }
            }
        }
        
        // Aggiungi shapefile se richiesto
        if (include_shapefile) {
            std::cout << "Caricamento shapefile..." << std::endl;
            renderer_->addShapefileLayer("../../data/ne_50m_admin_0_countries.shp", "countries");
            renderer_->addShapefileLayer("../../data/ne_50m_coastline.shp", "coastline");
        }
        
        // Renderizza i vari componenti
        std::cout << "Rendering limiti sigma..." << std::endl;
        renderSigmaLimits();
        
        std::cout << "Rendering linea centrale..." << std::endl;
        renderCentralLine();
        
        std::cout << "Rendering time markers..." << std::endl;
        renderTimeMarkers();
        
        std::cout << "Rendering stazioni osservazione..." << std::endl;
        renderObservationStations();
        
        // Renderizza la mappa finale
        std::cout << "Rendering finale..." << std::endl;
        bool success = renderer_->renderToFile(output_path);
        
        if (success) {
            std::cout << "\n✓ Mappa occultazione generata: " << output_path << std::endl;
            std::cout << "\nDettagli:" << std::endl;
            std::cout << "  Evento: " << data_.event_id << std::endl;
            std::cout << "  Asteroide: " << data_.asteroid_name << std::endl;
            std::cout << "  Stella: " << data_.star_name << std::endl;
            std::cout << "  Tempo: " << data_.date_time_utc << std::endl;
            std::cout << "  Durata: " << data_.duration_seconds << " secondi" << std::endl;
            std::cout << "  Calo magnitudine: " << data_.magnitude_drop << std::endl;
            if (style_.show_grid) {
                std::cout << "  ✓ Griglia RA/Dec ogni " << style_.grid_step_degrees << "° visibile" << std::endl;
            }
        }
        
        return success;
        
    } catch (const std::exception& e) {
        std::cerr << "Error rendering occultation map: " << e.what() << std::endl;
        return false;
    }
}

bool OccultationRenderer::renderToBuffer(std::vector<uint8_t>& png_data,
                                         bool include_shapefile) {
    try {
        // Crea un file temporaneo
        std::string temp_file = "/tmp/occultation_temp_" + 
                               std::to_string(std::time(nullptr)) + ".png";
        
        // Renderizza su file
        bool success = renderOccultationMap(temp_file, include_shapefile);
        
        if (!success) {
            return false;
        }
        
        // Leggi il file in memoria
        std::ifstream file(temp_file, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot read temporary file" << std::endl;
            return false;
        }
        
        // Ottieni la dimensione del file
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Leggi i dati
        png_data.resize(file_size);
        file.read(reinterpret_cast<char*>(png_data.data()), file_size);
        file.close();
        
        // Rimuovi il file temporaneo
        std::remove(temp_file.c_str());
        
        // Salva nella cache
        last_rendered_buffer_ = png_data;
        
        std::cout << "✓ Immagine PNG generata in buffer (" << png_data.size() << " bytes)" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error rendering to buffer: " << e.what() << std::endl;
        return false;
    }
}

bool OccultationRenderer::exportToHTML(const std::string& output_html_path,
                                       bool include_shapefile,
                                       const std::string& page_title) {
    try {
        std::cout << "\n=== Exporting to HTML ===" << std::endl;
        
        // Renderizza in buffer
        std::vector<uint8_t> png_data;
        if (!renderToBuffer(png_data, include_shapefile)) {
            return false;
        }
        
        // Converti in base64
        std::string base64_image = base64_encode(png_data);
        
        // Crea la pagina HTML
        std::ofstream html_file(output_html_path);
        if (!html_file) {
            std::cerr << "Error: Cannot create HTML file " << output_html_path << std::endl;
            return false;
        }
        
        // Scrivi l'HTML
        html_file << "<!DOCTYPE html>\n";
        html_file << "<html lang=\"it\">\n";
        html_file << "<head>\n";
        html_file << "    <meta charset=\"UTF-8\">\n";
        html_file << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        html_file << "    <title>" << page_title << "</title>\n";
        html_file << "    <style>\n";
        html_file << "        body {\n";
        html_file << "            font-family: Arial, sans-serif;\n";
        html_file << "            margin: 0;\n";
        html_file << "            padding: 20px;\n";
        html_file << "            background-color: #f5f5f5;\n";
        html_file << "        }\n";
        html_file << "        .container {\n";
        html_file << "            max-width: 1200px;\n";
        html_file << "            margin: 0 auto;\n";
        html_file << "            background-color: white;\n";
        html_file << "            padding: 30px;\n";
        html_file << "            border-radius: 10px;\n";
        html_file << "            box-shadow: 0 2px 10px rgba(0,0,0,0.1);\n";
        html_file << "        }\n";
        html_file << "        h1 {\n";
        html_file << "            color: #333;\n";
        html_file << "            border-bottom: 3px solid #4CAF50;\n";
        html_file << "            padding-bottom: 10px;\n";
        html_file << "        }\n";
        html_file << "        .info-box {\n";
        html_file << "            background-color: #f9f9f9;\n";
        html_file << "            border-left: 4px solid #4CAF50;\n";
        html_file << "            padding: 15px;\n";
        html_file << "            margin: 20px 0;\n";
        html_file << "        }\n";
        html_file << "        .info-box h2 {\n";
        html_file << "            margin-top: 0;\n";
        html_file << "            color: #4CAF50;\n";
        html_file << "            font-size: 1.2em;\n";
        html_file << "        }\n";
        html_file << "        .info-grid {\n";
        html_file << "            display: grid;\n";
        html_file << "            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));\n";
        html_file << "            gap: 15px;\n";
        html_file << "        }\n";
        html_file << "        .info-item {\n";
        html_file << "            padding: 10px;\n";
        html_file << "            background-color: white;\n";
        html_file << "            border-radius: 5px;\n";
        html_file << "        }\n";
        html_file << "        .info-label {\n";
        html_file << "            font-weight: bold;\n";
        html_file << "            color: #666;\n";
        html_file << "            font-size: 0.9em;\n";
        html_file << "        }\n";
        html_file << "        .info-value {\n";
        html_file << "            color: #333;\n";
        html_file << "            font-size: 1.1em;\n";
        html_file << "            margin-top: 5px;\n";
        html_file << "        }\n";
        html_file << "        .map-container {\n";
        html_file << "            text-align: center;\n";
        html_file << "            margin: 30px 0;\n";
        html_file << "        }\n";
        html_file << "        .map-container img {\n";
        html_file << "            max-width: 100%;\n";
        html_file << "            height: auto;\n";
        html_file << "            border: 2px solid #ddd;\n";
        html_file << "            border-radius: 5px;\n";
        html_file << "            box-shadow: 0 4px 8px rgba(0,0,0,0.1);\n";
        html_file << "        }\n";
        html_file << "        .legend {\n";
        html_file << "            background-color: #f9f9f9;\n";
        html_file << "            padding: 15px;\n";
        html_file << "            border-radius: 5px;\n";
        html_file << "            margin-top: 20px;\n";
        html_file << "        }\n";
        html_file << "        .legend h3 {\n";
        html_file << "            margin-top: 0;\n";
        html_file << "            color: #333;\n";
        html_file << "        }\n";
        html_file << "        .legend-item {\n";
        html_file << "            margin: 8px 0;\n";
        html_file << "            display: flex;\n";
        html_file << "            align-items: center;\n";
        html_file << "        }\n";
        html_file << "        .legend-color {\n";
        html_file << "            width: 30px;\n";
        html_file << "            height: 3px;\n";
        html_file << "            margin-right: 10px;\n";
        html_file << "        }\n";
        html_file << "        .footer {\n";
        html_file << "            text-align: center;\n";
        html_file << "            color: #999;\n";
        html_file << "            font-size: 0.9em;\n";
        html_file << "            margin-top: 30px;\n";
        html_file << "            padding-top: 20px;\n";
        html_file << "            border-top: 1px solid #ddd;\n";
        html_file << "        }\n";
        html_file << "    </style>\n";
        html_file << "</head>\n";
        html_file << "<body>\n";
        html_file << "    <div class=\"container\">\n";
        html_file << "        <h1>" << page_title << "</h1>\n";
        html_file << "        \n";
        html_file << "        <div class=\"info-box\">\n";
        html_file << "            <h2>Informazioni Evento</h2>\n";
        html_file << "            <div class=\"info-grid\">\n";
        html_file << "                <div class=\"info-item\">\n";
        html_file << "                    <div class=\"info-label\">ID Evento</div>\n";
        html_file << "                    <div class=\"info-value\">" << data_.event_id << "</div>\n";
        html_file << "                </div>\n";
        html_file << "                <div class=\"info-item\">\n";
        html_file << "                    <div class=\"info-label\">Asteroide</div>\n";
        html_file << "                    <div class=\"info-value\">" << data_.asteroid_name << "</div>\n";
        html_file << "                </div>\n";
        html_file << "                <div class=\"info-item\">\n";
        html_file << "                    <div class=\"info-label\">Stella</div>\n";
        html_file << "                    <div class=\"info-value\">" << data_.star_name << "</div>\n";
        html_file << "                </div>\n";
        html_file << "                <div class=\"info-item\">\n";
        html_file << "                    <div class=\"info-label\">Data/Ora (UTC)</div>\n";
        html_file << "                    <div class=\"info-value\">" << data_.date_time_utc << "</div>\n";
        html_file << "                </div>\n";
        html_file << "                <div class=\"info-item\">\n";
        html_file << "                    <div class=\"info-label\">Durata</div>\n";
        html_file << "                    <div class=\"info-value\">" << std::fixed << std::setprecision(1) 
                  << data_.duration_seconds << " secondi</div>\n";
        html_file << "                </div>\n";
        html_file << "                <div class=\"info-item\">\n";
        html_file << "                    <div class=\"info-label\">Calo Magnitudine</div>\n";
        html_file << "                    <div class=\"info-value\">" << std::fixed << std::setprecision(1) 
                  << data_.magnitude_drop << " mag</div>\n";
        html_file << "                </div>\n";
        html_file << "            </div>\n";
        html_file << "        </div>\n";
        html_file << "        \n";
        html_file << "        <div class=\"map-container\">\n";
        html_file << "            <img src=\"data:image/png;base64," << base64_image 
                  << "\" alt=\"Mappa Occultazione\">\n";
        html_file << "        </div>\n";
        html_file << "        \n";
        html_file << "        <div class=\"legend\">\n";
        html_file << "            <h3>Legenda</h3>\n";
        html_file << "            <div class=\"legend-item\">\n";
        html_file << "                <div class=\"legend-color\" style=\"background-color: " 
                  << style_.central_line_color << "; height: 3px;\"></div>\n";
        html_file << "                <span>Percorso centrale dell'ombra</span>\n";
        html_file << "            </div>\n";
        html_file << "            <div class=\"legend-item\">\n";
        html_file << "                <div class=\"legend-color\" style=\"background-color: " 
                  << style_.sigma_lines_color << "; height: 3px;\"></div>\n";
        html_file << "                <span>Limiti 1-sigma (incertezza)</span>\n";
        html_file << "            </div>\n";
        html_file << "            <div class=\"legend-item\">\n";
        html_file << "                <div class=\"legend-color\" style=\"background-color: " 
                  << style_.time_markers_color << "; height: 10px; width: 10px; border-radius: 50%;\"></div>\n";
        html_file << "                <span>Marker temporali lungo il percorso</span>\n";
        html_file << "            </div>\n";
        html_file << "            <div class=\"legend-item\">\n";
        html_file << "                <span style=\"margin-left: 40px;\">• Stazioni di osservazione con risultati</span>\n";
        html_file << "            </div>\n";
        html_file << "        </div>\n";
        html_file << "        \n";
        html_file << "        <div class=\"footer\">\n";
        html_file << "            Generato da IOC_Earth - Libreria C++ per visualizzazione occultazioni asteroidali<br>\n";
        html_file << "            <small>Dati compatibili con IOCalc</small>\n";
        html_file << "        </div>\n";
        html_file << "    </div>\n";
        html_file << "</body>\n";
        html_file << "</html>\n";
        
        html_file.close();
        
        std::cout << "✓ Pagina HTML generata: " << output_html_path << std::endl;
        std::cout << "  Dimensione immagine embedded: " << png_data.size() << " bytes" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error exporting to HTML: " << e.what() << std::endl;
        return false;
    }
}

std::string OccultationRenderer::getLastRenderedImageBase64() const {
    if (last_rendered_buffer_.empty()) {
        return "";
    }
    return base64_encode(last_rendered_buffer_);
}

} // namespace ioc_earth
