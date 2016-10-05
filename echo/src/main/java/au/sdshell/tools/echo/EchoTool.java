package au.sdshell.tools.echo;

import java.util.Arrays;

/**
 * Prints arguments into stdout
 * Created by andy on 9/20/16.
 */
public class EchoTool {
    public static void main(String[] args) {
        Arrays.stream(args).forEach(s -> System.out.print(s + " "));
        System.out.println("");
    }
}
