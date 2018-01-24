#pragma once

#include "collision.hpp"
#include "map.hpp"
#include "move.hpp"
#include "util.hpp"

namespace hlt {
    namespace navigation {
        static void check_and_add_entity_between(
                std::vector<const Entity *>& entities_found,
                const Location& start,
                const Location& target,
                const Entity& entity_to_check)
        {
            const Location &location = entity_to_check.location;
            if (location == start || location == target) {
                return;
            }
            if (collision::segment_circle_intersect(start, target, entity_to_check, constants::FORECAST_FUDGE_FACTOR)) {
                entities_found.push_back(&entity_to_check);
            }
        }

        static std::vector<const Entity *> objects_between(const Map& map, const Location& start, const Location& target) {
            std::vector<const Entity *> entities_found;

            for (const Planet& planet : map.planets) {
                check_and_add_entity_between(entities_found, start, target, planet);
            }

            for (const auto& player_ship : map.ships) {
                for (const Ship& ship : player_ship.second) {
                    check_and_add_entity_between(entities_found, start, target, ship);
                }
            }

            return entities_found;
        }

        static possibly<Move> navigate_ship_towards_target(
                const Map& map,
                const Ship& ship,
                const Location& target,
                const int max_thrust,
                const bool avoid_obstacles,
                const int max_corrections,
                const double angular_step_rad)
        {
            if (max_corrections <= 0) {
                return { Move::noop(), false };
            }

            const double distance = ship.location.get_distance_to(target);
            const double angle_rad = ship.location.orient_towards_in_rad(target);

            if (avoid_obstacles && !objects_between(map, ship.location, target).empty()) {
                const double new_target_dx = cos(angle_rad + angular_step_rad) * distance;
                const double new_target_dy = sin(angle_rad + angular_step_rad) * distance;
                const Location new_target = { ship.location.pos_x + new_target_dx, ship.location.pos_y + new_target_dy };

                return navigate_ship_towards_target(
                        map, ship, new_target, max_thrust, true, (max_corrections - 1), angular_step_rad);
            }

            int thrust;
            if (distance < max_thrust) {
                const double remainder = distance - int(distance);
                if (remainder >= 0.5) {
                    thrust = (int)distance + 1;
                }
                else {
                    thrust = (int)distance;
                }
            } else {
                thrust = max_thrust;
            }

            const int angle_deg = util::angle_rad_to_deg_clipped(angle_rad);

            return { Move::thrust(ship.entity_id, thrust, angle_deg), true };
        }

        static possibly<Move> navigate_ship_to_dock(
                const Map& map,
                const Ship& ship,
                const Entity& dock_target,
                const int max_thrust)
        {
            const int max_corrections = constants::MAX_NAVIGATION_CORRECTIONS;
            const bool avoid_obstacles = true;
            const double angular_step_rad = M_PI / 180.0;
            const Location& target = ship.location.get_closest_point(dock_target.location, dock_target.radius);

            return navigate_ship_towards_target(
                    map, ship, target, max_thrust, avoid_obstacles, max_corrections, angular_step_rad);
        }
		//Personalized navigation functions.
		static possibly<Move> navigate_ship_to_almost_target(
			const Map& map,
			const Ship& ship,
			const Entity& dock_target,
			const int max_thrust)
		{
			const int max_corrections = constants::MAX_NAVIGATION_CORRECTIONS;
			const bool avoid_obstacles = true;
			const double angular_step_rad = M_PI / 180.0;
			//																					the change is here
			const Location& target = ship.location.get_closest_point(dock_target.location, dock_target.radius - 2);

			return navigate_ship_towards_target(
				map, ship, target, max_thrust, avoid_obstacles, max_corrections, angular_step_rad);
		}
		static bool check_planet_between(
			const Location& start,
			const Location& target,
			const Entity& entity_to_check,
			const bool unit_leader)
		{
			const Location &location = entity_to_check.location;
			if (location == start || location == target) {
				return true;
			}
			if (unit_leader) {
				if (collision::segment_circle_intersect(start, target, entity_to_check, 1.5)) {
					return true;
				}
			}
			else {
				if (collision::segment_circle_intersect(start, target, entity_to_check, constants::FORECAST_FUDGE_FACTOR)) {
					return true;
				}
			}
			return false;
		}
		static bool check_target_between(
			const Location& start,
			const Location& target,
			const Location& ship_target)
		{
			const Location &location = ship_target;
			if (location == start || location == target) {
				return true;
			}
			//The only change in this function is that the return in a boolean.
			if (collision::segment_circle_intersect_for_targets(start, target, ship_target, constants::FORECAST_FUDGE_FACTOR)) {
				return true;
			}
			return false;
		}
    }
}
