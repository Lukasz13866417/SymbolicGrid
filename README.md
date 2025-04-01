# Robust API for Logarithmic-Time 2D Grid Queries

The grid can be used to represent a maze or any layout of non-overlapping "tiles" in systems where **randomization** is needed. In this grid, a position is either *reserved* or *free*. I used this as part of my custom, sophisticated terrain generation API for my game. See: [project repo](https://github.com/Lukasz13866417/Game3D_OpenGL)

To use this API, simply copy the package `symbolic` into your project. It includes everything you need, including the `GridCreator` class. There are no external dependencies.

## Supported Queries

- **Creating a Grid:**  
  Create a 2D grid with a specified number of rows and columns. Every position is initially *free*.  
  **Time Complexity:** $O((n + m)\log(n+m))$, where $n$ is the number of rows and $m$ is the number of columns.

- **Reserving Vertical Segments:**  
  Reserve a vertical segment (given start position and length) in the grid. Throws an error if any position within the segment is already *reserved*.  
  **Time Complexity:** $O(\log(n+m)d)$, where $d$ is the length of the segment.

- **Reserving Random Vertical Segments:**  
  Reserve (and return) a *random, uniformly picked* vertical segment of a given length that is entirely *free*.  
  **Time Complexity:** $O(\log(n+m)d)$

- **Reserving Horizontal Segments:**  
  Reserve a horizontal segment with the same semantics as the vertical query.  
  **Time Complexity:** $O(\log(n+m)d)$

- **Reserving Random Horizontal Segments:**  
  Reserve (and return) a random horizontal segment of a given length that is entirely *free*.  
  **Time Complexity:** $O(\log(n+m)d)$

- **Adding a Child Grid:**  
  Add a **child** grid to a given grid at a specified position within the parent grid. The child grid inherits the number of columns from the parent. Reserving a segment in the child grid will reserve the corresponding segment in the parent. *For now*, the area in the parent grid must be entirely free.  
  **Time Complexity:** Same as creating a new grid.

## Example Usage

### Code:
Reserving specific segments:
```Java
import symbolic.GridCreator;

// Create grid with 7 rows, 5 cols
GridCreator parent = new GridCreator(7, 5);
// Args for both methods: row, column, length
parent.reserveVertical(2, 2, 2);
parent.reserveHorizontal(1, 1, 4);

parent.printGrid();
```
Adding a child grid:
```Java
// Create child grid - 4 rows, 5 cols
// It will be inside the first grid, with an offset of 3,
// covering rows 4, 5, 6, and 7.
GridCreator child = new GridCreator(4, 5, parent, 3);
// Position (4,5) in the child grid corresponds to (7,5) in the parent grid.
child.reserveVertical(4, 5, 1);
parent.printGrid();
```
Reserving random fitting segments:
```Java
GridCreator another = new GridCreator(7,5);
another.reserveHorizontal(2,2,3);
// Re-running will cause it to appear in different places
GridSegment randomFitting = another.reserveRandomFittingVertical(3);
another.printGrid();
```

Remember to clean up after you're done:

```Java
child.destroy();
parent.destroy();
another.destroy();
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
```

```
####.
.#...
.#...
.....
.....
.....
....#
```

```
.....
####.
#....
#....
.....
.....
.....
```

### Note

When you're done using a specific instance of `GridCreator`, call `destroy()` on it! The class uses preallocation in its data structures to optimize performance by increasing **cache locality, reducing allocations and garbage collector usage**. If the preallocated pool of memory runs out, it will resize. Calling `destroy()` allows the memory to be reused. Not doing so may cause extra resizings. You are welcome to adjust the preallocated memory size in the code. A smaller preallocation size introduces minimal overhead while maintaining a logarithmic number of resizings.

## Future Enhancements
The most important and challenging functionality has been achieved. Here are things that can be added too:
- **Freeing Segments:**  
  Allow users to make segments *free* again. It can be done with the existing data structures and the same time complexity as current queries.

- **Flexible Child Grids:**  
  Modify `GridCreator` to allow the child grid to be created in an area that is not entirely free, or to have a different number of columns than the parent and be placed anywhere within it.

- **Dynamic Row Increases:**  
  Add a query to increase the number of rows. While the time complexities remain the same, the constant factors may grow substantially due to new data structures (most likely BSTs with lazy propagation).

## Internal Data Structures Overview

- **Sparse Segment Trees:**  
  Advanced queries using **hashing** to represent the *free* subsegments (only those that can't be extended—i.e., from each side they either touch the border or a *reserved* position).

- **Balanced BST:**  
  A custom AVL stores information about the end positions of the free segments.

- **Preallocated Node Pools:**  
  These help avoid frequent allocations and minimizes the impact on the Java Garbage Collector. They are used both in
the segtrees and the BSTs.


### Why $d$ in Time Complexity doesn't matter in practice:

The $d$ factors are almost negligible if you perform further operations on the reserved positions. Given the disjoint nature of the reserved segments, the overhead is proportional to the amount of processing done on the resulting segments. <br>
If you choose to perform only horizontal queries followed by vertical ones (or vice versa), the structure can be further modified to achieve completely logarithmic time by utilizing the `PartialSegementHandler` class, which handles all queries for one orientation in logarithmic time. I plan to outline this enhancement or provide modified code in the future.

## License

This project is licensed under the **Creative Commons Attribution 4.0 International License**.  
You are free to share and adapt the material as long as you provide proper attribution. For more details, please see the [Creative Commons Attribution 4.0 International License](https://creativecommons.org/licenses/by/4.0/).

---
