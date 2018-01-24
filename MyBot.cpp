#include "hlt/hlt.hpp"
#include "hlt/navigation.hpp"
#include <math.h>

#define NO_RADIUS 100000000
#define SET_TARGET_LAST 500.0
//#define DEFENSE_MULTIPLIER 3.0

/*
	Folder Name:	Last Version + Distraction Refactor
	Last Versions:	Last Version - Strategic Attacks Refactor (Rank 11)
					Last Version + Strategic Attacks Refactor + No Collision (Rank 12)
					Last Version + Ally/Enemy Bunny Ship Fixes + Collision Refactor (Rank 16)
					Rank 22 Bot + Distraction Prevention (Rank 21)
 */

/*
    Terms referenced:
        -Bunny ship: A ship that "hops away" from enemy ships, distracting enemies by being uncatchable yet likely the closest ship they're chasing.
		-Strafe: "Strafing" is used, in all seriousness, in reference to the Minecraft term. Basically, you attack while moving perpendicular to your target.
				 It's like "kiting" but staying just as close to your real target, the enemy docked ships.
		-Noob-rush: A reference to Rise of Nations, basically when a bot rushes your ships at the beginning of the game. A low-skill-highly-obnoxious strategy in RoN.
 */

/*
	Next to fix:
	1. Set target last issues (see below).
	2. Noob rush detection.
*/

/*
	Major issues:

	Note: Code needs major refactoring and the function groups need their own file.
	Idea: Don't! Takes too much time!

	Note: The current Outnumbered navigation scheme can leave ships behind due to being unable to find a safe location.
	Idea: Sort the Outnumbered ships by distance to target after they're assigned separate from the rest of the bubble sort.
	Note: This failed.
	Idea: Try some crazy idea at the last minute of submission.
	Note: Didn't come up with one. :(
 */

/*
	Minor issues:

	Idea: Functions looping through all ships in the game can be simplified by checking if it matches player_id before the second for loop.

	Note: Heading to an allied planet to dock there does not take into account if the planet will be full because of production before the ship gets there, only closer ships
		  that already exist.
	Idea: Nobody seems to have implemented this, why bother? It might be important to getting an early ship lead though, idk.
 */




//
//Commented functions have been phased out and not used in the current version, but were recent enough I might've wanted them again/didn't take the time to remove them.
//

//Special Case Functions
bool NoobRush(const hlt::Ship& ship, const hlt::Ship& enemy, const hlt::Planet& planet, const unsigned int& players);
hlt::Ship FarthestShip(const hlt::Map& map, hlt::PlayerId player_id);
bool InFormation(const std::vector<hlt::Ship>& ships);
hlt::Ship ShipClosestToPlanet(const std::vector<hlt::Ship>& ships, const std::vector<hlt::Planet>& planets);
bool GoAround(const hlt::Ship& unit_leader, const hlt::Location& ship_target, const std::vector<hlt::Planet>& planets);
//hlt::Ship ClosestUndockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId enemy_id);
hlt::Ship ClosestDockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
//hlt::Location Strafe(const hlt::Ship& ship, const hlt::Ship& target, const hlt::Ship& hunter, const hlt::Map& map);
//bool HunterDanger(const hlt::Ship& ship, const hlt::Location& target, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Location NearestCorner(const hlt::Ship& ship, const hlt::Map& map);
//Victory Shape Functions
bool IsInVector(const hlt::EntityId& ship_id, const std::vector<hlt::EntityId>& vector);


//Defensive Functions
hlt::Planet FindClosestAnyPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets);
//bool EnemyNearPlanet(const hlt::Planet& planet, const hlt::Map& map, hlt::PlayerId player_id);
//bool EnemyIsolated(const hlt::Ship& enemy, const hlt::Map& map);
//bool EnoughShipsCloser(const hlt::Ship& ship, const hlt::Ship& enemy, const std::vector<hlt::Ship> my_ships);
//Distraction Prevention
//double DistanceToAllyPlanet(const hlt::Ship enemy, const std::vector<hlt::Planet> planets, const hlt::PlayerId player_id);
//bool ClosestAllyToEnemy(const hlt::Ship ship, const std::vector<hlt::Ship> ships, const hlt::Ship enemy, const std::vector<hlt::EntityId> defending_allies);
hlt::Ship ClosestAlly(const hlt::Ship enemy, const std::vector<hlt::Ship> ships, const std::vector<hlt::EntityId> defending_allies, hlt::EntityId bunny_id);
hlt::Ship FindNearestNotAssignedShip(const hlt::Ship ship, const hlt::Map map, hlt::PlayerId player_id, const std::vector<hlt::Ship> attacking_enemies, const std::vector<hlt::Ship> neutral_enemies);
bool IsInShipVector(hlt::Ship ship, std::vector<hlt::Ship> vector);
hlt::Ship NearestAnyAlly(hlt::Ship ship, std::vector<hlt::Ship> ships);
hlt::Ship ClosestDockedAlly(hlt::Ship ship, std::vector<hlt::Ship> ships);

//Offensive Functions
hlt::Ship FindClosestShip(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id, double radius);
//Credit to skylarh for FindClosestPlanet (not neccessarily in its original form).
hlt::Planet FindClosestPlanet(const hlt::Ship& ship, std::vector<hlt::Planet> planets, hlt::PlayerId player_id);
bool EnoughAlliesCloser(const hlt::Planet& planet, const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Planet FindNextPlanet(const hlt::Planet& planet, const hlt::Ship& ship, std::vector<hlt::Planet> planets, hlt::PlayerId player_id);
hlt::Ship NearestDockedShip(const hlt::Ship& ship, const hlt::Planet& planet, const hlt::Map& map);
hlt::Planet NearestEnemyPlanet(const hlt::Ship& ship, const hlt::Planet& target_planet, const std::vector<hlt::Planet>& planets, hlt::PlayerId player_id);
//Grouping Functions
bool Outnumbered(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId player_id);
hlt::Location ClosestTarget(const hlt::Ship& ship, std::vector<hlt::Location> targets, std::vector<double> distances);

//Navigation Functions
hlt::Location SetTarget(const hlt::Ship& ship, const hlt::Location& target);
bool TargetOrPlanetInTheWay(const hlt::Location& location, const hlt::Location& target, const hlt::Map& map, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, std::vector<double> distances, unsigned int index, bool unit_leader);
//bool CrissCross(const hlt::Location& location, const hlt::Location& target, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, unsigned int index);
//int orientation(const hlt::Location& p, const hlt::Location& q, const hlt::Location& r);
bool OutOfBounds(const hlt::Location& target, const hlt::Map& map);


int main() {
	const hlt::Metadata metadata = hlt::initialize("JustinianAndTheodora");
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

	//Defense related vectors
	std::vector<hlt::Ship> attacking_enemies;
	std::vector<hlt::Ship> neutral_enemies;
	//Entity ID is stored for IsInVector.
	std::vector<hlt::EntityId> defending_allies;

	//Special case variables
	//Bunny_id was used when my defense against noob rushes was to assign one ship to be a bunny ship and the other two to go dock. It's not used for that anymore,
	//but it's passed into another function so I have to intialize it/bother to change the function.
	hlt::EntityId bunny_id = 999;
	int move_number = 0;
	double enemy_distance = 0;
	double previous_distance = 0;
	bool noob_rush = false;
	hlt::EntityId unit_leader_id = 999;
	bool defeated = false;
	//These next two variables make a nice "1" in the bottom-left corner of the map when I'm certainly going to win. Old versions of my code would glitch out at that point and
	//give me enough turns to make some art.
	std::vector<hlt::EntityId> victory_ships;
	hlt::Location victory_shape[7] = { { 9.0, initial_map.map_height - 6.0 },{ 12.0, initial_map.map_height - 6.0 }, { 15.0, initial_map.map_height - 6.0 },{ 12.0, initial_map.map_height - 9.0 },{ 12.0, initial_map.map_height - 12.0 },{ 12.0, initial_map.map_height - 15.0 },{ 9.0, initial_map.map_height - 12.0 } };
	bool winning = false;

    for (;;) {
        moves.clear();
        const hlt::Map map = hlt::in::get_map();
        
        
		//Reset targets and vectors.
		targets.clear();
		ships.clear();
		distances.clear();
		attacking_enemies.clear();
		neutral_enemies.clear();
		defending_allies.clear();
		victory_ships.clear();
        
        
		//
        //Check for special cases and give unique instructions.
		//

		//
		//Noob rush
		//Detects if there's a noob rush then handles it accordingly. Current strategy: group up and charge in.
		//
		if (move_number < 3 || noob_rush) {
			move_number++;
			hlt::Ship farthest_ship = FarthestShip(map, player_id);
			hlt::Ship nearest_enemy = FindClosestShip(farthest_ship, map, player_id, NO_RADIUS);

			if (map.ships.at(player_id).size() > map.ships.at(nearest_enemy.owner_id).size()) {
				noob_rush = false;
			}
			else {
				enemy_distance = farthest_ship.location.get_distance_to(nearest_enemy.location);
				double units_closer = previous_distance - enemy_distance;
				previous_distance = enemy_distance;
				std::ostringstream turn_information;
				turn_information << "units closer: " << units_closer;
				hlt::Log::log(turn_information.str());
				if ((units_closer > 0 && enemy_distance < 98) || enemy_distance < 70) {
					noob_rush = true;
					if (unit_leader_id == 999) {
						unit_leader_id = farthest_ship.entity_id;
					}
					hlt::Ship unit_leader;
					for (hlt::Ship ally : map.ships.at(player_id)) {
						if (ally.entity_id == unit_leader_id) {
							unit_leader = ally;
						}
					}
					if (InFormation(map.ships.at(player_id))) {
						unit_leader = ShipClosestToPlanet(map.ships.at(player_id), map.planets);
						unit_leader_id = unit_leader.entity_id;
					}
					std::ostringstream turn_information;
					turn_information << "unit leader: " << unit_leader_id;
					hlt::Log::log(turn_information.str());
					hlt::Ship nearest_docked_enemy = ClosestDockedEnemy(unit_leader, map, player_id);
					if (nearest_docked_enemy.owner_id == nearest_enemy.owner_id) {
						nearest_enemy = nearest_docked_enemy;
					}
					//																				was nearest_enemy.radius - 2
					hlt::Location ship_target = unit_leader.location.get_closest_point(nearest_enemy.location, 1.9);
					ship_target = SetTarget(unit_leader, ship_target);
					if (!InFormation(map.ships.at(player_id))) {
						ship_target = { unit_leader.location.pos_x + 5.0 / 7.0 * (ship_target.pos_x - unit_leader.location.pos_x), unit_leader.location.pos_y + 5.0 / 7.0 * (ship_target.pos_y - unit_leader.location.pos_y) };
					}
					bool go_around = GoAround(unit_leader, ship_target, map.planets);
					int max_corrections = 181;
					bool left = true;
					while ((TargetOrPlanetInTheWay(unit_leader.location, ship_target, map, ships, targets, distances, 0, go_around) || OutOfBounds(ship_target, map)) && max_corrections) {
						max_corrections--;
						const double distance = unit_leader.location.get_distance_to(ship_target);
						const double angle_rad = unit_leader.location.orient_towards_in_rad(ship_target);
						double new_target_dx;
						double new_target_dy;
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
						ship_target = { unit_leader.location.pos_x + new_target_dx, unit_leader.location.pos_y + new_target_dy };
					}
					targets.push_back(SetTarget(unit_leader, ship_target));
					ships.push_back(unit_leader);
					distances.push_back(0.0);

					for (hlt::Ship ally : map.ships.at(player_id)) {
						if (ally.entity_id != unit_leader_id) {
							if (InFormation(map.ships.at(player_id))) {
								hlt::Location difference = { unit_leader.location.pos_x - ally.location.pos_x, unit_leader.location.pos_y - ally.location.pos_y };
								hlt::Location ship_target = { targets[0].pos_x - difference.pos_x, targets[0].pos_y - difference.pos_y };
								targets.push_back(SetTarget(ally, ship_target));
								ships.push_back(ally);
								distances.push_back(ally.location.get_distance_to(ship_target));
							}
							else {
								hlt::Location ship_target = ally.location.get_closest_point(targets[0], -4);
								targets.push_back(SetTarget(ally, ship_target));
								ships.push_back(ally);
								distances.push_back(ally.location.get_distance_to(ship_target));
							}
						}
					}
				}
				else {
					noob_rush = false;
				}
			}
		}
		
		//
		//Imminent defeat/victory.
		//Sends my ships to the nearest corner if I'm certain to lose/starts making art if I'm certain to win.
		//
		if (map.ships.size() == 4) {
			int owned_by_me = 0;
			int owned_by_enemy = 0;
			int planets = 0;
			for (const hlt::Planet& p : map.planets) {
				planets++;
				if (p.owned) {
					if (p.owner_id == player_id) {
						owned_by_me++;
					}
					else {
						owned_by_enemy++;
					}
				}
			}
			//Defeat
			if ((owned_by_me <= 3 && owned_by_enemy >= (planets * 3.0) / 4.0) || (owned_by_me <= 3 && owned_by_enemy >= 3.0 * (owned_by_me + 2.0)) || defeated) {
				defeated = true;
				for (const hlt::Ship& ship : map.ships.at(player_id)) {
					if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
						targets.push_back(ship.location);
						ships.push_back(ship);
						distances.push_back(0.0);
					}
					else {
						hlt::Location ship_target = NearestCorner(ship, map);
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));
					}
				}
			}
			//Victory
			if (owned_by_enemy == 0 && owned_by_me >= planets / 2.0) {
				int index = 0;
				for (const hlt::Ship& ship : map.ships.at(player_id)) {
					if (ship.docking_status == hlt::ShipDockingStatus::Undocked && index < sizeof(victory_ships)) {
						hlt::Location ship_target = victory_shape[index];
						index++;
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));
						victory_ships.push_back(ship.entity_id);
					}
				}
			}
		}

		//
		//Attacking enemies.
		//Finds ships closest to my planets/neutral planets and assigns a ship to each one. This uses the offensively-ineffective Outnumbered() function to ensure that the
		//ships never really attack their targets, and get cover if they're under attack.
		//
		for (const auto& player_ship : map.ships) {
			//Find attacking and neutral enemies.
			for (const hlt::Ship& s : player_ship.second) {
				if (s.owner_id != player_id && !noob_rush && !defeated) {
					hlt::Planet closest_planet = FindClosestAnyPlanet(s, map.planets);
					if (closest_planet.owner_id == player_id && s.location.get_distance_to(closest_planet.location) < 50) {
						attacking_enemies.push_back(s);
					}
					else if (!closest_planet.owned && s.location.get_distance_to(closest_planet.location) < 50) {
						neutral_enemies.push_back(s);
					}
				}
			}
		}
		std::ostringstream turn_information;
		for (hlt::Ship s : attacking_enemies) {
			turn_information << "attacking ship id: " << s.entity_id << " ";
		}
		hlt::Log::log(turn_information.str());
		for (hlt::Ship s : neutral_enemies) {
			turn_information << "neutral ship id: " << s.entity_id << " ";
		}
		hlt::Log::log(turn_information.str());
		//Assign an ally to the attacking and neutral ships and set its target.
		for (const hlt::Ship& enemy : attacking_enemies) {
			const hlt::Ship closest_ally = ClosestAlly(enemy, map.ships.at(player_id), defending_allies, bunny_id);
			if (closest_ally.entity_id != enemy.entity_id) {
				if (Outnumbered(closest_ally, map, player_id)) {
					hlt::Ship nearest_ally = NearestAnyAlly(closest_ally, map.ships.at(player_id));
					if (nearest_ally.docking_status != hlt::ShipDockingStatus::Undocked) {
						hlt::Ship closest_docked_ally_for_enemy = ClosestDockedAlly(enemy, map.ships.at(player_id));
						hlt::Location ship_target = closest_docked_ally_for_enemy.location;
						targets.push_back(SetTarget(closest_ally, ship_target));
						ships.push_back(closest_ally);
						distances.push_back(closest_ally.location.get_distance_to(ship_target));
					}
					else {
						targets.push_back({ 0.0, 0.0 });
						ships.push_back(closest_ally);
						distances.push_back(SET_TARGET_LAST);
					}
				}
				else {
					hlt::Location ship_target = closest_ally.location.get_closest_point(enemy.location, enemy.radius - 2);
					targets.push_back(SetTarget(closest_ally, ship_target));
					ships.push_back(closest_ally);
					distances.push_back(closest_ally.location.get_distance_to(ship_target));
				}
				defending_allies.push_back(closest_ally.entity_id);
			}
		}
		for (const hlt::Ship& enemy : neutral_enemies) {
			const hlt::Ship closest_ally = ClosestAlly(enemy, map.ships.at(player_id), defending_allies, bunny_id);
			if (closest_ally.entity_id != enemy.entity_id) {
				if (Outnumbered(closest_ally, map, player_id)) {
					targets.push_back({ 0.0, 0.0 });
					ships.push_back(closest_ally);
					distances.push_back(SET_TARGET_LAST);
				}
				else {
					hlt::Location ship_target = closest_ally.location.get_closest_point(enemy.location, enemy.radius - 2);
					targets.push_back(SetTarget(closest_ally, ship_target));
					ships.push_back(closest_ally);
					distances.push_back(closest_ally.location.get_distance_to(ship_target));
				}
				defending_allies.push_back(closest_ally.entity_id);
			}
		}

		//
		//Ship advantage - use it!
		//If I have 5 more ships than the everyone, I don't care about ensuring each ship only goes in when it outnumbers the enemy - just attack!
		//
		int my_ships = 0;
		int their_ships = 0;
		for (const auto player_ship : map.ships) {
			if (player_ship.first == player_id) {
				my_ships = player_ship.second.size();
			}
			else {
				if (player_ship.second.size() > their_ships) {
					their_ships = player_ship.second.size();
				}
			}
		}
		if (my_ships >= 50 && their_ships + 5 <= my_ships) {
			//This bool makes my attacking ships not care about numbers - I have more! Time to blitz!
			winning = true;
		}

		//
		//Outnumbered Refactor
		//This code sorts ships by their distance to their closest enemy, has ship, by distance and if not already checked, identify any clusters around it, determine if the
		//allied cluster is outnumbered, then sets each ship in that identified cluster to know it's been checked so that the loop really only does a small fraction of the
		//ships. Works pretty well, written pretty poorly and quickly!
		//
		//No detailed comments, just written all at once. Not my problem! Written 2 hours before Halite ended, so the key-phrase "EXPERIMENTAL, MOSTLY MENTAL" is commented
		//throughout the code I used it in so I could efficiently remove this if it failed. It worked nicely for being a first implementation in my final bot!
		//EXPERIMENTAL, MOSTLY MENTAL.
		std::vector<double> enemy_distances;
		std::vector<hlt::Ship> each_ship;
		std::vector<bool> outnumbered;
		for (hlt::Ship s : map.ships.at(player_id)) {
			hlt::Ship nearest_enemy = FindClosestShip(s, map, player_id, NO_RADIUS);
			enemy_distances.push_back(s.location.get_distance_to(nearest_enemy.location));
			each_ship.push_back(s);
			outnumbered.push_back(true);
		}
		for (unsigned int i = 0; i < enemy_distances.size() - 1; i++) {
			for (unsigned int j = 0; j < enemy_distances.size() - i - 1; j++) {
				if (enemy_distances[j] > enemy_distances[j + 1]) {
					hlt::Ship ship_holder = each_ship[j];
					double distance_holder = enemy_distances[j];
					each_ship[j] = each_ship[j + 1];
					enemy_distances[j] = enemy_distances[j + 1];
					each_ship[j + 1] = ship_holder;
					enemy_distances[j + 1] = distance_holder;
				}
			}
		}
		std::vector<hlt::Ship> checked;
		for (unsigned int i = 0; i < each_ship.size(); i++) {
			if (!IsInShipVector(each_ship[i], checked)) {
				std::vector<double> ally_distances;
				std::vector<hlt::Ship> allies;
				for (unsigned int j = 0; j < each_ship.size(); j++) {
					ally_distances.push_back(each_ship[i].location.get_distance_to(each_ship[j].location));
					allies.push_back(each_ship[j]);
				}
				for (unsigned int k = 0; k < ally_distances.size() - 1; k++) {
					for (unsigned int l = 0; l < ally_distances.size() - k - 1; l++) {
						if (ally_distances[l] > ally_distances[l + 1]) {
							hlt::Ship ship_holder = allies[l];
							double distance_holder = ally_distances[l];
							allies[l] = allies[l + 1];
							ally_distances[l] = ally_distances[l + 1];
							allies[l + 1] = ship_holder;
							ally_distances[l + 1] = distance_holder;
						}
					}
				}
				unsigned int cluster_number = 0;
				bool end_early = false;
				for (unsigned int k = 0; k < ally_distances.size() - 1; k++) {
					if (!end_early) {
						cluster_number++;
					}
					if (ally_distances[k + 1] > 3 + ally_distances[k]) {
						end_early = true;
					}
				}
				std::ostringstream info;
				info << cluster_number;
				hlt::Log::log(info.str());
				for (unsigned int y = 0; y < cluster_number; y++) {
					checked.push_back(allies[y]);
				}
				unsigned int enemies = 0;
				for (const auto& player_ship : map.ships) {
					for (const hlt::Ship& s : player_ship.second) {
						if (s.owner_id != player_id && each_ship[i].location.get_distance_to(s.location) <= (hlt::constants::MAX_SPEED * 2.0) + hlt::constants::WEAPON_RADIUS && s.docking_status == hlt::ShipDockingStatus::Undocked) {
							enemies++;
						}
					}
				}
				if (cluster_number > enemies) {
					for (unsigned int x = 0; x < cluster_number; x++) {
						for (unsigned int z = 0; z < allies.size(); z++) {
							if (each_ship[z].entity_id == allies[x].entity_id) {
								outnumbered[z] = false;
							}
						}
					}
				}
			}
		}



		//
		//Standard logic for each ship not used in a special case (except the Outnumbered Refactor, that just sets a boolean for the ship).
		//

		//Set targets.
        for (const hlt::Ship& ship : map.ships.at(player_id)) {
			//Check if the ship was already given a target in a special case.
			if (noob_rush || ship.entity_id == bunny_id || defeated || IsInVector(ship.entity_id, victory_ships) || IsInVector(ship.entity_id, defending_allies)) {
				continue;
			}

			//Stay docked if you are.
            if (ship.docking_status != hlt::ShipDockingStatus::Undocked) {
				targets.push_back(ship.location);
				ships.push_back(ship);
				distances.push_back(0.0);
                continue;
            }
            

			/*
			Defense strategy:
				1. Is there a near enemy?
				2. Defend against it with the previous assignments in attacking_enemies.
				(There used to be logic here that was since removed by the "Attacking Ships" special case.)
			*/

			/*
			The current offensive strategy is to locate the closest planet that is either not full or not owned by me, and run a unique strategy for
			each three cases of ownership:

			1. Not owned.
				- If there are nearby enemies (scaling with planet radius), target the nearest docked enemy.
				- Otherwise, try to dock.
				- Otherwise, if you have no enemies in a really large radius and there are more than enough ships heading to the planet, find a new planet to target.
				- Otherwise, continue moving towards the planet.
			2. Owned by me but it needs more docked ships.
				- If there are enough allies closer, go to the next planet.
				- Otherwise dock or move to it.
			3. Owned by someone else.
				- Attack the closest docked ship.
			*/

			//Find the closest planet that is not fully docked by me.
			const hlt::Planet& target_planet = FindClosestPlanet(ship, map.planets, player_id);

			//EXPERIMENTAL, MOSTLY MENTAL.
			bool get_help = false;
			for (unsigned int i = 0; i < each_ship.size(); i++) {
				if (each_ship[i].entity_id == ship.entity_id) {
					if (outnumbered[i]) {
						get_help = true;
					}
				}
			}

			//If nobody owns the planet,
            if (!target_planet.owned) {
				//find the nearest enemy,
				//hlt::Ship nearest_enemy = FindClosestShip(ship, map, player_id, NO_RADIUS);
				hlt::Ship nearest_enemy = FindNearestNotAssignedShip(ship, map, player_id, attacking_enemies, neutral_enemies);
				hlt::Ship actual_nearest = FindClosestShip(ship, map, player_id, NO_RADIUS);
				//if there are no nearby enemies within a certain range,
				//EXPERIMENTAL, MOSTLY MENTAL.
				//if (ship.location.get_distance_to(nearest_enemy.location) > (19.1) || ship.location.get_distance_to(actual_nearest.location) > (target_planet.radius * 3.0 + 15)) {
				if (ship.location.get_distance_to(nearest_enemy.location) > (target_planet.radius * 3.0 + 15) || ship.location.get_distance_to(actual_nearest.location) > (19.1)) {
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
						if ((target_planet.location.get_distance_to(nearest_enemy.location) < 50.0 && ship.entity_id != nearest_enemy.entity_id) || !EnoughAlliesCloser(target_planet, ship, map, player_id)) {
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
					//Outnumbered() is an inefficient means of detecting a number advantage, mostly phased-out.
					//if (Outnumbered(ship, map, player_id)) {
					if (get_help) {
						targets.push_back({ 0, 0 });
						ships.push_back(ship);
						distances.push_back(SET_TARGET_LAST);
					}
					else {
						hlt::Planet nearest_enemy_planet = NearestEnemyPlanet(ship, target_planet, map.planets, player_id);
						hlt::Ship nearest_docked_ship = NearestDockedShip(ship, nearest_enemy_planet, map);
						hlt::Location ship_target = ship.location.get_closest_point(nearest_docked_ship.location, nearest_docked_ship.radius - 2);
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));
						
						//Other logic that attacks the nearest ship. That's no fun, though, because I want to rush the nearest docked ship.
						/*hlt::Location ship_target = ship.location.get_closest_point(nearest_enemy.location, nearest_enemy.radius - 2);
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));*/
					}
				}
			}
			
			//or I need more ships docked to mine,
			else if (target_planet.owner_id == player_id) {
				if (!EnoughAlliesCloser(target_planet, ship, map, player_id)) {
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
				else {
					//Outnumbered() is an inefficient means of detecting a number advantage, mostly phased-out.
					//if (Outnumbered(ship, map, player_id)) {
					if (get_help) {
						targets.push_back({ 0, 0 });
						ships.push_back(ship);
						distances.push_back(SET_TARGET_LAST);
					}
					else {
						const hlt::Planet& new_planet = FindNextPlanet(target_planet, ship, map.planets, player_id);
						hlt::Location ship_target = ship.location.get_closest_point(new_planet.location, new_planet.radius);
						targets.push_back(SetTarget(ship, ship_target));
						ships.push_back(ship);
						distances.push_back(ship.location.get_distance_to(ship_target));
					}
				}
			}
			//otherwise it's not mine,
			else {
				//so find the closest ship docked to it,
				hlt::Ship nearest_docked_ship = NearestDockedShip(ship, target_planet, map);
				//Outnumbered() is an inefficient means of detecting a number advantage, mostly phased-out.
				//if (Outnumbered(ship, map, player_id) && !winning) {
				//Winning, from the special case "Ship Advantage," makes the ships ignore how outnumbered they are if I have more total ships.
				if (get_help && !winning) {
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


		//Set Outnumbered targets and prioritize them between stationary and the rest of the ships.
		for (unsigned int i = 0; i < distances.size(); i++) {
			if (distances[i] == SET_TARGET_LAST) {
				//Go to the target of the closest ship that isn't outnumbered/is docked.
				targets[i] = ClosestTarget(ships[i], targets, distances);
				//A small number that sorts these distances between stationary and other moving ships.
				distances[i] = 0.01;
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
				//Sorts the Outnumbered ships by farthest-first. The idea is that they become more likely to be able to move away from the danger they're closer to.
				//Doesn't really work.
				else if (distances[j] == 0.01 && distances[j + 1] == 0.01) {
					if (ships[j].location.get_distance_to(targets[j]) < ships[j + 1].location.get_distance_to(targets[j + 1])) {
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
		}


		//Manage any collision between ally ships.
		bool correction;
		do {
			correction = false;
			for (unsigned int i = 0; i < targets.size(); i++) {
				//If the ship is already docked/has been commanded to dock, skip this loop.
				if (targets[i] == ships[i].location) {
					continue;
				}

				//Move targets that will hit planets or other targets.
				int max_corrections;
				//Special 360 degree navigation check for bunny ships, including some altered collision checks for docked enemies/nearby enemies.
				//I don't have bunny ships in this version.
				if (ships[i].entity_id == bunny_id) {
					/* This is phased out, as this version does not set a bunny id.
					max_corrections = 361;
					bool left = true;
					//The loop now only checks ally targets with a distance closer than the ship target.
					//while ((HunterDanger(ships[i], targets[i], map, player_id) || TargetOrPlanetInTheWay(ships[i].location, targets[i], map, ships, targets, distances, i) || CrissCross(ships[i].location, targets[i], ships, targets, i) || OutOfBounds(targets[i], map)) && max_corrections) {
					while ((HunterDanger(ships[i], targets[i], map, player_id) || TargetOrPlanetInTheWay(ships[i].location, targets[i], map, ships, targets, distances, i, false) || OutOfBounds(targets[i], map)) && max_corrections) {
						correction = true;
						max_corrections--;
						const double distance = ships[i].location.get_distance_to(targets[i]);
						const double angle_rad = ships[i].location.orient_towards_in_rad(targets[i]);
						double new_target_dx;
						double new_target_dy;
						//Alternate checking 1 degree left/right.
						if (left) {
							new_target_dx = cos(angle_rad + (M_PI / 180.0)*(361.0 - max_corrections)) * distance;
							new_target_dy = sin(angle_rad + (M_PI / 180.0)*(361.0 - max_corrections)) * distance;
							left = false;
						}
						else {
							new_target_dx = cos(angle_rad - (M_PI / 180.0)*(361.0 - max_corrections)) * distance;
							new_target_dy = sin(angle_rad - (M_PI / 180.0)*(361.0 - max_corrections)) * distance;
							left = true;
						}
						targets[i] = { ships[i].location.pos_x + new_target_dx, ships[i].location.pos_y + new_target_dy };
					}*/
				}
				//Regular ship collision check.
				else {
					//The corrections go 90 degrees either way, and 1 is added because of the initial decrement.
					max_corrections = 181;
					bool left = true;
					//The loop now only checks ally targets with a distance closer than the ship target.
					//while ((TargetOrPlanetInTheWay(ships[i].location, targets[i], map, ships, targets, distances, i) || CrissCross(ships[i].location, targets[i], ships, targets, i) || OutOfBounds(targets[i], map)) && max_corrections) {
					while ((TargetOrPlanetInTheWay(ships[i].location, targets[i], map, ships, targets, distances, i, false) || OutOfBounds(targets[i], map)) && max_corrections) {
						correction = true;
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
				}
				//If a safe target could not be found, don't move, resort the vectors.
				if (!max_corrections) {
					targets[i] = ships[i].location;
					targets.insert(targets.begin(), targets[i]);
					ships.insert(ships.begin(), ships[i]);
					distances.insert(distances.begin(), 0.0);
					targets.erase(targets.begin() + i + 1);
					ships.erase(ships.begin() + i + 1);
					distances.erase(distances.begin() + i + 1);
					continue;
				}
			}
		} while (correction);

		//Move each target/log info.
		for (unsigned int i = 0; i < targets.size(); i++) {
			std::ostringstream turn_information;
			turn_information << "ship id: " << ships[i].entity_id << " target x: " << targets[i].pos_x << " target y: " << targets[i].pos_y << " distance: " << distances[i];
			hlt::Log::log(turn_information.str());

			if (ships[i].location == targets[i]) {
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
//Special Case functions
//

bool NoobRush(const hlt::Ship& ship, const hlt::Ship& enemy, const hlt::Planet& planet, const unsigned int& players) {
	if (ship.entity_id == enemy.entity_id) {
		return false;
	}

	hlt::Location docking_spot = ship.location.get_closest_point(planet.location, planet.radius);

	if (players == 2) {
		if ((int)(enemy.location.get_distance_to(docking_spot) / 7) - (int)(ship.location.get_distance_to(docking_spot) / 7) <= 12) {
			return true;
		}
	}
	else {
		if ((int)(enemy.location.get_distance_to(docking_spot) / 7) - (int)(ship.location.get_distance_to(docking_spot) / 7) <= 9) {
			return true;
		}
	}
	return false;
}

hlt::Ship FarthestShip(const hlt::Map& map, hlt::PlayerId player_id) {
	hlt::Ship farthest_ally = map.ships.at(player_id)[0];
	hlt::Ship closest_enemy;
	double range = NO_RADIUS;

	for (const auto& player_ship : map.ships) {
		for (const hlt::Ship& potential_ship : player_ship.second) {
			if (potential_ship.owner_id != player_id) {
				double dist = potential_ship.location.get_distance_to(farthest_ally.location);
				if (dist < range) {
					range = dist;
					closest_enemy = potential_ship;
				}
			}
		}
	}

	for (const hlt::Ship& ally : map.ships.at(player_id)) {
		//												for a ghost ship that appears when you're at 1 ship
		if (ally.docking_status == hlt::ShipDockingStatus::Undocked && ally.health != 15) {
			double dist = ally.location.get_distance_to(closest_enemy.location);
			if (dist > range) {
				range = dist;
				farthest_ally = ally;
			}
		}
	}

	return farthest_ally;
}

bool InFormation(const std::vector<hlt::Ship>& ships) {
	for (hlt::Ship ship : ships) {
		double range = NO_RADIUS;
		for (hlt::Ship ally : ships) {
			if (ally.entity_id != ship.entity_id) {
				double dist = ally.location.get_distance_to(ship.location);
				if (dist < range) {
					range = dist;
				}
			}
		}

		if (range > 1.5) {
			return false;
		}
	}

	return true;
}

hlt::Ship ShipClosestToPlanet(const std::vector<hlt::Ship>& ships, const std::vector<hlt::Planet>& planets) {
	double range = NO_RADIUS;
	hlt::Ship current_ship;

	for (hlt::Ship ally : ships) {
		for (hlt::Planet p : planets) {
			double dist = ally.location.get_distance_to(p.location);
			if (dist < range) {
				range = dist;
				current_ship = ally;
			}
		}
	}

	return current_ship;
}

bool GoAround(const hlt::Ship& unit_leader, const hlt::Location& ship_target, const std::vector<hlt::Planet>& planets) {
	for (hlt::Planet p : planets) {
		if (hlt::navigation::check_planet_between(unit_leader.location, ship_target, p, false)) {
			return true;
		}
	}
	
	return false;
}

//Phased-out function that was used in some logic for noob rushes.
/*hlt::Ship ClosestUndockedEnemy(const hlt::Ship& ship, const hlt::Map& map, hlt::PlayerId enemy_id) {
	//Returns the ship if there are no near enemies.
	hlt::Ship current_ship = ship;
	double radius = NO_RADIUS;

	for (const hlt::Ship& potential_ship : map.ships.at(enemy_id)) {
		if (potential_ship.docking_status == hlt::ShipDockingStatus::Undocked) {
			double dist = ship.location.get_distance_to(potential_ship.location);
			if (dist < radius) {
				current_ship = potential_ship;
				radius = dist;
			}
		}
	}

	return current_ship;
}*/

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

//Phased-out function that helped my bunny ship get around a defender and closer to enemy docked ships.
/*hlt::Location Strafe(const hlt::Ship& ship, const hlt::Ship& target, const hlt::Ship& hunter, const hlt::Map& map) {
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
}*/

//Phased-out function that most-nearly kept the bunny ship out of harm's way.
/*bool HunterDanger(const hlt::Ship& ship, const hlt::Location& target, const hlt::Map& map, hlt::PlayerId player_id) {
	for (const auto& player_ship : map.ships) {
		if (player_ship.first != player_id) {
			for (const hlt::Ship& s : player_ship.second) {
				if (s.docking_status != hlt::ShipDockingStatus::Undocked) {
					if (hlt::navigation::check_target_between(ship.location, target, s.location)) {
						return true;
					}
				}
				else {
					if (target.get_distance_to(s.location) <= 12.1) {
						return true;
					}
				}
			}
		}
	}
	return false;
}*/

hlt::Location NearestCorner(const hlt::Ship& ship, const hlt::Map& map) {
	if (ship.location.pos_x <= map.map_width / 2.0 && ship.location.pos_y <= map.map_height / 2.0) {
		return { 0.0, 0.0 };
	}
	if (ship.location.pos_x > map.map_width / 2.0 && ship.location.pos_y <= map.map_height / 2.0) {
		return { (double)map.map_width, 0.0 };
	}
	if (ship.location.pos_x <= map.map_width / 2.0 && ship.location.pos_y > map.map_height / 2.0) {
		return { 0.0, (double)map.map_height };
	}
	return { (double)map.map_width, (double)map.map_height };
}

//Victory Shape functions

bool IsInVector(const hlt::EntityId& ship_id, const std::vector<hlt::EntityId>& vector) {
	for (hlt::EntityId id : vector) {
		if (ship_id == id) {
			return true;
		}
	}
	return false;
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

//Phased-out function used to find and assign a ship to an isolated enemy to prevent a bunny ship from distracting a swarm of allies.
/*bool EnemyNearPlanet(const hlt::Planet& planet, const hlt::Map& map, hlt::PlayerId player_id) {
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
}*/

//Phased-out function used to find and assign a ship to an isolated enemy to prevent a bunny ship from distracting a swarm of allies.
/*bool EnemyIsolated(const hlt::Ship& enemy, const hlt::Map& map) {
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
}*/

//Phased-out function used as a quick toggle for how many ships I send to an attacking, isolated enemy (I think).
/*bool EnoughShipsCloser(const hlt::Ship& ship, const hlt::Ship& enemy, const std::vector<hlt::Ship> my_ships) {
	int closer = 0;
	double range = ship.location.get_distance_to(enemy.location);
	for (const hlt::Ship& a : my_ships) {
		if (a.entity_id != ship.entity_id && a.docking_status == hlt::ShipDockingStatus::Undocked) {
			if (a.location.get_distance_to(enemy.location) < range) {
				closer++;
			}
		}
	}

	if (closer >= 1) {
		return true;
	}
	return false;
}*/

//Distraction Prevention Functions

//Phased-out junk-code that returned the distance to the nearest planet if it was an allied planet, or 999 otherwise.
/*double DistanceToAllyPlanet(const hlt::Ship enemy, const std::vector<hlt::Planet> planets, const hlt::PlayerId player_id) {
	//Returns the distance to the closest allied planet, or 999 if 
	double range = 999;
	hlt::Planet closest_planet = planets[0];

	for (const hlt::Planet p : planets) {
		double dist = enemy.location.get_distance_to(p.location);
		if (dist < range) {
			closest_planet = p;
			range = dist;
		}
	}

	if (closest_planet.owner_id == player_id) {
		return range;
	}
	return 999;
}*/

//Phased-out function used to see if a ship is the closest to an enemy, aside from ships assigned to defend.
/*bool ClosestAllyToEnemy(const hlt::Ship ship, const std::vector<hlt::Ship> ships, const hlt::Ship enemy, const std::vector<hlt::EntityId> defending_allies) {
	double dist = ship.location.get_distance_to(enemy.location);

	for (const hlt::Ship s : ships) {
		if (s.entity_id != ship.entity_id && s.docking_status == hlt::ShipDockingStatus::Undocked && !IsInVector(s.entity_id, defending_allies)) {
			if (s.location.get_distance_to(enemy.location) < dist) {
				return false;
			}
		}
	}

	return true;
}*/

hlt::Ship ClosestAlly(const hlt::Ship enemy, const std::vector<hlt::Ship> ships, const std::vector<hlt::EntityId> defending_allies, hlt::EntityId bunny_id) {
	//Find the closest ally within range or returns the enemy ship if there isn't one.
	double range = 50;
	hlt::Ship closest_ally = enemy;

	for (const hlt::Ship s : ships) {
		if (s.docking_status == hlt::ShipDockingStatus::Undocked && !IsInVector(s.entity_id, defending_allies) && s.entity_id != bunny_id) {
			double dist = enemy.location.get_distance_to(s.location);
			if (enemy.location.get_distance_to(s.location) < range) {
				range = dist;
				closest_ally = s;
			}
		}
	}

	return closest_ally;
}

hlt::Ship FindNearestNotAssignedShip(const hlt::Ship ship, const hlt::Map map, hlt::PlayerId player_id, const std::vector<hlt::Ship> attacking_enemies, const std::vector<hlt::Ship> neutral_enemies) {
	//Find the closest not assigned enemy ship, or return the current ship if there isn't one.
	double range = NO_RADIUS;
	hlt::Ship current_ship = ship;

	for (const auto& player_ship : map.ships) {
		for (const hlt::Ship& s : player_ship.second) {
			if (s.owner_id != player_id && !IsInShipVector(s, attacking_enemies) && !IsInShipVector(s, neutral_enemies)) {
				double dist = ship.location.get_distance_to(s.location);
				if (dist < range) {
					current_ship = s;
					range = dist;
				}
			}
		}
	}

	return current_ship;
}

bool IsInShipVector(hlt::Ship ship, std::vector<hlt::Ship> vector) {
	for (hlt::Ship s : vector) {
		if (ship.entity_id == s.entity_id) {
			return true;
		}
	}
	return false;
}

hlt::Ship NearestAnyAlly(hlt::Ship ship, std::vector<hlt::Ship> ships) {
	double range = NO_RADIUS;
	hlt::Ship current_ship = ship;

	for (hlt::Ship ally : ships) {
		if (ship.entity_id != ally.entity_id) {
			double dist = ship.location.get_distance_to(ally.location);
			if (dist < range) {
				range = dist;
				current_ship = ally;
			}
		}
	}

	return current_ship;
}

hlt::Ship ClosestDockedAlly(hlt::Ship ship, std::vector<hlt::Ship> ships) {
	double range = NO_RADIUS;
	hlt::Ship current_ship = ship;

	for (hlt::Ship ally : ships) {
		if (ship.entity_id != ally.entity_id && ally.docking_status != hlt::ShipDockingStatus::Undocked) {
			double dist = ship.location.get_distance_to(ally.location);
			if (dist < range) {
				range = dist;
				current_ship = ally;
			}
		}
	}

	return current_ship;
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
	hlt::Planet current_planet = planet;
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
	//hlt::Ship current_ship;
	hlt::Ship current_ship = ship;
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

hlt::Planet NearestEnemyPlanet(const hlt::Ship& ship, const hlt::Planet& target_planet, const std::vector<hlt::Planet>& planets, hlt::PlayerId player_id) {
	hlt::Planet current_planet = target_planet;
	double shortest = NO_RADIUS;

	for (const hlt::Planet& p : planets) {
		if (p.owned && p.owner_id != player_id) {
			double dist = ship.location.get_distance_to(p.location);
			if (dist < shortest) {
				current_planet = p;
				shortest = dist;
			}
		}
	}

	return current_planet;
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
			//else if (s.owner_id != player_id && ship.location.get_distance_to(s.location) <= hlt::constants::MAX_SPEED + hlt::constants::WEAPON_RADIUS && s.docking_status == hlt::ShipDockingStatus::Undocked) {
			else if (s.owner_id != player_id && ship.location.get_distance_to(s.location) <= (hlt::constants::MAX_SPEED * 2.0) + hlt::constants::WEAPON_RADIUS && s.docking_status == hlt::ShipDockingStatus::Undocked) {
				enemies++;
			}
		}
	}
	hlt::Ship nearest_enemy = FindClosestShip(ship, map, player_id, NO_RADIUS);
	if (map.ships.at(player_id).size() > map.ships.at(nearest_enemy.owner_id).size() + 1) {
		if (enemies + 1 > allies) {
			return true;
		}
	}
	else {
		if (enemies >= allies && enemies > 0) {
			return true;
		}
	}
	return false;
}

hlt::Location ClosestTarget(const hlt::Ship& ship, std::vector<hlt::Location> targets, std::vector<double> distances) {
	double shortest = NO_RADIUS;
	hlt::Location closest_target;

	for (unsigned int i = 0; i < targets.size(); i++) {
		double distance = ship.location.get_distance_to(targets[i]);
		if (distance < shortest && distances[i] != SET_TARGET_LAST && distances[i] != 0.01) {
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

bool TargetOrPlanetInTheWay(const hlt::Location& location, const hlt::Location& target, const hlt::Map& map, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, std::vector<double> distances, unsigned int index, bool unit_leader) {
	for (const hlt::Planet& planet : map.planets) {
		if (unit_leader) {
			if (hlt::navigation::check_planet_between(location, target, planet, true)) {
				return true;
			}
		}
		else {
			if (hlt::navigation::check_planet_between(location, target, planet, false)) {
				return true;
			}
		}
	}
	//for (unsigned int i = 0; i < index; i++) {
	/*
		Reasoning for the next two lines: This is the last collision bug. On rare occasion, given a < b, ships[a] will be Outnumbered and target ships[b], and ships[b] will
		be Outnumbered and start another ship that happens to point very very close (or exactly) at ships[b].location. ships[a] doesn't check for it since the index is higher,
		and ships[b] doesn't move out of the way since its target is its location. And so, on the second line from this, is "|| ships[i].location == targets[i]".
		As frustrated as I sound, rest easy in knowing that in the process of debugging this, I also discovered a situation that let an Outnumbered ship target another
		Outnumbered ship's target, and fixed it.
	*/
    for (unsigned int i = 0; i < targets.size(); i++) {
        if (i < index || ships[i].location == targets[i]) {
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

//Phased-out function and helper-function that prevented ship paths from criss-crossing that reduced ship collisions before I elminated them entirely. It resticted a bit of
//movement for ships to not be able to criss-cross so eventually it had to go.
/*bool CrissCross(const hlt::Location& location, const hlt::Location& target, std::vector<hlt::Ship> ships, std::vector<hlt::Location> targets, unsigned int index) {
	/*
		location & target, ships[i].location & targets[i] need to not be crossing.
		p1			q1		p2					q2
		Thanks to www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/ for writing a C++ example.
		Note: This does not check for a special collinear case because TargetOrPlanetInTheWay will pick that off much better.
	*//*
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
}*/

bool OutOfBounds(const hlt::Location& target, const hlt::Map& map) {
	if (target.pos_x < 3.0 || target.pos_x > map.map_width - 3 || target.pos_y < 3.0 || target.pos_y > map.map_height - 3) {
		return true;
	}
	return false;
}
