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

// Function for determining whether a move is blocked.

#ifndef BLOCKS_H
#define BLOCKS_H

static bool blocklegal(Position& state, std::set<Block>& blocks, Position& move){
	Block changed;
	Position::iterator iter;
	for (iter = move.begin(); iter != move.end(); iter++){
		int setsize = iter->second.size;
		for (int i = 0; i < setsize; i++){
			if (iter->second.permutation[i] != i+1)
				changed[iter->first].insert(state[iter->first].permutation[i]);
			else if (iter->second.orientation[i] != 0)
				changed[iter->first].insert(state[iter->first].permutation[i]);
		}
	}
	
	std::set<Block>::iterator block_iter;
	Block::iterator set_iter;
	std::set<int>::iterator piece_iter;
	for (block_iter = blocks.begin(); block_iter != blocks.end(); block_iter++){
		bool block_moved = true;
		bool block_stationary = true;
		
		Block test = (*block_iter);
		for (set_iter = test.begin(); set_iter != test.end(); set_iter++){
			for (piece_iter = set_iter->second.begin(); piece_iter != set_iter->second.end(); piece_iter++){
				if (changed[set_iter->first].find(*piece_iter) == changed[set_iter->first].end())
					block_moved = false;
				else 
					block_stationary = false;
			}
		}
		if (!block_moved && !block_stationary)
			return false;
	}
	return true;
}

#endif
