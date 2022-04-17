#pragma once

#include <cmath>

const double RADIUS_EARTH = 6371000.0;

namespace transport {

    namespace geo_coordinates {

        struct Coordinates {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const;
            bool operator!=(const Coordinates& other) const;
        };

         double ComputeDistance(Coordinates from, Coordinates to);
    }
}