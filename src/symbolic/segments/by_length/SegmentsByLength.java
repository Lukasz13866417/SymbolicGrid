package symbolic.segments.by_length;

import symbolic.GridSegment;

public interface SegmentsByLength {

    void insert(int row, int col, int length);

    void delete(int row, int col, int length);

    int countFittingSpaces(int spaceSize);

    GridSegment getKthFittingSpace(int spaceSize, int k);

    void destroy();
}
