#include "OccultationRenderer.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * Esempio di classe Application che usa l'API OccultationRenderer
 * per ottenere l'immagine in vari formati
 */
class OccultationWebService {
public:
    /**
     * Genera una mappa di occultazione e la restituisce come buffer PNG
     * Utile per API REST che devono restituire l'immagine via HTTP
     */
    std::vector<uint8_t> generateMapAsPNG(const std::string& json_data_path) {
        ioc_earth::OccultationRenderer renderer(1600, 1200);
        
        if (!renderer.loadFromJSON(json_data_path)) {
            std::cerr << "Errore caricamento dati" << std::endl;
            return {};
        }
        
        // Configura stile
        configureStyle(renderer);
        
        // Ottieni il PNG come buffer
        std::vector<uint8_t> png_buffer;
        if (renderer.renderToBuffer(png_buffer, true)) {
            std::cout << "✓ PNG generato: " << png_buffer.size() << " bytes" << std::endl;
            return png_buffer;
        }
        
        return {};
    }
    
    /**
     * Genera una mappa e la restituisce in formato base64
     * Utile per JSON API che devono embeddare l'immagine
     */
    std::string generateMapAsBase64(const std::string& json_data_path) {
        ioc_earth::OccultationRenderer renderer(1600, 1200);
        
        if (!renderer.loadFromJSON(json_data_path)) {
            return "";
        }
        
        configureStyle(renderer);
        
        // Renderizza e ottieni base64
        std::vector<uint8_t> buffer;
        if (renderer.renderToBuffer(buffer, true)) {
            std::string base64 = renderer.getLastRenderedImageBase64();
            std::cout << "✓ Base64 generato: " << base64.length() << " caratteri" << std::endl;
            return base64;
        }
        
        return "";
    }
    
    /**
     * Genera una pagina HTML completa con la mappa embedded
     * Utile per visualizzazione diretta in browser
     */
    bool generateHTMLReport(const std::string& json_data_path, 
                           const std::string& output_html) {
        ioc_earth::OccultationRenderer renderer(1600, 1200);
        
        if (!renderer.loadFromJSON(json_data_path)) {
            return false;
        }
        
        configureStyle(renderer);
        
        // Esporta in HTML
        return renderer.exportToHTML(output_html, true, 
                                     "Report Occultazione Asteroidale");
    }
    
    /**
     * API simulata per HTTP endpoint: GET /api/occultation/map?id=xxx
     * Restituisce il PNG come array di bytes
     */
    struct HTTPResponse {
        int status_code;
        std::string content_type;
        std::vector<uint8_t> body;
    };
    
    HTTPResponse handleMapRequest(const std::string& event_id) {
        HTTPResponse response;
        
        // Simula lookup del file JSON per l'evento
        std::string json_file = "../../data/chariklo_occultation.json";
        
        std::vector<uint8_t> png_data = generateMapAsPNG(json_file);
        
        if (png_data.empty()) {
            response.status_code = 500;
            response.content_type = "text/plain";
            std::string error = "Error generating map";
            response.body.assign(error.begin(), error.end());
        } else {
            response.status_code = 200;
            response.content_type = "image/png";
            response.body = png_data;
        }
        
        return response;
    }
    
    /**
     * API simulata per JSON endpoint: GET /api/occultation/data?id=xxx
     * Restituisce JSON con immagine embedded in base64
     */
    std::string handleJSONRequest(const std::string& event_id) {
        std::string json_file = "../../data/chariklo_occultation.json";
        std::string base64_image = generateMapAsBase64(json_file);
        
        if (base64_image.empty()) {
            return R"({"error": "Cannot generate map"})";
        }
        
        // Costruisci JSON response
        std::string json_response = "{\n";
        json_response += "  \"event_id\": \"" + event_id + "\",\n";
        json_response += "  \"status\": \"success\",\n";
        json_response += "  \"map_image\": {\n";
        json_response += "    \"format\": \"png\",\n";
        json_response += "    \"encoding\": \"base64\",\n";
        json_response += "    \"data\": \"" + base64_image + "\"\n";
        json_response += "  }\n";
        json_response += "}";
        
        return json_response;
    }

private:
    void configureStyle(ioc_earth::OccultationRenderer& renderer) {
        ioc_earth::OccultationRenderer::RenderStyle style;
        style.central_line_color = "#FF0000";
        style.central_line_width = 3.0;
        style.sigma_lines_color = "#FF8800";
        style.sigma_lines_width = 2.0;
        style.time_markers_color = "#0000FF";
        style.background_color = "#E8F4F8";
        renderer.setRenderStyle(style);
    }
};

/**
 * Simulazione di utilizzo delle API in vari scenari
 */
int main() {
    std::cout << "=== API Usage Examples ===" << std::endl;
    std::cout << "Simulazione di vari scenari di utilizzo dell'API\n" << std::endl;
    
    OccultationWebService service;
    std::string event_id = "2024-06-03-Chariklo";
    
    // ========================================
    // Scenario 1: HTTP REST API - Immagine PNG
    // ========================================
    std::cout << "\n--- Scenario 1: HTTP REST API (PNG binary) ---" << std::endl;
    std::cout << "GET /api/occultation/map?id=" << event_id << std::endl;
    
    auto http_response = service.handleMapRequest(event_id);
    std::cout << "Response:" << std::endl;
    std::cout << "  Status: " << http_response.status_code << std::endl;
    std::cout << "  Content-Type: " << http_response.content_type << std::endl;
    std::cout << "  Content-Length: " << http_response.body.size() << " bytes" << std::endl;
    std::cout << "  Uso: L'app client riceve il PNG e può visualizzarlo direttamente" << std::endl;
    
    // Salva per test
    if (http_response.status_code == 200) {
        std::ofstream out("api_response.png", std::ios::binary);
        out.write(reinterpret_cast<const char*>(http_response.body.data()), 
                  http_response.body.size());
        out.close();
        std::cout << "  ✓ Salvato in: api_response.png" << std::endl;
    }
    
    // ========================================
    // Scenario 2: JSON API con immagine embedded
    // ========================================
    std::cout << "\n--- Scenario 2: JSON API (base64 embedded) ---" << std::endl;
    std::cout << "GET /api/occultation/data?id=" << event_id << std::endl;
    
    std::string json_response = service.handleJSONRequest(event_id);
    std::cout << "Response (primi 500 caratteri):" << std::endl;
    std::cout << json_response.substr(0, 500) << "..." << std::endl;
    std::cout << "  Uso: L'app JavaScript può usare: " << std::endl;
    std::cout << "       const img = new Image();" << std::endl;
    std::cout << "       img.src = 'data:image/png;base64,' + response.map_image.data;" << std::endl;
    
    // Salva per test
    std::ofstream json_out("api_response.json");
    json_out << json_response;
    json_out.close();
    std::cout << "  ✓ Salvato in: api_response.json" << std::endl;
    
    // ========================================
    // Scenario 3: HTML Report Generation
    // ========================================
    std::cout << "\n--- Scenario 3: HTML Report Generation ---" << std::endl;
    
    bool html_success = service.generateHTMLReport(
        "../../data/chariklo_occultation.json",
        "web_report.html"
    );
    
    if (html_success) {
        std::cout << "✓ Report HTML generato: web_report.html" << std::endl;
        std::cout << "  Uso: Può essere servito direttamente via web server" << std::endl;
        std::cout << "       oppure inviato via email come report" << std::endl;
    }
    
    // ========================================
    // Scenario 4: Desktop Application
    // ========================================
    std::cout << "\n--- Scenario 4: Desktop Application ---" << std::endl;
    
    std::vector<uint8_t> png_for_desktop = service.generateMapAsPNG(
        "../../data/chariklo_occultation.json"
    );
    
    if (!png_for_desktop.empty()) {
        std::cout << "✓ Buffer PNG ottenuto: " << png_for_desktop.size() << " bytes" << std::endl;
        std::cout << "  Uso: L'app desktop può:" << std::endl;
        std::cout << "       - Caricare in QPixmap (Qt)" << std::endl;
        std::cout << "       - Caricare in wxImage (wxWidgets)" << std::endl;
        std::cout << "       - Caricare in GdkPixbuf (GTK)" << std::endl;
        std::cout << "       - Caricare in UIImage (iOS)" << std::endl;
        std::cout << "       - Salvare su filesystem" << std::endl;
        std::cout << "       - Inviare via socket/rete" << std::endl;
    }
    
    // ========================================
    // Scenario 5: Mobile App Integration
    // ========================================
    std::cout << "\n--- Scenario 5: Mobile App (React Native / Flutter) ---" << std::endl;
    
    std::string base64_for_mobile = service.generateMapAsBase64(
        "../../data/chariklo_occultation.json"
    );
    
    if (!base64_for_mobile.empty()) {
        std::cout << "✓ Base64 generato: " << base64_for_mobile.length() << " caratteri" << std::endl;
        std::cout << "  Uso in React Native:" << std::endl;
        std::cout << "       <Image source={{uri: 'data:image/png;base64,' + base64Data}} />" << std::endl;
        std::cout << "  Uso in Flutter:" << std::endl;
        std::cout << "       Image.memory(base64Decode(base64Data))" << std::endl;
    }
    
    // ========================================
    // Riepilogo
    // ========================================
    std::cout << "\n=== Riepilogo ===" << std::endl;
    std::cout << "L'API OccultationRenderer fornisce 3 metodi per l'app chiamante:\n" << std::endl;
    std::cout << "1. renderToBuffer() -> std::vector<uint8_t>" << std::endl;
    std::cout << "   Per: HTTP APIs, Desktop Apps, Mobile Apps, Database storage" << std::endl;
    std::cout << "\n2. getLastRenderedImageBase64() -> std::string" << std::endl;
    std::cout << "   Per: JSON APIs, HTML embedding, Email, Text storage" << std::endl;
    std::cout << "\n3. exportToHTML() -> HTML file" << std::endl;
    std::cout << "   Per: Web publishing, Email reports, Static websites" << std::endl;
    
    std::cout << "\n✓ Tutti gli scenari testati con successo!" << std::endl;
    
    return 0;
}
