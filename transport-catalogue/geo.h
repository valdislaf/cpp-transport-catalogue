#pragma once

#include <cmath>

const double RADIUS_EARTH = 6371000.0;

namespace transport {

    namespace geocoordinates {

        struct Coordinates {
            double lat;
            double lng;
            bool operator==(const Coordinates& other) const;
            bool operator!=(const Coordinates& other) const;
        };

         double ComputeDistance(Coordinates from, Coordinates to);
    }
}