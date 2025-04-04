package symbolic;

import symbolic.segments.PartialSegmentHandler;

public class GridCreator {
    public static final int NODE_POOL_SIZE = 10000;
    private final PartialSegmentHandler vertical, horizontal;
    private final GridCreator parent;
    private final int parentRowOffset;

    public GridCreator(int nRows, int nCols, GridCreator parentGrid, int parentRowOffset) {
        this.horizontal = new PartialSegmentHandler(nRows, nCols, false);
        this.vertical = new PartialSegmentHandler(nRows, nCols, true);
        this.parentRowOffset = parentRowOffset;
        this.parent = parentGrid;
    }

    public GridCreator(int nRows, int nCols) {
        this(nRows, nCols, null, 0);
    }

    public void reserveVertical(int row, int col, int length) {
        if (parent != null) {
            parent.reserveHorizontal(row + parentRowOffset, col, length);
        }
        vertical.reserve(row, col, length);
        for (int r = row; r < row + length; r++) {
            horizontal.reserve(r, col, 1);
        }
    }

    public void reserveHorizontal(int row, int col, int length) {
        if (parent != null) {
            parent.reserveHorizontal(row + parentRowOffset, col, length);
        }
        horizontal.reserve(row, col, length);
        for (int c = col; c < col + length; c++) {
            vertical.reserve(row, c, 1);
        }
    }

    public GridSegment reserveRandomFittingVertical(int length) {
        GridSegment res = vertical.reserveRandomFitting(length);
        if (parent != null) {
            parent.reserveVertical(res.row + parentRowOffset, res.col, res.length);
        }
        for (int r = res.row; r < res.row + length; r++) {
            horizontal.reserve(r, res.col, 1);
        }
        return res;
    }

    public GridSegment reserveRandomFittingHorizontal(int length) {
        GridSegment res = horizontal.reserveRandomFitting(length);
        if (parent != null) {
            parent.reserveHorizontal(res.row + parentRowOffset, res.col, res.length);
        }
        for (int c = res.col; c < res.col + length; c++) {
            vertical.reserve(res.row, c, 1);
        }
        return res;
    }

    public void printGrid() {
        horizontal.printGrid(); // can also be vertical.printGrid(), since they represent the same grid
    }

    public void destroy() {
        vertical.flush();
        horizontal.flush();
    }


    /**
     * Example usage, same as in README
     */
    public static void main(String[] args) {

        // Create grid with 7 rows, 5 cols
        GridCreator parent = new GridCreator(7, 5);
        // Args for both methods: row, column, length
        parent.reserveVertical(2, 2, 2);
        parent.reserveHorizontal(1, 1, 4);

        parent.printGrid();

        System.out.println();
        System.out.println("------------------------------");
        System.out.println();

        // Create child grid - 4 rows, 5 cols
        // It will be inside the first grid, with an offset of 3,
        // covering rows 4, 5, 6, and 7.
        GridCreator child = new GridCreator(4, 5, parent, 3);
        // Position (4,5) in the child grid corresponds to (7,5) in the parent grid.
        child.reserveVertical(4, 5, 1);
        parent.printGrid();

        System.out.println();
        System.out.println("------------------------------");
        System.out.println();

        GridCreator another = new GridCreator(7, 5);
        another.reserveHorizontal(2, 2, 3);
        // Re-running will cause it to appear in different places
        GridSegment randomFitting = another.reserveRandomFittingVertical(3);
        another.printGrid();

        child.destroy();
        parent.destroy();
        another.destroy();
    }

}
