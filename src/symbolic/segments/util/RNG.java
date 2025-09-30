package symbolic.segments.util;

import static java.lang.Math.pow;

import java.util.Random;

public class RNG {

    private static long seed = new Random().nextLong();

    // Useful for testing
    /*static{
        System.out.println("SEED: "+seed);
    }*/

    private static Random RANDOM = new Random(seed);

    public static void setSeed(long newSeed) {
        seed = newSeed;
        RANDOM = new Random(seed);
    }

    public static int randInt(int l, int r){
        return l+RANDOM.nextInt(r-l+1);
    }

}
