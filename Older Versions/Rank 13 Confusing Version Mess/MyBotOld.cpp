#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"
#include <math.h>

#define NO_RADIUS 100000000
#define SET_TARGET_LAST 500.0
#define DEFENSE_MULTIPLIER 3.0

/*
	Folder Name:	Last Version + Distraction Prevention
	Last Versions:	Rank 22 + Outnumbered Defense + Bunny Ship (v2.1) + Collision Rounding Fixes
					Rank 43 + Outnumbered Defense + Bunny Ship (v2)
					+/--Rank 50 (43-54) + Kamikaze Defense
					-Rank 50 (46) + Outnumbered Defense
					+Rank 50 (51) + Kamikaze Defense + Bunny Ship (v1)
 */

/*
    Terms referenced:
        -Bunny ship: A ship that "hops away" from enemy ships, distracting enemies by being uncatchable yet likely the closest ship they're chasing.
 */

/*
	Major issues:

	Note: Code needs major refactoring and the functions need their own file.
	Note: Still can't test locally.
	Note: The last collision issues (which only take place by the time I'm going to win and I have 10+ swarms) takes place regarding the SET_TARGET_LAST ships and ships not moving.
	Note: Enemy "bunny ships" are distracting and build up a large, unproductive swarm that can't catch it when I'm defending.
 */

/*
	Minor issues:
	
	Note: My "bunny ship" only moves away from the closest enemy, and still might get caught.
	Idea: Implement a check for distance to enemies within the collision check.

	Note: The bot is getting beat by about a turn in setting up and producing the first ship.
	Idea: This is less of a priority than developing strategic attacks, and requites extensive optimizations.
 */


//Special Case Functions
bool NoobRush(const hlt::Ship& ship, const hlt::Ship& enemy, const hlt::Planet& planet);
hlt::Ship ClosestUndockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Ship ClosestDockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Location Strafe(const hlt::Ship& ship, hlt::Ship& target, const hlt::Ship& hunter, const hlt::Map& map);


//Defensive Functions
hlt::Planet FindClosestAnyPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets);
bool EnemyNearPlanet(const hlt::Planet& planet, const hlt::Map& map, hlt::PlayerId player_id);
bool EnemyIsolated(const hlt::Ship& enemy, const hlt::Map& map);
bool EnoughShipsCloser(const hlt::Ship& ship, const hlt::Ship& enemy, const std::vector<hlt::Ship> my_ships);


//Offensive Functions
hlt::Ship FindClosestShip(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id, double radius);
//Credit to skylarh for FindClosestPlanet (not neccessarily in its original form).
hlt::Planet FindClosestPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets, hlt::PlayerId player_id);
bool EnoughAlliesCloser(const hlt::Planet& planet, const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Planet FindNextPlanet(const hlt::Planet& planet, const hlt::Ship& ship, std::vector<hlt::Planet> planets, hlt::PlayerId player_id);
hlt::Ship NearestDockedShip(const hlt::Ship& ship, const hlt::Planet& planet, const hlt::Map& map);
//Grouping Functions
bool Outnumbered(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Location ClosestTarget(const hlt::Ship& ship, std::vector<hlt::Location> targets, std::vector<double> distances);


//Navigation Functions
hlt::Location SetTarget(const hlt::Ship& ship, const hlt::Location& target);
bool TargetOrPlanetInTheWay(const hlt::Location& location, const hlt::Location& target, const hlt::Map& map, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, std::vector<double> distances, unsigned int index);
bool CrissCross(const hlt::Location& location, const hlt::Location& target, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, unsigned int index);
int orientation(const hlt::Location& p, const hlt::Location& q, const hlt::Location& r);
bool OutOfBounds(const hlt::Location& target, const hlt::Map& map);


int main() {
    const hlt::Metadata metadata = hlt::initialize("IvanTheTerrible");
    const hlt::PlayerId player_id = metadata.player_id;

    const hlt::Map& initial_map = metadata.initial_map;

    //We now have 1 full minute to analyse the initial map.
    std::ostringstream initial_map_intelligence;
    initial_map_intelligence
            << "width: " << initial_map.map_width
            << "; height: " << initial_map.map_height
            << "; players: " << initial_map.ship_map.size()
            << "; my ships: " << initial_map.ship_map.at(player_id).size()
            << "; planets: " << initial_map.planets.size();
    hlt::Log::log(initial_map_intelligence.str());

    std::vector<hlt::Move> moves;

	//By using push_back's together, each index "i" will correspond to the other vector's index "i."
	//Instead of storing the ship's location, the whole ship has to be stored just for the movement command at the end.
	std::vector<hlt::Location> targets;
	std::vector<hlt::Ship> ships;
	std::vector<double> distances;

    for (;;) {
        moves.clear();
        const hlt::Map map = hlt::in::get_map();
        
        
		//Reset targets.
		targets.clear();
		ships.clear();
		distances.clear();
        
        
        //Check for special cases (like a noob rush) and give unique instructions.
        hlt::EntityId special_id = 999;
        if (map.ships.size() == 2) {
            const hlt::Ship first_ship = map.ships.at(player_id)[0];
            const hlt::Planet& nearest_planet = FindClosestPlanet(first_ship, map.planets, player_id);
            hlt::Ship first_enemy;
            if (player_id == 1) {
                first_enemy = map.ships.at(0)[0];
            }
            else {
                first_enemy = map.ships.at(1)[0];
            }
            if (first_ship.docking_status == hlt::ShipDockingStatus::Undocked && NoobRush(first_ship, first_enemy, nearest_planet)) {
                special_id = first_ship.entity_id;
                hlt::Location ship_target;
                hlt::Ship hunter = ClosestUndockedEnemy(first_ship, map, player_id);
				hlt::Ship target = ClosestDockedEnemy(first_ship, map, player_id);
                //If there's no undocked enemies,
                if (hunter.entity_id == first_ship.entity_id) {
                    //Attack the nearest docked ship.
                    ship_target = first_ship.location.get_closest_point(target.location, target.radius - 2);
                }
                else {
                    if (hunter.location.get_distance_to(first_ship.location) >= 19.1) {
                        //If there are no docked ships,
                        if (target.entity_id == first_ship.entity_id) {
                            //Chase the hunter.
                            ship_target = hunter.location;
                        }
                        else {
                            //Attack the nearest docked ship.
                            ship_target = first_ship.location.get_closest_point(target.location, target.radius - 2);
                        }
                    }
                    else if (hunter.location.get_distance_to(first_ship.location) >= 12.1) {
						if (target.entity_id == first_ship.entity_id) {
							//Don't move.
							ship_target = first_ship.location;
						}
						else {
							//Move perpendicular from the hunter, whichever is closer to the closest docked ship.
							ship_target = Strafe(first_ship, target, hunter, map);
						}
                    }
                    else {
						//Move opposite of the hunter.
                        ship_target = { 2 * first_ship.location.pos_x - hunter.location.pos_x, 2 * first_ship.location.pos_y - hunter.location.pos_y };
                    }
                }
                targets.push_back(SetTarget(first_ship, ship_target));
                ships.push_back(first_ship);
                distances.push_back(first_ship.location.get_distance_to(ship_target));
            }
        }


		//Set targets.
        for (const hlt::Ship& ship : map.ships.at(player_id)) {
			//Stay docked if you are.
            if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				targets.push_back(ship.location);
				ships.push_back(ship);
				distances.push_back(0.0);
                continue;
            }
            
            //Check if the ship was already given a target in a special case.
            if (ship.entity_id == special_id) {
                continue;
            }

			/*
			Defense strategy:
			1. Is there a near enemy?
			2. Attack it.
			*/

			hlt::Planet nearest_planet = FindClosestAnyPlanet(ship, map.planets);
			if (nearest_planet.owner_id == player_id) {
				if (EnemyNearPlanet(nearest_planet, map, player_id)) {
					hlt::Ship nearest_enemy = FindClosestShip(ship, map, player_id, NO_RADIUS);
                    //Kamikaze defense.
                    /*hlt::Location ship_target = nearest_enemy.location;
                    targets.push_back(SetTarget(ship, ship_target));
                    ships.push_back(ship);
                    distances.push_back(ship.location.get_distance_to(ship_target));*/
                    //Outnumbered defense.
                    if (Outnumbered(ship, map, player_id)) {
                        targets.push_back({ 0, 0 });
                        ships.push_back(ship);
                        distances.push_back(SET_TARGET_LAST);
                    }
                    else {
						//If the nearest enemy is alone and 2 or more allied ships are closer,
						if (EnemyIsolated(nearest_enemy, map) && EnoughShipsCloser(ship, nearest_enemy, map.ships.at(player_id))) {
							//Move towards the closest planet not full and owned by me.
							const hlt::Planet& target_planet = FindNextPlanet(nearest_planet, ship, map.planets, player_id);
							hlt::Location ship_target = ship.location.get_closest_point(target_planet.location, target_planet.radius);
							targets.push_back(SetTarget(ship, ship_target));
							ships.push_back(ship);
							distances.push_back(ship.location.get_distance_to(ship_target));
						}
						else {
							hlt::Location ship_target = ship.location.get_closest_point(nearest_enemy.location, nearest_enemy.radius - 2);
							targets.push_back(SetTarget(ship, ship_target));
							ships.push_back(ship);
							distances.push_back(ship.location.get_distance_to(ship_target));
						}
                    }
                    continue;
				}
			}

            /*
             The current offensive strategy is to locate the closest planet that is either not full or not owned by me, and run a unique strategy for each three cases of ownership:
             
             1. Not owned.
             - If there are nearby enemies (scaling with planet radius), target them first.
             - Otherwise, try to dock.
             - Otherwise, if you have no enemies in a really large radius and there are more than enough ships heading to the planet, find a new planet to target.
             - Otherwise, continue moving towards the planet.
             2. Owned by me but it needs more docked ships.
             - Attack any nearby enemies (why did this planet need another ship?).
             - Otherwise dock or move to it.
             3. Owned by someone else.
             - Attack the closest docked ship.
             */

			//Find the closest planet that is not fully docked by me.
			const hlt::Planet& target_planet = FindClosestPlanet(ship, map.planets, player_id);

			//If nobody owns the planet,
            if (!target_planet.owned) {
				//find the nearest enemy,
				hlt::Ship nearest_enemy = FindClosestShip(ship, map, player_id, NO_RADIUS);
				//if there are no nearby enemies within a certain range,
				if (ship.location.get_distance_to(nearest_enemy.location) > target_planet.radius * 3.0) {
					//try to dock,
					if (ship.can_dock(target_planet)) {
						targets.push_back(ship.location);
						ships.push_back(ship);
						distances.push_back(0.0);
						moves.push_back(hlt::Move::dock(ship.entity_id, target_planet.entity_id));
					}
					//or if I can't dock,
					else {
						//check if there is a need for more ships to go to the planet,
						if (target_planet.location.get_distance_to(nearest_enemy.location) < 50.0 || !EnoughAlliesCloser(target_planet, ship, map, player_id)) {
							hlt::Location ship_target = ship.location.get_closest_point(target_planet.location, target_planet.radius);
							targets.push_back(SetTarget(ship, ship_target));
							ships.push_back(ship);
							distances.push_back(ship.location.get_distance_to(ship_target));
						}
						//otherwise locate the second closest planet,
						else {
							const hlt::Planet& new_planet = FindNextPlanet(target_planet, ship, map.planets, player_id);
							hlt::Location ship_target = ship.location.get_closest_point(new_planet.location, new_planet.radius);
							targets.push_back(SetTarget(ship, ship_target));
							ships.push_back(ship);
							distances.push_back(ship.location.get_distance_to(ship_target));
						}
					}
				}
				//otherwise there is a nearby enemy,
				else {
					if (Outnumbered(ship, map, player_id)) {
						targets.push_back({ 0, 0 });
						ships.push_back(ship);
						distances.push_back(SET_TARGET_LAST);
					}
					else {
						hlt::Location ship_target = ship.location.get_closest_point(nearest_enemy.location, nearest_enemy.radius - 2);
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));
					}
				}
			}
			
			//or I need more ships docked to mine,
			else if (target_planet.owner_id == player_id) {
				//check for nearby enemies,
				hlt::Ship nearest_enemy = FindClosestShip(ship, map, player_id, NO_RADIUS);
				//and if there are no nearby enemies relative to the planet and the ship is needed,
				if (target_planet.location.get_distance_to(nearest_enemy.location) > 20 && !EnoughAlliesCloser(target_planet, ship, map, player_id)) {
					//dock if you can,
					if (ship.can_dock(target_planet)) {
						targets.push_back(ship.location);
						ships.push_back(ship);
						distances.push_back(0.0);
						moves.push_back(hlt::Move::dock(ship.entity_id, target_planet.entity_id));
					}
					else {
						hlt::Location ship_target = ship.location.get_closest_point(target_planet.location, target_planet.radius);
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));
					}
				}
				//or if there's an enemy nearby/enough ships heading to fill the spot,
				else {
					if (Outnumbered(ship, map, player_id)) {
						targets.push_back({ 0, 0 });
						ships.push_back(ship);
						distances.push_back(SET_TARGET_LAST);
					}
					else {
						//If the nearest enemy is alone and 2 or more allied ships are closer,
						if (EnemyIsolated(nearest_enemy, map) && EnoughShipsCloser(ship, nearest_enemy, map.ships.at(player_id))) {
							//Move towards the closest planet not full and owned by me.
							const hlt::Planet& target_planet = FindNextPlanet(nearest_planet, ship, map.planets, player_id);
							hlt::Location ship_target = ship.location.get_closest_point(target_planet.location, target_planet.radius);
							targets.push_back(SetTarget(ship, ship_target));
							ships.push_back(ship);
							distances.push_back(ship.location.get_distance_to(ship_target));
						}
						else {
							hlt::Location ship_target = ship.location.get_closest_point(nearest_enemy.location, nearest_enemy.radius - 2);
							targets.push_back(SetTarget(ship, ship_target));
							ships.push_back(ship);
							distances.push_back(ship.location.get_distance_to(ship_target));
						}
					}
				}
			}
			//otherwise it's not mine,
			else {
				//so find the closest ship docked to it,
				hlt::Ship nearest_docked_ship = NearestDockedShip(ship, target_planet, map);
				if (Outnumbered(ship, map, player_id)) {
					targets.push_back({ 0, 0 });
					ships.push_back(ship);
					distances.push_back(SET_TARGET_LAST);
				}
				else {
					hlt::Location ship_target = ship.location.get_closest_point(nearest_docked_ship.location, nearest_docked_ship.radius - 2);
					targets.push_back(SetTarget(ship, ship_target));
					ships.push_back(ship);
					distances.push_back(ship.location.get_distance_to(ship_target));
				}
			}
        }


		//Rearrange the ships by distance to their target with a bubble sort.
		for (unsigned int i = 0; i < distances.size() - 1; i++) {
			for (unsigned int j = 0; j < distances.size() - i - 1; j++) {
				if (distances[j] > distances[j + 1]) {
					hlt::Location target_holder = targets[j];
					hlt::Ship ship_holder = ships[j];
					double distance_holder = distances[j];
					targets[j] = targets[j + 1];
					ships[j] = ships[j + 1];
					distances[j] = distances[j + 1];
					targets[j + 1] = target_holder;
					ships[j + 1] = ship_holder;
					distances[j + 1] = distance_holder;
				}
			}
		}


		//Manage any collision between ally ships.
		for (unsigned int i = 0; i < targets.size(); i++) {
			//If the ship is already docked/has been commanded to dock, skip this loop.
			if (targets[i] == ships[i].location) {
				continue;
			}

			if (distances[i] == SET_TARGET_LAST) {
				targets[i] = ClosestTarget(ships[i], targets, distances);
			}

			//Move targets that will hit planets or other targets.
			//The corrections go 90 degrees either way, and 1 is added because of the initial decrement.
			int max_corrections = 181;
			bool left = true;
			//The loop now only checks ally targets with a distance closer than the ship target.
			while ((TargetOrPlanetInTheWay(ships[i].location, targets[i], map, ships, targets, distances, i) || CrissCross(ships[i].location, targets[i], ships, targets, i)) && max_corrections) {
				max_corrections--;
				const double distance = ships[i].location.get_distance_to(targets[i]);
				const double angle_rad = ships[i].location.orient_towards_in_rad(targets[i]);
				double new_target_dx;
				double new_target_dy;
				//Alternate checking 1 degree left/right.
				if (left) {
					new_target_dx = cos(angle_rad + (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					new_target_dy = sin(angle_rad + (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					left = false;
				}
				else {
					new_target_dx = cos(angle_rad - (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					new_target_dy = sin(angle_rad - (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					left = true;
				}
				targets[i] = { ships[i].location.pos_x + new_target_dx, ships[i].location.pos_y + new_target_dy };
			}
			//If a safe target could not be found, don't move.
			if (!max_corrections) {
				targets[i] = ships[i].location;
				continue;
			}
		}
		//TIME_OUT REFACTORING: This loop may not be needed with the "closest ship priority."
		//A second correction attempts to fix targets corrected before a ship in front of it decided to not move, then moves each ship.
		//Most locations will remain the same and not be too time consuming.
		for (unsigned int i = 0; i < targets.size(); i++) {
			//If the ship is already docked/has been commanded to dock, skip this loop.
			if (targets[i] == ships[i].location) {
				continue;
			}

			//Move targets that will hit planets or other targets.
			//The corrections go 90 degrees either way, and 1 is added because of the initial decrement.
			int max_corrections = 181;
			bool left = true;
			while ((TargetOrPlanetInTheWay(ships[i].location, targets[i], map, ships, targets, distances, i) || CrissCross(ships[i].location, targets[i], ships, targets, i) || OutOfBounds(targets[i], map)) && max_corrections) {
				max_corrections--;
				const double distance = ships[i].location.get_distance_to(targets[i]);
				const double angle_rad = ships[i].location.orient_towards_in_rad(targets[i]);
				double new_target_dx;
				double new_target_dy;
				//Alternate checking 1 degree left/right.
				if (left) {
					new_target_dx = cos(angle_rad + (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					new_target_dy = sin(angle_rad + (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					left = false;
				}
				else {
					new_target_dx = cos(angle_rad - (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					new_target_dy = sin(angle_rad - (M_PI / 180.0)*(181.0 - max_corrections)) * distance;
					left = true;
				}
				targets[i] = { ships[i].location.pos_x + new_target_dx, ships[i].location.pos_y + new_target_dy };
			}
			//If a safe target could not be found, don't move.
			if (!max_corrections) {
				targets[i] = ships[i].location;
				continue;
			}

			//Move.
			const hlt::possibly<hlt::Move> move =
				hlt::navigation::navigate_ship_towards_target(map, ships[i], targets[i], hlt::constants::MAX_SPEED, false, hlt::constants::MAX_NAVIGATION_CORRECTIONS, 0.0);
			if (move.second) {
				moves.push_back(move.first);
			}
		}

        if (!hlt::out::send_moves(moves)) {
            hlt::Log::log("send_moves failed; exiting");
            break;
        }
    }
}

//
//Special Case Functions
//

bool NoobRush(const hlt::Ship& ship, const hlt::Ship& enemy, const hlt::Planet& planet) {
    hlt::Location docking_spot = ship.location.get_closest_point(planet.location, planet.radius);
    if ((int)(enemy.location.get_distance_to(docking_spot) / 7) - (int)(ship.location.get_distance_to(docking_spot) / 7) <= 12) {
        return true;
    }
    return false;
}

hlt::Ship ClosestUndockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id) {
    //Returns the ship if there are no near enemies.
    hlt::Ship current_ship = ship;
    double radius = NO_RADIUS;
    
    for (const auto& player_ship : map.ships) {
        for (const hlt::Ship& potential_ship : player_ship.second) {
            if (potential_ship.owner_id != player_id && potential_ship.docking_status == hlt::ShipDockingStatus::Undocked) {
                double dist = ship.location.get_distance_to(potential_ship.location);
                if (dist < radius) {
                    current_ship = potential_ship;
                    radius = dist;
                }
            }
        }
    }
    
    return current_ship;
}

hlt::Ship ClosestDockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id) {
    //Returns the ship if there are no near enemies.
    hlt::Ship current_ship = ship;
    double radius = NO_RADIUS;
    
    for (const auto& player_ship : map.ships) {
        for (const hlt::Ship& potential_ship : player_ship.second) {
            if (potential_ship.owner_id != player_id && potential_ship.docking_status != hlt::ShipDockingStatus::Undocked) {
                double dist = ship.location.get_distance_to(potential_ship.location);
                if (dist < radius) {
                    current_ship = potential_ship;
                    radius = dist;
                }
            }
        }
    }
    
    return current_ship;
}

hlt::Location Strafe(const hlt::Ship& ship, hlt::Ship& target, const hlt::Ship& hunter, const hlt::Map& map) {
	int angle_deg = ship.location.orient_towards_in_deg(hunter.location);
	double left_dx = cos(angle_deg * (M_PI / 180) + (M_PI / 2.0)) * 7.0;
	double left_dy = sin(angle_deg * (M_PI / 180) + (M_PI / 2.0)) * 7.0;
	double right_dx = cos(angle_deg * (M_PI / 180) - (M_PI / 2.0)) * 7.0;
	double right_dy = sin(angle_deg * (M_PI / 180) - (M_PI / 2.0)) * 7.0;
	hlt::Location left_strafe = { ship.location.pos_x + left_dx, ship.location.pos_y + left_dy };
	hlt::Location right_strafe = { ship.location.pos_x + right_dx, ship.location.pos_y + right_dy };
	if (left_strafe.get_distance_to(target.location) <= right_strafe.get_distance_to(target.location)) {
		return left_strafe;
	}
	return right_strafe;
}

//
//Defensive functions
//

hlt::Planet FindClosestAnyPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets) {
	//Returns the closest planet.
	hlt::Planet current_planet = planets[0];
	double shortest = NO_RADIUS;

	for (const hlt::Planet& planet : planets) {
		double dist = ship.location.get_distance_to(planet.location);
		if (dist < shortest) {
			current_planet = planet;
			shortest = dist;
		}
	}

	return current_planet;
}

bool EnemyNearPlanet(const hlt::Planet& planet, const hlt::Map& map, hlt::PlayerId player_id) {
	for (const auto& player_ship : map.ships) {
		for (const hlt::Ship& potential_ship : player_ship.second) {
			if (potential_ship.owner_id != player_id) {
				if (potential_ship.location.get_distance_to(planet.location) < planet.radius * DEFENSE_MULTIPLIER) {
					return true;
				}
			}
		}
	}

	return false;
}

bool EnemyIsolated(const hlt::Ship& enemy, const hlt::Map& map) {
	hlt::PlayerId enemy_id = enemy.owner_id;
	double isolated_radius = 40.0;

	for (const hlt::Ship& ship : map.ships.at(enemy_id)) {
		if (ship.entity_id != enemy.entity_id) {
			double dist = enemy.location.get_distance_to(ship.location);
			if (dist < isolated_radius) {
				return false;
			}
		}
	}
	return true;
}

bool EnoughShipsCloser(const hlt::Ship& ship, const hlt::Ship& enemy, const std::vector<hlt::Ship> my_ships) {
	int closer = 0;
	double range = ship.location.get_distance_to(enemy.location);
	for (const hlt::Ship& a : my_ships) {
		if (a.entity_id != ship.entity_id && a.docking_status == hlt::ShipDockingStatus::Undocked) {
			if (a.location.get_distance_to(enemy.location) < range) {
				closer++;
			}
		}
	}

	if (closer >= 2) {
		return true;
	}
	return false;
}

//
//Offensive functions
//

hlt::Ship FindClosestShip(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id, double radius) {
	//Returns the ship if there are no near enemies.
	hlt::Ship current_ship = ship;

	for (const auto& player_ship : map.ships) {
		for (const hlt::Ship& potential_ship : player_ship.second) {
			if (potential_ship.owner_id != player_id) {
				double dist = ship.location.get_distance_to(potential_ship.location);
				if (dist < radius) {
					current_ship = potential_ship;
					radius = dist;
				}
			}
		}
	}

	return current_ship;
}

hlt::Planet FindClosestPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets, hlt::PlayerId player_id) {
	//Returns the closest planet unless it's owned by me and full.
	hlt::Planet current_planet = planets[0];
	double shortest = NO_RADIUS;

	for (const hlt::Planet& planet : planets) {
		if ((!planet.is_full() && planet.owner_id == player_id) || planet.owner_id != player_id) {
			double dist = ship.location.get_distance_to(planet.location);
			if (dist < shortest) {
				current_planet = planet;
				shortest = dist;
			}
		}
	}

	return current_planet;
}

bool EnoughAlliesCloser(const hlt::Planet& planet, const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id) {
	//Returns true if there are at least the amount of docking spots left in numbers of allied ships closer to the target planet.
	double range = ship.location.get_distance_to(planet.location);
	unsigned int closer = 0;

	//If any ship is not the entered ship, docked, or farther than the entered ship, add one to closer.
	for (const hlt::Ship& s : map.ships.at(player_id)) {
		if (s.entity_id != ship.entity_id && s.docking_status == hlt::ShipDockingStatus::Undocked && s.location.get_distance_to(planet.location) < range) {
			closer++;
		}
	}

	//If close is greater than or equal to the amount of spots left, return true.
	if (closer >= planet.docking_spots - planet.docked_ships.size()) {
		return true;
	}
	else {
		return false;
	}
}

hlt::Planet FindNextPlanet(const hlt::Planet& planet, const hlt::Ship& ship, std::vector<hlt::Planet> planets, hlt::PlayerId player_id) {
	//Returns the second closest planet unless it's owned by me and full. Uses the current closest to sort.
	hlt::Planet current_planet = planets[0];
	double shortest = NO_RADIUS;

	for (const hlt::Planet& p : planets) {
		if ((!p.is_full() && p.owner_id == player_id) || p.owner_id != player_id) {
			if (p.entity_id != planet.entity_id) {
				double dist = ship.location.get_distance_to(p.location);
				if (dist < shortest) {
					current_planet = p;
					shortest = dist;
				}
			}
		}
	}

	return current_planet;
}

hlt::Ship NearestDockedShip(const hlt::Ship& ship, const hlt::Planet& planet, const hlt::Map& map) {
	hlt::Ship current_ship;
	double shortest = NO_RADIUS;

	for (const hlt::EntityId& e : planet.docked_ships) {
		hlt::Ship s = map.get_ship(planet.owner_id, e);
		double dist = ship.location.get_distance_to(s.location);
		if (dist < shortest) {
			current_ship = s;
			shortest = dist;
		}
	}

	return current_ship;
}

//Grouping functions.

bool Outnumbered(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id) {
	int allies = 0;
	int enemies = 0;
	for (const auto& player_ship : map.ships) {
		for (const hlt::Ship& s : player_ship.second) {
			if (s.owner_id == player_id && ship.location.get_distance_to(s.location) <= hlt::constants::WEAPON_RADIUS && s.docking_status == hlt::ShipDockingStatus::Undocked) {
				allies++;
			}
			else if (s.owner_id != player_id && ship.location.get_distance_to(s.location) <= hlt::constants::MAX_SPEED + hlt::constants::WEAPON_RADIUS && s.docking_status == hlt::ShipDockingStatus::Undocked) {
				enemies++;
			}
		}
	}
	if (enemies >= allies && enemies > 0) {
		return true;
	}
	else {
		return false;
	}
}

hlt::Location ClosestTarget(const hlt::Ship& ship, std::vector<hlt::Location> targets, std::vector<double> distances) {
	double shortest = NO_RADIUS;
	hlt::Location closest_target;

	for (unsigned int i = 0; i < targets.size(); i++) {
		double distance = ship.location.get_distance_to(targets[i]);
		if (distance < shortest && distances[i] != SET_TARGET_LAST) {
			shortest = distance;
			closest_target = targets[i];
		}
	}

    return SetTarget(ship, closest_target);
}

//
//Navigation Functions
//

hlt::Location SetTarget(const hlt::Ship& ship, const hlt::Location& target) {
	const double distance = ship.location.get_distance_to(target);
	if (distance > hlt::constants::MAX_SPEED) {
		//Ships only travel to the nearest degree.
		const int angle_deg = ship.location.orient_towards_in_deg(target);
        const double deg_in_rad = angle_deg * M_PI / 180.0;
		const double new_target_dx = cos(deg_in_rad) * hlt::constants::MAX_SPEED;
		const double new_target_dy = sin(deg_in_rad) * hlt::constants::MAX_SPEED;
		return { ship.location.pos_x + new_target_dx, ship.location.pos_y + new_target_dy };
	}
	else {
        //Needs an integer distance to travel.
        const double remainder = distance - (int)distance;
        int int_dist;
        if (remainder >= 0.5) {
            int_dist = (int)distance + 1;
        }
        else {
            int_dist = (int)distance;
        }
		//Ships only travel to the nearest degree.
		const int angle_deg = ship.location.orient_towards_in_deg(target);
        const double deg_in_rad = angle_deg * M_PI / 180.0;
        const double new_target_dx = cos(deg_in_rad) * int_dist;
        const double new_target_dy = sin(deg_in_rad) * int_dist;
        return { ship.location.pos_x + new_target_dx, ship.location.pos_y + new_target_dy };
	}
}

bool TargetOrPlanetInTheWay(const hlt::Location& location, const hlt::Location& target, const hlt::Map& map, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, std::vector<double> distances, unsigned int index) {
	for (const hlt::Planet& planet : map.planets) {
		if (hlt::navigation::check_planet_between(location, target, planet)) {
			return true;
		}
	}
	//for (unsigned int i = 0; i < index; i++) {
    for (unsigned int i = 0; i < targets.size(); i++) {
        if (i < index || (distances[i] == SET_TARGET_LAST && i != index)) {
            if (ships[i].location == targets[i]) {
                if (hlt::navigation::check_target_between(location, target, targets[i])) {
                    return true;
                }
            } else {
                hlt::Location distance_start = { location.pos_x - ships[i].location.pos_x, location.pos_y - ships[i].location.pos_y };
                hlt::Location distance_end = { target.pos_x - targets[i].pos_x, target.pos_y - targets[i].pos_y };
                hlt::Location origin = { 0.0, 0.0 };
                if (hlt::navigation::check_target_between(distance_start, distance_end, origin)) {
                    return true;
                }
            }
        }
	}
	return false;
}

bool CrissCross(const hlt::Location& location, const hlt::Location& target, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, unsigned int index) {
	/*
		location & target, ships[i].location & targets[i] need to not be crossing.
		p1			q1		p2					q2
		Thanks to www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/ for writing a C++ example.
		Note: This does not check for a special collinear case because TargetOrPlanetInTheWay will pick that off much better.
	*/
	for (unsigned int i = 0; i < index; i++) {
		// Find the four orientations needed for general and
		// special cases
		int o1 = orientation(location, target, ships[i].location);
		int o2 = orientation(location, target, targets[i]);
		int o3 = orientation(ships[i].location, targets[i], location);
		int o4 = orientation(ships[i].location, targets[i], target);

		// General case
		if (o1 != o2 && o3 != o4) {
			return true;
		}
	}
	return false;
}
//Helper function also written by geeksforgeeks.
int orientation(const hlt::Location& p, const hlt::Location& q, const hlt::Location& r) {
	// See www.geeksforgeeks.org/orientation-3-ordered-points/
	// for details of below formula.
	int val = (q.pos_y - p.pos_y) * (r.pos_x - q.pos_x) -
		(q.pos_x - p.pos_x) * (r.pos_y - q.pos_y);

	if (val == 0) return 0;  // colinear

	return (val > 0) ? 1 : 2; // clock or counterclock wise
}

bool OutOfBounds(const hlt::Location& target, const hlt::Map& map) {
	if (target.pos_x < 7.0 || target.pos_x > map.map_width - 7 || target.pos_y < 7.0 || target.pos_y > map.map_height - 7) {
		return true;
	}
	return false;
}