#pragma once
#include <cstddef>
namespace SideViewV2 {
class ServicioNavegacionV2 {
public:
    size_t Anterior(size_t actual, size_t total) const {
        if (total == 0) return 0;
        if (actual >= total) return total - 1;
        return actual == 0 ? total - 1 : actual - 1;
    }

    size_t Siguiente(size_t actual, size_t total) const {
        if (total == 0) return 0;
        if (actual >= total - 1) return 0;
        return actual + 1;
    }
};
}
