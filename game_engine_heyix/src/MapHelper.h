#ifndef MAPHELPER_H
#define MAPHELPER_H

/* MapHelper.h : Utility header containing convenient data structures for homework #3 */
/* (mainly so that you don't need to type or copy-paste directly from the spec) */

#include <string>
#include <vector>
#include "Actor.h"
#include "glm/glm.hpp" // Student : You need to get glm added to your project source code or this line will fail.
					   // Search for your IDE's "include / header directories" option (it's called something different everywhere)
					   // Ensure you have the glm folder listed so your compiler can actually find these glm headers.






const int HARDCODED_MAP_WIDTH = 50;
const int HARDCODED_MAP_HEIGHT = 26;
const char hardcoded_map[HARDCODED_MAP_HEIGHT][HARDCODED_MAP_WIDTH+1] = { // Know why we need "+1"? Hint : These are c-style strings.
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
	"bb    b       b                   b        b    bb",
	"bb            b                   b             bb",
	"bb    bbbbbbbbbbbbbb              bbbbbbbbbb    bb",
	"bb            b      b            b             bb",
	"bb    b       b       b   bb      b        b    bb",
	"bb    b  bb bbb     bb      bb    bbbbbbbbbb    bb",
	"bb    b  b    b   bb          bb           b    bb",
	"bb    b  b   b  bb        bb    bb         b    bb",
	"bbb   bbbb  bbbb                  bbbbbbbbbb   bbb",
	"b                   bb                           b",
	"b                                                b",
	"bbb    bbbb  bbbb          bb    bbbbbbbbbbbb  bbb",
	"bb       b      bbbbb        bbb           b    bb",
	"bb       b            b    b               b    bb",
	"bb       b            b    b               b    bb",
	"bb       b            b    b               b    bb",
	"bb       bbbbbbbbbbbbb      bbbbbbbbbbbbbbbb    bb",
	"bb                                              bb",
	"bb        bb           b  b         bb          bb",
	"bb                    b    b            bb      bb",
	"bbbbb    bb           b    b                   bbb",
	"bb   bb                bbbb      bb      bb      b",
	"bb     bb    bb                      bb          b",
	"bbbbbbbbbbbbbbbb   bbbbbbbbbbbb   bbbbbbbbbbbbbbbb",
	"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
};


#endif
