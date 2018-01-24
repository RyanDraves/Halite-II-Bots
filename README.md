# Halite-II-Bots
###### This repository contains my bots for the 2017-2018 Halite II AI Challenge


## Overview:
  I finished ~rank 14 in the challenge (finals still pending but is turning out the same). Right now I'm a fairly inexperienced coder in my senior year of high school and was just finishing up my first real programming course when I started the challenge. Most of my code is what I perceive to be poorly written, but I'll go over how my final bot version operates here and in the source code itself. The other bot versions will contain comments as-is which are probably outdated!
  
  
#### Some quick jargon I used through comments/names within the code:
  * Bunny ship: A ship that "hops away" from enemy ships, distracting enemies by being uncatchable yet likely the closest ship they're chasing.
  * Strafe: "Strafing" is used, in all seriousness, in reference to the Minecraft term. Basically, you attack while moving perpendicular to your target. It's like "kiting" but staying just as close to your real target, the enemy docked ships.
  * Noob-rush: A reference to Rise of Nations, basically when a bot rushes your ships at the beginning of the game. A low-skill-highly-obnoxious strategy in RoN.
  * Target: A target is where a particular ship wants to go next turn. Each target starts as the desired location the ship is heading to (say a planet 5 moves away), then SetTarget() corrects it to where it'll be after 1 turn. Targets that are the same as the ship's current location are for ships that shouldn't move/are docked.
  
  
## Basic Bot Functionality
  The final bot is designed to gain a small advantage early-on then steam-roll the other bots. It functions by going over a set number of special cases, like a noob-rushes or attacking ships, each turn then going over each left-over ship and sending them generic behavior based on their closest planet. It (attempts) to always take winning fights until it has a significant enough advantage to take losing fights, if only for the sake of whittling down the enemy.
  Also important to the code is the customized navigation code. Up until the "Manage Collisions" section, ships that intend to move only set a target they'd like to go to. Once every ship has a target, they're adjusted and sorted until all ships are collision-free. To compare for collisions, the vector of one ship is subtracted from another, and the resultant vector is compared to the origin using a slightly modified collision check (the starter bot collision check, but takes in the 2 locations that make up the resultant vector as parameters). If the minimum distance is less than 1.1, there could be a collision, so return true!
  
  
## Breakdown of the code (the important parts):

### hlt folder
  This is 90% the starter bot hlt folder, with some files tweaked with copies of almost the exact same function, but different parameters so I can pass in different types. The tweaks regard navigation code to avoid collision.

### Some comments
  Outdated details about what bot version I'm on, a repeat of the jargon I use, some major issues, and some minor issues.

### Special case: Noob rush
  Detects if there's a noob rush then handles it accordingly. Current strategy: group up and charge in.
  
### Special case: Imminent defeat/victory.
  Sends my ships to the nearest corner if I'm certain to lose/starts making art if I'm certain to win.
  
### Special case: Attacking enemies.
  Finds ships closest to my planets/neutral planets and assigns a ship to each one. This uses the offensively-ineffective Outnumbered() function to ensure that the ships never really attack their targets, and get cover if they're under attack.
  
### Special case: Ship advantage
  If I have 5 more ships than the everyone, I don't care about ensuring each ship only goes in when it outnumbers the enemy - just attack!
  
### Special case: Outnumbered Refactor
  This code sorts ships by their distance to their closest enemy, has ship, by distance and if not already checked, identify any clusters around it, determine if the allied cluster is outnumbered, then sets each ship in that identified cluster to know it's been checked so that the loop really only does a small fraction of the ships. Works pretty well, written pretty poorly and quickly!
  
### Offensive strategy for leftover ships
  The current offensive strategy is to locate the closest planet that is either not full or not owned by me, and run a unique strategy for each three cases of ownership:

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
        
### Manage collisions
  Sets targets for the Outnumbered ships, sorts all the desired targets, and manages collisions between ships and where they all want to go. Nifty and effective - some of my best and most efficient code.
  
### Send moves
  Sends the moves.
  
### Define a ton of functions
  What each function does (hopefully) is made obvious by its name and type, but I admit some of the grouping I did was by their initial use, and some functions later become general utility functions poorly grouped. See: bool IsInVector(ship_id, vector).
