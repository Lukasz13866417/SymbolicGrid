# Robust API for logarithmic-time 2D grid queries
The grid can be used to represent a maze, or any layout of non-overlapping "tiles" in any system where **randomization** is needed. A position in the grid is either *reserved* or *free*.
I used this as part of my custom, sophisticated terrain generation API for my game. See: [project repo](https://github.com/Lukasz13866417/Game3D_OpenGL) <br>
Allows for the following operations, all in $O(log((n+m)^{2} * m))$ time, where $n$ is the number of rows and $m$ - the number of columns. <br>
## Supported queries:
- Creating a 2D grid with a set number of rows and columns. Every posotion is initially *free* 
- Reserving a given (by start position and length) vertical segment in the grid. Throws error if a *reserved* position already lies within that segment
- Reserving (and returning) a *random, uniformly picked* vertical segment in the grid that has a given length, and is entirely *free*
- Reserving a given horizontal segment, with the same semantics as the corresponding query with vertical segments.
- Reserving (and returning) a random horizontal segment that has a given length, in the same was as the corresponding query with vertical segments.
- Adding a **child** grid to a given grid, in a given place. It gives a fine-grained control over a given "region" of the parent grid. The child grid inherits the number of columns from the parent, but in fact it can have any number of columns up to the width of the parent grid. Reserving a segment in the child grid will reserve the corresponding segment in the parent.
## Future:
- The internal data structures can be used to allow the user to make segments *free* again!
- These operations will have the same time complexity
## Internal data structures used (overview):
- **Sparse segment trees** with advanced queries, using **hashing** to represent information about the *free* subsegments (but only the ones that can't be extended - from each side they either touch the border or a *reserved* position)
- **Preallocated Node Pool** for the segment tree to avoid frequent allocations and annoying the Java Garbage Collector.
- **Balanced BST** storing information about the end position of mentioned free segments. Currently a bulitin tree set.
