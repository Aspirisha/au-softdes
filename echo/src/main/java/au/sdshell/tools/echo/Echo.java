package au.sdshell.tools.echo;

import java.util.Arrays;

/**
 * Created by andy on 9/20/16.
 */
public class Echo {
    public static void main(String[] args) {
        Arrays.stream(args).forEach(System.out::print);
    }
}
