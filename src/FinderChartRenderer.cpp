#include "FinderChartRenderer.h"
#include <MapPathRenderer.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

namespace ioc_earth {

// Implementazione semplificata usando Mapnik
class FinderChartRenderer::Impl {
public:
    std::unique_ptr<MapPathRenderer> renderer;
    
    Impl(unsigned int w, unsigned int h) {
        renderer = std::make_unique<MapPathRenderer>(w, h);
    }
};

FinderChartRenderer::FinderChartRenderer(unsigned int width, unsigned int height)
    : pImpl_(std::make_unique<Impl>(width, height))
    , width_(width)
    , height_(height)
    , center_ra_(0.0)
    , center_dec_(0.0)
    , field_of_view_(60.0)
    , mag_limit_(12.0) {
}

FinderChartRenderer::~FinderChartRenderer() = default;

void FinderChartRenderer::setFieldOfView(double center_ra_deg, double center_dec_deg, 
                                         double field_of_view_deg) {
    center_ra_ = center_ra_deg;
    center_dec_ = center_dec_deg;
    field_of_view_ = field_of_view_deg;
    
    std::cout << "Campo visivo impostato:" << std::endl;
    std::cout << "  Centro RA: " << center_ra_ << "°" << std::endl;
    std::cout << "  Centro Dec: " << center_dec_ << "°" << std::endl;
    std::cout << "  Campo: " << field_of_view_ << "°" << std::endl;
}

void FinderChartRenderer::setMagnitudeLimit(double mag_limit) {
    mag_limit_ = mag_limit;
    std::cout << "Magnitudine limite: " << mag_limit_ << std::endl;
}

void FinderChartRenderer::addSAOStars(const std::vector<SAOStar>& stars) {
    stars_ = stars;
    std::cout << "Aggiunte " << stars_.size() << " stelle SAO" << std::endl;
}

void FinderChartRenderer::addConstellationLines(const std::vector<ConstellationLine>& lines) {
    constellation_lines_ = lines;
    std::cout << "Aggiunte " << constellation_lines_.size() << " linee di costellazioni" << std::endl;
}

void FinderChartRenderer::addConstellationBoundaries(const std::vector<ConstellationBoundary>& boundaries) {
    constellation_boundaries_ = boundaries;
    std::cout << "Aggiunti " << constellation_boundaries_.size() << " confini di costellazioni" << std::endl;
}

void FinderChartRenderer::setTarget(const TargetInfo& target) {
    target_ = target;
    std::cout << "Target impostato: " << target_.name << std::endl;
    std::cout << "  RA: " << target_.ra_deg << "° Dec: " << target_.dec_deg << "°" << std::endl;
}

void FinderChartRenderer::setChartStyle(const ChartStyle& style) {
    style_ = style;
}

void FinderChartRenderer::celestialToPixel(double ra, double dec, int& x, int& y) const {
    // Proiezione semplice: ra/dec -> x/y
    // Centro del campo = centro immagine
    double half_fov = field_of_view_ / 2.0;
    
    // Delta rispetto al centro
    double delta_ra = ra - center_ra_;
    double delta_dec = dec - center_dec_;
    
    // Converti in pixel (scala lineare semplificata)
    x = static_cast<int>(width_ / 2.0 + (delta_ra / half_fov) * (width_ / 2.0));
    y = static_cast<int>(height_ / 2.0 - (delta_dec / half_fov) * (height_ / 2.0));
}

bool FinderChartRenderer::renderFinderChart(const std::string& output_path) {
    try {
        std::cout << "\n=== Rendering Finder Chart ===" << std::endl;
        
        // Imposta sfondo bianco
        pImpl_->renderer->setBackgroundColor(style_.background_color);
        
        // Imposta l'estensione (usa RA/Dec come coordinate)
        double half_fov = field_of_view_ / 2.0;
        pImpl_->renderer->setExtent(
            center_ra_ - half_fov,
            center_dec_ - half_fov,
            center_ra_ + half_fov,
            center_dec_ + half_fov
        );
        
        // Renderizza componenti
        std::cout << "Rendering confini costellazioni..." << std::endl;
        renderConstellationBoundaries();
        
        std::cout << "Rendering linee costellazioni..." << std::endl;
        renderConstellationLines();
        
        std::cout << "Rendering stelle SAO..." << std::endl;
        renderStars();
        
        std::cout << "Rendering target..." << std::endl;
        renderTarget();
        
        // Renderizza
        bool success = pImpl_->renderer->renderToFile(output_path);
        
        if (success) {
            std::cout << "\n✓ Finder Chart generata: " << output_path << std::endl;
            std::cout << "\nDettagli:" << std::endl;
            std::cout << "  Target: " << target_.name << std::endl;
            std::cout << "  Campo visivo: " << field_of_view_ << "°" << std::endl;
            std::cout << "  Stelle visualizzate: " << stars_.size() << std::endl;
            std::cout << "  Magnitudine limite: " << mag_limit_ << std::endl;
        }
        
        return success;
        
    } catch (const std::exception& e) {
        std::cerr << "Error rendering finder chart: " << e.what() << std::endl;
        return false;
    }
}

void FinderChartRenderer::renderConstellationBoundaries() {
    for (const auto& boundary : constellation_boundaries_) {
        if (boundary.points.size() < 2) continue;
        
        std::vector<GPSPoint> path;
        for (const auto& point : boundary.points) {
            path.emplace_back(point.first, point.second, "");
        }
        
        pImpl_->renderer->addGPSPath(path, style_.constellation_boundary_color, 
                                     style_.constellation_boundary_width);
    }
}

void FinderChartRenderer::renderConstellationLines() {
    for (const auto& line : constellation_lines_) {
        std::vector<GPSPoint> path;
        path.emplace_back(line.ra1_deg, line.dec1_deg, "");
        path.emplace_back(line.ra2_deg, line.dec2_deg, "");
        
        pImpl_->renderer->addGPSPath(path, style_.constellation_line_color,
                                     style_.constellation_line_width);
    }
}

void FinderChartRenderer::renderStars() {
    std::vector<GPSPoint> star_points;
    
    for (const auto& star : stars_) {
        if (star.magnitude > mag_limit_) continue;
        
        // Controlla se la stella è nel campo visivo
        double half_fov = field_of_view_ / 2.0;
        if (std::abs(star.ra_deg - center_ra_) > half_fov) continue;
        if (std::abs(star.dec_deg - center_dec_) > half_fov) continue;
        
        std::string label;
        if (style_.show_star_labels) {
            label = "SAO " + std::to_string(star.sao_number);
        }
        
        star_points.emplace_back(star.ra_deg, star.dec_deg, label);
    }
    
    if (!star_points.empty()) {
        pImpl_->renderer->addPointLabels(star_points, "star", style_.label_font_size);
    }
}

void FinderChartRenderer::renderTarget() {
    if (target_.name.empty()) return;
    
    // Renderizza il target
    std::vector<GPSPoint> target_point;
    target_point.emplace_back(target_.ra_deg, target_.dec_deg, target_.name);
    pImpl_->renderer->addPointLabels(target_point, "target", style_.label_font_size + 2);
    
    // Renderizza la traiettoria se presente
    if (!target_.trajectory.empty()) {
        std::vector<GPSPoint> trajectory;
        for (size_t i = 0; i < target_.trajectory.size(); ++i) {
            std::string label = i < target_.trajectory_times.size() ? 
                              target_.trajectory_times[i] : "";
            trajectory.emplace_back(target_.trajectory[i].first,
                                  target_.trajectory[i].second,
                                  label);
        }
        pImpl_->renderer->addGPSPath(trajectory, style_.trajectory_color, 2.0);
    }
}

void FinderChartRenderer::renderGrid() {
    // TODO: Implementare griglia RA/Dec
}

void FinderChartRenderer::renderLabels() {
    // TODO: Etichette costellazioni
}

bool FinderChartRenderer::renderToBuffer(std::vector<uint8_t>& png_data) {
    std::string temp_file = "/tmp/finder_chart_temp_" + 
                           std::to_string(std::time(nullptr)) + ".png";
    
    if (!renderFinderChart(temp_file)) {
        return false;
    }
    
    std::ifstream file(temp_file, std::ios::binary);
    if (!file) return false;
    
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    png_data.resize(file_size);
    file.read(reinterpret_cast<char*>(png_data.data()), file_size);
    file.close();
    
    std::remove(temp_file.c_str());
    last_rendered_buffer_ = png_data;
    
    return true;
}

bool FinderChartRenderer::exportToHTML(const std::string& output_html_path,
                                       const std::string& page_title) {
    // TODO: Implementare export HTML
    return false;
}

std::string FinderChartRenderer::getLastRenderedImageBase64() const {
    // TODO: Implementare base64 encoding
    return "";
}

} // namespace ioc_earth
