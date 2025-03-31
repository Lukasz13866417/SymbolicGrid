# Robust API for logarithmic-time 2D grid queries
The grid can be used to represent a maze, or any layout of non-overlapping "tiles" in any system where **randomization** is needed. A position in the grid is either *reserved* or *free*.
I used this as part of my custom, sophisticated terrain generation API for my game. See: [project repo](https://github.com/Lukasz13866417/Game3D_OpenGL) <br>
To use this API, simply copy the package "symbolic" into your project - it has everything needed, including the ```GridCreator``` class.
## Supported queries:
- Creating a 2D grid with a set number of rows and columns. Every posotion is initially *free*. $O((n + m)log(n+m))$ time, where $n$ is the number of rows and $m$ - the number of columns.
- Reserving a given (by start position and length) vertical segment in the grid. Throws error if a *reserved* position already lies within that segment. $O(log(n+m)d)$ time, where $d$ is the length of reserved segment.
- Reserving (and returning) a *random, uniformly picked* vertical segment in the grid that has a given length, and is entirely *free*. $O(log(n+m)d)$ 
- Reserving a given horizontal segment, with the same semantics as the corresponding query with vertical segments. $O(log(n+m)d)$ 
- Reserving (and returning) a random horizontal segment that has a given length, in the same was as the corresponding query with vertical segments. $O(log(n+m)d)$ 
- Adding a **child** grid to a given grid, in a given position within the parent grid. It gives a fine-grained control over a given "region" of the parent grid. The child grid inherits the number of columns from the parent. Reserving a segment in the child grid will reserve the corresponding segment in the parent. *For now*, the area in the parent grid has to be entirely free. Same time complexity as creating a new grid.
## Example usage:
### Code:
```Java
GridCreator parent = new GridCreator(7,5);
parent.reserveVertical(2,2,2);
parent.reserveHorizontal(1,1,4);

parent.printGrid();
```
```Java
parent.reserveRandomFittingHorizontal(3);
parent.printGrid();
```
```Java
GridCreator child = new GridCreator(4,5,parent,3);
child.reserveVertical(4,5,1);
parent.printGrid(); 
```
Remember to do this when you're done:
```Java
child.destroy();
parent.destroy();
```

### Output:
```
####.
.#...
.#...
.....
.....
.....
.....

####.
.#...
.#...
.....
.....
###..
.....

####.
.#...
.#...
.....
.....
###..
....#
```
###Note!
When you're done using a specific instance of ```GridCreator```, call ```destroy()``` on it! The grid creator uses preallocation in its data structures to give the best possible performance (if the preallocated pool of memory runs out, it's resized). Destroying will allow the preallocated memory to be reused. Not doing this will simply cause more resizings. **However, feel free** to change the amount of pre-allocated memory in the code. Since the memory pool is resized just like the array used internally in an ```ArrayList```, setting the pre-allocated size to something small will introduce minimal overhead, while still providing a logarithmic number or resizings (but now a few times more). 
## Future
Although the main functionality has been achieved, more queries can still be added!
- For instance, the internal data structures can be used to allow the user to make segments *free* again, with the same time complexity as any current segment query.
- It's **easy** to modify the ```GridCreator``` class to allow the child grid to be created in an area that is *not* entirely free.
- It's also **easy** to modify the ```GridCreator```, so that children can have different number of columns than the parent grid and be placed *anywhere* within the parent grid.
- It's *possible* to add a  query that increases the number of rows. Time complexities will be unchanged, but the constant factor will grow signicantly due to a new data structure. 
## Internal data structures used (overview):
- **Sparse segment trees** with advanced queries, using **hashing** to represent information about the *free* subsegments (but only the ones that can't be extended - from each side they either touch the border or a *reserved* position)
- **Preallocated Node Pool** for the segment tree to avoid frequent allocations and annoying the Java Garbage Collector.
- **Balanced BST** storing information about the end position of mentioned free segments. 
### Why the $d$ factors don't matter in practice:
However, the $d$ factors are negligible if you actually do something later with the reserved positions. Due to the obvious disjointness of the reserved segments, you put in the same amount of time as the all the $d$ factors in the performed queries. In that case, my solution only takes "logarithmically" more time than the amount of time you took anyway in order to process the obtained results.
<br><br>
Moreover, if you decide to first only perform horizontal segment queries, and after that only vertical segment queries (or the other way around), this structure can be easily modified to get completely logarithmic time. That's because the most important class in the implementation, the ```PartialSegementHandler```, does all types of queries for just one "orientation" of segments (all vertical / all horizontal) and does everything in logarithmic time. I will outline how to do that in the future (or provide the modified version's code)<br>

