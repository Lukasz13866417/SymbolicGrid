package symbolic.segments;

import symbolic.GridSegment;

import java.util.*;

import static java.lang.Math.max;


/**
 * Random tester for SegmentsByLength (no duplicates, no deletions).
 */
public class TestSegmentsByLengthRandom {

    public static void main(String[] args) {
        final int NROWS = 1000;
        final int NCOLS = 7;
        final boolean ARE_VERTICAL = false;  // change as desired

        // 1) Create the SegmentsByLength instance
        SegmentsByLength segs = new PreallocatedHashedSegmentsByLengthNodes(NROWS, NCOLS, ARE_VERTICAL);

        // 2) Generate a set of unique random "hashes"
        //    We won't exceed the maximum possible: (maxLen+1) * NROWS * NCOLS
        //    The length portion is "hash // (NROWS * NCOLS)" so let's aim for lengths up to 7
        final int MAX_LENGTH = 70;
        final int MAX_HASH = (MAX_LENGTH + 1) * (NROWS * NCOLS) - 1;  // inclusive
        final int NUM_INSERTS = 50;  // how many unique inserts we do
        Random rng = new Random(0);

        Set<Integer> usedHashes = new HashSet<>();
        // keep the actual segments in a set (brute force)
        ArrayList<GridSegment> bruteSegments = new ArrayList<>();

        // Generate unique random hashes
        while (usedHashes.size() < NUM_INSERTS) {
            int candidate = rng.nextInt(MAX_HASH + 1);
            if (!usedHashes.contains(candidate)) {
                usedHashes.add(candidate);

                // convert hash -> GridSegment
                GridSegment seg = fromHash(candidate, NROWS, NCOLS);
                // skip any segment that has length=0 (hash might produce length=0).
                // If you do allow length=0, that might be a special case.
                if (seg.length == 0) continue;

                // Insert into your data structure
                System.out.println("segments.insert("+seg.row+","+seg.col+","+seg.length+");");
                segs.insert(seg.row, seg.col, seg.length);
                // Also store in brute force collection
                bruteSegments.add(seg);
            }
        }

        // 3) We'll do random queries for countFittingSpaces and getKthFittingSpace
        final int NUM_QUERIES = 100;
        for (int i = 0; i < NUM_QUERIES; i++) {
            int spaceSize = 1 + rng.nextInt(MAX_LENGTH);  // random size in [1..MAX_LENGTH]

            // 3a) Test countFittingSpaces
            int expectedCount = bruteForceCount(bruteSegments, spaceSize, ARE_VERTICAL);
            int actualCount = segs.countFittingSpaces(spaceSize);
            if (expectedCount != actualCount) {
                throw new AssertionError("countFittingSpaces("
                        + spaceSize + ") mismatch. Expected="
                        + expectedCount + ", got=" + actualCount);
            }

            if(expectedCount==0){
                continue;
            }

            int k = 1 + rng.nextInt(expectedCount);
            GridSegment actualSeg = segs.getKthFittingSpace(spaceSize, k);

            GridSegment expectedSeg = (k > expectedCount)
                    ? null
                    : bruteForceKthSubSpace(bruteSegments, spaceSize, k, ARE_VERTICAL);

            if (!Objects.equals(actualSeg, expectedSeg)) {
                throw new AssertionError("getKthFittingSpace("
                        + spaceSize + "," + k + ") mismatch. Expected="
                        + expectedSeg + ", got=" + actualSeg);
            }
        }

        System.out.println("All randomized tests passed successfully!");
    }

    /**
     * Convert a hash to a GridSegment using your logic:
     *  int col = hash % nCols;  hash /= nCols;
     *  int row = hash % nRows;  hash /= nRows;
     *  length = hash;
     */
    private static GridSegment fromHash(int hash, int totalRows, int nCols) {
        int col = hash % nCols;
        hash /= nCols;
        int row = hash % totalRows;
        hash /= totalRows;
        int length = hash;

        // Because your class expects row,col in [1..N], do row+1, col+1
        return new GridSegment(row + 1, col + 1, length);
    }

    /**
     * Brute force: how many sub-spaces of size `spaceSize` exist among all segments?
     */
    private static int bruteForceCount(List<GridSegment> segments, int spaceSize, boolean areVertical) {
        int count = 0;
        for (GridSegment seg : segments) {
            if (seg.length >= spaceSize) {
                // a segment of length L has (L - spaceSize + 1) sub-segments
                count += (seg.length - spaceSize + 1);
            }
        }
        return count;
    }

    /**
     * Brute force: find the k-th sub-space of size `spaceSize` among all segments,
     * sorted by (row, col).
     */
    private static GridSegment bruteForceKthSubSpace(ArrayList<GridSegment> segments,
                                                     int spaceSize,
                                                     int k,
                                                     boolean areVertical) {

        segments.sort(Comparator.<GridSegment>comparingInt(a -> a.length)
                .thenComparingInt(a -> a.row)
                .thenComparingInt(a -> a.col));

        int i=0;
        while(segments.get(i).length < spaceSize || segments.get(i).length - spaceSize + 1 < k) {
            k -= max(0,segments.get(i).length - spaceSize + 1);
            ++i;
        }
        if(areVertical) {
            return GridSegment.GS(segments.get(i).row+k-1, segments.get(i).col, spaceSize);
        }else{
            return GridSegment.GS(segments.get(i).row, segments.get(i).col+k-1, spaceSize);
        }
    }
}
