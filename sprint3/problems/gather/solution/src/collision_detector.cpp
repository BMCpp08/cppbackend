#include "collision_detector.h"
#include <cassert>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {
    // Проверим, что перемещение ненулевое.
    // Тут приходится использовать строгое равенство, а не приближённое,
    // пскольку при сборе заказов придётся учитывать перемещение даже на небольшое
    // расстояние.
    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    std::vector<GatheringEvent> result;
    size_t gatherers_count = provider.GatherersCount();
    size_t items_count = provider.ItemsCount();

    if (gatherers_count == 0 || items_count == 0) {
        return result;
    }
       
    for (int g_idx = 0; g_idx < gatherers_count; ++g_idx) {
        auto gatherer = provider.GetGatherer(g_idx);

        if (gatherer.start_pos.x == gatherer.end_pos.x &&
            gatherer.start_pos.y == gatherer.end_pos.y) {
            continue;
        }

        for (int i_idx = 0; i_idx < items_count; ++i_idx) {
            auto item = provider.GetItem(i_idx);

            auto collection_result = TryCollectPoint(gatherer.start_pos, gatherer.end_pos, item.position);
            if (collection_result.IsCollected(item.width + gatherer.width)) {
                result.emplace_back({ i_idx, g_idx , collection_result.sq_distance, collection_result.proj_ratio);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const GatheringEvent& r, const GatheringEvent& l) {
        return r.time < l.time;
        });

    return result;
}


}  // namespace collision_detector