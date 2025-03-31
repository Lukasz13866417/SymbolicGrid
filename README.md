# Robust API for logarithmic-time 2D grid queries
The grid can be used to represent a maze, or any layout of non-overlapping "tiles" in any system where **randomization** is needed. A position in the grid is either *reserved* or *free*.
I used this as part of my custom, sophisticated terrain generation API for my game. See: [project repo](https://github.com/Lukasz13866417/Game3D_OpenGL) <br>
## Supported queries:
- Creating a 2D grid with a set number of rows and columns. Every posotion is initially *free*. $O((n + m)log(n+m))$ time, where $n$ is the number of rows and $m$ - the number of columns.
- Reserving a given (by start position and length) vertical segment in the grid. Throws error if a *reserved* position already lies within that segment. $O(log(n+m)d)$ time, where $d$ is the length of reserved segment.
- Reserving (and returning) a *random, uniformly picked* vertical segment in the grid that has a given length, and is entirely *free*. $O(log(n+m)d)$ 
- Reserving a given horizontal segment, with the same semantics as the corresponding query with vertical segments. $O(log(n+m)d)$ 
- Reserving (and returning) a random horizontal segment that has a given length, in the same was as the corresponding query with vertical segments. $O(log(n+m)d)$ 
- Adding a **child** grid to a given grid, in a given position within the parent grid. It gives a fine-grained control over a given "region" of the parent grid. The child grid inherits the number of columns from the parent, but in fact it can have any number of columns up to the width of the parent grid. Reserving a segment in the child grid will reserve the corresponding segment in the parent. Same time complexity as creating a new grid.
## Future:
- The internal data structures can be used to allow the user to make segments *free* again!
- These operations will have the same time complexity as the current ones
## Internal data structures used (overview):
- **Sparse segment trees** with advanced queries, using **hashing** to represent information about the *free* subsegments (but only the ones that can't be extended - from each side they either touch the border or a *reserved* position)
- **Preallocated Node Pool** for the segment tree to avoid frequent allocations and annoying the Java Garbage Collector.
- **Balanced BST** storing information about the end position of mentioned free segments. Currently a bulitin tree set.
### Why the $d$ factors don't matter in practice:
Removing either all horizontal segment queries or all vertical segment queries would **remove this factor** from all queries. The most important class in the implementation, the ```PartialSegementHandler```, does queries for just one "orientation" of segments (all vertical / all horizontal) and does everything in logarithmic time. <br>
However, the $d$ factors are negligible if you actually do something later with the reserved positions. Due to the obvious disjointness of the reserved segments, you put in the same amount of time as the all the $d$ factors in the performed queries. In that case, my solution only takes "logarithmically" more time than the amount of time you took anyway in order to process the obtained results.
