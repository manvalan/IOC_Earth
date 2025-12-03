#include "SkyMapRenderer.h"
#include "MapPathRenderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace ioc_earth {

class SkyMapRenderer::Impl {
public:
    std::unique_ptr<MapPathRenderer> renderer;
    
    Impl(unsigned int w, unsigned int h) {
        renderer = std::make_unique<MapPathRenderer>(w, h);
    }
};

SkyMapRenderer::SkyMapRenderer(unsigned int width, unsigned int height)
    : pImpl_(std::make_unique<Impl>(width, height))
    , width_(width)
    , height_(height)
    , center_ra_(0.0)
    , center_dec_(0.0)
    , field_of_view_(60.0)
    , mag_limit_(12.0)
    , target_{"", 0.0, 0.0, 0.0, {}, {}} {
}

SkyMapRenderer::~SkyMapRenderer() = default;

void SkyMapRenderer::setFieldOfView(double center_ra_deg, double center_dec_deg, 
                                     double field_of_view_deg) {
    center_ra_ = center_ra_deg;
    center_dec_ = center_dec_deg;
    field_of_view_ = field_of_view_deg;
    
    std::cout << "📐 Campo visivo impostato:" << std::endl;
    std::cout << "   Centro: RA " << center_ra_ << "° Dec " << center_dec_ << "°" << std::endl;
    std::cout << "   Campo visivo: " << field_of_view_ << "°" << std::endl;
}

void SkyMapRenderer::setMagnitudeLimit(double mag_limit) {
    mag_limit_ = mag_limit;
    std::cout << "🔆 Magnitudine limite: " << mag_limit_ << std::endl;
}

void SkyMapRenderer::addStars(const std::vector<StarData>& stars) {
    stars_ = stars;
    std::cout << "⭐ Aggiunte " << stars_.size() << " stelle SAO" << std::endl;
}

void SkyMapRenderer::addConstellationLines(const std::vector<ConstellationLineData>& lines) {
    constellation_lines_ = lines;
    std::cout << "🔷 Aggiunte " << constellation_lines_.size() << " linee di costellazioni" << std::endl;
}

void SkyMapRenderer::addConstellationBoundaries(const std::vector<ConstellationBoundaryData>& boundaries) {
    constellation_boundaries_ = boundaries;
    std::cout << "🔶 Aggiunti " << constellation_boundaries_.size() << " confini di costellazioni" << std::endl;
}

void SkyMapRenderer::setTarget(const TargetData& target) {
    target_ = target;
    std::cout << "🎯 Target impostato: " << target_.name << std::endl;
    std::cout << "   RA: " << target_.ra_deg << "° Dec: " << target_.dec_deg << "°" << std::endl;
    if (!target_.trajectory.empty()) {
        std::cout << "   Traiettoria: " << target_.trajectory.size() << " punti" << std::endl;
    }
}

void SkyMapRenderer::setFinderChartBounds(double center_ra, double center_dec, double fov_deg) {
    has_finder_chart_bounds_ = true;
    finder_chart_ra_ = center_ra;
    finder_chart_dec_ = center_dec;
    finder_chart_fov_ = fov_deg;
    std::cout << "📦 Rettangolo FOV finder chart impostato:" << std::endl;
    std::cout << "   Centro: RA " << finder_chart_ra_ << "° Dec " << finder_chart_dec_ << "°" << std::endl;
    std::cout << "   Campo: " << finder_chart_fov_ << "°" << std::endl;
}

void SkyMapRenderer::setStyle(const SkyMapStyle& style) {
    style_ = style;
}

bool SkyMapRenderer::renderSkyMap(const std::string& output_path) {
    try {
        std::cout << "\n🎨 === Rendering Mappa Celeste ===" << std::endl;
        
        // Imposta sfondo bianco
        pImpl_->renderer->setBackgroundColor(style_.background_color);
        
        // Imposta estensione mappa (RA/Dec)
        double half_fov = field_of_view_ / 2.0;
        pImpl_->renderer->setExtent(
            center_ra_ - half_fov,
            center_dec_ - half_fov,
            center_ra_ + half_fov,
            center_dec_ + half_fov
        );
        
        // Renderizza confini costellazioni
        if (style_.show_constellation_boundaries) {
            std::cout << "📍 Rendering confini costellazioni..." << std::endl;
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
        
        // Renderizza linee costellazioni
        if (style_.show_constellation_lines) {
            std::cout << "📐 Rendering linee costellazioni..." << std::endl;
            for (const auto& line : constellation_lines_) {
                std::vector<GPSPoint> path;
                path.emplace_back(line.ra1_deg, line.dec1_deg, "");
                path.emplace_back(line.ra2_deg, line.dec2_deg, "");
                
                pImpl_->renderer->addGPSPath(path, style_.constellation_line_color,
                                             style_.constellation_line_width);
            }
        }
        
        // Renderizza stelle SAO
        std::cout << "⭐ Rendering stelle SAO..." << std::endl;
        std::vector<GPSPoint> star_points;
        for (const auto& star : stars_) {
            if (star.magnitude > mag_limit_) continue;
            
            // Controlla se la stella è nel campo visivo
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
        std::cout << "   Stelle visualizzate: " << star_points.size() << std::endl;
        
        // Renderizza target e traiettoria
        if (!target_.name.empty()) {
            std::cout << "🎯 Rendering target e traiettoria..." << std::endl;
            
            // Traiettoria
            if (!target_.trajectory.empty()) {
                std::vector<GPSPoint> trajectory;
                for (size_t i = 0; i < target_.trajectory.size(); ++i) {
                    std::string label = i < target_.trajectory_timestamps.size() ? 
                                       target_.trajectory_timestamps[i] : "";
                    trajectory.emplace_back(target_.trajectory[i].first,
                                           target_.trajectory[i].second,
                                           label);
                }
                
                pImpl_->renderer->addGPSPath(trajectory, style_.trajectory_color, 
                                             style_.trajectory_line_width);
            }
            
            // Target
            std::vector<GPSPoint> target_point;
            target_point.emplace_back(target_.ra_deg, target_.dec_deg, target_.name);
            pImpl_->renderer->addPointLabels(target_point, "target", style_.label_font_size + 2);
        }
        
        // Renderizza rettangolo FOV del finder chart se impostato
        if (has_finder_chart_bounds_) {
            std::cout << "📦 Rendering rettangolo FOV finder chart (tratteggiato)..." << std::endl;
            
            double half_fc_fov = finder_chart_fov_ / 2.0;
            
            // Creiamo un rettangolo come 4 linee
            std::vector<std::pair<double, double>> rect_points = {
                {finder_chart_ra_ - half_fc_fov, finder_chart_dec_ - half_fc_fov},
                {finder_chart_ra_ + half_fc_fov, finder_chart_dec_ - half_fc_fov},
                {finder_chart_ra_ + half_fc_fov, finder_chart_dec_ + half_fc_fov},
                {finder_chart_ra_ - half_fc_fov, finder_chart_dec_ + half_fc_fov},
                {finder_chart_ra_ - half_fc_fov, finder_chart_dec_ - half_fc_fov}
            };
            
            std::vector<GPSPoint> rect_path;
            for (const auto& pt : rect_points) {
                rect_path.emplace_back(pt.first, pt.second, "");
            }
            
            pImpl_->renderer->addGPSPath(rect_path, style_.fov_rect_color, 
                                         style_.fov_rect_line_width);
        }
        
        // Renderizza e salva
        std::cout << "💾 Salvataggio mappa..." << std::endl;
        bool success = pImpl_->renderer->renderToFile(output_path);
        
        if (success) {
            std::cout << "\n✅ Mappa celeste generata: " << output_path << std::endl;
            std::cout << "\n📋 Dettagli renderizzazione:" << std::endl;
            std::cout << "   Centro: RA " << center_ra_ << "° Dec " << center_dec_ << "°" << std::endl;
            std::cout << "   Campo visivo: " << field_of_view_ << "°" << std::endl;
            std::cout << "   Dimensioni: " << width_ << "x" << height_ << " px" << std::endl;
            std::cout << "   Stelle visualizzate: " << star_points.size() << std::endl;
            std::cout << "   Magnitudine limite: " << mag_limit_ << std::endl;
            std::cout << "   Linee costellazioni: " << constellation_lines_.size() << std::endl;
            std::cout << "   Confini costellazioni: " << constellation_boundaries_.size() << std::endl;
            if (!target_.name.empty()) {
                std::cout << "   Target: " << target_.name << std::endl;
            }
            if (has_finder_chart_bounds_) {
                std::cout << "   ✓ Rettangolo FOV finder chart visibile" << std::endl;
            }
        }
        
        return success;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Errore nel rendering: " << e.what() << std::endl;
        return false;
    }
}

std::vector<uint8_t> SkyMapRenderer::getLastRenderedBuffer() const {
    return last_rendered_buffer_;
}

} // namespace ioc_earth
