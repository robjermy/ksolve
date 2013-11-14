/*
 KSolve+ - Puzzle solving program.
 Copyright (C) 2007-2013 K�re Krig and Michael Gottlieb

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Main struct and control flow of program, with all includes used in it

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
#include <windows.h>

struct ksolve {
	#include "data.h"
	#include "move.h"
	#include "blocks.h"
	#include "checks.h"
	#include "indexing.h"
	#include "pruning.h"
	#include "search.h"
	#include "readdef.h"
	#include "readscramble.h"
	#include "god.h"

	static int ksolveMain(int argc, char *argv[]) {
		clock_t start; 
		start = clock();
		srand(time(NULL)); // initialize RNG in case we need it

		if (argc != 3){
			std::cerr << "ksolve+ v1.0\n";
			std::cerr << "(c) 2007-2013 by Kare Krig and Michael Gottlieb\n";
			std::cerr << "Usage: ksolve [def-file] [scramble-file]\n";
			std::cerr << "See readme for additional help.\n";
			return EXIT_FAILURE;
		}

		// Load the puzzle rules
		Rules ruleset(argv[1]);
		PieceTypes datasets = ruleset.getDatasets();
		Position solved = ruleset.getSolved();
		MoveList moves = ruleset.getMoves();
		std::set<MovePair> forbidden = ruleset.getForbiddenPairs();
		Position ignore = ruleset.getIgnore();
		std::set<Block> blocks = ruleset.getBlocks();
		std::map<string, int> moveLimits = ruleset.getMoveLimits();
		std::cout << "Ruleset loaded.\n";
		
		// Print all generated moves
		std::cout << "Generated moves: ";
		int i = 0;
		MoveList::iterator moveIter;
		for (moveIter = moves.begin(); moveIter != moves.end(); moveIter++) {
			if (moveIter->second.name != moveIter->second.parentMove) {
				if (i>0) std::cout << ", ";
				i++;
				std::cout << moveIter->second.name;
			}
		}
		std::cout << ".\n";

		// Compute or load the pruning tables
		PruneTable tables;
		string deffile(argv[1]);
		tables = getCompletePruneTables(solved, moves, datasets, ignore, deffile);
		std::cout << "Pruning tables loaded.\n";
		
		//datasets = updateDatasets(datasets, tables);
		updateDatasets(datasets, tables);
		
		// God's Algorithm tables
		std::string godHTM = "!";
		std::string godQTM = "!q";
		if (0==godHTM.compare(argv[2])) {
			std::cout << "Computing God's Algorithm tables (HTM)\n";
			bool success = godTable(solved, moves, datasets, forbidden, ignore, blocks, 0);
			std::cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";
			return EXIT_SUCCESS;
		} else if (0==godQTM.compare(argv[2])) {
			std::cout << "Computing God's Algorithm tables (QTM)\n";
			bool success = godTable(solved, moves, datasets, forbidden, ignore, blocks, 1);
			std::cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";
			return EXIT_SUCCESS;
		}

		// Load the scramble to be solved
		Scramble states(argv[2], solved, moves, datasets, blocks);
		std::cout << "Scrambles loaded.\n";

		ScrambleDef scramble = states.getScramble();

		while(scramble.state.size() != 0){
			int depth = 0;
			string temp_a, temp_b;
			temp_a = " ";
			temp_b = ".";

			std::cout << "\nSolving \"" << scramble.name << "\"\n";
			
			if (scramble.printState == 1) {
				std::cout << "Scramble position:\n";
				printPosition(scramble.state);
			}
			
			// give out a warning if we have some undefined permutations on a bandaged puzzle
			if (blocks.size() != 0) {
				bool hasUndefined = false;
				Position::iterator iter;
				for (iter = scramble.state.begin(); iter != scramble.state.end(); iter++) {
					int setsize = iter->second.size;
					for (int i = 0; i < setsize; i++) {
						if (iter->second.permutation[i] == -1) {
							hasUndefined = true;
						}
					}
				}
				if (hasUndefined) {
					std::cout << "Warning: using blocks, but scramble has unknown (?) permutations!\n";
				}
			}
			
			std::cout << "Depth 0\n";
	 
			// The tree-search for the solution(s)
			int usedSlack = 0;
			while(1) {
				boolean foundSolution = treeSolve(scramble.state, solved, moves, datasets, tables, forbidden, scramble.ignore, blocks, depth, scramble.metric, moveLimits, temp_a, temp_b);
				if (foundSolution || usedSlack > 0) {
					usedSlack++;
					if (usedSlack > scramble.slack) break;
				}
				depth++;
				if (depth > scramble.max_depth){
					std::cout << "\nMax depth reached, aborting.\n";
					break;
				}
				std::cout << "Depth " << depth << "\n";
			}
			std::cout << "\n";

			scramble = states.getScramble();
		}        

		std::cout << "Time: " << (clock() - start) / (double)CLOCKS_PER_SEC << "s\n";

		return EXIT_SUCCESS;
	}
};

int main(int argc, char *argv[]) {
	ksolve::ksolveMain(argc, argv);
}