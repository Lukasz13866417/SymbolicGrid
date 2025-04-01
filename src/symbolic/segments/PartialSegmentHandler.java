package symbolic.segments;

import symbolic.GridSegment;
import symbolic.segments.by_end_pos.SegmentsByEndPosition;
import symbolic.segments.by_length.segtree_implementation.PreallocatedHashedSegmentsByLengthNodes;
import symbolic.segments.by_length.SegmentsByLength;

import java.util.Random;

public class PartialSegmentHandler {

    private final int nRows, nCols;
    private final boolean vertical;

    private final SegmentsByLength segmentsByLength;
    private final SegmentsByEndPosition segmentsByEndPosition;

    public PartialSegmentHandler(int nRows, int nCols, boolean vertical) {
        this.nRows = nRows;
        this.nCols = nCols;
        this.vertical = vertical;
        this.segmentsByLength = new PreallocatedHashedSegmentsByLengthNodes(nRows, nCols, vertical);
        this.segmentsByEndPosition = new SegmentsByEndPosition(nRows, nCols, vertical);

        if(vertical) {
            for (int col = 1; col <= nCols; ++col) {
                this.segmentsByLength.insert(1, col, nRows);
                this.segmentsByEndPosition.insert(1, col, nRows);
            }
        }else{
            for (int row = 1; row <= nRows; ++row) {
                this.segmentsByLength.insert(row, 1, nCols);
                this.segmentsByEndPosition.insert(row, 1, nCols);
            }
        }
    }

    public void reserve(int row, int col, int length) {
        GridSegment[] reserve = segmentsByEndPosition.reserve(row, col, length);
        segmentsByLength.delete(reserve[0].row, reserve[0].col, reserve[0].length);
        if (reserve[1] != null) {
            segmentsByLength.insert(reserve[1].row, reserve[1].col, reserve[1].length);
        }
        if (reserve[2] != null) {
            segmentsByLength.insert(reserve[2].row, reserve[2].col, reserve[2].length);
        }
    }

    public GridSegment reserveRandomFitting(int length) {
        int total = segmentsByLength.countFittingSpaces(length);
        int k = new Random().nextInt(total)+1;
        GridSegment found = segmentsByLength.getKthFittingSpace(length,k);
        reserve(found.row, found.col, length);
        return found;
    }

    public void printGrid(){
        segmentsByEndPosition.printGrid();
    }

    public void flush(){
        /*while(!segmentsByEndPosition.tree.isEmpty()){
            GridSegment curr = segmentsByEndPosition.tree.pollFirst();
            segmentsByLength.delete(curr.row,curr.col,curr.length);
        }*/
        segmentsByLength.destroy();
    }



}
